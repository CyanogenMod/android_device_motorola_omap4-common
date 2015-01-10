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
import android.telephony.SmsMessage;
import android.os.SystemProperties;
import android.os.SystemClock;
import android.provider.Settings;
import android.text.TextUtils;
import android.telephony.Rlog;

import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.gsm.SmsBroadcastConfigInfo;
import com.android.internal.telephony.cdma.CdmaSmsBroadcastConfigInfo;

public class motoOmap4RIL extends RIL implements CommandsInterface {

    public motoOmap4RIL(Context context, int preferredNetworkType, int cdmaSubscription) {
        super(context, preferredNetworkType, cdmaSubscription);
    }

    public motoOmap4RIL(Context context, int preferredNetworkType,
                    int cdmaSubscription, Integer instanceId) {
        super(context, preferredNetworkType, cdmaSubscription, instanceId);
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
}
