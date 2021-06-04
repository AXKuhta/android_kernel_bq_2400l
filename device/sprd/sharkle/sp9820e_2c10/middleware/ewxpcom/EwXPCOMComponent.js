/*
    ========================================
    EXPWAY eMBMS Middleware XPCOM component.
    (c) Copyright 2018 EXPWAY
    ========================================
*/

"use strict";



const {classes: Cc, interfaces: Ci, utils: Cu, results: Cr} = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");

/**********************************/
/* LteBroadcastCallback Component */
/**********************************/

const CALLBACK_CLASS_ID = Components.ID("{a6d742ec-b7a9-4b6b-a2c4-e6655ce6186b}");
const CALLBACK_CONTRACT_ID = "@expway.com/embms/callback;1";
const CALLBACK_CLASS_NAME = "LteBroadcastCallback";

function LteBroadcastCallback() {
    dump("EwXPCOM : [LteBroadcastCallback] Constructor");

    this.notifySuccess = function () {
        dump("EwXPCOM : [LteBroadcastCallback] success");
    };

    this.notifyError = function (error) {
        dump("EwXPCOM : [LteBroadcastCallback] error: " + error);
    };
}

LteBroadcastCallback.prototype = {
    classID: CALLBACK_CLASS_ID,
    QueryInterface: XPCOMUtils.generateQI([Ci.nsILteBroadcastCallback]),
    classInfo: XPCOMUtils.generateCI({
        classID: CALLBACK_CLASS_ID,
        contractID: CALLBACK_CONTRACT_ID,
        interfaces: [Ci.nsILteBroadcastCallback],
        classDescription: CALLBACK_CLASS_NAME,
    }),
};

/**********************************/
/* LteBroadcastListener Component */
/**********************************/

const LISTENER_CLASS_ID = Components.ID("{fb41157c-120a-415f-9aad-dee97f7cfc50}");
const LISTENER_CONTRACT_ID = "@expway.com/embms/listener;1";
const LISTENER_CLASS_NAME = "LteBroadcastListener";

function LteBroadcastListener() {
    dump("EwXPCOM : [LteBroadcastListener] Constructor");

    this.notifyError = function (error) {
        dump("EwXPCOM : [LteBroadcastListener] error: " + error + "");

    };
}

LteBroadcastListener.prototype = {
    classID: LISTENER_CLASS_ID,
    QueryInterface: XPCOMUtils.generateQI([Ci.nsILteBroadcastListener]),
    classInfo: XPCOMUtils.generateCI({
        classID: LISTENER_CLASS_ID,
        contractID: LISTENER_CONTRACT_ID,
        interfaces: [Ci.nsILteBroadcastListener],
        classDescription: LISTENER_CLASS_NAME,
    }),
};

/*****************************************/
/* LteBroadcastManagerListener Component */
/*****************************************/

const MANAGERLISTENER_CLASS_ID = Components.ID("{ccaea7d2-7e5b-4875-9077-279a4f2702ce}");
const MANAGERLISTENER_CONTRACT_ID = "@expway.com/embms/managerlistener;1";
const MANAGERLISTENER_CLASS_NAME = "LteBroadcastManagerListener";

function LteBroadcastManagerListener() {
    dump("EwXPCOM : [LteBroadcastManagerListener] Constructor");

    this.notifyCoverageChanged = function (coverage) {
        dump("EwXPCOM : [LteBroadcastManagerListener] coverage changed: " + coverage);
    };

    this.notifySAIChanged = function () {
        dump("EwXPCOM : [LteBroadcastManagerListener] sai changed");
    };
}

LteBroadcastManagerListener.prototype = {
    classID: MANAGERLISTENER_CLASS_ID,
    QueryInterface: XPCOMUtils.generateQI([Ci.nsILteBroadcastManagerListener]),
    classInfo: XPCOMUtils.generateCI({
        classID: MANAGERLISTENER_CLASS_ID,
        contractID: MANAGERLISTENER_CONTRACT_ID,
        interfaces: [Ci.nsILteBroadcastManagerListener],
        classDescription: MANAGERLISTENER_CLASS_NAME,
    }),
};

/*********************************/
/* LteBroadcastManager Component */
/*********************************/

const MANAGER_CLASS_ID = Components.ID("{a0f8a2d4-8db6-4393-b457-33d839417300}");
const MANAGER_CONTRACT_ID = "@kaiostech.com/ltebroadcast/manager;1";
const MANAGER_CLASS_NAME = "LteBroadcastManager";

const SERVER_URI = "http://127.0.0.1:8080";

var serviceTypeEnum = {
    SERVICE_TYPE_STREAMING: 0,
    SERVICE_TYPE_FILE_DOWNLOAD: 1
};

var coverageEnum = {
    COVERAGE_IN: 0,
    COVERAGE_OUT: 1
};

var ewmw = null;

function LteBroadcastManager() {
    dump("EwXPCOM : [LteBroadcastManager] Constructor");

    var _appId = null;
    var _classList = [];
    var _lteMgrStartCall = false;
    var _lteMgrSetClassCall = false;
    var _lteMgrRegisterCb = null;
    var _lteMgrStartCb = null;
    var _lteMgrStopCb = null;
    var _lteMgrSetClassCb = null;
    var _lteMgrListener = null;
    var _coverage = coverageEnum.COVERAGE_OUT;
    var _saiList = [];

    const ENUM_SERVICE_TYPE = {
        FILE_CASTING: 1,
        DASH_STREAMING: 2,
        HLS_STREAMING: 3,
        RTP_STREAMING: 4,
        UNKNOWN_SERVICE_TYPE: 5, /* last value*/
        properties: {
            1: {name: "File Casting", value: 1},
            2: {name: "DASH Streaming", value: 2},
            3: {name: "HLS Streaming", value: 3},
            4: {name: "RTP Streaming", value: 4},
            5: {name: "Unknown service Type", value: 5} /* last value*/
        }
    };

    var _lteStreamingService = new LteBroadcastStreamingService();
    _lteStreamingService.serviceType = serviceTypeEnum.SERVICE_TYPE_STREAMING;

    function _registrationCallback(event, user, device, error) {
        dump("EwXPCOM : [LteBroadcastManager] Registration callback");

        switch (event) {
            case "registration_started":
                dump("EwXPCOM : [LteBroadcastManager] Registration started");
                break;

            case 'registration_success':
                dump("EwXPCOM : [LteBroadcastManager] Registration succeeded");
                if (_lteMgrRegisterCb) {
                    dump("[LteBroadcastManager] Registration succeeded Notification sent");
                    _lteMgrRegisterCb.notifySuccess();
                }

                if (    _lteMgrListener
                    && (_coverage == coverageEnum.COVERAGE_IN)) {
                    dump("EwXPCOM : Notify coverage changed (coverage=" + _coverage + ")");
                    _lteMgrListener.notifyCoverageChanged(_coverage);
                }
                break;

            case 'registration_not_allowed':
                dump("EwXPCOM : [LteBroadcastManager] Registration not allowed");
                if (_lteMgrRegisterCb) {
                    dump("[LteBroadcastManager] Registration: not allowed Notification sent");
                    _lteMgrRegisterCb.notifyError("Registration: not allowed");
                }
                break;

            case 'registration_error':
                dump("EwXPCOM : [LteBroadcastManager] Registration failed");
                if (_lteMgrRegisterCb) {
                    dump("[LteBroadcastManager] Registration: error Notification sent");
                    _lteMgrRegisterCb.notifyError("Registration: error");
                }
                break;

            default:
                dump("EwXPCOM : [LteBroadcastManager] Registration: Unknown event(" + event + ")");
                break;
        }
    }

    // function _displayServiceType(type)
    // {
    //     var result;

    //     switch (type)
    //     {
    //         case 1: 
    //             result = "File Casting"; 
    //             break;
    //         case 2: 
    //             result = "DASH Streaming"; 
    //             break;
    //         case 3: 
    //             result = "HLS Streaming"; 
    //             break;
    //         case 4: 
    //             result = "RTP Streaming"; 
    //             break;

    //         default: 
    //             result = "Unknown service type"; 
    //             break;
    //     }

    //     return result;
    // }

    function _printService(index, service) {
        dump("EwXPCOM :     [Service " + index + "]");

        dump("EwXPCOM :            > ServiceId: " + service.id + "");

        if (service.names) {
            dump("EwXPCOM :            > Service names: ");

            var name = service.names;

            for (var key in name) {
                var sLangCode = key;
                var sName = name[key];
                dump("EwXPCOM :              [language: " + sLangCode + ", name: " + sName + "] ");
            }
        }

        // var serviceType = _displayServiceType(service.service_type);
        var serviceType = Object.freeze(ENUM_SERVICE_TYPE);
        if (service.service_type < ENUM_SERVICE_TYPE.UNKNOWN_SERVICE_TYPE) {
            serviceType = ENUM_SERVICE_TYPE.properties[service.service_type].name;
        } else {
            serviceType = ENUM_SERVICE_TYPE.properties[ENUM_SERVICE_TYPE.UNKNOWN_SERVICE_TYPE].name;
        }

        dump("EwXPCOM :            > Service type: " + serviceType + "");

        if (service.service_area) {
            dump("EwXPCOM :            > Service areas: ");

            for (var i = 0; i < service.service_area.length; i++) {
                var sai = service.service_area[i];
                dump("EwXPCOM :              [" + sai + "] ");
            }
        }

        if (service.schedules) {
            dump("EwXPCOM :            > Schedules: ");

            for (var m = 0; m < service.schedules.length; m++) {
                var schedule = service.schedules[m];

                if (schedule.type == "file") {
                    if (schedule.file_uri) {
                        dump("EwXPCOM :               [file uri: " + schedule.file_uri + ", ");

                        if (schedule.times) {
                            for (var j = 0; j < schedule.times.length; j++) {
                                var time = schedule.times[j];
                                var time_start = time[0];
                                var time_end = time[1];
                                dump("EwXPCOM :                times: [start: " + time_start + ", end: " + time_end + "]]");
                            }
                        } else {
                            dump("EwXPCOM :                times: [-]]");
                        }

                    }
                } else if (schedule.type == "session") {
                    if (schedule.times) {
                        for (var k = 0; k < schedule.times.length; k++) {
                            var time_tmp = schedule.times[k];
                            var time_start_tmp = time_tmp[0];
                            var time_end_tmp = time_tmp[1];

                            dump("EwXPCOM :               [times: [start: " + time_start_tmp + ", end: " + time_end_tmp + "]]");
                        }
                    } else {
                        dump("EwXPCOM :                times: [-]]");
                    }
                }
            }
        }

        if (service.mpd_uri)
            dump("EwXPCOM :            > Mpd uri: " + service.mpd_uri + "");

        if (service.service_class)
            dump("EwXPCOM :            > Service class: " + service.service_class + "");
    }

    function _updateServiceList(newServiceList) {
        dump("EwXPCOM : [LteBroadcastManager] UpdateServiceList function");

        _lteStreamingService.handleList = [];
        _lteStreamingService.handleList.length = 0;

        if (    newServiceList == null
            ||  newServiceList.length == 0) {
            dump("EwXPCOM : [LteBroadcastManager] No service available");
        } else {
            dump("EwXPCOM : [LteBroadcastManager] " + newServiceList.length + " new services");

            for (var i = 0; i < newServiceList.length; i++) {
                var service = newServiceList[i];

                _printService(i + 1, service);

                if (    (ENUM_SERVICE_TYPE.DASH_STREAMING == service.service_type)
                    ||  (ENUM_SERVICE_TYPE.HLS_STREAMING == service.service_type)
                    ||  (ENUM_SERVICE_TYPE.RTP_STREAMING == service.service_type)) {
                    var lteStreamingHandle = new LteBroadcastStreamingHandle();

                    lteStreamingHandle.handleId = service.id;
                    lteStreamingHandle.class = service.service_class;
                    lteStreamingHandle.mpdUri = service.mpd_uri;

                    if ( service.names ) {
                        var name = service.names;

                        for ( var lang in name ) {
                            var lteHandleName = new LteBroadcastHandleName();

                            lteHandleName.language = lang;
                            lteHandleName.name = name[ lang ];

                            lteStreamingHandle.nameList.push( lteHandleName );
                        }
                    }

                    if (    service.schedules
                        &&  service.schedules.length > 0) {
                        var schedule = service.schedules[0];

                        if (    schedule.times
                            &&  schedule.times.length > 0) {
                            var time = schedule.times[0];

                            lteStreamingHandle.sessionStartTime = (time[0] / 1000)>>0; // convert to s
                            lteStreamingHandle.sessionEndTime   = (time[1] / 1000)>>0; // convert to s
                        }
                    }

                    _lteStreamingService.handleList.push(lteStreamingHandle);
                }
            }

            dump("EwXPCOM : [LteBroadcastManager] _lteStreamingService.handleList contains " + _lteStreamingService.handleList.length + "services");

            if (_lteStreamingService.listener) {
                dump("EwXPCOM : [LteBroadcastManager] send event SERVICE_STATE_HANDLES_UPDATED");
                _lteStreamingService.listener.notifyStateChange(SERVICE_STATE_HANDLES_UPDATED);
            }
        }
    }

    function _metadataChangedCallback(status, result, error) {
        dump("EwXPCOM : [LteBroadcastManager] metadataChanged callback " + status + "");

        if (status == 0) {
            var serviceList = result;

            _updateServiceList(serviceList);
        } else {
            dump("EwXPCOM : [LteBroadcastManager] error (" + error + "): " + result);
        }
    }

    function _acquisitionCallback(event, progress) {
        dump("EwXPCOM : [LteBroadcastManager] acquisition callback");

        switch (event) {
            case 'acquisition_started':
                dump("EwXPCOM : Acquisition started");
                break;

            case 'acquisition_progress':
                dump("EwXPCOM : Acquistion in progress");
                break;

            case 'acquisition_ended':
                dump("EwXPCOM : Acquisition ended");
                break;

            case 'acquisition_error':
                dump("EwXPCOM : Acquisition error" + error);
                break;

            case 'metadata_changed':
                dump("EwXPCOM : Metadata changed");
                ewmw.get_services(_metadataChangedCallback);
                break;

            default:
                dump("EwXPCOM : Acquisition: Unknown event(" + event + ")");
                break;
        }
    }

    function _notifyServiceStateChange(serviceId, event, mpdUri) {
        dump("EwXPCOM : [LteBroadcastManager] notifyServiceStateChanged function");

        for (var i = 0; i < _lteStreamingService.handleList.length; i++) {
            var handle = _lteStreamingService.handleList[i];
            if (serviceId == handle.handleId) {
                if (_lteStreamingService.listener) {
                    if (event == serviceStateEnum.HANDLE_STATE_STREAMING_STARTED) {
                        dump("EwXPCOM : [LteBroadcastManager] send event HANDLE_STATE_STREAMING_STARTED");
                    } else if (event == serviceStateEnum.HANDLE_STATE_STREAMING_PAUSED)
                        dump("EwXPCOM : [LteBroadcastManager] send event HANDLE_STATE_STREAMING_PAUSED");

                    _lteStreamingService.listener.notifyHandleStateChange(handle.handleId, event);
                }
                break;
            }
        }
    }

    function _liveCallback(event, service, uri) {
        dump("EwXPCOM : [LteBroadcastManager] live callback");

        switch (event) {
            case 'live_opened':
                dump("EwXPCOM : [" + service + "] Live opened");
                break;

            case 'live_already_opened':
                dump("EwXPCOM : [" + service + "] Live already opened");
                break;

            case 'live_closed':
                dump("EwXPCOM : [" + service + "] Live closed");
                _notifyServiceStateChange(service, serviceStateEnum.HANDLE_STATE_STREAMING_PAUSED, null);
                break;

            case 'live_ready':
                dump("EwXPCOM : [" + service + "] Live ready " + " (mpdUri=" + uri + "): let's notify SUCCESS");

                //_notifyServiceStateChange(service, serviceStateEnum.HANDLE_STATE_STREAMING_STARTED, uri);
                if (_lteStreamingService.notifySuccess) {
                    _lteStreamingService.notifySuccess();
                }
                break;

            case 'live_quality_indication':
                dump("EwXPCOM : [" + service + "] Live quality indication");
                break;

            case 'live_bad_presentation':
                dump("EwXPCOM : [" + service + "] Live bad presentation");
                break;

            default:
                dump("EwXPCOM : [" + service + "] Live: Unknown event(" + event + ")");
                break;
        }
    }

    function _modemCallback(event, state, saiList) {
        dump("EwXPCOM : [LteBroadcastManager] modem callback");

        switch (event) {
            case 'modem_enabled':
                dump("EwXPCOM : Modem enabled");
                break;

            case 'modem_disabled':
                dump("EwXPCOM : Modem disabled");
                break;

            case 'modem_no_device':
                dump("EwXPCOM : Modem no device");
                break;

            case 'signal_strength':
                dump("EwXPCOM : Modem signal strength");
                break;

            case 'signal_info':
                dump("EwXPCOM : Modem signal information");
                break;

            case 'signal_coverage':
                dump("EwXPCOM : Modem signal coverage");
                if (state == 2) {
                    _coverage = coverageEnum.COVERAGE_IN;
                } else {
                    _coverage = coverageEnum.COVERAGE_OUT;
                }

                if (_lteMgrListener) {
                    dump("EwXPCOM : Notify coverage changed (coverage=" + _coverage + ")");
                    _lteMgrListener.notifyCoverageChanged(_coverage);
                }
                break;

            case 'sai_updated':
                dump("EwXPCOM : Sai updated :");
                for (var i = 0; i < saiList.length; i++) {
                    dump("EwXPCOM :		>" + saiList[i]);
                    _saiList[i] = saiList[i];
                }

                if (_lteMgrListener) {
                    dump("EwXPCOM : Notify sai updated");
                    _lteMgrListener.notifySAIChanged();
                }
                break;

            default:
                dump("EwXPCOM : [" + service + "] Modem: Unknown event(" + event + ")");
                break;
        }
    }

    var _event = {
        notify: function (timer) {
            if (ewmw) {
                dump("EwXPCOM : [LteBroadcastManager] Timer ended => Start msp client again.");
                ewmw.start();
            }
        }
    };

    var _timer = null;

    function _errorCallback(level, origin, message) {
        switch (level) {
            case 'F':
                dump("EwXPCOM : [LteBroadcastManager] Waiting for 5s before trying to start msp client again...");
                if (!_timer)
                    _timer = Cc["@mozilla.org/timer;1"].createInstance(Components.interfaces.nsITimer);
                else
                    _timer.cancel();
                _timer.initWithCallback(_event, 5000, Ci.nsITimer.TYPE_ONE_SHOT);
                break;

            case 'E':
                if (message)
                    dump("EwXPCOM : [LteBroadcastManager] ERROR: message: <" + message + ">");
                if (origine)
                    dump("EwXPCOM : [LteBroadcastManager] ERROR: origine: <" + origine + ">");
                break;

            case 'W':
                if (message)
                    dump("EwXPCOM : [LteBroadcastManager] WARNING: message: <" + message + ">");
                if (origine)
                    dump("EwXPCOM : [LteBroadcastManager] WARNING: origine: <" + origine + ">");
                break;

            default:
                if (level)
                    dump("EwXPCOM : [LteBroadcastManager] Unknown level: <" + level + ">");
                if (message)
                    dump("EwXPCOM : [LteBroadcastManager] Unknown level: message: <" + message + ">");
                if (origine)
                    dump("EwXPCOM : [LteBroadcastManager] Unknown level: origine: <" + origine + ">");
                break;
        }
    }

    function _startEmbmsClient() {
        dump("EwXPCOM : [LteBroadcastManager] server address=" + SERVER_URI + "");
        ewmw = new ew_embms({
            "server": SERVER_URI,
            "user": _appId,
            "service_classes": _classList
        });

        ewmw.set_callback('registration_started', function (user, device) {
            _registrationCallback('registration_started', user, device, null);
        });
        ewmw.set_callback('registration_success', function (user, device) {
            _registrationCallback('registration_success', user, device, null);
        });
        ewmw.set_callback('registration_not_allowed', function (user, device) {
            _registrationCallback('registration_not_allowed', user, device, null);
        });
        ewmw.set_callback('registration_error', function (user, device, error) {
            _registrationCallback('registration_error', user, device, error);
        });

        ewmw.set_callback('acquisition_started', function () {
            _acquisitionCallback('acquisition_started', null);
        });
        ewmw.set_callback('acquisition_progress', function (progress) {
            _acquisitionCallback('acquisition_progress', progress);
        });
        ewmw.set_callback('acquisition_ended', function () {
            _acquisitionCallback('acquisition_ended', null);
        });
        ewmw.set_callback('acquisition_error', function () {
            _acquisitionCallback('acquisition_error', null);
        });
        ewmw.set_callback('metadata_changed', function () {
            _acquisitionCallback('metadata_changed', null);
        });

        ewmw.set_callback('live_opened', function (service) {
            _liveCallback('live_opened', service, null);
        });
        ewmw.set_callback('live_closed', function (service, cause) {
            _liveCallback('live_closed', service, null);
        });
        ewmw.set_callback('live_ready', function (service, uri) {
            _liveCallback('live_ready', service, uri);
        });
        ewmw.set_callback('live_quality_indication', function (service, b_success) {
            _liveCallback('live_quality_indication', service, null);
        });
        ewmw.set_callback('live_bad_presentation', function (service, b_bad) {
            _liveCallback('live_bad_presentation', service, null);
        });


        ewmw.set_callback('modem_enabled', function () {
            _modemCallback('modem_enabled', 0, null);
        });
        ewmw.set_callback('modem_disabled', function () {
            _modemCallback('modem_disabled', 0, null);
        });
        ewmw.set_callback('modem_no_device', function () {
            _modemCallback('modem_no_device', 0, null);
        });
        ewmw.set_callback('signal_strength', function (value) {
            _modemCallback('signal_strength', 0, null);
        });
        ewmw.set_callback('signal_info', function (osig) {
            _modemCallback('signal_info', 0, null);
        });
        ewmw.set_callback('signal_coverage', function (state) {
            _modemCallback('signal_coverage', state, null);
        });
        ewmw.set_callback('sai_updated', function (saiList) {
            _modemCallback('sai_updated', 0, saiList);
        });


        ewmw.set_callback('event', function (name, params) {
            dump("EwXPCOM : Event received: " + name + ": " + JSON.stringify(params) + "");
        });
        ewmw.set_callback('error', function (level, origin, message) {
            _errorCallback(level, origin, message);
        });

        ewmw.start();
    }

    this.start = function (appId, callback) {
        dump("EwXPCOM : [LteBroadcastManager] start function");

        if (    appId
            &&  callback) {
            if (!_appId) {
                _appId = appId;
            }

            if (appId == _appId) {
                if (ewmw) {
                    dump("EwXPCOM : [LteBroadcastManager] eMBMS client already started : stop and restart it.");
                    ewmw.stop();
                    ewmw = null;

                    _classList = [];
                    _lteMgrStartCall = false;
                    _lteMgrSetClassCall = false;
                    _lteMgrRegisterCb = null;
                    _lteMgrStartCb = null;
                    _lteMgrStopCb = null;
                    _lteMgrSetClassCb = null;
                }

                if (!ewmw) {
                    _lteMgrStartCall = true;
                    _lteMgrStartCb = callback;

                    if (_lteMgrSetClassCall == true) {
                        _lteMgrRegisterCb = _lteMgrStartCb;
                        _startEmbmsClient();
                    } else {
                        dump("EwXPCOM : [LteBroadcastManager] Waiting for setServiceClasses function call to start eMBMS client.");
                        _lteMgrStartCb.notifySuccess();
                    }
                }
            } else {
                callback.notifyError("Unknown application");
            }
        } else {
            dump("EwXPCOM : [LteBroadcastManager] Bad parameters");
            if (callback)
                callback.notifyError("Bad parameters");
        }
    };

    this.stop = function (appId, callback) {
        dump("EwXPCOM : [LteBroadcastManager] stop function");

        if (    appId
            &&  callback) {
            if (appId == _appId) {
                if (ewmw) {
                    _lteMgrStopCb = callback;
                    ewmw.stop();
                    ewmw = null;
                    _lteMgrStopCb.notifySuccess();

                    _classList = [];
                    _lteMgrStartCall = false;
                    _lteMgrSetClassCall = false;
                    _lteMgrRegisterCb = null;
                    _lteMgrStartCb = null;
                    _lteMgrStopCb = null;
                    _lteMgrSetClassCb = null;
                }
            } else {
                dump("EwXPCOM : [LteBroadcastManager] Unknown application");
                callback.notifyError("Unknown application");
            }
        } else {
            dump("EwXPCOM : [LteBroadcastManager] Bad parameters");
            if (callback)
                callback.notifyError("Bad parameters");
        }
        //_appId = null;
    };

    this.register = function (appId, listener) {
        dump("EwXPCOM : [LteBroadcastManager] register function");

        if (    appId
            &&  listener) {
            if (!_appId) {
                _appId = appId;
            }

            if (appId == _appId) {
                _lteMgrListener = listener;
            } else {
                dump("EwXPCOM : [LteBroadcastManager] Unknown application");
            }
        } else {
            dump("EwXPCOM : [LteBroadcastManager] Bad parameters");
        }
    };

    this.unregister = function (appId, listener) {
        dump("EwXPCOM : [LteBroadcastManager] unregister function");

        if (    appId
            &&  listener) {
            if (appId == _appId) {
                _lteMgrListener = null;
                // _appId = null;
            } else {
                dump("EwXPCOM : [LteBroadcastManager] Unknown application");
            }
        } else {
            dump("EwXPCOM : [LteBroadcastManager] Bad parameters");
        }
    };

    this.setServiceClasses = function (appId, len, classList, callback) {
        dump("EwXPCOM : [LteBroadcastManager] setServiceClasses function");

        if (    appId
            &&  callback) {
            if (!_appId) {
                _appId = appId;
            }

            if (appId == _appId) {
                if (ewmw) {
                    dump("EwXPCOM : [LteBroadcastManager] eMBMS client already started : stop and restart it!");
                    ewmw.stop();
                    ewmw = null;

                    _classList = [];
                    _lteMgrStartCall = false;
                    _lteMgrSetClassCall = false;
                    _lteMgrRegisterCb = null;
                    _lteMgrStartCb = null;
                    _lteMgrStopCb = null;
                    _lteMgrSetClassCb = null;
                }

                if (!ewmw) {
                    if (!classList ||
                        classList.length == 0) {
                        dump("EwXPCOM : [LteBroadcastManager] Service class: none");
                    } else {
                        dump("EwXPCOM : [LteBroadcastManager] Service class: ");
                        for (var i = 0; i < classList.length; i++) {
                            _classList[i] = classList[i];
                            dump(_classList[i] + " ");
                        }
                    }

                    _lteStreamingService.init(classList.length, classList);

                    _lteMgrSetClassCall = true;
                    _lteMgrSetClassCb = callback;

                    if (_lteMgrStartCall == true) {
                        _lteMgrRegisterCb = _lteMgrSetClassCb;
                        _startEmbmsClient();
                    } else {
                        dump("EwXPCOM : [LteBroadcastManager] Waiting for start function call to start eMBMS client.");
                        _lteMgrSetClassCb.notifySuccess();
                    }
                }
            } else {
                dump("EwXPCOM : [LteBroadcastManager] Unknown application");
                callback.notifyError("Unknown application");
            }
        } else {
            dump("EwXPCOM : [LteBroadcastManager] Bad parameters");
            if (callback)
                callback.notifyError("Bad parameters");
        }
    };

    this.getService = function (appId, type) {
        dump("EwXPCOM : [LteBroadcastManager] getService function");

        var result = {};

        if (appId) {
            if (appId == _appId) {
                if (type == serviceTypeEnum.SERVICE_TYPE_STREAMING) {
                    dump("EwXPCOM : [LteBroadcastManager] Live services list requested");

                    result = _lteStreamingService;
                } else if (type == serviceTypeEnum.SERVICE_TYPE_FILE_DOWNLOAD) {
                    dump("EwXPCOM : [LteBroadcastManager] File download service list not supported");
                } else {
                    dump("EwXPCOM : [LteBroadcastManager] Unknown service type");
                }
            } else {
                dump("EwXPCOM : [LteBroadcastManager] Unknown application");
            }
        } else {
            dump("EwXPCOM : [LteBroadcastManager] Bad parameters");
        }

        return result;
    };

    this.getSAI = function (saiArray) {
        dump("EwXPCOM : [LteBroadcastManager] getSAI function");

        if (saiArray) {
            saiArray.value = new Array();

            for (var i = 0; i < _saiList.length; i++) {
                saiArray.value[i] = _saiList[i];
            }
        } else {
            dump("EwXPCOM : [LteBroadcastManager] Bad parameters");
        }

        return saiArray.value.length;
    };
}

LteBroadcastManager.prototype = {
    classID: MANAGER_CLASS_ID,
    QueryInterface: XPCOMUtils.generateQI([Ci.nsILteBroadcastManager]),
    classInfo: XPCOMUtils.generateCI({
        classID: MANAGER_CLASS_ID,
        contractID: MANAGER_CONTRACT_ID,
        interfaces: [Ci.nsILteBroadcastManager],
        classDescription: MANAGER_CLASS_NAME,
    }),
};

/*****************************************/
/* LteBroadcastServiceListener Component */
/*****************************************/

const SERVICELISTENER_CLASS_ID = Components.ID("{45770b99-4b65-4314-9d47-be230a79c4a4}");
const SERVICELISTENER_CONTRACT_ID = "@expway.com/embms/servicelistener;1";
const SERVICELISTENER_CLASS_NAME = "LteBroadcastServiceListener";

const SERVICE_STATE_HANDLES_UPDATED = 1;

var serviceStateEnum = {
    HANDLE_STATE_STREAMING_AVAILABILITY_CHANGE: 1,
    HANDLE_STATE_STREAMING_STARTED: 2,
    HANDLE_STATE_STREAMING_PAUSED: 3
};

function LteBroadcastServiceListener() {
    dump("EwXPCOM : [LteBroadcastServiceListener] Constructor");

    this.notifyStateChange = function (event) {};

    this.notifyHandleStateChange = function (handleId, event) {};
}

LteBroadcastServiceListener.prototype = {
    classID: SERVICELISTENER_CLASS_ID,
    QueryInterface: XPCOMUtils.generateQI([Ci.nsILteBroadcastServiceListener]),
    classInfo: XPCOMUtils.generateCI({
        classID: SERVICELISTENER_CLASS_ID,
        contractID: SERVICELISTENER_CONTRACT_ID,
        interfaces: [Ci.nsILteBroadcastServiceListener],
        classDescription: SERVICELISTENER_CLASS_NAME,
    }),
};

/*******************************************/
/* LteBroadcastGetHandleCallback Component */
/*******************************************/

const GETHANDLECALLBACK_CLASS_ID = Components.ID("{c3d39c23-347d-4a7a-8671-5fbad4c15497}");
const GETHANDLECALLBACK_CONTRACT_ID = "@expway.com/embms/gethandlecallback;1";
const GETHANDLECALLBACK_CLASS_NAME = "LteBroadcastGetHandleCallback";

function LteBroadcastGetHandleCallback() {
    dump("EwXPCOM : [LteBroadcastGetHandleCallback] Constructor");

    this.notifyGetHandleSuccess = function (len, handleArray) {
        dump("EwXPCOM : [LteBroadcastGetHandleCallback] notifyGetHandleSuccess function");
    };
}

LteBroadcastGetHandleCallback.prototype = {
    classID: GETHANDLECALLBACK_CLASS_ID,
    QueryInterface: XPCOMUtils.generateQI([Ci.nsILteBroadcastGetHandleCallback]),
    classInfo: XPCOMUtils.generateCI({
        classID: GETHANDLECALLBACK_CLASS_ID,
        contractID: GETHANDLECALLBACK_CONTRACT_ID,
        interfaces: [Ci.nsILteBroadcastGetHandleCallback],
        classDescription: GETHANDLECALLBACK_CLASS_NAME,
    }),
};

/*********************************/
/* LteBroadcastService Component */
/*********************************/

const SERVICE_CLASS_ID = Components.ID("{1e50ca76-0ba9-4f22-b052-5ff2abb25fdb}");
const SERVICE_CONTRACT_ID = "@expway.com/embms/service;1";
const SERVICE_CLASS_NAME = "LteBroadcastService";

function LteBroadcastService() {
    dump("EwXPCOM : [LteBroadcastService] Constructor");

    this.serviceType = undefined;
    this.handleList = [];
    this.listener = null;

    this.init = function (serviceClassNb, serviceClassesArray) {
        dump("EwXPCOM : [LteBroadcastService] init function");
    };

    this.register = function (listener) {
        dump("EwXPCOM : [LteBroadcastService] register function");
        if (listener) {
            this.listener = listener;
        } else {
            dump("EwXPCOM : [LteBroadcastService] Bad parameters");
        }
    };

    this.unregister = function (listener) {
        dump("EwXPCOM : [LteBroadcastService] unregister function");
        if (listener) {
            this.listener = null;
        } else {
            dump("EwXPCOM : [LteBroadcastService] Bad parameters");
        }
    };

    this.getHandles = function (callback) {
        dump("EwXPCOM : [LteBroadcastService] getHandles function");
        if (callback) {
            if (this.handleList) {
                dump("EwXPCOM : Print handles list before sending the GetHandleSuccess notification : ");
                dump("EwXPCOM : ********************************************************************* ");

                for (var i = 0; i < this.handleList.length; i++) {
                    var handle = this.handleList[i];

                    if (handle.handleId) {
                        dump("EwXPCOM :  => [handle " + i + "] handleId: " + handle.handleId);
                    } else {
                        dump("EwXPCOM :  => [handle " + i + "] handleId: none");
                    }

                    if (handle.class) {
                        dump("EwXPCOM :         class: " + handle.class);
                    } else {
                        dump("EwXPCOM :         class: none");
                    }

                    dump("EwXPCOM :         language: " + handle.language);
                    dump("EwXPCOM :         mpdUri: " + handle.mpdUri);
                    dump("EwXPCOM :         availability: " + handle.availability);

                    dump("EwXPCOM :         sessionStartTime: " + handle.sessionStartTime);
                    dump("EwXPCOM :         sessionEndTime: " + handle.sessionEndTime);

                    var nameList = {};
                    var len = 0;

                    len = handle.getNames(nameList);

                    for (var j = 0; j < nameList.value.length; j++) {
                        var name = nameList.value[j];

                        dump("EwXPCOM :         aName[" + j + "].name: " + name.name);
                        dump("EwXPCOM :         aName[" + j + "].language: " + name.language);
                    }
                }
                dump("EwXPCOM : ********************************************************************* ");
            }
            callback.notifyGetHandleSuccess(this.handleList.length, this.handleList);
        } else {
            dump("EwXPCOM : [LteBroadcastService] Bad parameters");
        }
    };
}

LteBroadcastService.prototype = {
    classID: SERVICE_CLASS_ID,
    QueryInterface: XPCOMUtils.generateQI([Ci.nsILteBroadcastService]),
    classInfo: XPCOMUtils.generateCI({
        classID: SERVICE_CLASS_ID,
        contractID: SERVICE_CONTRACT_ID,
        interfaces: [Ci.nsILteBroadcastService],
        classDescription: SERVICE_CLASS_NAME,
    }),
};

/******************************************/
/* LteBroadcastStreamingService Component */
/******************************************/

const STREAMINGSERVICE_CLASS_ID = Components.ID("{d600d061-2418-43ef-a6c9-662913244f8b}");
const STREAMINGSERVICE_CONTRACT_ID = "@expway.com/embms/streamingservice;1";
const STREAMINGSERVICE_CLASS_NAME = "LteBroadcastStreamingService";

function LteBroadcastStreamingService() {
    dump("EwXPCOM : [LteBroadcastStreamingService] Constructor");

    LteBroadcastService.call(this);

    var _lteStreamingHandleStartCb = null;
    var _lteStreamingHandleStopCb = null;

    this.notifySuccess = function () {
        if (_lteStreamingHandleStartCb) {
            _lteStreamingHandleStartCb.notifySuccess();
        } else {
            dump("EwXPCOM : _lteStreamingHandleStartCb is NULL");
        }
    };

    function liveOpenCallback(state, result, error) {
        if (state == 0) {
            // Notify SUCCESS when LIVE_READY
            //_lteStreamingHandleStartCb.notifySuccess();
        } else if (result > 0) {
            dump("EwXPCOM : \tReceive Warning when live open <" + error + ">");
            dump("EwXPCOM : \t\tresult <" + result + ">");
            dump("EwXPCOM : \t\terror <" + error + ">");
        } else {
            _lteStreamingHandleStartCb.notifyError("Live: " + error);
        }
    }

    function liveCloseCallback(state, result, error) {
        if (state == 0) {
            _lteStreamingHandleStopCb.notifySuccess();
        } else {
            _lteStreamingHandleStopCb.notifyError("Live:" + error);
        }
    }

    this.start = function (handleId, callback) {
        dump("EwXPCOM : [LteBroadcastStreamingHandle] start function");
        if (callback) {
            _lteStreamingHandleStartCb = callback;

            if (ewmw) {
                if (handleId) {
                    ewmw.live_start(handleId, liveOpenCallback);
                } else {
                    _lteStreamingHandleStartCb.notifyError("Live: invalid handleId");
                }
            }
        } else {
            dump("EwXPCOM : [LteBroadcastStreamingHandle] Bad parameters");
        }
    };

    this.stop = function (handleId, callback) {
        dump("EwXPCOM : [LteBroadcastStreamingHandle] stop function");
        if (callback) {
            _lteStreamingHandleStopCb = callback;

            if (ewmw) {
                if (handleId) {
                    ewmw.live_stop(handleId, liveCloseCallback);
                } else {
                    _lteStreamingHandleStopCb.notifyError("Live: invalid handleId");
                }
            }
        } else {
            dump("EwXPCOM : [LteBroadcastStreamingHandle] Bad parameters");
        }
    };
}

LteBroadcastStreamingService.prototype = {
    classID: STREAMINGSERVICE_CLASS_ID,
    QueryInterface: XPCOMUtils.generateQI([Ci.nsILteBroadcastStreamingService]),
    classInfo: XPCOMUtils.generateCI({
        classID: STREAMINGSERVICE_CLASS_ID,
        contractID: STREAMINGSERVICE_CONTRACT_ID,
        interfaces: [Ci.nsILteBroadcastStreamingService],
        classDescription: STREAMINGSERVICE_CLASS_NAME,
    }),
};

/********************************/
/* LteBroadcastHandle Component */
/********************************/

const HANDLE_CLASS_ID = Components.ID("{50f63690-d8d6-4b9c-94a7-f7de54c0ff0d}");
const HANDLE_CONTRACT_ID = "@expway.com/embms/handle;1";
const HANDLE_CLASS_NAME = "LteBroadcastHandle";

function LteBroadcastHandle() {
    dump("EwXPCOM : [LteBroadcastHandle] Constructor");

    this.handleId = "";
    this.class = "";
    this.language = "";
    this.nameList = [];
    this.listener = null;

    this.getNames = function (nameArray) {
        dump("EwXPCOM : [LteBroadcastHandle] getNames function");

        if (nameArray) {
            nameArray.value = new Array();

            for (var i = 0; i < this.nameList.length; i++) {
                nameArray.value[i] = this.nameList[i];
            }
        } else {
            dump("EwXPCOM : [LteBroadcastHandle] Bad parameters");
        }

        return nameArray.value.length;
    }
}

LteBroadcastHandle.prototype = {
    classID: HANDLE_CLASS_ID,
    QueryInterface: XPCOMUtils.generateQI([Ci.nsILteBroadcastHandle]),
    classInfo: XPCOMUtils.generateCI({
        classID: HANDLE_CLASS_ID,
        contractID: HANDLE_CONTRACT_ID,
        interfaces: [Ci.nsILteBroadcastHandle],
        classDescription: HANDLE_CLASS_NAME,
    }),
};

/*****************************************/
/* LteBroadcastStreamingHandle Component */
/*****************************************/

const STREAMINGHANDLE_CLASS_ID = Components.ID("{d9c699f2-a053-4c0d-84ba-eed8f0582b45}");
const STREAMINGHANDLE_CONTRACT_ID = "@expway.com/embms/streaminghandle;1";
const STREAMINGHANDLE_CLASS_NAME = "LteBroadcastStreamingHandle";

var availabilityEnum = {
    AVAILABILITY_BROADCAST: 0,
    AVAILABILITY_UNICAST: 1
};

function LteBroadcastStreamingHandle() {
    dump("EwXPCOM : [LteBroadcastStreamingHandle] Constructor");

    LteBroadcastHandle.call(this);

    this.mpdUri = "";
    this.availability = availabilityEnum.AVAILABILITY_BROADCAST;
    this.sessionStartTime = 0;
    this.sessionEndTime = 0;
}

LteBroadcastStreamingHandle.prototype = {
    classID: STREAMINGHANDLE_CLASS_ID,
    QueryInterface: XPCOMUtils.generateQI([Ci.nsILteBroadcastStreamingHandle]),
    classInfo: XPCOMUtils.generateCI({
        classID: STREAMINGHANDLE_CLASS_ID,
        contractID: STREAMINGHANDLE_CONTRACT_ID,
        interfaces: [Ci.nsILteBroadcastStreamingHandle],
        classDescription: STREAMINGHANDLE_CLASS_NAME,
    }),
};

/************************************/
/* LteBroadcastHandleName Component */
/************************************/

const HANDLE_NAME_CLASS_ID = Components.ID("{fce895c3-7ff4-4080-b673-0a0363233ff3}");
const HANDLE_NAME_CONTRACT_ID = "@expway.com/embms/handlename;1";
const HANDLE_NAME_CLASS_NAME = "LteBroadcastHandleName";

function LteBroadcastHandleName() {
    dump("EwXPCOM : [LteBroadcastHandleName] Constructor");

    this.name = null;
    this.language = null;
}

LteBroadcastHandleName.prototype = {
    classID: HANDLE_NAME_CLASS_ID,
    QueryInterface: XPCOMUtils.generateQI([Ci.nsILteBroadcastHandleName]),
    classInfo: XPCOMUtils.generateCI({
        classID: HANDLE_NAME_CLASS_ID,
        contractID: HANDLE_NAME_CONTRACT_ID,
        interfaces: [Ci.nsILteBroadcastHandleName],
        classDescription: HANDLE_NAME_CLASS_NAME,
    }),
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([LteBroadcastManager]);

/*********************************/
/* MSP Client                    */
/*********************************/

// Get the class name of an object
function getObjectClassName(obj) {
    if (obj === undefined) return undefined;
    if (obj === null) return null;
    try {
        if (obj && obj.constructor && obj.constructor.toString) {
            var p = obj.constructor.toString().match(/function\s*(\w+)/);
            if (p && p.length == 2) return p[1];
        }
    } catch (e) {}
    var s = obj.toString();
    if (s.substring(0, 8) == "[object ") return s.substring(8, s.length - 1);
    return typeof (obj);
}

/*
    Constructor of the MSP client access object.

    Usage with default options:
        var ew_msp_client = new ew_embms(); // Use default options

    Usage with specific options:
        var ew_msp_client = new ew_embms( { option_name: option_value, option_name: option_value, ... } );

    Example:
        var ew_msp_client = new ew_embms( { "server": "http://127.0.0.1", "service_classes": [ "class_A", "class_B" ] } );

    Supported options:
        "server"
            Protocol and MSP server to use.
            By default: "" (local HTTP server).
            WARNING: A non-empty value will force using Cross-domain AJAX requests (CORS).

        "user"
            User name to use for middleware registration.
            By default: "EXPWAY".

        "device"
            Device identifier to use for middleware registration.
            By default: "EXPWAY".

        "package"
            Application package name to use for middleware registration.
            By default: "com.expway.webapp".

        "asp"
            URL of the Application Service Provider (ASP) to use for eMBMS application authentication.
            By default: null (no authentication).

        "signature_hash"
            Array of bytes of the SHA-1 hash of the application signature.
            By default: null (no authentication).

        "service_classes"
            Array of eMBMS service class names to register.
            By default: empty (no service classes).
*/
function ew_embms(options) {
    var _ew_call_id = 0;
    var _ew_event_timer = null;
    var _ew_callbacks = {};
    var _ew_enabled = false;
    var _ew_state = 0;
    var req_r = 0; // REGISTRATION
    var req_e = 0; // POLLING
    var req_c = 0; // REQUEST

    //var xhr = new XMLHttpRequest();
    var xhr = Cc["@mozilla.org/xmlextras/xmlhttprequest;1"].createInstance();

    var try_count = 3;
    var cur_request = null;
    var qcb = null;
    var q_requests = [];

    var _ew_signal_infos = ['bssi', 'rssi', 'snr', 'bler', 'rsrp', 'rsrq'];
    /*
    	.get_signal_names()
    		Get the signal names.
    */
    this.get_signal_names = function () {
        return _ew_signal_infos.slice(0);
    }

    // These functions need to be used for library defined exceptions
    // to enable differenciation from other exception caused by syntax errors, ...
    function ewthrow(e) {
        throw {
            "ew_exception": true,
            "value": e
        };
    }

    function ewcatch(e) {
        return (e && e.ew_exception) || false;
    }

    /*
        .set_async_callback(function, object)
            Set the function called after all asynchronous API calls are done.
            Multiple asynchonous API calls can be initiated, and then this function
            can be called to define the function to call:
            * When an API call terminates, the function is called
              with the returned result as argument (status==2) if OK.
            * When any API call terminates with an error, all pending calls are cancelled
              and the function is called with the error message (status==1).
            * When all API calls terminates with success, the function is called
              with null argument (status==0).

            function      The function to call (see below table for required arguments).
            object        (optional) The this object to use when calling the function.

        Function arguments   Description
        ===================  =============================================================
          status               0=OK, 1=ERROR, 2=Intermediate result
          arg                  OK:null   ERROR:message   RESULT:data
        ===================  =============================================================
        --------------  -----------------  -------------------------------------------------------
        STATUS          ARGUMENT           DESCRIPTION
        --------------  -----------------  -------------------------------------------------------
        0 (OK)          null               All asynchronous calls are terminated with success.
        1 (ERROR)       error message      One of the asynchronous calls returned an error.
                                           All pending calls are cancelled.
        2 (RESULT)      result             Result returned by one asynchronous call.
                                           There may be other pending asynchronous calls.
        --------------  -----------------  -------------------------------------------------------
    */
    this.set_async_callback = function (func, obj) {
        if (getObjectClassName(func) != "Function") throw "First argument must be a function.";
        if (qcb != null) throw "Invalid state: call this function when there is no pending requests.";
        if (q_requests.length == 0) throw "Invalid state: call this function after asynchronous calls.";
        qcb = [func, obj];
    }

    /*
        .set_callback(name, function, object)
            Set the function for the named callback.

            name          One of the Expway callback name (see below table).
            function      The function to call (see below table for required arguments).
            object        (optional) The this object to use when calling the function.

        Callback name                 Function arguments    Description
        ============================  ====================  =============================================================
            error                                           Error/Warning from middleware or application
                                        level                   "F" for fatal failure (client is stopped),
                                                                "E" for error,
                                                                "W" for warning
                                        origin                  Error/Warning origin
                                        message                 Error/Warning message
        ----------------------------  --------------------  -------------------------------------------------------------
            event                                           Event received (DEBUG ONLY)
                                        name                    Event name: One of the event name described below
                                                                    registration_started, registration_success, ...
                                        params                  Event parameters: The parameters array for the event as
                                                                described below.
        ----------------------------  --------------------  -------------------------------------------------------------
            registration_started                            Registration start event
                                        user_id                 Identifier of the user
                                        device_id               Identifier of the device
        ----------------------------  --------------------  -------------------------------------------------------------
            registration_success                            Registration success event
                                        user_id                 Identifier of the user
                                        device_id               Identifier of the device
        ----------------------------  --------------------  -------------------------------------------------------------
            registration_error                              Registration error event
                                        user_id                 Identifier of the user
                                        device_id               Identifier of the device
                                        error_type              Error code:
                                                                    3  URL mismatch: The given ASP server is not in the ASP list.
                                                                    4  The ASP server is not reachable.
                                                                    5  Error during access to the dynamic configuration file.
                                                                    6  Another user with the same service class has already been registered.
                                                                    7  Registration process ended with an internal error.
        ----------------------------  --------------------  -------------------------------------------------------------
            registration_not_allowed                        Unallowed registration event
                                        user_id                 Identifier of the user
                                        device_id               Identifier of the device
        ----------------------------  --------------------  -------------------------------------------------------------
            live_opened                                     Live service open event
                                        service_id              Identifier of the service
        ----------------------------  --------------------  -------------------------------------------------------------
            live_closed                                     Live service close event
                                        service_id              Identifier of the service
                                        cause                   The close cause:
                                                                    0  Stopped because content schedule ended.
                                                                    1  Stopped because no data has been received within timeout delay.
                                                                    2  Stopped by the application.
                                                                    3  Stopped because session ended by FDT.
                                                                    4  Stopped because mode is downgraded.
        ----------------------------  --------------------  -------------------------------------------------------------
            live_mpd_ready                                  MPD is ready for the live service
                                        service_id              Identifier of the service
                                        mpd_url                 URL of the MPD.
        ----------------------------  --------------------  -------------------------------------------------------------
            live_quality_indication                         Live service quality indication event
                                        service_id              Identifier of the service
                                        b_success               Quality indicator: true=success false=failure
        ----------------------------  --------------------  -------------------------------------------------------------
            live_bad_presentation                           Live service bad presentation event
                                        b_bad_presentation      Bad presentation state: true=bad false=good
        ----------------------------  --------------------  -------------------------------------------------------------
            file_download_started                           File download started
                                        service_id              Identifier of the service
                                        file_uri                URI of the file, or null for bookall.
        ----------------------------  --------------------  -------------------------------------------------------------
            file_download_cancelled                         File download cancelled
                                        service_id              Identifier of the service
                                        file_uri                URI of the file, or null for bookall.
        ----------------------------  --------------------  -------------------------------------------------------------
            file_download_failed                            File download failed
                                        service_id              Identifier of the service
                                        file_uri                URI of the file, or null for bookall.
                                        cause                   Integer number indicating the cause:
                                                                    0 Other failure reason.
                                                                    1 FEC failed to decode received data blocks.
                                                                    2 Received file corrupted (bad MD5 after decoding).
                                                                    3 Error in filesystem.
                                                                    4 Not enough available space on file system.
                                                                    5 File Repair Cancelled.
                                                                    6 File Casting Cancelled.
                                                                    7 File repair failed because of roaming.
        ----------------------------  --------------------  -------------------------------------------------------------
            file_download_stopped                           File download stopped
                                        service_id              Identifier of the service
                                        file_uri                URI of the file, or null for bookall.
                                        cause                   Integer number indicating the cause:
                                                                    0 Stopped because content schedule ended.
                                                                    1 Stopped because no data has been received within timeout delay.
                                                                    2 Stopped by application.
                                                                    3 Stopped because no longer available after a circle change.
                                                                    4 Stopped because session ended by FDT.
                                                                    5 Stopped because mode is downgraded.
        ----------------------------  --------------------  -------------------------------------------------------------
            file_download_progress                          File download progression notification
                                        service_id              Identifier of the service
                                        file_uri                URI of the file, or null for bookall.
                                        total_size              Total number of bytes
                                        current_size            Current number of downloaded bytes
        ----------------------------  --------------------  -------------------------------------------------------------
            file_download_completed                         File download completed
                                        service_id              Identifier of the service
                                        file_uri                URI of the file, or null for bookall.
        ----------------------------  --------------------  -------------------------------------------------------------
            fec_decoding_started                            FEC decoding started for the file
                                        service_id              Identifier of the service
                                        file_uri                URI of the file, or null for bookall.
        ----------------------------  --------------------  -------------------------------------------------------------
            fec_decoding_ended                              FEC decoding ended for the file
                                        service_id              Identifier of the service
                                        file_uri                URI of the file, or null for bookall.
        ----------------------------  --------------------  -------------------------------------------------------------
            file_repair_by_application                      File repair to be done by the application
                                        service_id              Identifier of the service
                                        file_uri                URI of the file, or null for bookall.
        ----------------------------  --------------------  -------------------------------------------------------------
            file_repair_expected                            File repair expected for the file
                                        service_id              Identifier of the service
                                        file_uri                URI of the file, or null for bookall.
        ----------------------------  --------------------  -------------------------------------------------------------
            file_repair_started                             Unicast file repair started
                                        service_id              Identifier of the service
                                        file_uri                URI of the file, or null for bookall.
        ----------------------------  --------------------  -------------------------------------------------------------
            file_repair_cancelled                           Unicast file repair cancelled
                                        service_id              Identifier of the service
                                        file_uri                URI of the file, or null for bookall.
        ----------------------------  --------------------  -------------------------------------------------------------
            modem_enabled                                   eMBMS modem is enabled
        ----------------------------  --------------------  -------------------------------------------------------------
            modem_disabled                                  eMBMS modem is disabled
        ----------------------------  --------------------  -------------------------------------------------------------
            modem_no_device                                 No eMBMS modem is available
        ============================  ====================  =============================================================
    */
    this.set_callback = function (name, func, obj) {
        if (getObjectClassName(func) != "Function") throw "Second argument must be a function.";
        if (_ew_enabled) throw "Invalid state: call this function before starting eMBMS.";
        _ew_callbacks[name] = [func, obj];
    }

    function _ew_call_function(origin, desc, f, params, is_error_cb) {
        try {
            if (f != null && f[0] != null) {
                var o = f[1];
                f[0].apply(o, params);
            }
        } catch (e) {
            if (!is_error_cb)
                _ew_call_callback('error',
                    ["E", origin, desc + " returned an error: " + e.message, e]);
        }
    }

    function _ew_call_callback(name, params) {
        _ew_call_function("Application", "Application callback function for " + name, _ew_callbacks[name], params, name == 'error');
    }

    function _ew_call_async_callback(params) {
        _ew_call_function("Application", "Application asynchronous callback function", qcb, params);
    }

    function _ew_error_is_no_user(e) {
        return (e instanceof Object) && (e.code == -20002 || e.code == -20003);
    }

    var _ew_server = "";

    function _ew_call_async_begin(method, params) {
        if (method == "/") {
            xhr.open("GET", _ew_server + "/", true);
            xhr.onreadystatechange = _ew_call_async_end;
            xhr.send();
        } else {
            var call_id = _ew_call_id++;
            var request = {
                "jsonrpc": "2.0",
                "method": method,
                "params": params,
                "id": call_id
            };
            _ew_call_callback('debug', ['CALL >> ' + JSON.stringify(request)]); // DEBUG

            xhr.open("POST", _ew_server + "/ewmsp", true);
            xhr.setRequestHeader("Connection", "keep-alive");
            xhr.setRequestHeader("Content-type", "application/json-rpc");
            xhr.onreadystatechange = _ew_call_async_end;
            xhr.send(JSON.stringify(request));
        }
    }

    var _ew_reg_user = "EXPWAY"; /* options.user */
    var _ew_reg_device = "EXPWAY"; /* options.device */
    var _ew_reg_package = "com.expway.webapp"; /* options.package */
    var _ew_reg_asp = null; /* options.asp */
    var _ew_reg_sig_hash = null; /* options.signature_hash */
    var _ew_reg_service_classes = []; /* options.service_classes */

    // All changes to protocol must have a flag to enable old behavior with default set to new behavior.
    var _ew_compatiblity_reg_device_id = false; // Flag to include DeviceID as parameter to register (NO by default)

    function _ew_call_register() {
        req_r = 2;
        _ew_call_async_begin("register_user",
            _ew_compatiblity_reg_device_id ? [
                _ew_reg_user, /* user */
                _ew_reg_device, /* device */
                _ew_reg_package, /* package name */
                _ew_reg_asp, /* ASP URL */
                _ew_reg_sig_hash, /* Signature SHA-1 */
                _ew_reg_service_classes
            ] : [
                _ew_reg_user, /* user */
                _ew_reg_package, /* package name */
                _ew_reg_asp, /* ASP URL */
                _ew_reg_sig_hash, /* Signature SHA-1 */
                _ew_reg_service_classes
            ]); /* Service classes */
    }

    function _ew_call_get_events() {
        req_e = 2;
        _ew_call_async_begin("get_events", []);
    }

    function _ew_call_async_next(queue_only_if_ready) {
        //_ew_call_callback('debug',['---- == R:'+req_r+' E:'+req_e+' C:'+req_c+' (('+cur_request+' :'+try_count+'))']); // DEBUG
        if (req_r == 2 || req_e == 2 || req_c == 2) return;
        for (;;) {
            if (req_r == 1) // Registration requested
            {
                //_ew_call_callback('debug',['++++ ** REGISTER']); // DEBUG
                _ew_call_register();
                return;
            } else if (req_e == 1) // Event polling requested
            {
                //_ew_call_callback('debug',['++++ ** EVENT POLL'+(_ew_state==0?' after REGISTER':' now!')]); // DEBUG
                if (_ew_state == 0) _ew_call_register();
                else _ew_call_get_events();
                return;
            } else if (q_requests.length == 0) {
                req_c = 0;
                return;
            } // No queued call request
            else {
                req_c = 1;
                if (cur_request == null) {
                    cur_request = q_requests[0];
                    try_count = 3;
                    //_ew_call_callback('debug',['++++ -- NEW API CALL, INIT try_count='+try_count]); // DEBUG
                }
                //else _ew_call_callback('debug',['++++ -- RETRY API CALL, try_count='+try_count]); // DEBUG
                /*
                    3 attempts for an API call:
                    The HTTP protocol 1.0+ allow using the same socket for multiple requests
                    by using a Keep-Alive connection. The HTTP server and the HTTP client can
                    decides how many requests can be done per connection which will be closed
                    after this limit has been reached.
                    Scenario:
                        [SOCKET 1] >> (client) CALL: Attempt 1 to call the API
                        [SOCKET 1] << (server) ERROR: Not registered
                        [SOCKET 1] >> (client) CALL: Register (implicit call)
                        [SOCKET 1] << (server) OK
                        [SOCKET 1] >> (client) CALL: Get event (implicit call)
                        [SOCKET 1] << (server) OK, register success
                        **** Number of HTTP requests as reached the server or client limit, connection closed ****
                        **** New socket will be opened (port is different, will not be recognized by the MSP) ****
                        [SOCKET 2] >> (client) CALL: Attempt 2 to call the API
                        [SOCKET 2] << (server) ERROR: Not registered
                        [SOCKET 2] >> (client) CALL: Register (implicit call)
                        [SOCKET 2] << (server) OK
                        [SOCKET 2] >> (client) CALL: Get event (implicit call)
                        [SOCKET 2] << (server) OK, register success
                        [SOCKET 2] >> (client) CALL: Attempt 3 to call the API
                        [SOCKET 2] << (server) OK
                    ==> Need 3 attempts for each API call
                */

                var reg_level = cur_request[3];
                var pre_register = (_ew_state == 0 && reg_level > 0);
                var pre_events = (_ew_state == 1 && reg_level > 1);
                /*_ew_call_callback('debug',['++++ ..   METHOD '+cur_request[0]+'[reg'+reg_level+'][state'+_ew_state+'] --> '+(
                    pre_register?'Need register before'+(queue_only_if_ready?' (wait)':' (now)'):
                    pre_events?'Need event for register success before'+(queue_only_if_ready?' (wait)':' (now)'):
                    (cur_request[0]) ? 'API CALL' : 'SKIP empty request')]);*/ // DEBUG
                if (queue_only_if_ready && (pre_register || pre_events)) return; // Not ready, Don't do prerequired actions.

                if (pre_register) {
                    _ew_call_register();
                    return;
                } // Prerequired registration
                else if (pre_events) {
                    _ew_call_get_events();
                    return;
                } // Prerequired event (register:success)
                else if (cur_request[0]) {
                    if (--try_count < 0) {
                        _ew_call_callback('error', ["F", "API call failure",
                            "API function still fail after multiple attempts", cur_request
                        ]);
                        _ew_stop(true);
                        return;
                    }
                    req_c = 2;
                    _ew_call_async_begin(cur_request[0], cur_request[1]);
                    return;
                } else {
                    cur_request = null;
                    q_requests.shift();
                }
            }
        }
    }

    function _ew_call_async(reg_level, method, params, fcb) {
        if (q_requests.push([method, params, fcb, reg_level]) == 1) // Only 1 in the queue
        {
            _ew_call_async_next(false); // Start it immediately
        }
    }

    function _ew_call_api(method, params, fcb) {
        if (!_ew_enabled) throw 'eMBMS not started';
        _ew_call_async(2, method, params, fcb);
    }

    function _ew_current_is_json() {
        return (req_c != 2) || (cur_request != null && cur_request[0] != "/");
    }

    /*
        .get_lib_version_code()
            Return the version code of the JS client library.
            WARNING: This is not a version number, but an internal mean to identify the library version.
    */
    this.get_lib_version_code = function () {
        return 'QIKH';
    }

    function _ew_call_async_end() {
        if (xhr.readyState != 4) return; // Not interested before end of request
        var queue_only_if_ready = false;
        try {
            if (xhr.status != 200) {
                if (xhr.status < 100) ewthrow("Connection error");
                else ewthrow("HTTP error " + xhr.status + ": " + xhr.statusText);
            }
            _ew_call_callback('debug', ['RESP << ' + xhr.responseText]); // DEBUG

            var resp = xhr.responseText;
            if (_ew_current_is_json()) {
                resp = JSON.parse(resp);
                if (resp.jsonrpc != "2.0") ewthrow("Invalid JSON-RPC version");
                if (resp.id != _ew_call_id - 1) ewthrow("Invalid call identifier");
                if (resp.error != null) ewthrow(resp.error);
            } else {
                resp = {
                    "result": resp
                };
            }
            // OK
            if (req_r == 2) // Register response OK
            {
                _ew_state = 1; // Will be 2 on register success event received
                req_r = 0;
                req_e = 1;
                // queue_only_if_ready will not be used because event polling requested
            } else if (req_e == 2) // Get events response OK
            {
                req_e = 0;
                _ew_process_events(resp.result);
                queue_only_if_ready = true;
            } else if (req_c == 2) {
                req_c = 0;
                _ew_call_function("Application", "API callback function", cur_request[2], [0, resp.result]);
                q_requests.shift();
                cur_request = null;
                if (q_requests.length == 0) _ew_call_async_callback([0, null]); // End of sequence => Call async callback (OK)
            }
        } catch (e) {
            if (ewcatch(e)) e = e.value;
            else throw e; // Javascript syntax error
            // Error occured
            if (req_r == 2) {
                req_r = 0;
                // ?
                _ew_call_callback('error', ["F", "Register call failure",
                    "Register function returned an error", e
                ]);
                _ew_stop(true);
                return;
            }
            if (_ew_error_is_no_user(e)) {
                if (_ew_state == 2) {
                    _ew_state = 0;
                }
                if (req_e == 2) {
                    req_e = 0;
                }
                if (req_c == 2) {
                    req_c = 0;
                }
                req_r = 1;
                // queue_only_if_ready will not be used because registration requested
            } else if (req_e == 2) {
                req_e = 0;
                _ew_call_callback('error', ["F", "Event polling call failure",
                    "Event polling function returned an error", e
                ]);
                _ew_stop(true);
                return;
            } else if (req_c == 2) {
                req_c = 0;
                var code = (e instanceof Object && e.code) ? e.code : -1;
                var errnum = (e instanceof Object && e.number) ? e.number : -1;
                var errmsg =    "API function " + cur_request[0] +
                                " returned " + (code < 0 ? "an error" : "a warning") +
                                ": " + (errnum < 0 ? "" : "(" + errnum + ") ") +
                                ((e instanceof Object && e.message) ? e.message : e);

                // dump("errmsg <" + errmsg + "> - code <" + code + ">"); // DEBUG

                _ew_call_function("Application", "API callback function", cur_request[2], [1, code, errmsg]);
                _ew_call_callback('error', [code < 0 ? "E" : "W", "API function " + cur_request[0] + " failure", errmsg]);
                cur_request = null;
                if (qcb == null) q_requests.shift();
                else {
                    q_requests = [];
                    _ew_call_async_callback([1, errmsg]); // Call async callback (ERROR)
                }
            }
        }
        _ew_call_async_next(queue_only_if_ready); // Do next pending request
    }

    function _ew_process_events(events) {
        try {
            for (var i = 0; i < events.length; i++) {
                var event = events[i];

                // Process some events internally
                switch (event.name) {
                    case 'registration_started':
                        _ew_state = 1;
                        break;
                    case 'registration_success':
                        _ew_state = 2;
                        break;
                    case 'registration_not_allowed':
                    case 'registration_error':
                        _ew_state = 0;
                        _ew_call_callback('error', ["E", "Registration failure", "Registration error or not allowed", event]);
                        //_ew_stop(true);
                        break;
                    case 'signal_information':
                        if (getObjectClassName(event.params) == "Array") {
                            var sigo = event.params;
                            if (event.params.length == 0) sigo = [{}];
                            else if (typeof (event.params[0]) == "Object" && event.params[0]["type"] && event.params[0]["value"]) {
                                var o = {};
                                for (var j = 0; j < event.params.length; j++) {
                                    var siginfo = event.params[j];
                                    var sname = _ew_signal_infos[siginfo.type];
                                    if (sname) o[sname] = siginfo.value;
                                    else _ew_call_callback('error', ["W", "Unknown signal information: type=" + siginfo.type]);
                                }
                                sigo = [o];
                            }
                            _ew_call_callback('signal_info', sigo); // Event built from signal_information event
                        }
                        break;
                    case 'sai_update':
                        _ew_call_callback('sai_updated', [event.params]); // Event built from sai_updated event
                        break;
                }
                // Notify event to application
                _ew_call_callback('event', [event.name, event.params]); // DEBUG
                _ew_call_callback(event.name, event.params);
            }
        } catch (e) {
            _ew_call_callback('error', ["E", "Event polling", "Event processing failed", e]);
        }
    }

    function _ew_event_poll() {
        try {
            if (req_e == 0) req_e = 1;
            _ew_call_async_next(false);
        } catch (e) {
            _ew_call_callback('error', ["E", "Event polling", "Event polling failed", e]);
        }
    }

    var event = {
        observe: function (subject, topic, data) {
            _ew_event_poll();
        }
    }

    function _ew_poll_event_start() {
        if (_ew_event_timer == null) {
            _ew_event_timer = Components.classes["@mozilla.org/timer;1"].createInstance(Components.interfaces.nsITimer);
        }

        _ew_event_timer.init(event, 200, Ci.nsITimer.TYPE_REPEATING_PRECISE_CAN_SKIP);
    }

    function _ew_poll_event_stop() {
        if (_ew_event_timer != null) {
            _ew_event_timer.cancel();
        }
    }

    function _ew_stop(abort, fcb) {
        _ew_poll_event_stop();
        q_requests = [];
        if (req_c == 2 || req_e == 2 || req_r == 2) {
            cur_request = null;
            xhr.abort();
        }
        req_c = 0;
        if (abort) {
            req_e = req_r = 0;
        } else if (_ew_enabled) {
            try {
                // Require no registration because event polling timer stopped.
                _ew_call_async(0, "stop", [], fcb);
            } catch (e) {}
        }
        _ew_enabled = false;
        _ew_state = 0;
    }

    /*
        .stop(function,object)
            Stop the eMBMS middleware (disable eMBMS).

            function      (optional) The function to call when the asynchronous API call is terminated.
            object        (optional) The this object to use when calling the function.

        Function arguments   Description
        ===================  =============================================================
          status               0=OK   Asynchronous call success.
          result               result data.
        ===================  =============================================================

        Function arguments   Description
        ===================  =============================================================
          status               1=ERROR   The asynchronous calls returned an error.
          error code           API error code
          error message        API error message
        ===================  =============================================================
    */
    this.stop = function (fcb, ocb) {
        _ew_stop(false, [fcb, ocb]);
    }

    /*
        .start(function,object)
            Start the eMBMS middleware (enable eMBMS).

            function      (optional) The function to call when the asynchronous API call is terminated.
            object        (optional) The this object to use when calling the function.
    */
    this.start = function (fcb, ocb) {
        _ew_poll_event_start();
        if (!_ew_enabled) {
            _ew_call_async(2, "start", [], [fcb, ocb]);
            _ew_enabled = true;
        }
    }

    /*
        .get_version(function,object)
            Get the middleware version information.

            function      The function to call when the asynchronous API call is terminated.
            object        (optional) The this object to use when calling the function.
    */
    this.get_version = function (fcb, ocb) {
        _ew_call_async(0, "/", [], [fcb, ocb]);
    }

    /*
        .get_protocol_version(function,object)
            Get the protocol version information.

            function      The function to call when the asynchronous API call is terminated.
            object        (optional) The this object to use when calling the function.
    */
    this.get_protocol_version = function (fcb, ocb) {
        _ew_call_async(0, "get_version", [], [fcb, ocb]);
    }

    /*
        .live_start(service_id,function,object)
            Start an eMBMS live service.

            service_id    Identifier of the live service to start.
            function      (optional) The function to call when the asynchronous API call is terminated.
            object        (optional) The this object to use when calling the function.
    */
    this.live_start = function (service_id, fcb, ocb) {
        _ew_call_api("live_open", [service_id], [fcb, ocb]);
    }
    /*
        .live_stop(service_id,function,object)
            Stop an eMBMS live service.

            service_id    Identifier of the live service to stop.
            function      (optional) The function to call when the asynchronous API call is terminated.
            object        (optional) The this object to use when calling the function.
    */
    this.live_stop = function (service_id, fcb, ocb) {
        _ew_call_api("live_close", [service_id], [fcb, ocb]);
    }
    /*
    	.download_start(service_id,file_uri,function,object)
    	    Start an eMBMS file casting file download.

    		service_id    Identifier of the file service.
    		file_uri      URI of the file, or null for bookall.
    		function      (optional) The function to call when the asynchronous API call is terminated.
    		object        (optional) The this object to use when calling the function.
    */
    this.download_start = function (service_id, file_uri, fcb, ocb) {
        _ew_call_api("download_start", [service_id, file_uri], [fcb, ocb]);
    }
    /*
    	.download_stop(service_id,file_uri,function,object)
    	    Stop an eMBMS file casting file download.

    		service_id    Identifier of the file service.
    		file_uri      URI of the file, or null for bookall.
    		function      (optional) The function to call when the asynchronous API call is terminated.
    		object        (optional) The this object to use when calling the function.
    */
    this.download_stop = function (service_id, file_uri, fcb, ocb) {
        _ew_call_api("download_cancel", [service_id, file_uri], [fcb, ocb]);
    }
    /*
    	.download_suspend(service_id,file_uri,function,object)
    	    Stop an eMBMS file casting file download.

    		service_id    Identifier of the file service.
    		file_uri      URI of the file, or null for bookall.
    		function      (optional) The function to call when the asynchronous API call is terminated.
    		object        (optional) The this object to use when calling the function.
    */
    this.download_suspend = function (service_id, file_uri, fcb, ocb) {
        _ew_call_api("download_suspend", [service_id, file_uri], [fcb, ocb]);
    }
    /*
    	.download_get_files(service_id,function,object)
    	    Get the list of eMBMS services with list of downloaded files for each service.

    		function      The function to call when the asynchronous API call is terminated.
    		object        (optional) The this object to use when calling the function.
    */
    this.download_get_files = function (fcb, ocb) {
        _ew_call_api("get_downloaded_files", [], [fcb, ocb]);
    }
    /*
        .get_services(service_id,function,object)
            Get the list of eMBMS services.

            function      The function to call when the asynchronous API call is terminated.
            object        (optional) The this object to use when calling the function.
    */
    this.get_services = function (fcb, ocb) {
        _ew_call_api("get_services", [], [fcb, ocb]);
    }

    // Setup from options map
    if (options) {
        // Replace default parameter values by provided option values
        if (options["server"]) _ew_server = options["server"];
        if (options["user"]) _ew_reg_user = options["user"];
        if (options["device"]) _ew_reg_device = options["device"];
        if (options["package"]) _ew_reg_package = options["package"];
        if (options["asp"]) _ew_reg_asp = options["asp"];
        if (options["signature_hash"]) _ew_reg_sig_hash = options["signature_hash"];
        if (options["service_classes"]) _ew_reg_service_classes = options["service_classes"];
        if (options["compatibilities"]) // Array of character string
        {
            // Activate compatibilities with old protocol version (use with older middleware)
            //   "reg_device_id"    Include device ID in parameters to call register user.
            var compatibilities = options["compatibilities"];
            for (var i = 0; i < compatibilities.length; i++)
                eval('_ew_compatiblity_' + compatibilities[i] + ' = true;');
        }
        // Check configuration
        if (_ew_server != "") // Not local server => Need CORS support
        {
            if (!("withCredentials" in xhr)) // If CORS not supported by XMLHttpRequest
            {
                if (typeof XDomainRequest != "undefined") // If XDomainRequest exists
                {
                    // XDomainRequest for old IE versions
                    xhr = new XDomainRequest();
                } else throw "An explicit server has been defined but CORS is not supported by the web browser.";
            }
        }
    }
}