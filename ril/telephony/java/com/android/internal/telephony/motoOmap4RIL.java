/*
 * Copyright (C) 2014 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.internal.telephony;

import static com.android.internal.telephony.RILConstants.*;

import android.content.Context;
import android.media.AudioManager;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.ServiceManager;
import android.telephony.SmsMessage;
import android.os.SystemProperties;
import android.os.SystemClock;
import android.provider.Settings;
import android.text.TextUtils;
import android.telephony.Rlog;
import android.net.NetworkUtils;
import android.net.InterfaceConfiguration;
import android.net.LinkAddress;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.io.BufferedReader;
import java.io.InputStreamReader;

import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.gsm.SmsBroadcastConfigInfo;
import com.android.internal.telephony.cdma.CdmaSmsBroadcastConfigInfo;
import com.android.internal.telephony.dataconnection.DataCallResponse;

public class motoOmap4RIL extends RIL implements CommandsInterface {

    private INetworkManagementService mNwService;
    private boolean initialAttachApnSeen = false;
    private String voiceRegState = "0";
    private String voiceDataTech = "0";

    public motoOmap4RIL(Context context, int preferredNetworkType, int cdmaSubscription) {
        super(context, preferredNetworkType, cdmaSubscription);

        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        mNwService = INetworkManagementService.Stub.asInterface(b);
    }

    public motoOmap4RIL(Context context, int preferredNetworkType,
                    int cdmaSubscription, Integer instanceId) {
        super(context, preferredNetworkType, cdmaSubscription, instanceId);

        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        mNwService = INetworkManagementService.Stub.asInterface(b);
    }

    @Override
    public void getGsmBroadcastConfig(Message response) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: getGsmBroadcastConfig");

        if (response != null) {
            CommandException e = new CommandException(CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(response, null, e);
            response.sendToTarget();
        }
    }

    @Override
    public void setGsmBroadcastConfig(SmsBroadcastConfigInfo[] config, Message response) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: setGsmBroadcastConfig");

        if (response != null) {
            CommandException e = new CommandException(CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(response, null, e);
            response.sendToTarget();
        }
    }

    @Override
    public void setGsmBroadcastActivation(boolean activate, Message response) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: setGsmBroadcastActivation");

        if (response != null) {
            CommandException e = new CommandException(CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(response, null, e);
            response.sendToTarget();
        }
    }

    @Override
    public void getCdmaBroadcastConfig(Message response) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: getCdmaBroadcastConfig");

        if (response != null) {
            CommandException e = new CommandException(CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(response, null, e);
            response.sendToTarget();
        }
    }

    @Override
    public void setCdmaBroadcastConfig(CdmaSmsBroadcastConfigInfo[] configs, Message response) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: setCdmaBroadcastConfig");

        if (response != null) {
            CommandException e = new CommandException(CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(response, null, e);
            response.sendToTarget();
        }
    }

    @Override
    public void setCdmaBroadcastActivation(boolean activate, Message response) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: setCdmaBroadcastActivation");

        if (response != null) {
            CommandException e = new CommandException(CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(response, null, e);
            response.sendToTarget();
        }
    }

    @Override
    public void getCellInfoList(Message result) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: getCellInfoList");

        if (result != null) {
            CommandException e = new CommandException(CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(result, null, e);
            result.sendToTarget();
        }
    }

    @Override
    public void setDataAllowed(boolean allowed, Message result) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: setDataAllowed");

        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: faking VoiceNetworkState");
        mVoiceNetworkStateRegistrants.notifyRegistrants(new AsyncResult(null, null, null));

        if (result != null) {
            CommandException e = new CommandException(CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(result, null, e);
            result.sendToTarget();
        }
    }

    @Override
    public void setCellInfoListRate(int rateInMillis, Message response) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: setCellInfoListRate");

        if (response != null) {
            CommandException e = new CommandException(CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(response, null, e);
            response.sendToTarget();
        }
    }

    @Override
    public void startLceService(int reportIntervalMs, boolean pullMode, Message response) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: startLceService");

        if (response != null) {
            CommandException e = new CommandException(CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(response, null, e);
            response.sendToTarget();
        }
    }

    @Override
    public void stopLceService(Message response) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: stopLceService");

        if (response != null) {
            CommandException e = new CommandException(CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(response, null, e);
            response.sendToTarget();
        }
    }

    @Override
    public void getRadioCapability(Message response) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: getRadioCapability");

        if (response != null) {
            CommandException e = new CommandException(CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(response, null, e);
            response.sendToTarget();
        }
    }

    public void getImsRegistrationState(Message result) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: getImsRegistrationState");

        if (result != null) {
            CommandException e = new CommandException(CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(result, null, e);
            result.sendToTarget();
        }
    }

    @Override
    public void getHardwareConfig (Message result) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: getHardwareConfig");

        if (result != null) {
            CommandException e = new CommandException(CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(result, null, e);
            result.sendToTarget();
        }
    }


    @Override
    public void setupDataCall(String radioTechnology, String profile, String apn,
            String user, String password, String authType, String protocol,
            Message result) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: setupDataCall");

        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: faking VoiceNetworkState");
        mVoiceNetworkStateRegistrants.notifyRegistrants(new AsyncResult(null, null, null));

        super.setupDataCall(radioTechnology, profile, apn, user, password, authType, protocol, result);
    }

    @Override
    public void setInitialAttachApn(String apn, String protocol, int authType, String username,
            String password, Message result) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: setInitialAttachApn");

        initialAttachApnSeen = true;

        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: faking VoiceNetworkState");
        mVoiceNetworkStateRegistrants.notifyRegistrants(new AsyncResult(null, null, null));

        if (result != null) {
            AsyncResult.forMessage(result, null, null);
            result.sendToTarget();
        }
    }

    @Override
    protected DataCallResponse getDataCallResponse(Parcel p, int version) {
        DataCallResponse dataCall = new DataCallResponse();
        int dataPosition = p.dataPosition();

        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: getDataCallRespone");

        if (version >= 5) {
            dataCall.status = p.readInt();
            dataCall.suggestedRetryTime = p.readInt();
            dataCall.cid = p.readInt();
            dataCall.active = p.readInt();
            dataCall.type = p.readString();
            dataCall.ifname = p.readString();
            String addrs = p.readString();
            String dnses = p.readString();
            String gws = p.readString();

            String gateways[] = null;
            if (gws != null) {
                Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: GW: " + gws);
                gateways = gws.split(" ");
            }

            if (dataCall.ifname != null) {
                Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: IF: " + dataCall.ifname);
            }

            InetAddress gw = null;
            if (dataCall.ifname != null && gateways != null && gateways.length > 0) {
                for (String a : gateways) {
                    a = a.trim();
                    if (a.isEmpty()) continue;

                    try {
                        gw = NetworkUtils.numericToInetAddress(a);
                    } catch (Exception e) {
                        Rlog.w(RILJ_LOG_TAG, "motoOmap4RIL: can't parse " + a + ": " + e);
                    }

                    if (gw != null && (gw instanceof Inet4Address)) {
                        try {
                            String cmd[] = {"/system/bin/motorilc", dataCall.ifname, a};
                            Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: executing motorilc");
                            ProcessBuilder motorilcPb = new ProcessBuilder(cmd);
                            Process motorilc  = motorilcPb.start();
                            BufferedReader motorilcOut = new BufferedReader(new InputStreamReader(motorilc.getInputStream()));
                            String l;
                            while ((l = motorilcOut.readLine()) != null) {
                                Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: " + l);
                            }
                            motorilc.waitFor();
                        } catch (Exception e) {
                            Rlog.e(RILJ_LOG_TAG, "motoOmap4RIL: Can't add host-route: " + e);
                        }
                        break;
                    } else {
                        gw = null;
                    }
                }
            }
        }

        p.setDataPosition(dataPosition);
        return super.getDataCallResponse(p, version);
    }

    protected Object responseIccCardStatus(Parcel p) {
        int dataPosition = p.dataPosition(); // save off position within the Parcel

        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: responseIccCardStatus");

        int cardState = p.readInt(); //CardState
        if (cardState == 1) { //Present
            p.readInt(); //PinState
            int gsmIndex = p.readInt();
            int cdmaIndex = p.readInt();

            if ((cdmaIndex == -1) && (gsmIndex != -1)) {
                //No CDMA application on card, but GSM/UMTS present
                if (getLteOnCdmaMode() == PhoneConstants.LTE_ON_CDMA_TRUE) {
                    Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: disabling telephony.lteOnCdmaDevice");
                    SystemProperties.set("telephony.lteOnCdmaDevice", "0");

                    Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: faking VoiceNetworkState");
                    mVoiceNetworkStateRegistrants.notifyRegistrants(new AsyncResult(null, null, null));
                    Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: faking VoiceRadioTech");
                    if (mVoiceRadioTechChangedRegistrants != null) {
                        String tech[] = { voiceDataTech };
                        mVoiceRadioTechChangedRegistrants.notifyRegistrants(new AsyncResult(null, tech, null));
                    }
                }
            }
        }

        p.setDataPosition(dataPosition);
        return super.responseIccCardStatus(p);
    }

    @Override
    protected RILRequest
    processSolicited (Parcel p) {
        int serial, error, request;
        RILRequest rr;
        int dataPosition = p.dataPosition(); // save off position within the Parcel

        serial = p.readInt();
        error = p.readInt();

        rr = mRequestList.get(serial);
        if (rr == null || error != 0 || p.dataAvail() <= 0) {
            p.setDataPosition(dataPosition);
            return super.processSolicited(p);
        }

        try { switch (rr.mRequest) {
            case RIL_REQUEST_OPERATOR:
                String operators[] = (String [])responseStrings(p);

                Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: Operator response");

                /* LTE information available? */
                if (operators.length >= 6) {
                    if ((operators[5] != null) &&
                        (!(operators[5].equals("00000"))) &&
                        (!(operators[5].equals("6553565535")))) {

                        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: fixing OPERATOR, original value from ril: " +
                            retToString(rr.mRequest, operators));
                        for (int i = 3; i < operators.length; i++) {
                            operators[i-3] = operators[i];
                            operators[i] = null;
                        }
                    }
                }

                if (RILJ_LOGD) riljLog(rr.serialString() + "< " + requestToString(rr.mRequest)
                                + " " + retToString(rr.mRequest, operators));

                if (rr.mResult != null) {
                        AsyncResult.forMessage(rr.mResult, operators, null);
                        rr.mResult.sendToTarget();
                }
                mRequestList.remove(serial);
                break;
            case RIL_REQUEST_VOICE_REGISTRATION_STATE:
                String voiceRegStates[] = (String [])responseStrings(p);

                Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: VoiceRegistrationState response");

                if (voiceRegStates.length > 0 && voiceRegStates[0] != null) {
                    voiceRegState = voiceRegStates[0];
                }

                if (voiceRegStates.length > 3 && voiceRegStates[3] != null) {
                    voiceDataTech = voiceRegStates[3];
                }

                if (RILJ_LOGD) riljLog(rr.serialString() + "< " + requestToString(rr.mRequest)
                                + " " + retToString(rr.mRequest, voiceRegStates));

                if (rr.mResult != null) {
                        AsyncResult.forMessage(rr.mResult, voiceRegStates, null);
                        rr.mResult.sendToTarget();
                }
                mRequestList.remove(serial);
                break;
            case RIL_REQUEST_DATA_REGISTRATION_STATE:
                String dataRegStates[] = (String [])responseStrings(p);

                Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: DataRegistrationState response");

                if (dataRegStates.length > 0) {
                    if (dataRegStates[0] != null) {
                        if (!initialAttachApnSeen) {
                            if (Integer.parseInt(dataRegStates[0]) > 0) {
                                Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: modifying dataRegState to 0 from " + dataRegStates[0]);
                                dataRegStates[0] = "0";
                            }
                        } else {
                            if ((Integer.parseInt(dataRegStates[0]) != 1) && (Integer.parseInt(dataRegStates[0]) != 5) &&
                                ((Integer.parseInt(voiceRegState) == 1) || (Integer.parseInt(voiceRegState) == 5))) {
                                Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: modifying dataRegState from " + dataRegStates[0] + " to " + voiceRegState);
                                dataRegStates[0] = voiceRegState;
                                if (dataRegStates.length > 3) {
                                    Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: modifying dataTech from " + dataRegStates[3] + " to " + voiceDataTech);
                                    dataRegStates[3] = voiceDataTech;
                                }
                            }
                        }
                    }
                }

                if (RILJ_LOGD) riljLog(rr.serialString() + "< " + requestToString(rr.mRequest)
                                + " " + retToString(rr.mRequest, dataRegStates));

                if (rr.mResult != null) {
                        AsyncResult.forMessage(rr.mResult, dataRegStates, null);
                        rr.mResult.sendToTarget();
                }
                mRequestList.remove(serial);
                break;
            default:
                p.setDataPosition(dataPosition);
                return super.processSolicited(p);
        }} catch (Throwable tr) {
                // Exceptions here usually mean invalid RIL responses

                Rlog.w(RILJ_LOG_TAG, rr.serialString() + "< "
                                + requestToString(rr.mRequest)
                                + " exception, possible invalid RIL response", tr);

                if (rr.mResult != null) {
                        AsyncResult.forMessage(rr.mResult, null, tr);
                        rr.mResult.sendToTarget();
                }
                return rr;
        }

        return rr;
    }

    @Override
    protected void
    processUnsolicited (Parcel p) {
        int dataPosition = p.dataPosition(); // save off position within the Parcel
        int response;

        response = p.readInt();

        switch(response) {
            case RIL_UNSOL_CALL_RING:
                try {
                    String cmd[] = {"/system/bin/motorilc", "ring"};
                    Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: executing motorilc ring");
                    ProcessBuilder motorilcPb = new ProcessBuilder(cmd);
                    Process motorilc  = motorilcPb.start();
                    BufferedReader motorilcOut = new BufferedReader(new InputStreamReader(motorilc.getInputStream()));
                    String l;
                    while ((l = motorilcOut.readLine()) != null) {
                        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: " + l);
                    }
                    motorilc.waitFor();
                } catch (Exception e) {
                    Rlog.e(RILJ_LOG_TAG, "motoOmap4RIL: Can't send ring-indication");
                }

                break;
        }

        p.setDataPosition(dataPosition);
        super.processUnsolicited(p);
    }
}
