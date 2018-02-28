// Copyright (c) 2014-2018 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "WebPageBlink.h"

#include <cmath>

#include <QtCore/QDir>
#include <QtCore/QMultiMap>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QTextStream>

#include "ApplicationDescription.h"
#include "BlinkWebProcessManager.h"
#include "BlinkWebView.h"
#include "LogManager.h"
#include "PalmSystemBlink.h"
#include "WebAppManagerConfig.h"
#include "WebAppManagerTracer.h"
#include "WebPageObserver.h"

/**
 * Hide dirty implementation details from
 * public API
 */

static const int kExecuteCloseCallbackTimeOutMs = 10000;

QString getHostname(const std::string& url)
{
  // Convert given url to QURL and
  // return its hostname.
  QString q_url = QString::fromStdString(url);
  return QUrl(q_url).host();
}

class WebPageBlinkPrivate : public QObject
{
    Q_OBJECT

public:
    WebPageBlinkPrivate(WebPageBlink * page)
        : q(page)
        , pageView(0)
        , m_palmSystem(0)
    {
    }

    ~WebPageBlinkPrivate()
    {
        delete pageView;

        if (m_palmSystem)
            delete m_palmSystem;
    }


public:
    WebPageBlink *q;
    BlinkWebView *pageView;
    PalmSystemBlink* m_palmSystem;
};


WebPageBlink::WebPageBlink(const QUrl& url, ApplicationDescription* desc, const QString& params)
    : WebPageBase(url, desc, params)
    , d(new WebPageBlinkPrivate(this))
    , m_isPaused(false)
    , m_isSuspended(false)
    , m_hasCustomPolicyForResponse(false)
    , m_hasBeenShown(false)
    , m_vkbHeight(0)
    , m_vkbWasOverlap(false)
    , m_hasCloseCallback(false)
    , m_trustLevel(QString::fromStdString(desc->trustLevel()))
{
}

WebPageBlink::~WebPageBlink()
{
    if(m_domSuspendTimer.isRunning())
        m_domSuspendTimer.stop();

    delete d;
    d = NULL;
}

void WebPageBlink::init()
{
    d->pageView = createPageView();
    d->pageView->setDelegate(this);
    d->pageView->Initialize(m_appDesc->id(),
                            m_appDesc->folderPath(),
                            m_appDesc->trustLevel(),
                            m_appDesc->v8SnapshotPath(),
                            m_appDesc->v8ExtraFlags(),
                            m_appDesc->useNativeScroll());
    setViewportSize();

    d->pageView->SetVisible(false);
    d->pageView->SetUserAgent(d->pageView->DefaultUserAgent() + " " + getWebAppManagerConfig()->getName());

    // TODO : need to replace qgetenv
    if(qgetenv("ENABLE_INSPECTOR") == "1")
        d->pageView->SetInspectable(true);

    if(!qgetenv("PRIVILEGED_PLUGIN_PATH").isEmpty()) {
        QString privileged_plugin_path = QLatin1String(qgetenv("PRIVILEGED_PLUGIN_PATH"));
        d->pageView->AddAvailablePluginDir(privileged_plugin_path.toStdString());
    }

    d->pageView->SetAllowFakeBoldText(false);

    // FIXME: It should be permitted for backward compatibility for a limited list of legacy applications only.
    d->pageView->SetAllowRunningInsecureContent(true);
    d->pageView->SetAllowScriptsToCloseWindows(true);
    d->pageView->SetAllowUniversalAccessFromFileUrls(true);
    d->pageView->SetSuppressesIncrementalRendering(true);
    d->pageView->SetDisallowScrollbarsInMainFrame(true);
    d->pageView->SetDisallowScrollingInMainFrame(true);
    d->pageView->SetDoNotTrack(m_appDesc->doNotTrack());
    d->pageView->SetJavascriptCanOpenWindows(true);
    d->pageView->SetSupportsMultipleWindows(false);
    d->pageView->SetCSSNavigationEnabled(true);
    d->pageView->SetV8DateUseSystemLocaloffset(false);
    d->pageView->SetLocalStorageEnabled(true);
    d->pageView->SetShouldSuppressDialogs(true);
    d->pageView->SetFixedPositionCreatesStackingContext(false);
    d->pageView->SetNotifyFMPDirectly(m_appDesc->usePrerendering());
    setDisallowScrolling(m_appDesc->disallowScrollingInMainFrame());

    if (!std::isnan(m_appDesc->networkStableTimeout()) && (m_appDesc->networkStableTimeout() >= 0.0))
        d->pageView->SetNetworkStableTimeout(m_appDesc->networkStableTimeout());

    if (QString::fromStdString(m_appDesc->trustLevel()) == "trusted") {
        LOG_DEBUG("[%s] trustLevel : trusted; allow load local Resources", qPrintable(appId()));
        d->pageView->SetAllowLocalResourceLoad(true);
    }
    d->pageView->AddUserStyleSheet("body { -webkit-user-select: none; } :focus { outline: none }");
    d->pageView->SetBackgroundColor(29, 29, 29, 0xFF);

    setDefaultFont(defaultFont());

    d->pageView->SetFontHinting(webos::WebViewBase::FontRenderParams::HINTING_SLIGHT);

    QString language;
    getSystemLanguage(language);
    setPreferredLanguages(language);
    d->pageView->SetAppId(appId().toStdString());
    updateHardwareResolution();
    updateBoardType();
    updateDatabaseIdentifier();
    updateMediaCodecCapability();
    setupStaticUserScripts();
    setCustomPluginIfNeeded();
    setSupportDolbyHDRContents();
    setCustomUserScript();
    d->pageView->SetAudioGuidanceOn(isAccessibilityEnabled());
    updateBackHistoryAPIDisabled();

    d->pageView->UpdatePreferences();

    loadExtension();
}

void* WebPageBlink::getWebContents()
{
    return (void*)d->pageView->GetWebContents();
}

void WebPageBlink::handleBrowserControlCommand(const QString& command, const QStringList& arguments)
{
    handleBrowserControlMessage(command, arguments);
}

void WebPageBlink::handleBrowserControlFunction(const QString& command, const QStringList& arguments, QString* result)
{
    *result = handleBrowserControlMessage(command, arguments);
}

QString WebPageBlink::handleBrowserControlMessage(const QString& message, const QStringList& params)
{
    if (!d->m_palmSystem)
        return QString();

    QString res = d->m_palmSystem->handleBrowserControlMessage(message, params);
    return res;
}

bool WebPageBlink::canGoBack()
{
    return d->pageView->CanGoBack();
}

QString WebPageBlink::title()
{
    return QString(d->pageView->DocumentTitle().c_str());
}

void WebPageBlink::setFocus(bool focus)
{
    d->pageView->SetFocus(focus);
}

void WebPageBlink::loadDefaultUrl()
{
    d->pageView->LoadUrl(defaultUrl().toString().toStdString());
}

int WebPageBlink::progress() const
{
    return d->pageView->progress();
}

bool WebPageBlink::hasBeenShown() const
{
    return m_hasBeenShown;
}

void WebPageBlink::replaceBaseUrl(QUrl newUrl)
{
    d->pageView->ReplaceBaseURL(newUrl.toString().toStdString(), url().toString().toStdString());
}

QUrl WebPageBlink::url() const
{
    return QUrl(d->pageView->GetUrl().c_str());
}

uint32_t WebPageBlink::getWebProcessProxyID()
{
    return 0;
}

void WebPageBlink::setPreferredLanguages(const QString& language)
{
    // just set system language for accept-language for http header, navigator.language, navigator.languages
    // even window.languagechange event too
    d->pageView->SetAcceptLanguages(language.toStdString());
    d->pageView->UpdatePreferences();

    if (d->m_palmSystem)
        d->m_palmSystem->setLocale(language);
}

void WebPageBlink::setDefaultFont(const QString& font)
{
    d->pageView->SetStandardFontFamily(font.toStdString());
    d->pageView->SetFixedFontFamily(font.toStdString());
    d->pageView->SetSerifFontFamily(font.toStdString());
    d->pageView->SetSansSerifFontFamily(font.toStdString());
    d->pageView->SetCursiveFontFamily(font.toStdString());
    d->pageView->SetFantasyFontFamily(font.toStdString());
}

void WebPageBlink::reloadDefaultPage()
{
    // When WebProcess is crashed
    // not only default page reloading,
    // need to set WebProcess setting (especially the options not using Setting or preference)

    loadDefaultUrl();
}

void WebPageBlink::loadErrorPage(int errorCode)
{
    QString errorpage = getWebAppManagerConfig()->getErrorPageUrl();
    if(!errorpage.isEmpty()) {
        if(hasLoadErrorPolicy(false, errorCode)) {
            // has loadErrorPolicy, do not show error page
            LOG_DEBUG("[%s] has own policy for Error Page, do not load Error page; send webOSLoadError event; return", qPrintable(appId()));
            return;
        }

        QString language;
        getSystemLanguage(language);
        QLocale locale(language);

        // Break the provided URL down into it's component pieces
        // we always assume the error page will be a file:// url, because that's
        // the only thing that makes sense.
        QUrl errorUrl(errorpage);
        QFile file(errorUrl.toLocalFile());
        QFileInfo fileinfo(file);
        QString fileName = fileinfo.fileName();
        QString searchPath = fileinfo.canonicalPath();
        QString errCode = QString::number(errorCode);

        // search order:
        // searchPath/resources/<language>/<script>/<region>/html/fileName
        // searchPath/resources/<language>/<region>/html/fileName
        // searchPath/resources/<language>/html/fileName
        // searchPath/resources/html/fileName
        // searchPath/fileName

        // exception :
        // locale : zh-Hant-HK, zh-Hant-TW
        // searchPath/resources/zh/Hant/HK/html/fileName
        // searchPath/resources/zh/Hant/TW/html/fileName
        // es-ES has resources/es/ES/html but QLocale::bcp47Name() returns es not es-ES
        // fr-CA, pt-PT has its own localization folder and QLocale::bcp47Name() returns well

        QString bcp47Name;
        if(language.startsWith("zh", Qt::CaseSensitive) ||language.startsWith("es", Qt::CaseSensitive) )
            bcp47Name = language.replace(QString("-"), QString("/"));
        else
            bcp47Name = locale.bcp47Name().replace(QString("-"), QString("/"));

        QStringList paths = bcp47Name.split("/");
        bool bFound = false;
        while(!paths.isEmpty() && !bFound) {
            file.setFileName(searchPath + QStringLiteral("/resources/%1/html/%2").arg(paths.join("/")).arg(fileName));
            if(!file.exists())
                paths.removeLast();
            else
                bFound = true;
        }

        if(!bFound) {
            file.setFileName(searchPath + QStringLiteral("/resources/html/%1").arg(fileName));
            bFound = file.exists();
        }
        if(!bFound) {
            file.setFileName(searchPath + QStringLiteral("/%1").arg(fileName));
            bFound = file.exists();
        }

        // finally found something!
        if(bFound) {
            // re-create it as a proper URL, so WebKit can understand it
            m_isLoadErrorPageStart = true;
            errorUrl = QUrl::fromLocalFile(file.fileName());
            // set query items for error code and hostname to URL
            QUrlQuery query;
            query.addQueryItem(QStringLiteral("errorCode"), errCode);
            query.addQueryItem(QStringLiteral("hostname"), m_loadFailedHostname);
            errorUrl.setQuery(query);
            LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "LoadErrorPage : %s", qPrintable(errorUrl.toString()));
            d->pageView->LoadUrl(errorUrl.toString().toStdString());
        } else
            LOG_ERROR(MSGID_ERROR_ERROR, 1, PMLOGKS("PATH", qPrintable(errorpage)), "Error loading error page");
    }
}

void WebPageBlink::reload()
{
    d->pageView->Reload();
}

void WebPageBlink::loadUrl(const std::string& url)
{
    d->pageView->LoadUrl(url);
}

void WebPageBlink::setLaunchParams(const QString& params)
{
    WebPageBase::setLaunchParams(params);
    if (d->m_palmSystem)
        d->m_palmSystem->setLaunchParams(params);
}

void WebPageBlink::setUseLaunchOptimization(bool enabled){
    if (getWebAppManagerConfig()->isLaunchOptimizationEnabled())
        d->pageView->SetUseLaunchOptimization(enabled);
}

void WebPageBlink::setUseSystemAppOptimization(bool enabled) {
    d->pageView->SetUseEnyoOptimization(enabled);
}

void WebPageBlink::setUseAccessibility(bool enabled)
{
    d->pageView->SetUseAccessibility(enabled);
}

void WebPageBlink::setBlockWriteDiskcache(bool blocked)
{
    d->pageView->SetBlockWriteDiskcache(blocked);
}

void WebPageBlink::setForceActivateVtg(bool enabled)
{
    d->pageView->SetForceVideoTexture(enabled);
    d->pageView->UpdatePreferences();
}

void WebPageBlink::suspendWebPageAll()
{
    LOG_INFO(MSGID_SUSPEND_WEBPAGE, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "%s", __func__);

    d->pageView->SetVisible(false);
    if (m_isSuspended || m_enableBackgroundRun)
        return;

    if (!(qgetenv("WAM_KEEP_RTC_CONNECTIONS_ON_SUSPEND") == "1")) {
        // On sending applications to background, disconnect RTC
        d->pageView->DropAllPeerConnections(webos::DROP_PEER_CONNECTION_REASON_PAGE_HIDDEN);
    }

    suspendWebPageMedia();

    // suspend painting
    // set visibility : hidden
    // set send to plugin about this visibility change
    // but NOT suspend DOM and JS Excution
    /* actually suspendWebPagePaintingAndJSExecution will do this again,
      * but this visibilitychange event and paint suspend should be done ASAP
      */
    d->pageView->SuspendPaintingAndSetVisibilityHidden();


    if (isClosing()) {
        // In app closing scenario, loading about:blank and executing onclose callback should be done
        // For that, WebPage should be resume
        // So, do not suspend here
        LOG_INFO(MSGID_SUSPEND_WEBPAGE, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "InClosing; Don't start DOMSuspendTimer");
        return;
    }

    m_isSuspended = true;
    if (shouldStopJSOnSuspend()) {
        m_domSuspendTimer.start(suspendDelay(), this,
                            &WebPageBlink::suspendWebPagePaintingAndJSExecution);
    }
    LOG_INFO(MSGID_SUSPEND_WEBPAGE, 3, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), PMLOGKFV("DELAY", "%dms", suspendDelay()), "DomSuspendTimer Started");
}

void WebPageBlink::resumeWebPageAll()
{
    LOG_INFO(MSGID_RESUME_ALL, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "");
    // resume painting
    // Resume DOM and JS Excution
    // set visibility : visible (dispatch visibilitychange event)
    // set send to plugin about this visibility change
    if (shouldStopJSOnSuspend()) {
        resumeWebPagePaintingAndJSExecution();
    }
    resumeWebPageMedia();
    d->pageView->SetVisible(true);
}

void WebPageBlink::suspendWebPageMedia()
{
    if (m_isPaused || m_enableBackgroundRun) {
        LOG_INFO(MSGID_SUSPEND_MEDIA, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "%s; Already paused; return", __func__);
        return;
    }

    d->pageView->SuspendWebPageMedia();
    m_isPaused = true;

    LOG_INFO(MSGID_SUSPEND_MEDIA, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "");

}

void WebPageBlink::resumeWebPageMedia()
{
    if (!m_isPaused) {
        LOG_INFO(MSGID_RESUME_MEDIA, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "%s; Not paused; return", __func__);
        return;
    }

    //If there is a trouble while other app loading(loading fail or other unexpected cases)
    //Set use launching time optimization false.
    //This function call ensure that case.
    setUseLaunchOptimization(false);

    d->pageView->ResumeWebPageMedia();
    m_isPaused = false;

    LOG_INFO(MSGID_RESUME_MEDIA, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "");
}

void WebPageBlink::suspendWebPagePaintingAndJSExecution()
{
    LOG_INFO(MSGID_SUSPEND_WEBPAGE, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "%s; m_isSuspended : %s", __func__, m_isSuspended ? "true" : "false; will be returned");
    if (m_domSuspendTimer.isRunning()) {
        LOG_INFO(MSGID_SUSPEND_WEBPAGE_DELAYED, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "DomSuspendTimer Expired; suspend DOM");
        m_domSuspendTimer.stop();
    }

    if (m_enableBackgroundRun)
        return;

    if (!m_isSuspended)
        return;

    // if we haven't finished loading the page yet, wait until it is loaded before suspending
    bool isLoading = !hasBeenShown() && progress() < 100;
    if (isLoading) {
        LOG_INFO(MSGID_SUSPEND_WEBPAGE, 3, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()),  PMLOGKS("URL", qPrintable(url().toString())), "Currently loading, Do not suspend, return");
        m_suspendAtLoad = true;
    } else {
        d->pageView->SuspendPaintingAndSetVisibilityHidden();
        d->pageView->SuspendWebPageDOM();
        LOG_INFO(MSGID_SUSPEND_WEBPAGE, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "DONE");
    }
}

void WebPageBlink::resumeWebPagePaintingAndJSExecution()
{
    LOG_INFO(MSGID_RESUME_WEBPAGE, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "%s; m_isSuspended : %s ", __func__, m_isSuspended ? "true" : "false; nothing to resume");
    m_suspendAtLoad = false;
    if (m_isSuspended) {
        if (m_domSuspendTimer.isRunning()) {
            LOG_INFO(MSGID_SUSPEND_WEBPAGE, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "DomSuspendTimer canceled by Resume");
            m_domSuspendTimer.stop();
            d->pageView->ResumePaintingAndSetVisibilityVisible();
        } else {
            d->pageView->ResumeWebPageDOM();
            d->pageView->ResumePaintingAndSetVisibilityVisible();
            LOG_INFO(MSGID_RESUME_WEBPAGE, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "DONE");
        }
        m_isSuspended = false;
    }
}

QString WebPageBlink::escapeData(const QString& value)
{
    QString escapedValue(value);
    escapedValue.replace("\\", "\\\\")
                .replace("'", "\\'")
                .replace("\n", "\\n")
                .replace("\r", "\\r");
    return escapedValue;
}

void WebPageBlink::reloadExtensionData()
{
    QString eventJS = QStringLiteral(
       "if (typeof(PalmSystem) != 'undefined') {"
       "  PalmSystem.reloadInjectionData();"
       "};"
    );
    LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "Reload");
    evaluateJavaScript(eventJS);
}

void WebPageBlink::updateExtensionData(const QString& key, const QString& value)
{
    QString eventJS = QStringLiteral(
       "if (typeof(PalmSystem) != 'undefined') {"
       "  PalmSystem.updateInjectionData('%1', '%2');"
       "};"
    ).arg(escapeData(key)).arg(escapeData(value));
    LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "Update; key:%s; value:%s",
        qPrintable(key), qPrintable(value));
    evaluateJavaScript(eventJS);
}

void WebPageBlink::updatePageSettings()
{
    // When a container based app is launched
    // if there any properties different from container app then should update
    // ex, application description info
    if(!m_appDesc)
        return;

    if(QString::fromStdString(m_appDesc->trustLevel()) == "trusted") {
        LOG_DEBUG("[%s] trustLevel : trusted; allow load local Resources", qPrintable(appId()));
        d->pageView->SetAllowLocalResourceLoad(true);
    }

    LOG_DEBUG("[%s] WebPageBlink::updatePageSettings(); update appId to chromium", qPrintable(appId()));
    d->pageView->SetAppId(appId().toStdString());
    d->pageView->SetTrustLevel(m_appDesc->trustLevel());
    d->pageView->SetAppPath(m_appDesc->folderPath());

    if (!std::isnan(m_appDesc->networkStableTimeout()) && (m_appDesc->networkStableTimeout() >= 0.0))
        d->pageView->SetNetworkStableTimeout(m_appDesc->networkStableTimeout());

    setCustomPluginIfNeeded();
    updateBackHistoryAPIDisabled();
    d->pageView->UpdatePreferences();
}

void WebPageBlink::handleDeviceInfoChanged(const QString& deviceInfo)
{
    if (!d->m_palmSystem)
        return;

    if (deviceInfo == "LocalCountry" || deviceInfo == "SmartServiceCountry")
        d->m_palmSystem->setCountry();
}

void WebPageBlink::evaluateJavaScript(const QString& jsCode)
{
    d->pageView->RunJavaScript(jsCode.toStdString());
}

void WebPageBlink::evaluateJavaScriptInAllFrames(const QString &script, const char *method)
{
    d->pageView->RunJavaScriptInAllFrames(script.toStdString());
}

void WebPageBlink::cleanResources()
{
    WebPageBase::cleanResources();
    LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "StopLoading and load about:blank");
    d->pageView->StopLoading();
    d->pageView->LoadUrl(std::string("about:blank"));
}

void WebPageBlink::close()
{
    Q_EMIT webPageClosePageRequested();
}

void WebPageBlink::didFirstFrameFocused()
{
    LOG_DEBUG("[%s] render process frame focused for the first time", qPrintable(appId()));
    //App load is finished, set use launching time optimization false.
    setUseLaunchOptimization(false);
}

void WebPageBlink::didDropAllPeerConnections()
{
}

void WebPageBlink::loadFinished(const std::string& url)
{
    LOG_INFO(MSGID_WEBPAGE_LOAD_FINISHED, 2,
        PMLOGKS("APP_ID", qPrintable(appId())),
        PMLOGKFV("PID", "%d", getWebProcessPID()),
        "url from web engine : %s", url.c_str());

    if (cleaningResources()) {
        LOG_INFO(MSGID_WEBPAGE_LOAD_FINISHED,
            2,
            PMLOGKS("APP_ID", qPrintable(appId())),
            PMLOGKFV("PID", "%d", getWebProcessPID()),
            "cleaningResources():true; (should be about:blank) emit 'didDispatchUnload'");
        // TODO: Remove QSignal
        Q_EMIT didDispatchUnload();
        return;
    }
    handleLoadFinished();
}

void WebPageBlink::loadStarted()
{
    LOG_INFO(MSGID_PAGE_LOADING, 3, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()),  PMLOGKS("LOADING", "STARTED"), "");
    m_hasCloseCallback = false;
    handleLoadStarted();
}

void WebPageBlink::loadStopped(const std::string& url)
{
    LOG_INFO(MSGID_PAGE_LOADING, 3, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()),  PMLOGKS("LOADING", "STOPPED"), "");
}

void WebPageBlink::loadFailed(const std::string& url, int errCode, const std::string& errDesc)
{
    Q_EMIT webPageLoadFailed(errCode);

    // We follow through only if we have SSL error
    if (errDesc != "SSL_ERROR")
        return;

    LOG_WARNING(MSGID_PAGE_LOAD_FAILED, 4,
                PMLOGKS("APP_ID", qPrintable(appId())),
                PMLOGKFV("ERROR_CODE", "%d", errCode),
                PMLOGKS("ERROR_STR", errDesc.c_str()),
                PMLOGKS("URL", url.c_str()),
                " ");
    m_loadFailedHostname = getHostname(url);
    handleLoadFailed(errCode);
}

void WebPageBlink::loadVisuallyCommitted()
{
    m_hasBeenShown = true;
    FOR_EACH_OBSERVER(WebPageObserver,
                      m_observers, firstFrameVisuallyCommitted());
}

void WebPageBlink::renderProcessCreated(int pid)
{
    postWebProcessCreated(pid);
}

void WebPageBlink::titleChanged(const std::string& title)
{
    FOR_EACH_OBSERVER(WebPageObserver, m_observers, titleChanged());
}

void WebPageBlink::navigationHistoryChanged()
{
    FOR_EACH_OBSERVER(WebPageObserver, m_observers, navigationHistoryChanged());
}

void WebPageBlink::forwardEvent(void* event)
{
    d->pageView->ForwardWebOSEvent((WebOSEvent*)event);
}

void WebPageBlink::recreateWebView()
{
    LOG_INFO(MSGID_WEBPROC_CRASH, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "recreateWebView; initialize WebPage");
    delete d->pageView;
    if(!m_customPluginPath.isEmpty()) {
        // check setCustomPluginIfNeeded logic
        // not to set duplicated plugin path, it compares m_customPluginPath and new one
        m_customPluginPath = "";  // just make it init state
    }

    init();
    Q_EMIT webViewRecreated();

    if (!m_isSuspended) {
        // Remove white screen while reloading contents due to the renderer crash
        // 1. Reset state to mark next paint for notification when FMP done.
        //    It will be used to make webview visible later.
        d->pageView->ResetStateToMarkNextPaintForContainer();
        // 2. While rendering ready, set webview hidden.
        d->pageView->SetVisible(false);
        // 3. Set VisibilityState as Launching
        //    It will be used later, WebViewImpl set RenderWidgetCompositor visible,
        //    and make it keep to render the contents.
        setVisibilityState(WebPageBase::WebPageVisibilityState::WebPageVisibilityStateLaunching);
    }

    if (m_isSuspended)
        m_isSuspended = false;
}

void WebPageBlink::setVisible(bool visible)
{
    d->pageView->SetVisible(visible);
}

void WebPageBlink::setViewportSize()
{
    if (m_appDesc->widthOverride() && m_appDesc->heightOverride()) {
        d->pageView->SetViewportSize(m_appDesc->widthOverride(), m_appDesc->heightOverride());
    }
}

void WebPageBlink::notifyMemoryPressure(webos::WebViewBase::MemoryPressureLevel level)
{
    d->pageView->NotifyMemoryPressure(level);
}

void WebPageBlink::renderProcessCrashed()
{
    LOG_INFO(MSGID_WEBPROC_CRASH, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "m_isSuspended : %s", m_isSuspended?"true":"false");
    if (isClosing()) {
        LOG_INFO(MSGID_WEBPROC_CRASH, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "In Closing; return");
        if (m_closeCallbackTimer.isRunning())
            m_closeCallbackTimer.stop();

        Q_EMIT closingAppProcessDidCrashed();
        return;
    }

    d->m_palmSystem->setInitialized(false);
    recreateWebView();
    if (!processCrashed())
        handleForceDeleteWebPage();
}

void WebPageBlink::didFinishLaunchingSlot()
{
}

// functions from webappmanager2
BlinkWebView * WebPageBlink::createPageView()
{
    return new BlinkWebView();
}

BlinkWebView* WebPageBlink::pageView() const
{
    return d->pageView;
}

bool WebPageBlink::inspectable()
{
    return getWebAppManagerConfig()->isInspectorEnabled();
}


// webOSLaunch / webOSRelaunch event:
// webOSLaunch event should be fired after DOMContentLoaded, and contains the launch parameters as it's detail.
// webOSRelaunch event should be fired when an app that is already running is triggered from applicationManager/launch, and
// will also contain the launch parameters as it's detail.
// IF we fire webOSLaunch immediately at handleLoadFinished(), the document may receive it before it has parsed all of the scripts.

// We cannot setup a generic script at page creation, because we don't know the launch parameters at
// that time. So, at load start, we'll take care of adding a user script.  Once that script has been
// added, it does not need to be added again -- triggering a page reload will cause it to fire the
// event again.

// There are a few caveats here, though:
// 1- We don't want to make a seperate HTML file just for this, so we use the C API for adding a UserScript
// 2- The QT API for adding a user script only accepts a URL to a file, not absolute code.
// 3- We can't call WKPageGroupAddUserScript with the same argument more than once unless we want duplicate code to run

// So, we clear out any userscripts that may have been set, add any userscript files (ie Tellurium) via the QT API,
// then add any other userscripts that we might want via the C API, and then proceed.

// IF any further userscripts are desired in the future, they should be added here.
void WebPageBlink::addUserScript(const QString& script)
{
    d->pageView->addUserScript(script.toStdString());
}

void WebPageBlink::addUserScriptUrl(const QUrl& url)
{
    QString path;
    if (url.isLocalFile()) {
        path = url.toLocalFile();
    }
    else {
        LOG_DEBUG("WebPageBlink: Couldn't open '%s' as user script because only file:/// URLs are supported.", qPrintable(url.toString()));
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_DEBUG("WebPageBlink: Couldn't open '%s' as user script due to error '%s'.", qPrintable(url.toString()), qPrintable(file.errorString()));
        return;
    }

    QByteArray contents = file.readAll();
    if (contents.isEmpty()) {
        LOG_DEBUG("WebPageBlink: Ignoring '%s' as user script because file is empty.", qPrintable(url.toString()));
        return;
    }
    d->pageView->addUserScript(contents.toStdString());
}

void WebPageBlink::setupStaticUserScripts()
{
    d->pageView->clearUserScripts();

    // Load Tellurium test framework if available, as a UserScript
    QString telluriumNubPath_ = telluriumNubPath();
    if (!telluriumNubPath_.isEmpty()) {
        LOG_DEBUG("Loading tellurium nub at %s", qPrintable(telluriumNubPath_));
        addUserScriptUrl(QUrl::fromLocalFile(telluriumNubPath_));
    }
}

void WebPageBlink::closeVkb()
{
}

bool WebPageBlink::isKeyboardVisible() const
{
    return d->pageView->IsKeyboardVisible();
}

bool WebPageBlink::isInputMethodActive() const
{
    return d->pageView->IsInputMethodActive();
}

void WebPageBlink::setPageProperties()
{
    if (m_appDesc->isTransparent()) {
        d->pageView->SetTransparentBackground(true);
    }

    // set inspectable
    if (m_appDesc->isInspectable() || inspectable()) {
        LOG_DEBUG("[%s] inspectable : true or 'debug_system_apps' mode; setInspectablePage(true)", qPrintable(appId()));
        d->pageView->EnableInspectablePage();
    }

    setTrustLevel(defaultTrustLevel());
    d->pageView->UpdatePreferences();
}

void WebPageBlink::createPalmSystem(WebAppBase* app)
{
    d->m_palmSystem = new PalmSystemBlink(app);
    d->m_palmSystem->setLaunchParams(m_launchParams);
}

QString WebPageBlink::defaultTrustLevel() const
{
    return QString::fromStdString(m_appDesc->trustLevel());
}

void WebPageBlink::loadExtension()
{
    LOG_DEBUG("WebPageBlink::loadExtension(); Extension : palmsystem");
    d->pageView->LoadExtension("palmsystem");
}

void WebPageBlink::clearExtensions()
{
    if (d && d->pageView)
        d->pageView->ClearExtensions();
}

void WebPageBlink::setCustomPluginIfNeeded()
{
    if (!m_appDesc || !m_appDesc->useCustomPlugin())
        return;

    QString customPluginPath = QString::fromStdString(m_appDesc->folderPath());
    customPluginPath.append("/plugins");

    if (!QDir(customPluginPath).exists())
        return;
    if (!m_customPluginPath.compare(customPluginPath))
        return;

    m_customPluginPath = customPluginPath;
    LOG_INFO(MSGID_WAM_DEBUG, 3, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), PMLOGKS("CUSTOM_PLUGIN_PATH", qPrintable(m_customPluginPath)), "%s", __func__);

    d->pageView->AddCustomPluginDir(m_customPluginPath.toStdString());
    d->pageView->AddAvailablePluginDir(m_customPluginPath.toStdString());
}

void WebPageBlink::setDisallowScrolling(bool disallow)
{
    d->pageView->SetDisallowScrollbarsInMainFrame(disallow);
    d->pageView->SetDisallowScrollingInMainFrame(disallow);
}

int WebPageBlink::renderProcessPid() const
{
    return d->pageView->RenderProcessPid();
}

void WebPageBlink::didRunCloseCallback()
{
    m_closeCallbackTimer.stop();
    LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "WebPageBlink::didRunCloseCallback(); onclose callback done");
    Q_EMIT closeCallbackExecuted();
}

void WebPageBlink::setHasOnCloseCallback(bool hasCloseCallback)
{
    m_hasCloseCallback = hasCloseCallback;
}

void WebPageBlink::executeCloseCallback(bool forced)
{
    QString script = QStringLiteral(
       "window.PalmSystem._onCloseWithNotify_('%1');").arg(forced?"forced" : "normal");

    evaluateJavaScript(script);

    m_closeCallbackTimer.start(kExecuteCloseCallbackTimeOutMs, this, &WebPageBlink::timeoutCloseCallback);
}

void WebPageBlink::timeoutCloseCallback()
{
    m_closeCallbackTimer.stop();
    LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "WebPageBlink::timeoutCloseCallback(); onclose callback Timeout");
    Q_EMIT timeoutExecuteCloseCallback();
}

void WebPageBlink::setFileAccessBlocked(bool blocked)
{
    //TO_DO: Need to verify when shnapshot is ready.
    webos::WebViewBase::SetFileAccessBlocked(blocked);
}

void WebPageBlink::updateHardwareResolution()
{
    QString hardwareWidth, hardwareHeight;
    getDeviceInfo("HardwareScreenWidth", hardwareWidth);
    getDeviceInfo("HardwareScreenHeight", hardwareHeight);
    d->pageView->SetHardwareResolution(hardwareWidth.toInt(), hardwareHeight.toInt());
}

void WebPageBlink::updateBoardType()
{
    QString boardType;
    getDeviceInfo("boardType", boardType);
    d->pageView->SetBoardType(boardType.toStdString());
}

void WebPageBlink::updateMediaCodecCapability()
{
    QFile file("/etc/umediaserver/device_codec_capability_config.json");
    if (!file.exists())
        return;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString capability;
    QTextStream in(&file);
    capability.append(in.readAll());

    d->pageView->SetMediaCodecCapability(capability.toStdString());
}

double WebPageBlink::devicePixelRatio()
{
    double appWidth = static_cast<double>(m_appDesc->widthOverride());
    double appHeight =  static_cast<double>(m_appDesc->heightOverride());
    if(appWidth == 0) appWidth = static_cast<double>(currentUiWidth());
    if(appHeight == 0) appHeight = static_cast<double>(currentUiHeight());

    QString hardwareWidth, hardwareHeight;
    getDeviceInfo("HardwareScreenWidth", hardwareWidth);
    getDeviceInfo("HardwareScreenHeight", hardwareHeight);

    double deviceWidth = hardwareWidth.toDouble();
    double deviceHeight = hardwareHeight.toDouble();

    double ratioX = deviceWidth/appWidth;
    double ratioY = deviceHeight/appHeight;
    double devicePixelRatio = 1.0;
    if(ratioX != ratioY) {
        // device resolution : 5120x2160 (UHD 21:9 - D9)
        // - app resolution : 1280x720 ==> 4:3 (have to take 3)
        // - app resolution : 1920x1080 ==> 2.6:2 (have to take 2)
        devicePixelRatio = (ratioX < ratioY) ? ratioX : ratioY;
    } else {
        // device resolution : 1920x1080
        // - app resolution : 1280x720 ==> 1.5:1.5
        // - app resolution : 1920x1080 ==> 1:1
        // device resolution : 3840x2160
        // - app resolution : 1280x720 ==> 3:3
        // - app resolution : 1920x1080 ==> 2:2
        devicePixelRatio = ratioX;
    }
    LOG_DEBUG("[%s] WebPageBlink::devicePixelRatio(); devicePixelRatio : %f; deviceWidth : %f, deviceHeight : %f, appWidth : %f, appHeight : %f",
        qPrintable(appId()), devicePixelRatio, deviceWidth, deviceHeight, appWidth, appHeight);
    return devicePixelRatio;
}

void WebPageBlink::setSupportDolbyHDRContents()
{
    QString supportDolbyHDRContents;
    getDeviceInfo("supportDolbyHDRContents", supportDolbyHDRContents);
    LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "supportDolbyHDRContents:%s", qPrintable(supportDolbyHDRContents));
    d->pageView->SetSupportDolbyHDRContents(supportDolbyHDRContents == "true");
}

void WebPageBlink::updateDatabaseIdentifier()
{
    d->pageView->SetDatabaseIdentifier(m_appId.toStdString());
}

void WebPageBlink::deleteWebStorages(const QString& identifier)
{
    d->pageView->DeleteWebStorages(identifier.toStdString());
}

void WebPageBlink::setInspectorEnable()
{
    LOG_DEBUG("[%s] Inspector enable", qPrintable(appId()));
    d->pageView->EnableInspectablePage();
}

void WebPageBlink::setKeepAliveWebApp(bool keepAlive) {
    LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "setKeepAliveWebApp(%s)", keepAlive?"true":"false");
    d->pageView->SetKeepAliveWebApp(keepAlive);
    d->pageView->UpdatePreferences();
}

void WebPageBlink::setLoadErrorPolicy(const QString& policy)
{
    m_loadErrorPolicy = policy;
    if(!policy.compare("event")) {
        // policy : event
        m_hasCustomPolicyForResponse = true;
    } else if (!policy.compare("default")) {
        // policy : default, WAM and blink handle all load errors
        m_hasCustomPolicyForResponse = false;
    }
}

bool WebPageBlink::decidePolicyForResponse(bool isMainFrame, int statusCode, const std::string& url, const std::string& statusText)
{
    LOG_INFO(MSGID_WAM_DEBUG, 7, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), PMLOGKFV("STATUS_CODE", "%d", statusCode),
        PMLOGKS("URL", url.c_str()), PMLOGKS("TEXT", statusText.c_str()), PMLOGKS("MAIN_FRAME", isMainFrame ? "true" : "false"), PMLOGKS("RESPONSE_POLICY", isMainFrame ? "event" : "default"), "");

    // how to WAM3 handle this response
    applyPolicyForUrlResponse(isMainFrame, QString(url.c_str()), statusCode);

    // how to blink handle this response
    // ACR requirement : even if received error response from subframe(iframe)ACR app should handle that as a error
    return m_hasCustomPolicyForResponse;
}

bool WebPageBlink::acceptsVideoCapture()
{
  return m_appDesc->allowVideoCapture();
}

bool WebPageBlink::acceptsAudioCapture()
{
  return m_appDesc->allowAudioCapture();
}

void WebPageBlink::keyboardVisibilityChanged(bool visible)
{
    QString javascript = QStringLiteral(
        "console.log('[WAM] fires keyboardStateChange event : %1');"
        "    var keyboardStateEvent =new CustomEvent('keyboardStateChange', { detail: { 'visibility' : %2 } });"
        "    keyboardStateEvent.visibility = %3;"
        "    if(document) document.dispatchEvent(keyboardStateEvent);"
    ).arg(visible ? "true" : "false").arg(visible ? "true" : "false").arg(visible ? "true" : "false");
    evaluateJavaScript(javascript);
}

void WebPageBlink::updateIsLoadErrorPageFinish()
{
    // If currently loading finished URL is not error page,
    // m_isLoadErrorPageFinish will be updated
    bool wasErrorPage = m_isLoadErrorPageFinish;
    WebPageBase::updateIsLoadErrorPageFinish();

    if (trustLevel().compare("trusted") && wasErrorPage != m_isLoadErrorPageFinish) {
        if (m_isLoadErrorPageFinish) {
            LOG_DEBUG("[%s] WebPageBlink::updateIsLoadErrorPageFinish(); m_isLoadErrorPageFinish : %s, set trustLevel : trusted to WAM and PalmSystem_injection", qPrintable(appId()), m_isLoadErrorPageFinish ? "true" : "false");
            setTrustLevel("trusted");
            updateExtensionData("trustLevel", "trusted");
        }
    } else {
        setTrustLevel(defaultTrustLevel());
        updateExtensionData("trustLevel", trustLevel());
    }
}

void WebPageBlink::setAudioGuidanceOn(bool on)
{
    d->pageView->SetAudioGuidanceOn(on);
    d->pageView->UpdatePreferences();
}

void WebPageBlink::resetStateToMarkNextPaintForContainer()
{
    d->pageView->ResetStateToMarkNextPaintForContainer();
}

void WebPageBlink::updateBackHistoryAPIDisabled()
{
    d->pageView->SetBackHistoryAPIDisabled(m_appDesc->backHistoryAPIDisabled());
}

void WebPageBlink::setVisibilityState(WebPageVisibilityState visibilityState)
{
    d->pageView->SetVisibilityState(static_cast<webos::WebViewBase::WebPageVisibilityState>(visibilityState));
}

bool WebPageBlink::allowMouseOnOffEvent() const {
    return false;
}

#include "WebPageBlink.moc"

