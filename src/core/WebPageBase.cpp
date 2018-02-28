// Copyright (c) 2013-2018 LG Electronics, Inc.
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

#include "WebPageBase.h"

#include <QDir>
#include <QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include "ApplicationDescription.h"
#include "LogManager.h"
#include "WebAppManagerConfig.h"
#include "WebAppManager.h"
#include "WebPageObserver.h"
#include "WebProcessManager.h"

#define CONSOLE_DEBUG(AAA) evaluateJavaScript(QStringLiteral("console.debug('") + QStringLiteral(AAA) + QStringLiteral("');"))

WebPageBase::WebPageBase()
    : m_appDesc(0)
    , m_suspendAtLoad(false)
    , m_isClosing(false)
    , m_isLoadErrorPageFinish(false)
    , m_isLoadErrorPageStart(false)
    , m_enableBackgroundRun(false)
    , m_loadErrorPolicy(QStringLiteral("default"))
    , m_cleaningResources(false)
    , m_isPreload(false)
{
}

WebPageBase::WebPageBase(const QUrl& url, ApplicationDescription* desc, const QString& params)
    : m_appDesc(desc)
    , m_appId(QString::fromStdString(desc->id()))
    , m_suspendAtLoad(false)
    , m_isClosing(false)
    , m_isLoadErrorPageFinish(false)
    , m_isLoadErrorPageStart(false)
    , m_enableBackgroundRun(false)
    , m_defaultUrl(url)
    , m_launchParams(params)
    , m_loadErrorPolicy(QStringLiteral("default"))
    , m_cleaningResources(false)
    , m_isPreload(false)
{
}

WebPageBase::~WebPageBase()
{
    LOG_INFO(MSGID_WEBPAGE_CLOSED, 1, PMLOGKS("APP_ID", qPrintable(appId())), "");
}

QString WebPageBase::launchParams() const
{
    return m_launchParams;
}

void WebPageBase::setLaunchParams(const QString& params)
{
    m_launchParams = params;
}

void WebPageBase::setApplicationDescription(ApplicationDescription* desc)
{
    m_appDesc = desc;
    setPageProperties();
}

QString WebPageBase::getIdentifier() const
{
    // If appId is ContainerAppId then it should be ""? Why not just container appid?
    // I think there shouldn't be any chance to be returned container appid even for container base app

    if(appId().isEmpty() || appId() == WebAppManager::instance()->getContainerAppId())
        return QStringLiteral("");
    return m_appId;
}

void WebPageBase::load()
{
    LOG_INFO(MSGID_WEBPAGE_LOAD, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "m_launchParams:%s", qPrintable(m_launchParams));
    /* this function is main load of WebPage : load default url */
    setupLaunchEvent();
    if (!doDeeplinking(m_launchParams)) {
        LOG_INFO(MSGID_WEBPAGE_LOAD, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "loadDefaultUrl()");
        loadDefaultUrl();
    }
}

void WebPageBase::setupLaunchEvent()
{
    QString launchEventJS = QStringLiteral(
            "(function() {"
            "    var launchEvent = new CustomEvent('webOSLaunch', { detail: %1 });"
            "    if(document.readyState === 'complete') {"
            "        setTimeout(function() {"
            "            document.dispatchEvent(launchEvent);"
            "        }, 1);"
            "    } else {"
            "        document.addEventListener('DOMContentLoaded', function() {"
            "            setTimeout(function() {"
            "                document.dispatchEvent(launchEvent);"
            "            }, 1);"
            "        });"
            "    }"
            "})();"
            ).arg(launchParams().isEmpty() ? "{}" : launchParams());
    addUserScript(launchEventJS);
}

void WebPageBase::sendLocaleChangeEvent(const QString& language)
{
    evaluateJavaScript(QStringLiteral(
        "setTimeout(function () {"
        "    var localeEvent=new CustomEvent('webOSLocaleChange');"
        "    document.dispatchEvent(localeEvent);"
        "}, 1);"
    ));
}

void WebPageBase::cleanResources()
{
    setCleaningResources(true);
}

bool WebPageBase::relaunch(const QString& launchParams, const QString& launchingAppId)
{
    resumeWebPagePaintingAndJSExecution();

    // for common webapp relaunch scenario
    // 1. For hosted webapp deeplinking : reload default page
    // 2-1. check progress; to send webOSRelaunch event, then page loading progress should be 100
    // 2-2. Update launchParams
    // 2-3. send webOSRelaunch event

    if(doHostedWebAppRelaunch(launchParams)) {
        LOG_DEBUG("[%s] Hosted webapp; handled", qPrintable(m_appId));
        return true;
    }

    if (!hasBeenShown()){
        LOG_INFO(MSGID_WEBPAGE_RELAUNCH, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "In Loading(%d%%), Can not handle relaunch now, return false", progress());
        return false;
    }

    setLaunchParams(launchParams);

    // WebPageBase::relaunch handles setting the stageArgs for the launch/relaunch events
    sendRelaunchEvent();
    return true;
}

bool WebPageBase::doHostedWebAppRelaunch(const QString& launchParams)
{
    /* hosted webapp deeplinking spec
    // legacy case
    "deeplinkingParams":"{ \
        \"contentTarget\" : \"https://www.youtube.com/tv?v=$CONTENTID\" \
    }"
    // webOS4.0 spec
    "deeplinkingParams":"{ \
        \"handledBy\" : \"platform\" || \"app\" || \"default\", \
        \"contentTarget\" : \"https://www.youtube.com/tv?v=$CONTENTID\" \
    }"
    To support backward compatibility, should cover the case not having "handledBy"
    */
    // check deeplinking relaunch condition
    QJsonObject obj = QJsonDocument::fromJson(launchParams.toUtf8()).object();
    if (url().scheme() ==  "file"
        || m_defaultUrl.scheme() != "file"
        || obj.isEmpty() /* no launchParams, { }, and this should be check with object().isEmpty()*/
        || obj.value("contentTarget").isUndefined()
        || (m_appDesc && !m_appDesc->handlesDeeplinking())) {
        LOG_INFO(MSGID_WEBPAGE_RELAUNCH, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()),
            "%s; NOT enough deeplinking condition; return false", __func__);
        return false;
    }

    // Do deeplinking relaunch
    setLaunchParams(launchParams);
    return doDeeplinking(launchParams);
}

bool WebPageBase::doDeeplinking(const QString& launchParams)
{
    QJsonObject obj = QJsonDocument::fromJson(launchParams.toUtf8()).object();
    if (obj.isEmpty() || obj.value("contentTarget").isUndefined())
        return false;

    std::string handledBy = obj.value("handledBy").isUndefined() ? "default" : obj.value("handledBy").toString().toStdString();
    if (handledBy == "platform") {
        std::string targetUrl = obj.value("contentTarget").toString().toStdString();
        LOG_INFO(MSGID_DEEPLINKING, 3, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()),
            PMLOGKS("handledBy", handledBy.c_str()),
            "%s; load target URL:%s", __func__, targetUrl.c_str());
        // load the target URL directly
        loadUrl(targetUrl);
        return true;
    } else if(handledBy == "app") {
        // If "handledBy" == "app" return false
        // then it will be handled just like common relaunch case, checking progress
        return false;
    } else {
        // handledBy == "default" or "other values"
        LOG_INFO(MSGID_DEEPLINKING, 3, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()),
            PMLOGKS("handledBy", handledBy.c_str()), "%s; loadDefaultUrl", __func__);
        loadDefaultUrl();
        return true;
    }
}

void WebPageBase::sendRelaunchEvent()
{
    setVisible(true);
    LOG_INFO(MSGID_SEND_RELAUNCHEVENT, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "");
    // Send the relaunch event on the next tick after javascript is loaded
    // This is a workaround for a problem where WebKit can't free the page
    // if we don't use a timeout here.
    evaluateJavaScript(QStringLiteral(
        "setTimeout(function () {"
        "    console.log('[WAM] fires webOSRelaunch event');"
        "    var launchEvent=new CustomEvent('webOSRelaunch', { detail: %1 });"
        "    document.dispatchEvent(launchEvent);"
        "}, 1);").arg(launchParams().isEmpty() ? "{}" : launchParams()));
}

void WebPageBase::urlChangedSlot()
{
    Q_EMIT webPageUrlChanged();
}

void WebPageBase::handleLoadStarted()
{
    m_suspendAtLoad = true;
}

void WebPageBase::handleLoadFinished()
{
    LOG_INFO(MSGID_WEBPAGE_LOAD_FINISHED, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "m_suspendAtLoad : %s", m_suspendAtLoad ? "true; suspend in this time" : "false");
    if (appId() == WebAppManager::instance()->getContainerAppId())
        WebAppManager::instance()->setContainerAppLaunched(true);

    Q_EMIT webPageLoadFinished();

    // if there was an attempt made to suspend while this page was loading, then
    // we flag m_suspendAtLoad = true, and suspend it after it is loaded. This is
    // to prevent application load from failing.
    if(m_suspendAtLoad) {
        suspendWebPagePaintingAndJSExecution();
    }
    updateIsLoadErrorPageFinish();
}

void WebPageBase::handleLoadFailed(int errorCode)
{
    LOG_INFO(MSGID_WEBPAGE_LOAD_FAILED, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "");

    // errorCode 204 specifically states that the web browser not relocate
    // http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
    // we can't handle unknown protcol like mailto.
    // Client want to not show error page with unknown protocol like chrome.
    if (!m_isPreload && errorCode != 204 && errorCode != 301)
        loadErrorPage(errorCode);
}

void WebPageBase::cleanResourcesFinished()
{
    WebAppManager::instance()->postRunningAppList();
    if (m_cleaningResources) {
        WebAppManager::instance()->removeWebAppFromWebProcessInfoMap(appId());
        delete this;
    }
}

void WebPageBase::handleForceDeleteWebPage()
{
    delete this;
}

bool WebPageBase::getSystemLanguage(QString &value)
{
    return WebAppManager::instance()->getSystemLanguage(value);
}

bool WebPageBase::getDeviceInfo(QString name, QString &value)
{
    return WebAppManager::instance()->getDeviceInfo(name, value);
}

int WebPageBase::currentUiWidth()
{
    return WebAppManager::instance()->currentUiWidth();
}

int WebPageBase::currentUiHeight()
{
    return WebAppManager::instance()->currentUiHeight();
}

WebProcessManager* WebPageBase::getWebProcessManager()
{
    return WebAppManager::instance()->getWebProcessManager();
}

WebAppManagerConfig* WebPageBase::getWebAppManagerConfig()
{
    return WebAppManager::instance()->config();
}

bool WebPageBase::processCrashed()
{
    return WebAppManager::instance()->processCrashed(appId());
}

int WebPageBase::suspendDelay()
{
    return WebAppManager::instance()->getSuspendDelay();
}

QString WebPageBase::telluriumNubPath()
{
    return getWebAppManagerConfig()->getTelluriumNubPath();
}

void WebPageBase::doLoadSlot()
{
    loadDefaultUrl();
}

bool WebPageBase::hasLoadErrorPolicy(bool isHttpResponseError, int errorCode)
{
    if (!m_loadErrorPolicy.compare("event")) {
       evaluateJavaScript(QStringLiteral(
           "{"
           "    console.log('[WAM3] create webOSLoadError event');"
           "    var launchEvent=new CustomEvent('webOSLoadError', { detail : { genericError : %1, errorCode : %2}});"
           "    document.dispatchEvent(launchEvent);"
           "}" ).arg(isHttpResponseError?"false":"true").arg(errorCode));
       //App has load error policy, do not show platform load error page
       return true;
    }
    return false;
}

void WebPageBase::applyPolicyForUrlResponse(bool isMainFrame, const QString& url, int status_code)
{
    QUrl qUrl(url);
    static const int s_httpErrorStatusCode = 400;
    if (qUrl.scheme() != "file" &&  status_code >= s_httpErrorStatusCode) {
        if(!hasLoadErrorPolicy(true, status_code) && isMainFrame) {
            // If app does not have policy for load error and
            // this error response is from main frame document
            // then before open server error page, reset the body's background color to white
            setBackgroundColorOfBody(QStringLiteral("white"));
        }
    }
}

void WebPageBase::postRunningAppList()
{
    WebAppManager::instance()->postRunningAppList();
}

void WebPageBase::postWebProcessCreated(uint32_t pid)
{
    WebAppManager::instance()->postWebProcessCreated(m_appId, pid);
}

void WebPageBase::setBackgroundColorOfBody(const QString& color)
{
    // for error page only, set default background color to white by executing javascript
    QString whiteBackground = QStringLiteral(
        "(function() {"
        "    if(document.readyState === 'complete' || document.readyState === 'interactive') { "
        "       if(document.body.style.backgroundColor)"
        "           console.log('[Server Error] Already set document.body.style.backgroundColor');"
        "       else {"
        "           console.log('[Server Error] set background Color of body to %1');"
        "           document.body.style.backgroundColor = '%2';"
        "       }"
        "     } else {"
        "        document.addEventListener('DOMContentLoaded', function() {"
        "           if(document.body.style.backgroundColor)"
        "               console.log('[Server Error] Already set document.body.style.backgroundColor');"
        "           else {"
        "               console.log('[Server Error] set background Color of body to %3');"
        "               document.body.style.backgroundColor = '%4';"
        "           }"
        "        });"
        "    }"
        "})();"
    ).arg(color).arg(color).arg(color).arg(color);
    evaluateJavaScript(whiteBackground);
}

QString WebPageBase::defaultFont()
{
    QString defaultFont = "LG Display-Regular";
    QString language;
    QString country;
    getSystemLanguage(language);
    getDeviceInfo("LocalCountry", country);

    // for the model
    if(country == "JPN")
        defaultFont = "LG Display_JP";
    else if(country == "HKG")
        defaultFont = "LG Display GP4_HK";
    // for the locale(language)
    else if(language == "ur-IN")
        defaultFont = "LG Display_Urdu";

    LOG_DEBUG("[%s] country : [%s], language : [%s], default font : [%s]", qPrintable(appId()), qPrintable(country), qPrintable(language), qPrintable(defaultFont));
    return defaultFont;
}

void WebPageBase::updateIsLoadErrorPageFinish()
{
    // ex)
    // Target error page URL : file:///usr/share/localization/webappmanager2/resources/ko/html/loaderror.html?errorCode=65&webkitErrorCode=65
    // WAM error page : file:///usr/share/localization/webappmanager2/loaderror.html

    m_isLoadErrorPageFinish = false;

    if (!url().isLocalFile()) return;

    QString urlString = url().toString();
    QString urlFileName = url().fileName();
    QString errorPageFileName = QUrl(getWebAppManagerConfig()->getErrorPageUrl()).fileName();
    QString errorPageDirPath = getWebAppManagerConfig()->getErrorPageUrl().remove(errorPageFileName);
    if (urlString.startsWith(errorPageDirPath) && !urlFileName.compare(errorPageFileName)) {
        LOG_DEBUG("[%s] This is WAM ErrorPage; URL: %s ", qPrintable(appId()), qPrintable(urlString));
        m_isLoadErrorPageFinish = true;
    }
}

#define URL_SIZE_LIMIT 768
QString WebPageBase::truncateURL(const QString& url)
{
    if(url.size() < URL_SIZE_LIMIT)
        return url;
    QString res = QString(url);
    return res.replace(URL_SIZE_LIMIT / 2, url.size() - URL_SIZE_LIMIT, QStringLiteral(" ... "));
}


void WebPageBase::setCustomUserScript()
{
    // 1. check app folder has userScripts
    // 2. check userscript.js there is, appfolder/webOSUserScripts/*.js
    QString userScriptFilePath = QDir(QString::fromStdString(m_appDesc->folderPath())).filePath(getWebAppManagerConfig()->getUserScriptPath());
    if(!QFileInfo(userScriptFilePath).isReadable())
        return;

    LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", getWebProcessPID()), "User Scripts exists : %s", qPrintable(userScriptFilePath));
    addUserScriptUrl(QUrl::fromLocalFile(userScriptFilePath));
}

void WebPageBase::addObserver(WebPageObserver* observer)
{
    m_observers.addObserver(observer);
}

void WebPageBase::removeObserver(WebPageObserver* observer)
{
    m_observers.removeObserver(observer);
}

bool WebPageBase::isAccessibilityEnabled() const
{
     return WebAppManager::instance()->isAccessibilityEnabled();
}
