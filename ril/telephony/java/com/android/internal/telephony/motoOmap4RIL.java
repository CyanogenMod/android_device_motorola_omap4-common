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
    public void setupDataCall(String radioTechnology, String profile, String apn,
            String user, String password, String authType, String protocol,
            Message result) {
        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: setupDataCall");

        Rlog.v(RILJ_LOG_TAG, "motoOmap4RIL: faking VoiceNetworkState");
        mVoiceNetworkStateRegistrants.notifyRegistrants(new AsyncResult(null, null, null));

        super.setupDataCall(radioTechnology, profile, apn, user, password, authType, protocol, result);
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
}
