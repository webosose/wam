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

#include "WebAppWayland.h"

#include <unordered_map>
#include <sstream>
#include <json/json.h>

#include "ApplicationDescription.h"
#include "LogManager.h"
#include "WebAppWaylandWindow.h"
#include "WebAppManagerUtils.h"
#include "WebPageBase.h"
#include "WindowTypes.h"

#include "webos/common/webos_constants.h"
#include "webos/window_group_configuration.h"

static int kLaunchFinishAssureTimeoutMs = 5000;

#define URL_SIZE_LIMIT 512
static std::string truncateURL(const std::string& url)
{
    if(url.size() < URL_SIZE_LIMIT)
        return url;
    std::stringstream res;
    res << url.substr(0, URL_SIZE_LIMIT/2) << " ... ";
    return res.str();
}

WebAppWayland::WebAppWayland(const std::string& type, int surface_id, int width, int height)
    : WebAppBase()
    , m_appWindow(0)
    , m_windowType(type)
    , m_lastSwappedTime(0)
    , m_windowHandle(0)
    , m_enableInputRegion(false)
    , m_isFocused(false)
    , m_vkbHeight(0)
    , m_lostFocusBySetWindowProperty(false)
{
    init(width, height, surface_id);
}

WebAppWayland::WebAppWayland(const std::string& type, WebAppWaylandWindow* window, int width, int height)
    : WebAppBase()
    , m_appWindow(window)
    , m_windowType(type)
    , m_lastSwappedTime(0)
    , m_enableInputRegion(false)
    , m_isFocused(false)
    , m_vkbHeight(0)
    , m_lostFocusBySetWindowProperty(false)
{
    init(width, height, 0);
}

WebAppWayland::~WebAppWayland()
{
    delete m_appWindow;
}

void WebAppWayland::init(int width, int height, int surface_id)
{
    if (!m_appWindow)
        m_appWindow = WebAppWaylandWindow::take(surface_id);
    m_appWindow->SetWindowSurfaceId(surface_id);
    if (!(width && height)) {
        setUiSize(m_appWindow->DisplayWidth(), m_appWindow->DisplayHeight());
        m_appWindow->InitWindow(m_appWindow->DisplayWidth(), m_appWindow->DisplayHeight());
    }
    else {
        setUiSize(width, height);
        m_appWindow->InitWindow(width, height);
    }

    m_appWindow->setWebApp(this);

#if defined(OS_WEBOS)
    m_windowHandle = m_appWindow->GetWindowHandle();
#endif

    // set compositor window type
    setWindowProperty("_WEBOS_WINDOW_TYPE", m_windowType);
    LOG_DEBUG("App created window [%s]", m_windowType.c_str());

    try {
        std::string launchTimeoutStr = WebAppManagerUtils::getEnv("LAUNCH_FINISH_ASSURE_TIMEOUT");
        kLaunchFinishAssureTimeoutMs = std::stoi(launchTimeoutStr);
    } catch(...) {}

    if (!webos::WebOSPlatform::GetInstance()->GetInputPointer()) {
        // Create InputManager instance.
        InputManager::instance();
    }
}

void WebAppWayland::startLaunchTimer()
{
    if(!getHiddenWindow()) {
        LOG_DEBUG("APP_LAUNCHTIME_CHECK_STARTED [appId:%s]", appId().c_str());
        m_elapsedLaunchTimer.start();
    }
}

void WebAppWayland::onDelegateWindowFrameSwapped()
{
    if(m_elapsedLaunchTimer.isRunning()) {
        m_lastSwappedTime = m_elapsedLaunchTimer.elapsed_ms();

        m_launchTimeoutTimer.stop();
        m_launchTimeoutTimer.start(kLaunchFinishAssureTimeoutMs,
                                   this,
                                   &WebAppWayland::onLaunchTimeout);
    }
}

void WebAppWayland::onLaunchTimeout()
{
    if(m_elapsedLaunchTimer.isRunning()) {
        m_launchTimeoutTimer.stop();
        m_elapsedLaunchTimer.stop();
        LOG_DEBUG("APP_LAUNCHTIME_CHECK_ALL_FRAMES_DONE [appId:%s time:%d]", appId().c_str(), m_lastSwappedTime);
    }
}

void WebAppWayland::forwardWebOSEvent(WebOSEvent* event) const
{
    page()->forwardEvent(event);
}

void WebAppWayland::attach(WebPageBase *page)
{
    WebAppBase::attach(page);

    setWindowProperty("appId", appId());
    setWindowProperty("launchingAppId", launchingAppId());
    setWindowProperty("title", getAppDescription()->title());
    setWindowProperty("icon", getAppDescription()->icon());
    setWindowProperty("subtitle", "");
    setWindowProperty("_WEBOS_WINDOW_CLASS", std::to_string((int)getAppDescription()->windowClassValue()));
    setWindowProperty("_WEBOS_ACCESS_POLICY_KEYS_BACK", getAppDescription()->backHistoryAPIDisabled() ? "true" : "false");
    setWindowProperty("_WEBOS_ACCESS_POLICY_KEYS_EXIT", getAppDescription()->handleExitKey() ? "true" : "false");

    setKeyMask(webos::WebOSKeyMask::KEY_MASK_BACK, getAppDescription()->backHistoryAPIDisabled());
    setKeyMask(webos::WebOSKeyMask::KEY_MASK_EXIT, getAppDescription()->handleExitKey());

   doAttach();
}

void WebAppWayland::suspendAppRendering()
{
    m_appWindow->hide();
}

void WebAppWayland::resumeAppRendering()
{
    m_appWindow->show();
}

bool WebAppWayland::isFocused() const
{
    return m_isFocused;
}

void WebAppWayland::resize(int width, int height)
{
    m_appWindow->Resize(width, height);
}

bool WebAppWayland::isActivated() const
{
    return m_appWindow->GetWindowHostState() == webos::NATIVE_WINDOW_FULLSCREEN
        || m_appWindow->GetWindowHostState() == webos::NATIVE_WINDOW_MAXIMIZED
        || m_appWindow->GetWindowHostState() == webos::NATIVE_WINDOW_DEFAULT;
}

bool WebAppWayland::isMinimized()
{
    return m_appWindow->GetWindowHostState() == webos::NATIVE_WINDOW_MINIMIZED;
}

bool WebAppWayland::isNormal()
{
    return m_appWindow->GetWindowHostState() == webos::NATIVE_WINDOW_DEFAULT;
}

void WebAppWayland::onStageActivated()
{
    if (getCrashState()) {
        LOG_INFO(MSGID_WEBAPP_STAGE_ACITVATED, 3, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), PMLOGKS("getCrashState()", "true; Reload default Page"), "");
        page()->reloadDefaultPage();
        setCrashState(false);
    }

    page()->resumeWebPageAll();

    page()->setVisibilityState(WebPageBase::WebPageVisibilityState::WebPageVisibilityStateVisible);

    setActiveAppId(page()->getIdentifier());
    focus();

    if (getHiddenWindow() || keepAlive())
        m_appWindow->Show();

    LOG_INFO(MSGID_WEBAPP_STAGE_ACITVATED, 2, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "");
}

void WebAppWayland::onStageDeactivated()
{
    page()->suspendWebPageMedia();
    unfocus();
    page()->setVisibilityState(WebPageBase::WebPageVisibilityState::WebPageVisibilityStateHidden);
    page()->suspendWebPageAll();

    LOG_INFO(MSGID_WEBAPP_STAGE_DEACITVATED, 2, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "");
}

void WebAppWayland::configureWindow(const std::string& type)
{
    m_windowType = type;
    m_appWindow->setWebApp(this);

    setWindowProperty("_WEBOS_WINDOW_TYPE", type);
    setWindowProperty("launchingAppId", launchingAppId());
    setWindowProperty("appId", appId());
    setWindowProperty("title", getAppDescription()->title());
    setWindowProperty("icon", getAppDescription()->icon());
    setWindowProperty("subtitle", "");
    setWindowProperty("_WEBOS_WINDOW_CLASS", std::to_string((int)getAppDescription()->windowClassValue()));
    setWindowProperty("_WEBOS_ACCESS_POLICY_KEYS_BACK", getAppDescription()->backHistoryAPIDisabled() ? "true" : "false");
    setWindowProperty("_WEBOS_ACCESS_POLICY_KEYS_EXIT", getAppDescription()->handleExitKey() ? "true" : "false");

    setKeyMask(webos::WebOSKeyMask::KEY_MASK_BACK, getAppDescription()->backHistoryAPIDisabled());
    setKeyMask(webos::WebOSKeyMask::KEY_MASK_EXIT, getAppDescription()->handleExitKey());

    ApplicationDescription* appDesc = getAppDescription();
    if (!appDesc->groupWindowDesc().empty())
        setupWindowGroup(appDesc);

}

void WebAppWayland::setupWindowGroup(ApplicationDescription* desc)
{
    if (!desc)
        return;

    ApplicationDescription::WindowGroupInfo groupInfo = desc->getWindowGroupInfo();
    if (groupInfo.name.empty())
        return;

    if (groupInfo.isOwner) {
        ApplicationDescription::WindowOwnerInfo ownerInfo = desc->getWindowOwnerInfo();
        webos::WindowGroupConfiguration config(groupInfo.name);
        config.SetIsAnonymous(ownerInfo.allowAnonymous);
        for (const auto &l : ownerInfo.layers) {
          config.AddLayer(webos::WindowGroupLayerConfiguration(l.first, l.second));
        }
        m_appWindow->CreateWindowGroup(config);
        LOG_INFO(MSGID_CREATE_SURFACEGROUP, 2, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "");
    } else {
        ApplicationDescription::WindowClientInfo clientInfo = desc->getWindowClientInfo();
        m_appWindow->AttachToWindowGroup(groupInfo.name, clientInfo.layer);
        LOG_INFO(MSGID_ATTACH_SURFACEGROUP, 3, PMLOGKS("APP_ID", appId().c_str()), PMLOGKS("OWNER_ID", groupInfo.name.c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "");
    }
}

void WebAppWayland::setKeyMask(webos::WebOSKeyMask keyMask, bool value)
{
    m_appWindow->SetKeyMask(keyMask, value);
}

void WebAppWayland::applyInputRegion()
{
#if defined(OS_WEBOS)
    if (!m_enableInputRegion && !m_inputRegion.empty()) {
        m_enableInputRegion = true;
        webos::WebOSPlatform::GetInstance()->SetInputRegion(m_windowHandle, m_inputRegion);
    }
#endif
}

void WebAppWayland::setInputRegion(const Json::Value& jsonDoc)
{
    m_inputRegion.clear();

    if (jsonDoc.isArray()) {
        for (const Json::Value& map : jsonDoc) {
            m_inputRegion.push_back(gfx::Rect(
                                map["x"].asInt() * m_scaleFactor,
                                map["y"].asInt() * m_scaleFactor,
                                map["width"].asInt() * m_scaleFactor,
                                map["height"].asInt() * m_scaleFactor));
        }
    }
#if defined(OS_WEBOS)
    webos::WebOSPlatform::GetInstance()->SetInputRegion(m_windowHandle, m_inputRegion);
#endif
}


void WebAppWayland::setWindowProperty(const std::string& name, const std::string& value)
{
    webos::WebOSKeyMask mask = static_cast<webos::WebOSKeyMask>(0);
    if (name == "_WEBOS_ACCESS_POLICY_KEYS_BACK")
        mask = webos::WebOSKeyMask::KEY_MASK_BACK;
    else if (name == "_WEBOS_ACCESS_POLICY_KEYS_EXIT")
        mask = webos::WebOSKeyMask::KEY_MASK_EXIT;
    // if mask is not set, not need to call setKeyMask
    if (mask != static_cast<webos::WebOSKeyMask>(0)) {
        bool boolValue; // TODO: Maybe migrate to boost::lexical_cast<bool>()
        std::istringstream(value) >> std::boolalpha >> boolValue;
        setKeyMask(mask, boolValue);
    }
    m_appWindow->SetWindowProperty(name, value);
}

void WebAppWayland::platformBack()
{
    m_appWindow->platformBack();
}

void WebAppWayland::setCursor(const std::string& cursorArg, int hotspot_x, int hotspot_y)
{
    m_appWindow->setCursor(cursorArg, hotspot_x, hotspot_y);
}

static std::unordered_map<std::string, webos::WebOSKeyMask>& getKeyMaskTable()
{
    static std::unordered_map<std::string, webos::WebOSKeyMask> mapTable {
        { "KeyMaskNone", static_cast<webos::WebOSKeyMask>(0) },
        { "KeyMaskHome", webos::WebOSKeyMask::KEY_MASK_HOME },
        { "KeyMaskBack", webos::WebOSKeyMask::KEY_MASK_BACK },
        { "KeyMaskExit", webos::WebOSKeyMask::KEY_MASK_EXIT },
        { "KeyMaskLeft", webos::WebOSKeyMask::KEY_MASK_LEFT },
        { "KeyMaskRight", webos::WebOSKeyMask::KEY_MASK_RIGHT },
        { "KeyMaskUp", webos::WebOSKeyMask::KEY_MASK_UP },
        { "KeyMaskDown", webos::WebOSKeyMask::KEY_MASK_DOWN },
        { "KeyMaskOk", webos::WebOSKeyMask::KEY_MASK_OK },
        { "KeyMaskNumeric", webos::WebOSKeyMask::KEY_MASK_NUMERIC },
        { "KeyMaskRed", webos::WebOSKeyMask::KEY_MASK_REMOTECOLORRED },
        { "KeyMaskGreen", webos::WebOSKeyMask::KEY_MASK_REMOTECOLORGREEN },
        { "KeyMaskYellow", webos::WebOSKeyMask::KEY_MASK_REMOTECOLORYELLOW },
        { "KeyMaskBlue", webos::WebOSKeyMask::KEY_MASK_REMOTECOLORBLUE },
        { "KeyMaskProgramme", webos::WebOSKeyMask::KEY_MASK_REMOTEPROGRAMMEGROUP },
        { "KeyMaskPlayback", webos::WebOSKeyMask::KEY_MASK_REMOTEPLAYBACKGROUP },
        { "KeyMaskTeletext", webos::WebOSKeyMask::KEY_MASK_REMOTETELETEXTGROUP },
        { "KeyMaskDefault", webos::WebOSKeyMask::KEY_MASK_DEFAULT }
    };
    return mapTable;
}

void WebAppWayland::setKeyMask(const Json::Value& jsonDoc)
{
    static std::unordered_map<std::string, webos::WebOSKeyMask>& mapTable = getKeyMaskTable();
    unsigned int keyMask = 0;

    if (jsonDoc.isArray()) {
        for (const Json::Value& child : jsonDoc)
            keyMask |= mapTable[child.asString()];
    }
#if defined(OS_WEBOS)
    webos::WebOSPlatform::GetInstance()->SetKeyMask(m_windowHandle, static_cast<webos::WebOSKeyMask>(keyMask));
#endif
}

void WebAppWayland::setKeyMask(webos::WebOSKeyMask keyMask)
{
#if defined(OS_WEBOS)
    webos::WebOSPlatform::GetInstance()->SetKeyMask(m_windowHandle, keyMask);
#endif
}

void WebAppWayland::focusOwner()
{
    m_appWindow->FocusWindowGroupOwner();
    LOG_DEBUG("FocusOwner [%s]", appId().c_str());
}

void WebAppWayland::focusLayer()
{
    m_appWindow->FocusWindowGroupLayer();
    ApplicationDescription * desc = getAppDescription();
    if (desc) {
        ApplicationDescription::WindowClientInfo clientInfo = desc->getWindowClientInfo();
        LOG_DEBUG("FocusLayer(layer:%s) [%s]",qPrintable(clientInfo.layer) ,appId().c_str());
    }
}

void WebAppWayland::setOpacity(float opacity)
{
    m_appWindow->SetOpacity(opacity);
}

void WebAppWayland::hide(bool forcedHide)
{
    if (keepAlive() || forcedHide) {
        m_appWindow->hide();
        setHiddenWindow(true);
    }
}

void WebAppWayland::focus()
{
    m_isFocused = true;
    if(!isMinimized())
        page()->setFocus(true);
}

void WebAppWayland::unfocus()
{
    m_isFocused = false;
    page()->setFocus(false);
}

void WebAppWayland::doAttach()
{
    // Do App and window things
    ApplicationDescription* appDesc = getAppDescription();
    if (!appDesc->groupWindowDesc().empty())
        setupWindowGroup(appDesc);

    m_appWindow->attachWebContents(page()->getWebContents());
    // The attachWebContents causes visibilityState change to Visible (by default, init)
    // And now, should update the visibilityState to launching
    page()->setVisibilityState(WebPageBase::WebPageVisibilityState::WebPageVisibilityStateLaunching);

    // Do Page things
    page()->setPageProperties();

    if (keepAlive())
        page()->setKeepAliveWebApp(keepAlive());

    setForceActivateVtgIfRequired();
}

void WebAppWayland::raise()
{
    bool wasMinimizedState = isMinimized();

    //There's no fullscreen event from LSM for below cases, so onStageActivated should be called
    //1. When overlay window is raised
    //2. When there's only one keepAlive app, and this keepAlive app is closed and is shown again
    if ((getWindowType() == WT_OVERLAY) || (keepAlive() && !wasMinimizedState)) {
        LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "WebAppWayland::raise(); call onStageActivated");
        onStageActivated();
    } else {
        LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "WebAppWayland::raise(); call setWindowState(webos::NATIVE_WINDOW_FULLSCREEN)");
        m_appWindow->SetWindowHostState(webos::NATIVE_WINDOW_FULLSCREEN);
    }

    if (wasMinimizedState)
        page()->setVisibilityState(WebPageBase::WebPageVisibilityState::WebPageVisibilityStateVisible);
}

void WebAppWayland::goBackground()
{
    if (getWindowType() == WT_OVERLAY) {
        LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "WebAppWayland::goBackground(); windowType:OVERLAY; Try close; call doClose()");
        doClose();
    } else {
        LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "WebAppWayland::goBackground(); call setWindowState(webos::NATIVE_WINDOW_MINIMIZED)");
        m_appWindow->SetWindowHostState(webos::NATIVE_WINDOW_MINIMIZED);
    }
}

void WebAppWayland::webPageLoadFinished()
{
    if (getHiddenWindow())
        return;
    if(needReload()) {
        page()->reload();
        setNeedReload(false);
        return;
    }

    std::string logUrl = truncateURL(page()->url().toString().toStdString()); // FIXME: WebPage: qstr2stdstr
    LOG_INFO_WITH_CLOCK(MSGID_APP_LOADED, 5,
             PMLOGKS("PerfType", "AppLaunch"),
             PMLOGKS("PerfGroup", page()->appId().c_str()),
             PMLOGKS("APP_ID", page()->appId().c_str()),
             PMLOGKS("URL", logUrl.c_str()),
             PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "");

    doPendingRelaunch();
}

void WebAppWayland::webPageLoadFailed(int errorCode)
{
    // Do not load error page while preoload app launching.
    if (preloadState() != NONE_PRELOAD)
        closeAppInternal();
}

void WebAppWayland::doClose()
{
    if (forceClose()) {
        LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "WebAppWayland::doClose(); forceClose() TRUE; call forceCloseAppInternal() and return");
        forceCloseAppInternal();
        return;
    }

    if (keepAlive() && !page()->isLoadErrorPageFinish()) {
        LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "WebAppWayland::doClose(); KeepAlive; just Hide this app");
        page()->closeVkb();
        m_appWindow->hide();
        deleteSurfaceGroup();
        setHiddenWindow(true);
        m_addedToWindowMgr = false;
        return;
    }

    LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "WebAppWayland::doClose(); call closeAppInternal()");
    closeAppInternal();
}

void WebAppWayland::stateAboutToChange(webos::NativeWindowState willBe)
{
    if (willBe == webos::NATIVE_WINDOW_MINIMIZED) {
        LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "WebAppWayland::stateAboutToChange; will be Minimized; suspend media and fire visibilitychange event");
        page()->suspendWebPageMedia();
        page()->setVisibilityState(WebPageBase::WebPageVisibilityState::WebPageVisibilityStateHidden);
    }
}

void WebAppWayland::showWindow()
{
    setForceActivateVtgIfRequired();

    if (m_preloadState != NONE_PRELOAD) {
        LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "WebAppWayland::showWindow(); But Preloaded app; return");
        return;
    }

    setHiddenWindow(false);

    m_appWindow->show();
    m_addedToWindowMgr = true;
    WebAppBase::showWindow();
}

void WebAppWayland::titleChanged()
{
    setWindowProperty("subtitle", page()->title());
}

void WebAppWayland::firstFrameVisuallyCommitted()
{
    LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "firstFrameVisuallyCommitted");
    // if m_preloadState != NONE_PRELOAD, then we must ignore the first frame commit
    // if getHiddenWindow() == true, then we have specifically requested that the window is to be hidden,
    // and therefore we have to do an explicit show
    if (!getHiddenWindow() && m_preloadState == NONE_PRELOAD) {
        LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", appId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "Not hidden window, preload, call showWindow");
        showWindow();
    }
}

void WebAppWayland::postEvent(WebOSEvent* ev)
{
    m_appWindow->event(ev);
}

void WebAppWayland::navigationHistoryChanged()
{
    if (!getAppDescription()->backHistoryAPIDisabled()) {
        // if backHistoryAPIDisabled is true, no chance to change this value
        setWindowProperty("_WEBOS_ACCESS_POLICY_KEYS_BACK",
                          page()->canGoBack() ?
                          "true" : /* send next back key to WAM */
                          "false"); /* Do not send back key to WAM. LSM should handle it */
    }
}

void WebAppWayland::webViewRecreated()
{
    m_appWindow->attachWebContents(page()->getWebContents());
    m_appWindow->RecreatedWebContents();
    page()->setPageProperties();
    focus();
}

void WebAppWayland::setForceActivateVtgIfRequired()
{
    std::string screenRotation;
    if (WebAppManager::instance() && page()) {
        if (WebAppManager::instance()->getDeviceInfo("ScreenRotation", screenRotation) && screenRotation != "off")
            page()->setForceActivateVtg(true);
        else
            page()->setForceActivateVtg(false);
    }
}

void InputManager::OnCursorVisibilityChanged(bool visible)
{
    if (IsVisible() == visible) return;

    LOG_DEBUG("InputManager::onCursorVisibilityChanged; Global Cursor visibility Changed to %s; send cursorStateChange event to all app, all frames", visible? "true" : " false");
    SetVisible(visible);
    // send event about  cursorStateChange
    std::stringstream jss;
    std::string v = visible ? "true" : "false";
    jss << "    var cursorEvent=new CustomEvent('cursorStateChange', { detail: { 'visibility' : " << v << " } });"
        << "    cursorEvent.visibility = " << v << ";"
        << "    if(document) document.dispatchEvent(cursorEvent);";

    // send javascript event : cursorStateChange with param to All app
    // if javascript has setTimeout() like webOSlaunch or webOSRelaunch, then app can not get this event when app is in background
    // because javascript is freezed and timer is too, since app is in background, timer is never fired
    WebAppBase::onCursorVisibilityChanged(jss.str());
}

void WebAppWayland::sendWebOSMouseEvent(const std::string& eventName)
{
    if (eventName == "Enter" || eventName == "Leave") {
        // send webOSMouse event to app
        std::stringstream javascript;
        javascript
            << "console.log('[WAM] fires webOSMouse event : " << eventName << "');"
            << "var mouseEvent =new CustomEvent('webOSMouse', { detail: { type : '" << eventName << "' }});"
            << "document.dispatchEvent(mouseEvent);";
        LOG_DEBUG("[%s] WebAppWayland::sendWebOSMouseEvent; dispatch webOSMouse; %s", appId().c_str(), eventName.c_str());
        page()->evaluateJavaScript(javascript.str());
    }
}

void WebAppWayland::deleteSurfaceGroup()
{
    m_appWindow->DetachWindowGroup();
}

void WebAppWayland::setKeepAlive(bool keepAlive)
{
    WebAppBase::setKeepAlive(keepAlive);
    if (page())
        page()->setKeepAliveWebApp(keepAlive);
}

void WebAppWayland::moveInputRegion(int height)
{
    if (!m_enableInputRegion)
        return;

    if (height)
        m_vkbHeight = height;
    else
        m_vkbHeight = -m_vkbHeight;

    std::vector<gfx::Rect> newRegion;
    for (std::vector<gfx::Rect>::iterator it = m_inputRegion.begin(); it != m_inputRegion.end(); ++it) {
        gfx::Rect rect = static_cast<gfx::Rect>(*it);
        rect.SetRect(rect.x(),
                     rect.y() - m_vkbHeight,
                     rect.width(),
                     rect.height());
        newRegion.push_back(rect);
    }
    m_inputRegion.clear();
    m_inputRegion = newRegion;

#if defined(OS_WEBOS)
    webos::WebOSPlatform::GetInstance()->SetInputRegion(m_windowHandle, m_inputRegion);
#endif
}

void WebAppWayland::keyboardVisibilityChanged(bool visible, int height) {
    WebAppBase::keyboardVisibilityChanged(visible, height);
    moveInputRegion(height);
}

