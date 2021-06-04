
package com.android.sprd.telephony;

import com.android.sprd.telephony.RadioInteractorCore.SuppService;
import com.android.sprd.telephony.uicc.IccIoResult;
import com.android.sprd.telephony.uicc.IccUtils;

import android.content.Context;
import android.content.pm.PackageManager;
import android.os.AsyncResult;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
import android.telephony.IccOpenLogicalChannelResponse;
import android.telephony.TelephonyManager;
import com.android.sprd.telephony.uicc.IccCardApplicationStatusEx;
import com.android.sprd.telephony.uicc.IccCardApplicationStatusEx.AppState;
import com.android.sprd.telephony.uicc.IccCardApplicationStatusEx.PersoSubState;
import com.android.sprd.telephony.uicc.IccCardStatusEx;
import com.android.sprd.telephony.uicc.IccCardStatusEx.CardState;

public class RadioInteractorHandler extends Handler {
    public static final String TAG = "RadioInteractorHandler";

    RadioInteractorCore mRadioInteractorCore;
    RadioInteractorNotifier mRadioInteractorNotifier;
    SyncHandler mHandler;

 /*
  * This section defines all requests for events
  */
    protected static final int EVENT_GET_REQUEST_RADIOINTERACTOR_DONE = 1;
    protected static final int EVENT_INVOKE_OEM_RIL_REQUEST_STRINGS_DONE = 2;
    protected static final int EVENT_INVOKE_GET_SIM_CAPACITY_DONE = 3;
    protected static final int EVENT_INVOKE_ENABLE_RAU_NOTIFY_DONE = 4;
    protected static final int EVENT_GET_ATR_DONE = 5;

    // SPRD: add for HIGH_DEF_AUDIO
    protected static final int EVENT_GET_HD_VOICE_STATE_DONE = 6;
    // SPRD: Send request to set call forward number whether shown
    protected static final int EVENT_REQUEST_SET_COLP = 7;
    /*SPRD: Bug#542214 Add support for store SMS to Sim card @{*/
    protected static final int EVENT_REQUEST_STORE_SMS_TO_SIM_DONE = 8;
    protected static final int EVENT_QUERY_SMS_STORAGE_MODE_DONE = 9;
    /* @} */
    // Explicit Transfer Call REFACTORING
    protected static final int EVENT_ECT_RESULT = 10;
    // SPRD: add for trafficClass
    protected static final int EVENT_TRAFFIC_CLASS_DONE = 11;
    /* Add for Data Clear Code from Telcel @{ */
    protected static final int EVENT_SET_LTE_ENABLE_DONE = 12;
    protected static final int EVENT_ATTACH_DATA_DONE = 13;
    /* @} */
    // Add for shutdown optimization
    protected static final int EVENT_REQUEST_SHUTDOWN_DONE = 14;
    protected static final int EVENT_INVOKE_GET_DEFAULT_NAN_DONE = 15;
    protected static final int EVENT_GET_REMIAN_TIMES_DONE = 16;
    protected static final int EVENT_GET_SIMLOCK_STATUS_DONE = 17;
    protected static final int EVENT_GET_ICC_STATUS_DONE = 18;
    protected static final int EVENT_SIMLOCK_STATUS_CHANGED = 19;

    protected static final int EVENT_REQUEST_SET_SIM_POWER = 20;
    protected static final int EVENT_REQUEST_SET_PRE_NETWORK_TYPE = 21;
    protected static final int EVENT_REQUEST_UPDTAE_REAL_ECCLIST = 22;

    protected static final int EVENT_GET_BAND_INFO_DONE = 23;
    protected static final int EVENT_SET_BAND_INFO_MODE_DONE = 24;

    protected static final int EVENT_SET_SINGLE_PDN_DONE = 25;
    protected static final int EVENT_REQUEST_SET_SPECIAL_RATCAP = 26;

    protected static final int EVENT_REQUEST_QUERY_COLP = 27;
    protected static final int EVENT_REQUEST_QUERY_COLR = 28;
    protected static final int EVENT_REQUEST_MMI_ENTER_SIM = 29;
    protected static final int EVENT_REQUEST_UPDATE_OPERATOR_NAME = 30;
    protected static final int EVENT_GET_REALL_SIM_STATUS_DONE = 31;
    protected static final int EVENT_REATTACH_DONE = 32;
    protected static final int EVENT_SET_SMS_BEARER_DONE = 33;
    protected static final int EVENT_GET_SIMLOCK_DUMMYS = 34;
    protected static final int EVENT_GET_SIMLOCK_WHITE_LIST = 35;
    protected static final int EVENT_REQUEST_SET_VOICE_DOMAIN = 36;

    protected static final int EVENT_UNSOL_RADIOINTERACTOR = 100;
    /**
     *  Listen for update the list of embms programs.
     */
    protected static final int EVENT_UNSOL_RADIOINTERACTOR_EMBMS = 101;
    /**
     *  Listen for RI has connected.
     */
    protected static final int EVENT_UNSOL_RI_CONNECTED = 102;
    protected static final int EVENT_SWITCH_MULTI_CALLS_DONE = 103;

    protected static final int EVENT_UNSOL_BAND_INFO = 104;
    protected static final int EVENT_UNSOL_SIMMGR_SIM_STATUS_CHANGED = 105;
    protected static final int EVENT_UNSOL_EXPIRE_SIM  = 106;

    static protected final int RESPONSE_DATA_FILE_SIZE_1 = 2;
    static protected final int RESPONSE_DATA_FILE_SIZE_2 = 3;
    static protected final int COMMAND_GET_RESPONSE = 0xc0;
    static protected final int COMMAND_READ_BINARY = 0xb0;

    private RegistrantList mPersonalisationLockedRegistrants = new RegistrantList();
    private PersoSubState mPersoSubState;
    private AppState      mAppState;
    Context mContext;
    private CardState mCardState;

    public RadioInteractorHandler(RadioInteractorCore RadioInteractorCore,
            RadioInteractorNotifier RadioInteractorNotifier,Context context) {
        mRadioInteractorCore = RadioInteractorCore;
        mRadioInteractorNotifier = RadioInteractorNotifier;
        mContext = context;
        unsolicitedRegisters(this, EVENT_UNSOL_RADIOINTERACTOR);
        registerForRiConnected(this, EVENT_UNSOL_RI_CONNECTED);
        registerForRadioInteractorEmbms(this, EVENT_UNSOL_RADIOINTERACTOR_EMBMS);
        registerForBandInfo(this,EVENT_UNSOL_BAND_INFO);
        registerForRealSimStateChanged(this,EVENT_UNSOL_SIMMGR_SIM_STATUS_CHANGED);
        HandlerThread thread = new HandlerThread("RadioInteractor:SyncSender");
        thread.start();
        mHandler = new SyncHandler(thread.getLooper());
        registerForSimlockStatusChanged(this, EVENT_SIMLOCK_STATUS_CHANGED);
        registerForExpireSim(this, EVENT_UNSOL_EXPIRE_SIM);
    }

    public int invokeOemRILRequestStrings(String oemReq, String[] oemResp) {
        ThreadRequest request = new ThreadRequest(oemResp);
        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_INVOKE_OEM_RIL_REQUEST_STRINGS_DONE,
                    request);
            mRadioInteractorCore.sendCmdAsync(oemReq, response);
            waitForResult(request);
        }
        try {
            return (Integer) request.result;
        } catch (ClassCastException e) {
            return -1;
        }
    }

    public String getSimCapacity() {
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_INVOKE_GET_SIM_CAPACITY_DONE, request);
            mRadioInteractorCore.getSimCapacity(response);
            waitForResult(request);
        }
        try {
            return (String) request.result;
        } catch (ClassCastException e) {
            return null;
        }
    }

    public String getDefaultNetworkAccessName() {
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_INVOKE_GET_DEFAULT_NAN_DONE, request);
            mRadioInteractorCore.getDefaultNAN(response);
            waitForResult(request);
        }
        try {
            return (String) request.result;
        } catch (ClassCastException e) {
            return null;
        }
    }

    public void enableRauNotify() {
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_INVOKE_ENABLE_RAU_NOTIFY_DONE, request);
            mRadioInteractorCore.enableRauNotify(response);
            waitForResult(request);
        }
    }

    public String iccGetAtr() {
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_GET_ATR_DONE, request);
            mRadioInteractorCore.simGetAtr(response);
            waitForResult(request);
        }
        try {
            return (String) request.result;
        } catch (ClassCastException e) {
            return null;
        }
    }

    public boolean queryHdVoiceState() {
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_GET_HD_VOICE_STATE_DONE, request);
            mRadioInteractorCore.getHDVoiceState(response);
            waitForResult(request);
        }
        try {
            return (boolean) request.result;
        } catch (ClassCastException e) {
            return false;
        }
    }

    public void setCallingNumberShownEnabled(boolean enabled) {
        Message response = mHandler.obtainMessage(EVENT_REQUEST_SET_COLP);
        mRadioInteractorCore.setCOLP(enabled, response);
    }

    /* SPRD: Bug#542214 Add support for store SMS to Sim card @{ */
    public boolean storeSmsToSim(boolean enable) {
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_REQUEST_STORE_SMS_TO_SIM_DONE, request);
            mRadioInteractorCore.storeSmsToSim(enable, response);
            waitForResult(request);
        }
        try {
            return (boolean) request.result;
        } catch (ClassCastException e) {
            return false;
        }
    }

    public String querySmsStorageMode() {
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_QUERY_SMS_STORAGE_MODE_DONE, request);
            mRadioInteractorCore.querySmsStorageMode(response);
            waitForResult(request);
        }
        try {
            return (String) request.result;
        } catch (ClassCastException e) {
            return null;
        }
    }
    /* @} */

    public void setNetworkSpecialRATCap(int type){
        Message response = mHandler.obtainMessage(EVENT_REQUEST_SET_SPECIAL_RATCAP);
        mRadioInteractorCore.setSpecialRatcap(type, response);
    }

    /**
     * Explicit Transfer Call REFACTORING
     * @param result
     */
    public void explicitCallTransfer () {
        mRadioInteractorCore.explicitCallTransfer(mHandler.obtainMessage(EVENT_ECT_RESULT));
    }

    public void switchMultiCalls(int mode) {
        Message response = mHandler.obtainMessage(EVENT_SWITCH_MULTI_CALLS_DONE);
        mRadioInteractorCore.switchMultiCall(mode, response);
    }

    /* add for TV @{*/
    public void dialVP(String address, String sub_address, int clirMode, Message response) {
        mRadioInteractorCore.videoPhoneDial(address, sub_address, clirMode, response);
    }

    public void codecVP(int type, Bundle param, Message response) {
        mRadioInteractorCore.videoPhoneCodec(type, param, response);
    }

    public void fallBackVP(Message response) {
        mRadioInteractorCore.videoPhoneFallback(response);
    }

    public void sendVPString(String str, Message response) {
        mRadioInteractorCore.videoPhoneString(str, response);
    }

    public void controlVPLocalMedia(int datatype, int sw, boolean bReplaceImg, Message response) {
        mRadioInteractorCore.videoPhoneLocalMedia(datatype, sw, bReplaceImg, response);
    }

    public void controlIFrame(boolean isIFrame, boolean needIFrame, Message response) {
        mRadioInteractorCore.videoPhoneControlIFrame(isIFrame, needIFrame, response);
    }
    /* @} */
    /* Add for trafficClass @{ */
    public void requestDCTrafficClass(int type) {
        Message response = mHandler.obtainMessage(EVENT_TRAFFIC_CLASS_DONE);
        mRadioInteractorCore.setTrafficClass(type, response);
    }
    /* @} */

    /* Add for do recovery @{ */
    public void requestReattach() {
        Message response = mHandler.obtainMessage(EVENT_REATTACH_DONE);
        mRadioInteractorCore.reAttach(response);
    }
    /* @} */

    /*SPRD: bug618350 add single pdp allowed by plmns feature@{*/
    public void requestSetSinglePDNByNetwork(boolean isSinglePDN){
        Message response = mHandler.obtainMessage(EVENT_SET_SINGLE_PDN_DONE);
        mRadioInteractorCore.setSinglePDN(isSinglePDN, response);
    }
    /* @} */
    /* Add for Data Clear Code from Telcel @{ */
    public void setLteEnabled(boolean enable) {
        Message response = mHandler.obtainMessage(EVENT_SET_LTE_ENABLE_DONE);
        mRadioInteractorCore.enableLTE(enable, response);
    }

    public void attachDataConn(boolean enable) {
        Message response = mHandler.obtainMessage(EVENT_ATTACH_DATA_DONE);
        mRadioInteractorCore.attachData(enable, response);
    }
    /* @} */

    public void abortSearchNetwork(Message response) {
        mRadioInteractorCore.stopQueryNetwork(response);
    }

    public void forceDetachDataConn(Message response) {
        mRadioInteractorCore.forceDeatch(response);
    }

    public boolean requestShutdown() {
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_REQUEST_SHUTDOWN_DONE, request);
            mRadioInteractorCore.requestShutdown(response);
            waitForResult(request);
        }
        try {
            return (boolean) request.result;
        } catch (ClassCastException e) {
            return false;
        }
    }

    public void setSimPower(String pkgname,boolean enabled) {
        enforceSprdModifyPermission(pkgname);
        Message response = mHandler.obtainMessage(EVENT_REQUEST_SET_SIM_POWER);
        mRadioInteractorCore.simmgrSimPower(enabled, response);
    }

    public void setPreferredNetworkType(int networkType) {
        Message response = mHandler.obtainMessage(EVENT_REQUEST_SET_PRE_NETWORK_TYPE);
        mRadioInteractorCore.setPreferredNetworkType(networkType, response);
    }

    public void updateRealEccList(String realEccList) {
        Message response = mHandler.obtainMessage(EVENT_REQUEST_UPDTAE_REAL_ECCLIST);
        mRadioInteractorCore.updateEcclist(realEccList, response);
    }

    public String getBandInfo() {
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            UtilLog.logd(TAG,"getBandInfo request:"+request);
            Message response = mHandler.obtainMessage(EVENT_GET_BAND_INFO_DONE, request);
            mRadioInteractorCore.getBandInfo(response);
            waitForResult(request);
        }
        try {
            return (String) request.result;
        } catch (ClassCastException e) {
            return null;
        }
    }

    public void setBandInfoMode(int type) {
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            UtilLog.logd(TAG,"setBandInfoMode request:"+request);
            Message response = mHandler.obtainMessage(EVENT_SET_BAND_INFO_MODE_DONE, request);
            mRadioInteractorCore.setBandInfoMode(type,response);
            waitForResult(request);
        }
    }

    public int queryColp(){
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_REQUEST_QUERY_COLP,request);
            mRadioInteractorCore.queryColp(response);
            waitForResult(request);
        }
        try {
            return (Integer) request.result;
        } catch (ClassCastException e) {
            return -1;
        }
    }

    public int queryColr(){
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_REQUEST_QUERY_COLR,request);
            mRadioInteractorCore.queryColr(response);
            waitForResult(request);
        }
        try {
            return (Integer) request.result;
        } catch (ClassCastException e) {
            return -1;
        }
    }

    public int mmiEnterSim(String data){
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_REQUEST_MMI_ENTER_SIM,request);
            mRadioInteractorCore.mmiEnterSim(data,response);
            waitForResult(request);
        }
        try {
            return (Integer) request.result;
        } catch (ClassCastException e) {
            return -1;
        }
    }

    public void updateOperatorName(String plmn){
        Message response = mHandler.obtainMessage(EVENT_REQUEST_UPDATE_OPERATOR_NAME);
        mRadioInteractorCore.updateOperatorName(plmn, response);
    }

    public int getRealSimStatus() {
        return mCardState == null ? -1 : mCardState.ordinal();
    }

    public void setXcapIPAddress(String cid, String ipv4Addr, String ipv6Addr, Message response) {
        mRadioInteractorCore.setXcapIPAddress(cid, ipv4Addr, ipv6Addr, response);
    }

    public void setSmsBearer(int type) {
        Message response = mHandler.obtainMessage(EVENT_SET_SMS_BEARER_DONE);
        mRadioInteractorCore.setSmsBearer(type, response);
    }

    public int[] getSimlockDummys() {
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_GET_SIMLOCK_DUMMYS, request);
            mRadioInteractorCore.getSimlockDummys(response);
            waitForResult(request);
        }
        try {
            return (int[]) request.result;
        } catch (ClassCastException e) {
            return null;
        }
    }

    public String getSimlockWhitelist(int type) {
        ThreadRequest request = new ThreadRequest(null);
        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_GET_SIMLOCK_WHITE_LIST, request);
            mRadioInteractorCore.getSimlockWhitelist(type, response);
            waitForResult(request);
        }
        try {
            return (String) request.result;
        } catch (ClassCastException e) {
            return null;
        }
    }

    public void setVoiceDomain (int type) {
        Message response = mHandler.obtainMessage(EVENT_REQUEST_SET_VOICE_DOMAIN);
        mRadioInteractorCore.setVoiceDomain(type, response);
    }

    class SyncHandler extends Handler {
        SyncHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar;
            ThreadRequest request;
            String strCapacity[];
            UtilLog.logd(TAG, " handleMessage msg.what:" + msg.what);
            switch (msg.what) {

                case EVENT_GET_REQUEST_RADIOINTERACTOR_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_GET_REQUEST_RADIOINTERACTOR_DONE");
                    synchronized (request) {
                        if (ar.exception == null) {
                            request.result = (((int[]) ar.result))[0];
                        } else {
                            UtilLog.loge(TAG, "handleMessage registration state error!");
                            request.result = -1;
                        }
                        request.notifyAll();
                    }
                    break;
                case EVENT_INVOKE_OEM_RIL_REQUEST_STRINGS_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    String[] oemResp = (String[]) request.argument;
                    int returnOemValue = -1;
                    UtilLog.logd(TAG, "handleMessage EVENT_INVOKE_OEM_RIL_REQUEST_STRINGS_DONE");
                    synchronized (request) {
                        try {
                            if (ar.exception == null) {
                                if (ar.result != null) {
                                    String[] responseData = (String[]) (ar.result);
                                    if (responseData.length > oemResp.length) {
                                        UtilLog.logd(TAG,
                                                "Buffer to copy response too small: Response length is "
                                                        +
                                                        responseData.length
                                                        + "bytes. Buffer Size is " +
                                                        oemResp.length + "bytes.");
                                    }
                                    System.arraycopy(responseData, 0, oemResp, 0,
                                            responseData.length);
                                    returnOemValue = responseData.length;
                                }
                            } else {
                                CommandException ex = (CommandException) ar.exception;
                                returnOemValue = ex.getCommandError().ordinal();
                                if (returnOemValue > 0)
                                    returnOemValue *= -1;
                            }
                        } catch (RuntimeException e) {
                            UtilLog.loge(TAG, "sendOemRilRequestRaw: Runtime Exception");
                            returnOemValue = (CommandException.Error.GENERIC_FAILURE.ordinal());
                            if (returnOemValue > 0)
                                returnOemValue *= -1;
                        }
                        request.result = returnOemValue;
                        request.notifyAll();
                    }
                    break;
                case EVENT_INVOKE_GET_SIM_CAPACITY_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_INVOKE_GET_SIM_CAPACITY_DONE");
                    synchronized (request) {
                        if (ar.exception == null) {
                            strCapacity = (String[]) ar.result;
                            if (strCapacity != null && strCapacity.length >= 2) {
                                UtilLog.logd(TAG, "[sms]sim used:" + strCapacity[0] + " total:"
                                        + strCapacity[1]);
                                request.result = strCapacity[0] + ":" + strCapacity[1];
                                UtilLog.logd(TAG, "[sms]simCapacity: " + request.result);
                            } else {
                                request.result = "ERROR";
                            }
                        } else {
                            request.result = "ERROR";
                            UtilLog.loge(TAG, "[sms]get sim capacity fail");
                        }
                        request.notifyAll();
                    }
                    break;
                case EVENT_INVOKE_ENABLE_RAU_NOTIFY_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_INVOKE_ENABLE_RAU_NOTIFY_DONE");
                    synchronized (request) {
                        if (ar.exception == null) {
                            request.result = ar;
                            UtilLog.logd(TAG, "enable rau: " + request.result);
                        } else {
                            UtilLog.loge(TAG, "enable rau:fail");
                        }
                        request.notifyAll();
                    }
                    break;

                    case EVENT_GET_ATR_DONE:
                        ar = (AsyncResult) msg.obj;
                        request = (ThreadRequest) ar.userObj;
                        UtilLog.logd(TAG, "handleMessage EVENT_GET_ATR_DONE");
                        synchronized (request) {
                            if (ar.exception == null && ar.result != null) {
                                request.result = (String) ar.result;
                            } else {
                                request.result = "ERROR";
                                if (ar.result == null) {
                                    UtilLog.loge(TAG, "iccGetAtr: Empty response");
                                }
                                if (ar.exception != null) {
                                    UtilLog.loge(TAG, "iccGetAtr: Exception: " + ar.exception);
                                }
                            }
                            request.notifyAll();
                        }
                        break;

                case EVENT_GET_HD_VOICE_STATE_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_GET_HD_VOICE_STATE_DONE");
                    synchronized (request) {
                        if (ar.exception == null && ar.result != null) {
                            int resultArray[] = (int[]) ar.result;
                            request.result = (resultArray[0] == 1);
                        } else {
                            request.result = false;
                            if (ar.exception != null) {
                                UtilLog.loge(TAG, "get HD Voice state fail: " + ar.exception);
                            }
                        }
                        request.notifyAll();
                    }
                    break;

                case EVENT_REQUEST_SET_COLP:
                    ar = (AsyncResult) msg.obj;
                    UtilLog.logd(TAG, "handleMessage EVENT_REQUEST_SET_COLP");
                    if (ar.exception == null) {
                        UtilLog.logd(TAG, "set colp　:success"+ ar.result);
                    } else {
                        UtilLog.loge(TAG, "set colp　:fail");
                    }
                    break;

                /* SPRD: Bug#542214 Add support for store SMS to Sim card @{ */
                case EVENT_REQUEST_STORE_SMS_TO_SIM_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_REQUEST_STORE_SMS_TO_SIM_DONE");
                    synchronized (request) {
                        if (ar.exception == null) {
                            request.result = true;
                            UtilLog.logd(TAG, "store sms to sim: " + request.result);
                        } else {
                            request.result = false;
                            UtilLog.loge(TAG, "store sms to sim:fail" + ar.exception);
                        }
                        request.notifyAll();
                    }
                    break;

                case EVENT_QUERY_SMS_STORAGE_MODE_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_QUERY_SMS_STORAGE_MODE_DONE");
                    synchronized (request) {
                        if (ar.exception == null && ar.result != null) {
                            request.result = (String) ar.result;
                        } else {
                            request.result = "ERROR";
                            if (ar.result == null) {
                                UtilLog.loge(TAG, "query sms storage mode: Empty response");
                            }
                            if (ar.exception != null) {
                                UtilLog.loge(TAG,
                                        "query sms storage mode: Exception: " + ar.exception);
                            }
                        }
                        request.notifyAll();
                    }
                    break;
                /* @} */

                case EVENT_ECT_RESULT:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        UtilLog.logd(TAG, "Explicit call failed: " + ar.exception + ", failed reason is : " + msg.what);
                        mRadioInteractorNotifier.notifySuppServiceFailed(mRadioInteractorCore.getPhoneId(),
                                getFailedService(msg.what));
                    }
                    break;

                case EVENT_SWITCH_MULTI_CALLS_DONE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        UtilLog.logd(TAG, "Switch multi call failed: " + ar.exception + ", failed reason is : " + msg.what);
                        mRadioInteractorNotifier.notifySuppServiceFailed(mRadioInteractorCore.getPhoneId(),
                                getFailedService(msg.what));
                    }
                    break;

                case EVENT_TRAFFIC_CLASS_DONE:
                    ar = (AsyncResult) msg.obj;
                    UtilLog.logd(TAG, "handleMessage EVENT_TRAFFIC_CLASS_DONE");
                    if (ar.exception == null) {
                        UtilLog.logd(TAG, "traffic class :success");
                    } else {
                        UtilLog.loge(TAG, "traffic class :fail" + ar.exception);
                    }
                    break;

                case EVENT_SET_LTE_ENABLE_DONE:
                    ar = (AsyncResult) msg.obj;
                    UtilLog.logd(TAG, "handleMessage EVENT_SET_LTE_ENABLE_DONE");
                    if (ar.exception == null) {
                        UtilLog.logd(TAG, "set lte enable :success");
                    } else {
                        UtilLog.loge(TAG, "set lte enable :fail" + ar.exception);
                    }
                    break;

                case EVENT_ATTACH_DATA_DONE:
                    ar = (AsyncResult) msg.obj;
                    UtilLog.logd(TAG, "handleMessage EVENT_ATTACH_DATA_DONE");
                    if (ar.exception == null) {
                        UtilLog.logd(TAG, "attach data :success");
                    } else {
                        UtilLog.loge(TAG, "attach data :fail" + ar.exception);
                    }
                    break;

                case EVENT_REQUEST_SHUTDOWN_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_REQUEST_SHUTDOWN_DONE");
                    synchronized (request) {
                        if (ar.exception == null) {
                            request.result = true;
                        } else {
                            request.result = false;
                            UtilLog.loge(TAG, "shutdown fail: " + ar.exception);
                        }
                        request.notifyAll();
                    }
                    break;

                case EVENT_REQUEST_SET_SIM_POWER:
                    ar = (AsyncResult) msg.obj;
                    UtilLog.logd(TAG, "handleMessage EVENT_REQUEST_SET_SIM_POWER");
                    if (ar.exception == null) {
                        UtilLog.logd(TAG, "set sim power :success" + ar.result);
                    } else {
                        UtilLog.loge(TAG, "set sim power :fail");
                    }
                    break;

                case EVENT_GET_REMIAN_TIMES_DONE:
                case EVENT_GET_SIMLOCK_STATUS_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    synchronized (request) {
                        if (ar.exception == null && ar.result != null) {
                            request.result = ar.result;
                        } else {
                            request.result = -1;
                        }
                        request.notifyAll();
                    }
                    break;

                case EVENT_REQUEST_SET_PRE_NETWORK_TYPE:
                    ar = (AsyncResult) msg.obj;
                    UtilLog.logd(TAG, "handleMessage EVENT_REQUEST_SET_PRE_NETWORK_TYPE");
                    if (ar.exception == null) {
                        UtilLog.logd(TAG, "set preferred network type :success");
                    } else {
                        UtilLog.loge(TAG, "set preferred network type :fail"+ ar.exception);
                    }
                    break;

                case EVENT_INVOKE_GET_DEFAULT_NAN_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_INVOKE_GET_DEFAULT_NAN_DONE");
                    synchronized (request) {
                        if (ar.exception == null && ar.result != null) {
                            request.result = (String) ar.result;
                        } else {
                            request.result = "ERROR";
                            if (ar.result == null) {
                                UtilLog.loge(TAG, "getDefaultNetworkAccessName: Empty response");
                            }
                            if (ar.exception != null) {
                                UtilLog.loge(TAG, "getDefaultNetworkAccessName: Exception: " + ar.exception);
                            }
                        }
                        request.notifyAll();
                    }
                    break;

                case EVENT_REQUEST_UPDTAE_REAL_ECCLIST:
                    ar = (AsyncResult) msg.obj;
                    UtilLog.logd(TAG, "handleMessage EVENT_REQUEST_UPDTAE_REAL_ECCLIST");
                    if (ar.exception == null) {
                        UtilLog.logd(TAG, "update real ecclist :success");
                    } else {
                        UtilLog.loge(TAG, "update real ecclist :fail"+ ar.exception);
                    }
                    break;

                case EVENT_GET_BAND_INFO_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_GET_BAND_INFO_DONE");
                    UtilLog.logd(TAG,"EVENT_GET_BAND_INFO_DONE request:"+request);
                    synchronized (request) {
                        UtilLog.logd(TAG,"EVENT_GET_BAND_INFO_DONE");
                        if (ar.exception == null && ar.result != null) {
                            request.result = (String) ar.result;
                        } else {
                            request.result = "ERROR";
                            if (ar.result == null) {
                                UtilLog.loge(TAG, "get band info: Empty response");
                            }
                            if (ar.exception != null) {
                                UtilLog.loge(TAG,
                                        "get band info: Exception: " + ar.exception);
                            }
                        }
                        request.notifyAll();
                    }
                    break;
                case EVENT_SET_BAND_INFO_MODE_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_SET_BAND_INFO_MODE_DONE");
                    UtilLog.logd(TAG,"EVENT_SET_BAND_INFO_MODE_DONE request:"+request);
                    synchronized (request) {
                        UtilLog.logd(TAG,"EVENT_SET_BAND_INFO_MODE_DONE");
                        if (ar.exception == null) {
                            request.result = ar;
                            UtilLog.logd(TAG, "set band info mode: " + request.result);
                        } else {
                            UtilLog.loge(TAG, "set band info mode:fail");
                        }
                        request.notifyAll();
                    }
                    break;
                /*SPRD: bug618350 add single pdp allowed by plmns feature@{*/
                case EVENT_SET_SINGLE_PDN_DONE:
                    ar = (AsyncResult) msg.obj;
                    UtilLog.logd(TAG, "handleMessage EVENT_SET_SINGLE_PDN_DONE");
                    if (ar.exception == null) {
                        UtilLog.logd(TAG, "set single pdn :success");
                    } else {
                        UtilLog.loge(TAG, "set single pdn :fail" + ar.exception);
                    }
                    break;
                /* @} */
                case EVENT_REQUEST_SET_SPECIAL_RATCAP:
                    ar = (AsyncResult) msg.obj;
                    UtilLog.logd(TAG, "handleMessage EVENT_REQUEST_SET_SPECIAL_RATCAP");
                    if (ar.exception == null) {
                        UtilLog.logd(TAG, "set preferred network RAT:success");
                    } else {
                        UtilLog.loge(TAG, "set preferred network RAT:fail");
                    }
                    break;
                case EVENT_REQUEST_QUERY_COLP:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_REQUEST_QUERY_COLP");
                    synchronized (request) {
                        if (ar.exception == null && ar.result != null) {
                            int resultArray[] = (int[]) ar.result;
                            request.result = resultArray[0];
                        } else {
                            request.result = -1;
                            if (ar.exception != null) {
                                UtilLog.loge(TAG, "query colp fail: " + ar.exception);
                            }
                        }
                        request.notifyAll();
                    }
                    break;
                case EVENT_REQUEST_QUERY_COLR:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_REQUEST_QUERY_COLR");
                    synchronized (request) {
                        if (ar.exception == null && ar.result != null) {
                            int resultArray[] = (int[]) ar.result;
                            request.result = resultArray[0];
                        } else {
                            request.result = -1;
                            if (ar.exception != null) {
                                UtilLog.loge(TAG, "query colr fail: " + ar.exception);
                            }
                        }
                        request.notifyAll();
                    }
                    break;

                case EVENT_REQUEST_MMI_ENTER_SIM:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_REQUEST_MMI_ENTER_SIM");
                    synchronized (request) {
                        if (ar.exception == null) {
                            request.result = 0;
                            UtilLog.logd(TAG, "mmi enter sim: " + request.result);
                        } else {
                            if (ar.exception instanceof CommandException) {
                                CommandException.Error error = ((CommandException) (ar.exception))
                                        .getCommandError();
                                if (error == CommandException.Error.PASSWORD_INCORRECT) {
                                    request.result = 1;
                                } else {
                                    request.result = 2;
                                }
                            }else{
                                request.result = -1;
                            }
                            UtilLog.loge(TAG, "mmi enter sim:fail" +ar.exception);
                        }
                        request.notifyAll();
                    }
                    break;

                case EVENT_REQUEST_UPDATE_OPERATOR_NAME:
                    ar = (AsyncResult) msg.obj;
                    UtilLog.logd(TAG, "handleMessage EVENT_REQUEST_UPDATE_OPERATOR_NAME");
                    if (ar.exception == null) {
                        UtilLog.logd(TAG, "update operator name :success");
                    } else {
                        UtilLog.loge(TAG, "update operator name :fail" + ar.exception);
                    }
                    break;
                case EVENT_REATTACH_DONE:
                    ar = (AsyncResult) msg.obj;
                    UtilLog.logd(TAG, "handleMessage EVENT_REATTACH_DONE");
                    if (ar.exception == null) {
                        UtilLog.logd(TAG, "reattach :success");
                    } else {
                        UtilLog.loge(TAG, "reattach :fail" + ar.exception);
                    }
                    break;
                case EVENT_SET_SMS_BEARER_DONE:
                    ar = (AsyncResult) msg.obj;
                    UtilLog.logd(TAG, "handleMessage EVENT_SET_SMS_BEARER_DONE");
                    if (ar.exception == null) {
                        UtilLog.logd(TAG, "set sms bearer:success");
                    } else {
                        UtilLog.loge(TAG, "set sms bearer:fail");
                    }
                    break;
                case EVENT_GET_SIMLOCK_DUMMYS:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_GET_SIMLOCK_DUMMYS");
                    synchronized (request) {
                        if (ar.exception == null && ar.result != null) {
                            request.result = (int[])ar.result;
                        } else {
                            request.result = null;;
                            if (ar.result == null) {
                                UtilLog.loge(TAG, "get simlock dummys: Empty response");
                            }
                            if (ar.exception != null) {
                                UtilLog.loge(TAG,
                                        "get simlock dummys: Exception: " + ar.exception);
                            }
                        }
                        request.notifyAll();
                    }
                    break;
                case EVENT_GET_SIMLOCK_WHITE_LIST:
                    ar = (AsyncResult) msg.obj;
                    request = (ThreadRequest) ar.userObj;
                    UtilLog.logd(TAG, "handleMessage EVENT_GET_SIMLOCK_WHITE_LIST");
                    synchronized (request) {
                        if (ar.exception == null && ar.result != null) {
                            request.result = (String) ar.result;
                        } else {
                            request.result = "ERROR";
                            if (ar.result == null) {
                                UtilLog.loge(TAG, "get simlock white list: Empty response");
                            }
                            if (ar.exception != null) {
                                UtilLog.loge(TAG,
                                        "get simlock white list: Exception: " + ar.exception);
                            }
                        }
                        request.notifyAll();
                    }
                    break;
                case EVENT_REQUEST_SET_VOICE_DOMAIN:
                    ar = (AsyncResult) msg.obj;
                    UtilLog.logd(TAG, "handleMessage EVENT_REQUEST_SET_VOICE_DOMAIN");
                    if (ar.exception == null) {
                        UtilLog.logd(TAG, "set voice domain :success");
                    } else {
                        UtilLog.loge(TAG, "set voice domain :fail" + ar.exception);
                    }
                    break;
                default:
                    throw new RuntimeException("Unrecognized request event radiointeractor: " + msg.what);
            }
        }
    }

    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar;
        switch (msg.what) {

            case EVENT_UNSOL_RADIOINTERACTOR:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    UtilLog.logd(TAG, "EVENT_UNSOL_RADIOINTERACTOR");
                    mRadioInteractorNotifier
                            .notifyRadiointeractorEventForSubscriber(mRadioInteractorCore
                                    .getPhoneId());
                } else {
                    UtilLog.loge(TAG, "unsolicitedRegisters exception: " + ar.exception);
                }
                break;

            case EVENT_UNSOL_RI_CONNECTED:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    UtilLog.logd(TAG, "EVENT_UNSOL_RI_CONNECTED");
                } else {
                    UtilLog.loge(TAG, "unsolicitedRiConnected exception: " + ar.exception);
                }
                break;

            case EVENT_UNSOL_RADIOINTERACTOR_EMBMS:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    UtilLog.logd(TAG, "EVENT_UNSOL_RADIOINTERACTOR_EMBMS");
                    mRadioInteractorNotifier
                            .notifyRadiointeractorEventForEmbms(mRadioInteractorCore
                                    .getPhoneId());
                } else {
                    UtilLog.loge(TAG, "unsolicitedRadioInteractorEmbms exception: " + ar.exception);
                }
                break;

            case EVENT_SIMLOCK_STATUS_CHANGED:
                UtilLog.logd(TAG, "EVENT_SIMLOCK_STATUS_CHANGED");
                mRadioInteractorCore.getIccCardStatus(obtainMessage(EVENT_GET_ICC_STATUS_DONE));
                break;

            case EVENT_GET_ICC_STATUS_DONE:
                UtilLog.logd(TAG, "EVENT_GET_ICC_STATUS_DONE");
                ar = (AsyncResult) msg.obj;
                onGetIccCardStatusDone(ar);
                break;

            case EVENT_UNSOL_BAND_INFO:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null && ar.result != null) {
                    UtilLog.logd(TAG, "EVENT_UNSOL_BAND_INFO");
                    mRadioInteractorNotifier
                            .notifyRadiointeractorEventForbandInfo((String)ar.result,mRadioInteractorCore
                                    .getPhoneId());
                } else {
                    UtilLog.loge(TAG, "unsolicitedBandInfo exception: " + ar.exception);
                }
                break;

            case EVENT_UNSOL_SIMMGR_SIM_STATUS_CHANGED:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    UtilLog.logd(TAG, "EVENT_UNSOL_SIMMGR_SIM_STATUS_CHANGED - "
                            + mRadioInteractorCore.getPhoneId());
                    mRadioInteractorCore.simmgrGetSimStatus(obtainMessage(EVENT_GET_REALL_SIM_STATUS_DONE));
                } else {
                    UtilLog.loge(TAG, "unsolicitedRealSimStateChanged exception: " + ar.exception);
                }
                break;
            case EVENT_UNSOL_EXPIRE_SIM :
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null && ar.result != null) {
                    UtilLog.logd(TAG, "EVENT_UNSOL_SIMLOCK_SIM_EXPIRED ");
                    int result = (int) ar.result;
                    UtilLog.logd(TAG, "result = " + result);
                    mRadioInteractorNotifier.notifyExpireSimEvent(result,mRadioInteractorCore.getPhoneId());
                } else {
                    UtilLog.loge(TAG, "unsolicitedBandInfo exception: " + ar.exception);
                }
                break;

            case EVENT_GET_REALL_SIM_STATUS_DONE:
                ar = (AsyncResult) msg.obj;
                onGetRealIccCardStatusDone(ar);
                break;

            default:
                throw new RuntimeException("Unrecognized event unsol radiointeractor: " + msg.what);

        }
    }

    private synchronized void onGetRealIccCardStatusDone(AsyncResult ar) {
        UtilLog.logd(TAG, "onGetRealIccCardStatusDone");
        if (ar.exception != null) {
            UtilLog.loge(TAG,"Error getting ICC status. "
                    + "RIL_REQUEST_GET_ICC_STATUS should "
                    + "never return an error :" + ar.exception);
            return;
        }
        IccCardStatusEx ics = (IccCardStatusEx) ar.result;
        if (!ics.mCardState.equals(mCardState)) {
            mCardState = ics.mCardState;
            UtilLog.logd(TAG, "Notify real SIM state changed: " + mCardState);
            mRadioInteractorNotifier.notifyRealSimStateChanged(mRadioInteractorCore.getPhoneId());
        }
    }

    public void unsolicitedRegisters(Handler h, int what) {
        mRadioInteractorCore
                .registerForUnsolRadioInteractor(h, what, null);
    }

    public void unregisterForUnsolRadioInteractor(Handler h) {
        mRadioInteractorCore.unregisterForUnsolRadioInteractor(h);
    }

    public void registerForRiConnected(Handler h, int what) {
        mRadioInteractorCore.registerForUnsolRiConnected(h, what, null);
    }

    public void unregisterForRiConnected(Handler h) {
        mRadioInteractorCore.unregisterForUnsolRiConnected(h);
    }

    public void registerForRadioInteractorEmbms(Handler h, int what) {
        mRadioInteractorCore.registerForUnsolRadioInteractor(h, what, null);
    }

    public void unregisterForRadioInteractorEmbms(Handler h) {
        mRadioInteractorCore.unregisterForUnsolRadioInteractor(h);
    }

    public void registerForsetOnVPCodec(Handler h, int what) {
        mRadioInteractorCore.registerForsetOnVPCodec(h, what, null);
    }

    public void unregisterForsetOnVPCodec(Handler h) {
        mRadioInteractorCore.unregisterForsetOnVPCodec(h);
    }

    public void registerForsetOnVPFallBack(Handler h, int what) {
        mRadioInteractorCore.registerForsetOnVPFallBack(h, what, null);
    }

    public void unregisterForsetOnVPFallBack(Handler h) {
        mRadioInteractorCore.unregisterForsetOnVPFallBack(h);
    }

    public void registerForsetOnVPString(Handler h, int what) {
        mRadioInteractorCore.registerForsetOnVPString(h, what, null);
    }

    public void unregisterForsetOnVPString(Handler h) {
        mRadioInteractorCore.unregisterForsetOnVPString(h);
    }

    public void registerForsetOnVPRemoteMedia(Handler h, int what) {
        mRadioInteractorCore.registerForsetOnVPRemoteMedia(h, what, null);
    }

    public void unregisterForsetOnVPRemoteMedia(Handler h) {
        mRadioInteractorCore.unregisterForsetOnVPRemoteMedia(h);
    }

    public void registerForsetOnVPMMRing(Handler h, int what) {
        mRadioInteractorCore.registerForsetOnVPMMRing(h, what, null);
    }

    public void unregisterForsetOnVPMMRing(Handler h) {
        mRadioInteractorCore.unregisterForsetOnVPMMRing(h);
    }

    public void registerForsetOnVPFail(Handler h, int what) {
        mRadioInteractorCore.registerForsetOnVPFail(h, what, null);
    }

    public void unregisterForsetOnVPFail(Handler h) {
        mRadioInteractorCore.unregisterForsetOnVPFail(h);
    }

    public void registerForsetOnVPRecordVideo(Handler h, int what) {
        mRadioInteractorCore.registerForsetOnVPRecordVideo(h, what, null);
    }

    public void unregisterForsetOnVPRecordVideo(Handler h) {
        mRadioInteractorCore.unregisterForsetOnVPRecordVideo(h);
    }

    public void registerForsetOnVPMediaStart(Handler h, int what) {
        mRadioInteractorCore.registerForsetOnVPMediaStart(h, what, null);
    }

    public void unregisterForsetOnVPMediaStart(Handler h) {
        mRadioInteractorCore.unregisterForsetOnVPMediaStart(h);
    }

    public void registerForEccNetChanged(Handler h, int what) {
        mRadioInteractorCore.registerForEccNetChanged(h, what, null);
    }

    public void unregisterForEccNetChanged(Handler h) {
        mRadioInteractorCore.unregisterForEccNetChanged(h);
    }

    public void registerForRauSuccess(Handler h, int what) {
        mRadioInteractorCore.registerForRauSuccess(h, what, null);
    }

    public void unregisterForRauSuccess(Handler h) {
        mRadioInteractorCore.unregisterForRauSuccess(h);
    }

    public void registerForClearCodeFallback(Handler h, int what) {
        mRadioInteractorCore.registerForClearCodeFallback(h, what, null);
    }

    public void unregisterForClearCodeFallback(Handler h) {
        mRadioInteractorCore.unregisterForClearCodeFallback(h);
    }

    public void registerForBandInfo(Handler h, int what) {
        mRadioInteractorCore.registerForBandInfo(h, what, null);
    }

    public void unregisterForBandInfo(Handler h) {
        mRadioInteractorCore.unregisterForBandInfo(h);
    }
    public void registerForSwitchPrimaryCard(Handler h, int what) {
        mRadioInteractorCore.registerForSwitchPrimaryCard(h, what, null);
    }

    public void unregisterForSwitchPrimaryCard(Handler h) {
        mRadioInteractorCore.unregisterForSwitchPrimaryCard(h);
    }

    public void registerForRealSimStateChanged(Handler h,int what) {
        mRadioInteractorCore.registerForRealSimStateChanged(h,what,null);
    }

    public void unregisterForRealSimStateChanged(Handler h) {
        mRadioInteractorCore.unregisterForRealSimStateChanged(h);
    }

    public void registerForRadioCapabilityChanged(Handler h,int what) {
        mRadioInteractorCore.registerForRadioCapabilityChanged(h,what,null);
    }

    public void unregisterForRadioCapabilityChanged(Handler h) {
        mRadioInteractorCore.unregisterForRadioCapabilityChanged(h);
    }

    private static final class ThreadRequest {
        public Object argument;
        public Object result;

        public ThreadRequest(Object argument) {
            this.argument = argument;
        }
    }

    private void waitForResult(ThreadRequest request) {
        try {
            request.wait();
        } catch (InterruptedException e) {
            UtilLog.logd(TAG, "interrupted while trying to get remain times");
        }
    }

    /**
     * Some fields (like ICC ID) in GSM SIMs are stored as nibble-swizzled BCH
     */
    private String
    bchToString(byte[] data, int offset, int length) {
        StringBuilder ret = new StringBuilder(length*2);

        for (int i = offset ; i < offset + length ; i++) {
            int v;

            v = data[i] & 0xf;
            ret.append("0123456789abcdef".charAt(v));

            v = (data[i] >> 4) & 0xf;
            ret.append("0123456789abcdef".charAt(v));
        }

        return ret.toString();
    }

    private SuppService getFailedService(int what) {
        switch (what) {
            case EVENT_ECT_RESULT:
                return SuppService.TRANSFER;
        }
        return SuppService.UNKNOWN;
    }

    public void setFacilityLockByUser(String facility, boolean lockState, Message response) {
         mRadioInteractorCore.setFacilityLockForUser(facility, lockState, response);
    }

    public int getSimLockRemainTimes(int type) {
        ThreadRequest request = new ThreadRequest("getSimLockRemainTimes");

        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_GET_REMIAN_TIMES_DONE,
                    request);

            mRadioInteractorCore.getSimlockRemaintimes(type, response);
            waitForResult(request);
        }

        try {
            return ((int[])request.result)[0];
        } catch (ClassCastException e) {
            return -1;
        }
    }

    public int getSimLockStatus(int type) {
        ThreadRequest request = new ThreadRequest("getSimLockStatus");

        synchronized (request) {
            Message response = mHandler.obtainMessage(EVENT_GET_SIMLOCK_STATUS_DONE,
                    request);

            mRadioInteractorCore.getSimlockStatus(type, response);
            waitForResult(request);
        }

        try {
            return ((int[])request.result)[0];
        } catch (ClassCastException e) {
            return -1;
        }
    }

    private synchronized void onGetIccCardStatusDone(AsyncResult ar) {
        if (ar.exception != null) {
            UtilLog.loge(TAG,"Error getting ICC status. "
                    + "RIL_REQUEST_GET_ICC_STATUS should "
                    + "never return an error :" + ar.exception);
            return;
        }
        IccCardStatusEx ics = (IccCardStatusEx)ar.result;
        if (ics.mApplications.length == 0) {
            return;
        }
        IccCardApplicationStatusEx icas = ics.mApplications[0];
        UtilLog.logd(TAG,"result =  "  + icas.perso_substate);

        mAppState = icas.app_state;
        PersoSubState oldPersoSubState = mPersoSubState;
        mPersoSubState = icas.perso_substate;
        notifySimLockedRegistrantsIfNeeded(null);
    }

    private void notifySimLockedRegistrantsIfNeeded(Registrant r) {
        if (mAppState == AppState.APPSTATE_SUBSCRIPTION_PERSO) {
            if (r == null) {
                int simlockType =  getSimlockTypes(mPersoSubState);
                if (simlockType != 0) {
                    mPersonalisationLockedRegistrants.notifyRegistrants(
                            new AsyncResult(mRadioInteractorCore.getPhoneId(),simlockType,null));
                }
            } else {
                r.notifyRegistrant(new AsyncResult(null, null, null));
            }
        }
    }

    public int getSimlockTypes(PersoSubState persoSubState ) {
       int simlockType= 0;
       if (persoSubState == PersoSubState.PERSOSUBSTATE_SIM_NETWORK) {
            UtilLog.logd(TAG, "Notifying registrants: NetworkLocked");
            simlockType = TelephonyManager.SIM_STATE_NETWORK_LOCKED;
       } else if (persoSubState == PersoSubState.PERSOSUBSTATE_SIM_NETWORK_SUBSET) {
            UtilLog.logd(TAG, "Notifying registrants: NetworkSubsetLocked");
            simlockType = IccCardStatusEx.SIM_STATE_NETWORKSUBSET_LOCKED;
        } else if (persoSubState == PersoSubState.PERSOSUBSTATE_SIM_SERVICE_PROVIDER) {
            UtilLog.logd(TAG, "Notifying registrants: ServiceProviderLocked");
            simlockType = IccCardStatusEx.SIM_STATE_SERVICEPROVIDER_LOCKED;
        } else if (persoSubState == PersoSubState.PERSOSUBSTATE_SIM_CORPORATE) {
            UtilLog.logd(TAG, "Notifying registrants: corporateLocked");
            simlockType = IccCardStatusEx.SIM_STATE_CORPORATE_LOCKED;
        } else if (persoSubState == PersoSubState.PERSOSUBSTATE_SIM_SIM){
            UtilLog.logd(TAG, "Notifying registrants: simLocked");
            simlockType = IccCardStatusEx.SIM_STATE_SIM_LOCKED;
        }  else if(persoSubState == PersoSubState.PERSOSUBSTATE_SIM_NETWORK_PUK) {
            UtilLog.logd(TAG, "Notifying registrants: NetworkLocked puk");
            simlockType = IccCardStatusEx.SIM_STATE_NETWORK_LOCKED_PUK;
        }  else if(persoSubState == PersoSubState.PERSOSUBSTATE_SIM_NETWORK_SUBSET_PUK) {
            UtilLog.logd(TAG, "Notifying registrants: NetworkSubsetLocked puk");
            simlockType = IccCardStatusEx.SIM_STATE_NETWORK_SUBSET_LOCKED_PUK;
        }  else if(persoSubState == PersoSubState.PERSOSUBSTATE_SIM_CORPORATE_PUK) {
            UtilLog.logd(TAG, "Notifying registrants: corporateLocked puk");
            simlockType = IccCardStatusEx.SIM_STATE_CORPORATE_LOCKED_PUK;
        }  else if(persoSubState == PersoSubState.PERSOSUBSTATE_SIM_SERVICE_PROVIDER_PUK) {
            UtilLog.logd(TAG, "Notifying registrants: ServiceProviderLocked puk");
            simlockType = IccCardStatusEx.SIM_STATE_SERVICE_PROVIDER_LOCKED_PUK;
        }  else if(persoSubState == PersoSubState.PERSOSUBSTATE_SIM_SIM_PUK) {
            UtilLog.logd(TAG, "Notifying registrants: simLocked puk");
            simlockType = IccCardStatusEx.SIM_STATE_SIM_LOCKED_PUK;
        }  else if(persoSubState == PersoSubState.PERSOSUBSTATE_SIM_LOCK_FOREVER) {
            UtilLog.logd(TAG, "Notifying registrants: simlock forever");
            simlockType = IccCardStatusEx.SIM_STATE_SIM_LOCKED_FOREVER;
        }else{
            simlockType = 0;
        }
       return simlockType;
    }

    private int parsePinPukErrorResultEx(AsyncResult ar) {
        int[] result = (int[]) ar.result;
        if (result == null) {
            return -1;
        } else {
            int length = result.length;
            int attemptsRemaining = -1;
            if (length > 0) {
                attemptsRemaining = result[0];
            }
            UtilLog.logd(TAG, "parsePinPukErrorResult: attemptsRemaining=" + attemptsRemaining);
            return attemptsRemaining;
        }
    }

    public void registerForSimlockStatusChanged(Handler h, int what) {
        mRadioInteractorCore.registerForSimlockStatusChanged(h, what, null);
    }

    public void unregisterForSimlockStatusChanged(Handler h) {
        mRadioInteractorCore.unregisterForSimlockStatusChanged(h);
    }

    public synchronized void registerForPersonalisationLocked(Handler h, int what, Object obj) {
        Registrant r = new Registrant (h, what, obj);
        mPersonalisationLockedRegistrants.add(r);
        notifySimLockedRegistrantsIfNeeded(null);
    }

    public synchronized void unregisterForPersonalisationLocked(Handler h) {
        mPersonalisationLockedRegistrants.remove(h);
    }

    public void registerForExpireSim(Handler h, int what) {
        mRadioInteractorCore.registerForExpireSim(h, what, null);
    }

    public void unregisterForExpireSim(Handler h) {
        mRadioInteractorCore.unregisterForExpireSim(h);
    }

    public void getSimStatus() {
        mRadioInteractorCore.getIccCardStatus(obtainMessage(EVENT_GET_ICC_STATUS_DONE));
    }

    /**
     * Make sure the caller has the SPRD_MODIFY_PHONE_STATE permission.
     * @throws SecurityException if the caller does not have the required permission
     */
    private void enforceSprdModifyPermission(String packageName) {
        int resultOfCheck = mContext.getPackageManager()
                .checkPermission(android.Manifest.permission.SPRD_MODIFY_PHONE_STATE, packageName);
        if (resultOfCheck != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(
                    "Neither user " + Binder.getCallingUid() + " nor current process has " +
                            android.Manifest.permission.SPRD_MODIFY_PHONE_STATE +
                            ".");
        }
    }
}
