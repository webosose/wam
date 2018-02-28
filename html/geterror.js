webappResBundle = new ilib.ResBundle({
    loadParams: {
        root: "/usr/share/localization/wam/resources"
    }
});

webappLocaleInfo = new ilib.LocaleInfo();

var errorCode = getUrlParams("errorCode");
var hostname = getUrlParams("hostname");

var networkStatus = false;

/* Key code list for key event */
var leftkey = 37;
var rightkey = 39;
var enterkey = 13;
/* Array for buttons on page */
var g_buttons = new Array;
var g_numButtons = 0; // Number of buttons
var lastfocus = -1; // Button focus position
var cursor_x = 0; // X-position of cursor

/* Handle webOSRelaunch event */
function relaunchHandler(e) {
    function onclosecallback() {
        var param = {};
        var q_param = {};
        var serviceCall_url = "luna://com.webos.applicationManager/launch";
        param["id"] = PalmSystem.identifier;
        q_param["query"] = e.detail.query;
        param["params"] = q_param;
        PalmSystem.serviceCall(serviceCall_url, param);
    }
    PalmSystem.onclose = onclosecallback;
    window.close();
}
document.addEventListener('webOSRelaunch', relaunchHandler);

window.onmousemove = function (e) {
    cursor_x = e.clientX;
    if (cursor_x < 0)
        cursor_x = 0;
}

function cursorStateHandler(e) {
    var visibility = e.detail.visibility;
    if (!visibility && lastfocus === -1) {
        // Find nearest button from last cursor position
        var near_btn = 0;
        var near_btn_pos = 0.0;
        var button_pos;
        var rect;
        for (var b = 0; b < g_numButtons; b++) {
            rect = g_buttons[b][0].getBoundingClientRect();
            button_pos = (rect.right + rect.left) * 0.5;
            if (b === 0) {
                near_btn_pos = button_pos;
                continue;
            }
            if (Math.abs(button_pos - cursor_x) < Math.abs(near_btn_pos - cursor_x)) {
                near_btn = b;
                near_btn_pos = button_pos;
            }
        }
        // Set focus on default button
        setOnFocus(near_btn);
    }
}
document.addEventListener('cursorStateChange', cursorStateHandler);

function getUrlParams(target) {
    var parameters = location.href.slice(location.href.indexOf("?") + 1, location.href.length);
    var paramArray = parameters.split("&");

    for (var i = 0; i < paramArray.length; i++) {
        var targetArray = paramArray[i].split("=");
        if (targetArray[0] === target) {
            if (targetArray.length)
                return targetArray[1];
        }
    }
    return "";
}

window.onkeyup = function checkEnter(e) {
    var key = e.which || e.keyCode || 0;
    if (key === enterkey) {
        if (lastfocus == -1) { // Set focus on default button if there's no focus
            var btn_no = enyo.Control.prototype.rtl ? g_numButtons - 1 : 0;
            setOnFocus(btn_no);
        }
        else
            g_buttons[lastfocus][1](); // Execute function of entered button
    }
}

window.onkeydown = function checkMoveFocus(e) {
    var key = e.which || e.keyCode || 0;

    // Detect left or right keys only
    if (key != leftkey && key != rightkey) return;

    // If there is no focused button, focus on first button
    if (lastfocus == -1) {
        setOnFocus(0);
        return;
    }

    // Ignore input key if next button is not exist
    if (((key == leftkey) && (lastfocus - 1 < 0)) ||
        (key == rightkey) && (lastfocus + 1 > g_numButtons - 1))
        return;

    // Move focus to next buttn
    if (key === leftkey)
        lastfocus -= 1;
    else
        lastfocus += 1;

    setOnFocus(lastfocus);
}

function onMouseOverBtn() {
    for (var i = 0; i < g_numButtons; i++) {
        if (this.id === g_buttons[i][0].id)
            setOnFocus(i); // Focus on this button
        else if (g_buttons[i][0].classList.contains("spotlight"))
            setOutFocus(i); // Focus out from other buttons with spotlight
    }
}

function onMouseOutBtn() {
    this.classList.remove("spotlight");
    this.blur();
    lastfocus = -1;
}

var onExitApp = function () {
    window.close();
}

var onLaunchSetting = function (target) {
    var response;
    var palmObject = new PalmServiceBridge();
    palmObject.onservicecallback = function (msg) {
        response = JSON.parse(msg);
        console.log("response.returnValue : " + response.returnValue);
        if (response.returnValue === false)
            console.log("response.errorText : " + response.errorText);
    };
    var url = "palm://com.webos.applicationManager/launch";
    var params = "{\"id\": \"com.palm.app.settings\", \"params\" : {\"target\":\"" + target + "\"}}";
    palmObject.call(url, params);
}

var onLaunchNetworkSetting = function () {
    onLaunchSetting("network");
}

var onLaunchGeneralSetting = function () {
    onLaunchSetting("general");
}

function setOnFocus(no) {
    // Ignore invalid button index and already focused button
    if (no < 0 || no > g_numButtons - 1) return;
    if (g_buttons[no][0].classList.contains("spotlight"))
        return;

    // Set spotlight and focus on the button
    g_buttons[no][0].classList.add("spotlight");
    g_buttons[no][0].focus();
    lastfocus = no;

    // Make only one button be with spotlight
    for (var i = 0; i < g_numButtons; i++) {
        if ((g_buttons[i][0].classList.contains("spotlight")) && (i != no)) {
            g_buttons[i][0].classList.remove("spotlight");
            g_buttons[i][0].blur();
        }
    }
}

function setOutFocus(no) {
    // Ignore invalid button index and already not focused button
    if (no < 0 || no > g_numButtons - 1) return;
    if (!g_buttons[no][0].classList.contains("spotlight"))
        return;

    // Remove spotlight and focus from the button
    g_buttons[no][0].classList.remove("spotlight");
    g_buttons[no][0].blur();
    lastfocus = -1;
}

function setNetworkStatus() {
    var retry = 0;
    var isStatusUpdated = false;
    var interval = setInterval(
        function () {
            retry += 1;
            if (retry > 5) {
                clearInterval(interval);
                console.log("Retry Count: " + retry + " [X]");
                return;
            }
            var url = "palm://com.palm.connectionmanager/getStatus";
            var param = "{\"subscribe\":true}";
            var response;
            var palmObject = new PalmServiceBridge();

            palmObject.onservicecallback = function (msg) {
                response = JSON.parse(msg);
                networkStatus = response.isInternetConnectionAvailable;
                isStatusUpdated = true;
            };

            palmObject.call(url, param);
            if (isStatusUpdated === true) {
                clearInterval(interval);
                return;
            }
        }, 1000
    );
}

function createButtonOnPage(str_name, str_id, b_dir, func) {
    var btn = document.createElement("BUTTON");
    var btnString = document.createTextNode(webappResBundle.getString(str_name));
    var need_small_btn = window.matchMedia("(max-width: 1280px)").matches;

    // Set button properties
    btn.appendChild(btnString);
    btn.id = str_id;
    btn.className = "enyo-tool-decorator moon-large-button-text moon-button moon-composite min-width";
    if (need_small_btn) {
        // Need small button for low resolution
        btn.classList.add("small");
        btn.classList.add("moon-small-button-text");
    }
    btn.tabIndex = 0
    btn.type = "button";
    btn.addEventListener("click", func);
    btn.addEventListener("mouseover", onMouseOverBtn);
    btn.addEventListener("mouseout", onMouseOutBtn);
    btn.setAttribute("role", "button");

    // Set button position and append to parent element
    var btnContainer = (b_dir === "Button_Left") ? "Button_Left" : "Button_Right";
    document.getElementById(btnContainer).appendChild(btn);

    // Add button on list
    g_buttons[g_numButtons] = new Array;
    g_buttons[g_numButtons][0] = document.getElementById(str_id);
    g_buttons[g_numButtons][1] = func;
    g_numButtons += 1;
}

var onRetryApp = function () {
    function onclosecallback() {
        var bridge = new PalmServiceBridge();
        bridge.onservicecallback = function (msg) {
            var response = JSON.parse(msg);
            console.log("response.returnValue : " + response.returnValue);
            if (response.returnValue === false)
                console.log("response.errorText : " + response.errorText);
        };

        var url = "palm://com.webos.applicationManager/launch";
        var params = "{\"id\":\"" + PalmSystem.identifier + "\"}";
        bridge.call(url, params);
    }

    PalmSystem.onclose = onclosecallback;
    window.close();
}

function getCurrentTime() {
    var now = new Date();
    var currentTime = "";

    function setDateFormat(d) {
        // Set date format as double digits
        var result = "";
        if (d < 10)
            result = "0" + d;
        else
            result = d;
        return result;
    }

    currentTime = now.getFullYear() + "-" + setDateFormat(now.getMonth() + 1) + "-" + setDateFormat(now.getDate());
    currentTime = currentTime + "\u0009" + now.toTimeString();

    return currentTime + "\u200e";
}

function setButtonsOnPage(layoutCase, isRTL) {
    var dir_head = isRTL ? "Button_Right" : "Button_Left";
    var dir_tail = isRTL ? "Button_Left" : "Button_Right";

    switch (layoutCase) {
    case 0: // Exit
        createButtonOnPage(webappResBundle.getString("EXIT APP"), "ExitApp_Button", dir_head, onExitApp);
        break;
    case 1: // Exit, Retry
        if (isRTL) {
            createButtonOnPage(webappResBundle.getString("RETRY"), "Retry_Button", dir_tail, onRetryApp);
            createButtonOnPage(webappResBundle.getString("EXIT APP"), "ExitApp_Button", dir_head, onExitApp);
        }
        else {
            createButtonOnPage(webappResBundle.getString("EXIT APP"), "ExitApp_Button", dir_head, onExitApp);
            createButtonOnPage(webappResBundle.getString("RETRY"), "Retry_Button", dir_tail, onRetryApp);
        }
        break;
    case 2: // Exit, Network setting, Retry buttons
        if (isRTL) {
            createButtonOnPage(webappResBundle.getString("RETRY"), "Retry_Button", dir_tail, onRetryApp);
            createButtonOnPage(webappResBundle.getString("NETWORK SETTINGS"), "NetworkSetting_Button", dir_tail, onLaunchNetworkSetting);
            createButtonOnPage(webappResBundle.getString("EXIT APP"), "ExitApp_Button", dir_head, onExitApp);
        }
        else {
            createButtonOnPage(webappResBundle.getString("EXIT APP"), "ExitApp_Button", dir_head, onExitApp);
            createButtonOnPage(webappResBundle.getString("NETWORK SETTINGS"), "NetworkSetting_Button", dir_tail, onLaunchNetworkSetting);
            createButtonOnPage(webappResBundle.getString("RETRY"), "Retry_Button", dir_tail, onRetryApp);
        }
        break;
    case 3: // Exit, Settings, Retry buttons
        if (isRTL) {
            createButtonOnPage(webappResBundle.getString("RETRY"), "Retry_Button", dir_tail, onRetryApp);
            createButtonOnPage(webappResBundle.getString("SETTINGS"), "Setting_Button", dir_tail, onLaunchGeneralSetting);
            createButtonOnPage(webappResBundle.getString("EXIT APP"), "ExitApp_Button", dir_head, onExitApp);
        }
        else {
            createButtonOnPage(webappResBundle.getString("EXIT APP"), "ExitApp_Button", dir_head, onExitApp);
            createButtonOnPage(webappResBundle.getString("SETTINGS"), "Setting_Button", dir_tail, onLaunchGeneralSetting);
            createButtonOnPage(webappResBundle.getString("RETRY"), "Retry_Button", dir_tail, onRetryApp);
        }
        break;

    default: // Exit
        createButtonOnPage(webappResBundle.getString("EXIT APP"), "ExitApp_Button", dir_head, onExitApp);
        break;
    }
}

function initFocus() {
    // To prevent timing issue that button is ignored by alert elemets,
    // set alert role on header before set focus on button
    var header = document.getElementById("error_header");
    header.setAttribute("role", "alert");

    // Set 'Exit App' button by default.
    if (enyo.Control.prototype.rtl) {
        g_buttons[g_numButtons - 1][0].blur();
        setOnFocus(g_numButtons - 1);
    }
    else {
        g_buttons[0][0].blur();
        setOnFocus(0);
    }
}

function getErrorTitle(error) {
    if (error > -300 && error < -199)
        return webappResBundle.getString("UNABLE TO LOAD") + "\u200e(\u200e" + error + "\u200e)\u200e";
    else
        return webappResBundle.getString("CHECK NETWORK CONNECTION") + "\u200e(\u200e" + error + "\u200e)\u200e";
}

var errorDetails = {
    '0': webappResBundle.getString("File or directory cannot be found."),
    '1': webappResBundle.getString("The operation has timed out."),
    '2': webappResBundle.getString("The network is unstable."),
    '3': webappResBundle.getString("Connection to the server is unstable."),
    '4': webappResBundle.getString("Network is not connected."),
    '5': webappResBundle.getString("The requested address cannot be found."),
    '6': webappResBundle.getString("Connection attempt has timed out."),
    '7': webappResBundle.getString("Hostname does not match server certificate."),
    '8': webappResBundle.getString("Server certificate has expired or is not yet valid as the set time on device is different from the current time."),
    '10': webappResBundle.getString("Server certificate cannot be trusted."),
    '11': webappResBundle.getString("This may be a temporary issue.")
};

var errorGuides = {
    '0': webappResBundle.getString("Please contact content provider."),
    '1': webappResBundle.getString("Please check the network status and try again."),
    '2': webappResBundle.getString("Please check the network connection status."),
    '3': webappResBundle.getString("Please check time settings."),
    '4': webappResBundle.getString("Please try again later.")
};

function isFileNotFound(isDetail) {
    if (isDetail)
        return errorDetails[0];
    else
        return errorGuides[0];
}

function isOperationTimedOut(isDetail) {
    if (isDetail)
        return errorDetails[1];
    else
        return errorGuides[1];
}

function isNetworkUnstable(isDetail) {
    if (isDetail)
        return errorDetails[2];
    else
        return errorGuides[1];
}

function isServerConnectionUnstable(isDetail) {
    if (isDetail)
        return errorDetails[3];
    else
        return errorGuides[1];
}

function isNetworkNotConnected(isDetail) {
    if (isDetail)
        return errorDetails[4];
    else
        return errorGuides[2];
}

function isAddressNotFound(isDetail) {
    if (isDetail)
        return errorDetails[5];
    else
        return errorGuides[0];
}

function isDNSNotFound(isDetail) {
    if (isDetail)
        return errorDetails[5];
    else
        return errorGuides[2];
}

function isConnectionAttemptTimedOut(isDetail) {
    if (isDetail)
        return errorDetails[6];
    else
        return errorGuides[1];
}

function isCertNameInvalid(isDetail) {
    if (isDetail)
        return errorDetails[7];
    else
        return errorGuides[0];
}

function isCertDateInvalid(isDetail) {
    if (isDetail)
        return errorDetails[8];
    else
        return errorGuides[3];
}

function isCertNotTrusted(isDetail) {
    if (isDetail)
        return errorDetails[10];
    else
        return errorGuides[0];
}

function isDefault(isDetail) {
    if (isDetail)
        return errorDetails[11];
    else
        return errorGuides[4];
}

var getErrorMsg = {
    '-6': isFileNotFound,
    '-7': isOperationTimedOut,
    '-15': isNetworkUnstable,
    '-21': isNetworkUnstable,
    '-101': isNetworkUnstable,
    '-102': isServerConnectionUnstable,
    '-105': isDNSNotFound,
    '-106': isNetworkNotConnected,
    '-109': isAddressNotFound,
    '-118': isConnectionAttemptTimedOut,
    '-137': isDNSNotFound,
    '-200': isCertNameInvalid,
    '-201': isCertDateInvalid,
    '-202': isCertNotTrusted,
    '-324': isNetworkUnstable,
    '-501': isCertNotTrusted,
    'default': isDefault
};

function getErrorDetails(error) {
    var isDetail = true;
    // invoke it and return error details
    return (getErrorMsg[error] || getErrorMsg['default'])(isDetail);
}

function getErrorGuide(error) {
    var isDetail = false;
    // invoke it and return error guide
    return (getErrorMsg[error] || getErrorMsg['default'])(isDetail);
}

function getErrorInfo(error) {
    switch (error) {
    case -105:
    case -137:
        return hostname;
    case -201:
        return webappResBundle.getString("Current Time Setting") + " : " + getCurrentTime();
    default:
        return "";
    }
}

function getLayoutCase(error) {
    switch (error) {
    case -109: // ADDRESS_UNREACHABLE
        return 0; // : Exit App button
    case -105: // NAME_NOT_RESOLVED
    case -106: // INTERNET_DISCONNECTED
    case -137: // NAME_RESOLUTION_FAILED
        return 2; // : Exit App, Retry, Network settings
    case -201: // CERT_DATE_INVALID
        return 3; // EXIT APP, RETRY, SETTINGS
    default:
        return 1; // : Exit App, Retry
    }
}

function onload() {
    // Check internet connection and set network status value
    setNetworkStatus();
    var error = parseInt(errorCode);

    // Set error messages to display in error page
    var str_errorTitle, str_errorDetails, str_errorGuide, str_errorInfo;

    function setErrorMessages(e) {
        str_errorTitle = getErrorTitle(e);
        str_errorDetails = getErrorDetails(e);
        str_errorGuide = getErrorGuide(e);
        str_errorInfo = getErrorInfo(e);
        return;
    };

    // Display error page with proper elements and layout
    function showErrorPage() {
        var isRTL = enyo.Control.prototype.rtl;
        var layoutCase = getLayoutCase(error);

        if (isRTL) {
            document.getElementById("error_title").style.direction = "rtl";
            document.getElementById("error_details").style.direction = "rtl";
            document.getElementById("error_guide").style.direction = "rtl";
            document.getElementById("error_info").style.direction = "rtl";
            document.getElementById("error_info").style.cssFloat = "right";
        }
        document.getElementById("error_title").innerHTML = str_errorTitle;
        document.getElementById("error_details").innerHTML = str_errorDetails;
        document.getElementById("error_guide").innerHTML = str_errorGuide;
        document.getElementById("error_info").innerHTML = str_errorInfo;

        // Set buttons in need with proper position
        setButtonsOnPage(layoutCase, isRTL);

        // Set focus on first buttons
        // Checking Audio Guidance is on/off
        if (!document.accessibilityEnabled)
            initFocus();
        else {
            // checking webengine is ready to handle focus event
            if (document.accessibilityReady)
                initFocus();
            else
                document.addEventListener('webOSAccessibilityReady', initFocus, false);
        }
    };

    // Initialize error page
    if (error !== -201) {
        setErrorMessages(error);
        showErrorPage();
    }
}
