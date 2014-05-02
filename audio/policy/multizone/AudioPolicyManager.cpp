/*
 * Copyright (C) 2013 Texas Instruments
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
 *
 * This module is based on libhardware_legacy implementation.
 *
 */

#define LOG_TAG "AudioPolicyManager"
// #define LOG_NDEBUG 0
// #define VERY_VERBOSE_LOGGING
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

// A device mask for all audio input devices that are considered "virtual" when evaluating
// active inputs in getActiveInput()
#define APM_AUDIO_IN_DEVICE_VIRTUAL_ALL  AUDIO_DEVICE_IN_REMOTE_SUBMIX
// A device mask for all audio output devices that are considered "remote" when evaluating
// active output devices in isStreamActiveRemotely()
#define APM_AUDIO_OUT_DEVICE_REMOTE_ALL  AUDIO_DEVICE_OUT_REMOTE_SUBMIX

#include <cutils/properties.h>
#include <utils/Log.h>
#include <hardware/audio_effect.h>
#include <hardware/audio.h>
#include <math.h>
#include <hardware_legacy/audio_policy_conf.h>

#include "AudioPolicyManager.h"

namespace android_audio_legacy {

// ----------------------------------------------------------------------------
// AudioPolicyInterface implementation
// ----------------------------------------------------------------------------

status_t AudioPolicyManager::setDeviceConnectionState(audio_devices_t device,
                                                      AudioSystem::device_connection_state state,
                                                      const char *device_address)
{
    ALOGI("setDeviceConnectionState() device 0x%08x state %d address %s",
          device, state, device_address);

    // connect/disconnect only 1 device at a time
    if (!audio_is_output_device(device) && !audio_is_input_device(device)) {
        return BAD_VALUE;
    }

    if (strlen(device_address) >= MAX_DEVICE_ADDRESS_LEN) {
        ALOGE("setDeviceConnectionState() invalid address: %s", device_address);
        return BAD_VALUE;
    }

    // handle output devices
    if (audio_is_output_device(device)) {

        if (!mHasA2dp && audio_is_a2dp_device(device)) {
            ALOGE("setDeviceConnectionState() invalid A2DP device: 0x%08x", device);
            return BAD_VALUE;
        }
        if (!mHasUsb && audio_is_usb_device(device)) {
            ALOGE("setDeviceConnectionState() invalid USB audio device: 0x%08x", device);
            return BAD_VALUE;
        }
        if (!mHasRemoteSubmix && audio_is_remote_submix_device((audio_devices_t)device)) {
            ALOGE("setDeviceConnectionState() invalid remote submix audio device: 0x%08x",
                  device);
            return BAD_VALUE;
        }

        // HEADPHONE is mapped to a listening zone, discard its input
        if (device == AUDIO_DEVICE_OUT_WIRED_HEADSET) {
            device = AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
        }

        switch (state) {

        // handle output device connection
        case AudioSystem::DEVICE_STATE_AVAILABLE: {
            if (mAvailableOutputDevices & device) {
                ALOGW("setDeviceConnectionState() device already connected 0x%08x", device);
                return INVALID_OPERATION;
            }

            // register new device as available
            ALOGV("setDeviceConnectionState() connecting device 0x%08x", device);
            mAvailableOutputDevices = (audio_devices_t)(mAvailableOutputDevices | device);

            if (mHasA2dp && audio_is_a2dp_device(device)) {
                mA2dpDeviceAddress = String8(device_address, MAX_DEVICE_ADDRESS_LEN);
                mA2dpSuspended = false;
            } else if (audio_is_bluetooth_sco_device(device)) {
                mScoDeviceAddress = String8(device_address, MAX_DEVICE_ADDRESS_LEN);
            } else if (mHasUsb && audio_is_usb_device(device)) {
                mUsbCardAndDevice = String8(device_address, MAX_DEVICE_ADDRESS_LEN);
            }

            if ((device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) ||
                (device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE2)) {
                ALOGV("setDeviceConnectionState() headphone inserted (0x%08x), unmuting",
                      device);
                audio_io_handle_t output = findOutput(device);
                AudioOutputDescriptor *outputDesc = mOutputs.valueFor(output);
                // Mute/unmute is done only if the heaphone is the only device
                // handled by the output
                if (output && (outputDesc->device() == device))
                    muteOutput(output, false);
            }
        } break;

        // handle output device disconnection
        case AudioSystem::DEVICE_STATE_UNAVAILABLE: {
            if (!(mAvailableOutputDevices & device)) {
                ALOGW("setDeviceConnectionState() device not connected: 0x08%x", device);
                return INVALID_OPERATION;
            }

            // remove device from available output devices
            ALOGV("setDeviceConnectionState() disconnecting device 0x%08x", device);
            mAvailableOutputDevices = (audio_devices_t)(mAvailableOutputDevices & ~device);

            if (mHasA2dp && audio_is_a2dp_device(device)) {
                mA2dpDeviceAddress = "";
                mA2dpSuspended = false;
            } else if (audio_is_bluetooth_sco_device(device)) {
                mScoDeviceAddress = "";
            } else if (mHasUsb && audio_is_usb_device(device)) {
                mUsbCardAndDevice = "";
            }
            // not currently handling multiple simultaneous submixes: ignoring remote submix
            // case and address

            // HP1 and HP2 event don't cause the dynamic re-routing, just mute to
            // prevent high-volume when the headphone is connected again
            if ((device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) ||
                (device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE2)) {
                ALOGV("setDeviceConnectionState() headphone removed (0x%08x), muting",
                      device);
                audio_io_handle_t output = findOutput(device);
                AudioOutputDescriptor *outputDesc = mOutputs.valueFor(output);
                // Mute/unmute is done only if the heaphone is the only device
                // handled by the output
                if (output && (outputDesc->device() == device))
                    muteOutput(output, true);
            } else {
                disconnectDevice(device);
            }

        } break;

        default:
            ALOGE("setDeviceConnectionState() invalid state: %x", state);
            return BAD_VALUE;
        }

        checkA2dpSuspend();

        // We don't do dynamic routing based on new devices being attached

        // Also add its input device
        if ((device == AUDIO_DEVICE_OUT_BLUETOOTH_SCO) ||
            (device == AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET) ||
            (device == AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT)) {
            device = AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET;
        } else {
            return NO_ERROR;
        }
    }

    // handle input devices
    if (audio_is_input_device(device)) {

        switch (state) {

        // handle input device connection
        case AudioSystem::DEVICE_STATE_AVAILABLE: {
            if (mAvailableInputDevices & device) {
                ALOGW("setDeviceConnectionState() device already connected: 0x%08x", device);
                return INVALID_OPERATION;
            }
            ALOGV("setDeviceConnectionState() connecting device 0x%08x", device);
            mAvailableInputDevices = mAvailableInputDevices | (device & ~AUDIO_DEVICE_BIT_IN);
        } break;

        // handle input device disconnection
        case AudioSystem::DEVICE_STATE_UNAVAILABLE: {
            if (!(mAvailableInputDevices & device)) {
                ALOGW("setDeviceConnectionState() device not connected: 0x%08x", device);
                return INVALID_OPERATION;
            }
            ALOGV("setDeviceConnectionState() disconnecting device 0x%08x", device);
            mAvailableInputDevices = (audio_devices_t) (mAvailableInputDevices & ~device);
        } break;

        default:
            ALOGE("setDeviceConnectionState() invalid state: %x", state);
            return BAD_VALUE;
        }

        audio_io_handle_t activeInput = getActiveInput();
        if (activeInput != 0) {
            AudioInputDescriptor *inputDesc = mInputs.valueFor(activeInput);
            audio_devices_t newDevice = getDeviceForInputSource(inputDesc->mInputSource);
            if ((newDevice != AUDIO_DEVICE_NONE) && (newDevice != inputDesc->mDevice)) {
                ALOGV("setDeviceConnectionState() changing device from 0x%x to 0x%x for input %d",
                      inputDesc->mDevice, newDevice, activeInput);
                inputDesc->mDevice = newDevice;
                AudioParameter param = AudioParameter();
                param.addInt(String8(AudioParameter::keyRouting), (int)newDevice);
                mpClientInterface->setParameters(activeInput, param.toString());
            }
        }

        return NO_ERROR;
    }

    ALOGW("setDeviceConnectionState() invalid device: %x", device);
    return BAD_VALUE;
}

AudioSystem::device_connection_state
AudioPolicyManager::getDeviceConnectionState(audio_devices_t device,
                                             const char *device_address)
{
    AudioSystem::device_connection_state state = AudioSystem::DEVICE_STATE_UNAVAILABLE;
    String8 address = String8(device_address);

    if (audio_is_output_device(device)) {
        if (device & mAvailableOutputDevices) {
            if (audio_is_a2dp_device(device) &&
                (!mHasA2dp || (address != "" && mA2dpDeviceAddress != address))) {
                return state;
            }
            if (audio_is_bluetooth_sco_device(device) &&
                address != "" && mScoDeviceAddress != address) {
                return state;
            }
            if (audio_is_usb_device(device) &&
                (!mHasUsb || (address != "" && mUsbCardAndDevice != address))) {
                ALOGE("getDeviceConnectionState() invalid device: 0x%08x", device);
                return state;
            }
            if (audio_is_remote_submix_device((audio_devices_t)device) && !mHasRemoteSubmix) {
                return state;
            }
            state = AudioSystem::DEVICE_STATE_AVAILABLE;
        }
    } else if (audio_is_input_device(device)) {
        if (device & mAvailableInputDevices) {
            state = AudioSystem::DEVICE_STATE_AVAILABLE;
        }
    }

    return state;
}

void AudioPolicyManager::setPhoneState(int state)
{
    ALOGV("setPhoneState() state %d", state);

    if (state < 0 || state >= AudioSystem::NUM_MODES) {
        ALOGW("setPhoneState() invalid state %d", state);
        return;
    }

    if (state == mPhoneState ) {
        ALOGW("setPhoneState() setting same state %d", state);
        return;
    }

    // if leaving call state, handle special case of active streams
    // pertaining to sonification strategy see handleIncallSonification()
    if (isInCall()) {
        ALOGV("setPhoneState() in call state management: new state is %d", state);
        for (int stream = 0; stream < AudioSystem::NUM_STREAM_TYPES; stream++) {
            handleIncallSonification(stream, false, true);
        }
    }

    // store previous phone state for management of sonification strategy below
    int oldState = mPhoneState;
    mPhoneState = state;
    bool force = false;

    // are we entering or starting a call
    if (!isStateInCall(oldState) && isStateInCall(state)) {
        ALOGV("  Entering call in setPhoneState()");
        // force routing command to audio hardware when starting a call
        // even if no device change is needed
        force = true;
    } else if (isStateInCall(oldState) && !isStateInCall(state)) {
        ALOGV("  Exiting call in setPhoneState()");
        // force routing command to audio hardware when exiting a call
        // even if no device change is needed
        force = true;
    } else if (isStateInCall(state) && (state != oldState)) {
        ALOGV("  Switching between telephony and VoIP in setPhoneState()");
        // force routing command to audio hardware when switching between telephony and VoIP
        // even if no device change is needed
        force = true;
    }

    checkA2dpSuspend();

    // Restrict voice calls to cabin speakers
    audio_devices_t device = getRefinedZoneDevices(AUDIO_ZONE_CABIN,
                                                   AudioSystem::VOICE_CALL);

    // Set the voice call route in the audio hardware (primary output)
    AudioParameter param = AudioParameter();
    param.addInt(String8(AUDIO_PARAMETER_CALL_ROUTING), (int)device);
    mpClientInterface->setParameters(mPrimaryOutput, param.toString());

    // if entering in call state, handle special case of active streams
    // pertaining to sonification strategy see handleIncallSonification()
    if (isStateInCall(state)) {
        ALOGV("setPhoneState() in call state management: new state is %d", state);
        for (int stream = 0; stream < AudioSystem::NUM_STREAM_TYPES; stream++) {
            handleIncallSonification(stream, true, true);
        }
    }
}

void AudioPolicyManager::setForceUse(AudioSystem::force_use usage,
                                     AudioSystem::forced_config config)
{
    ALOGV("setForceUse() usage %d, config %d, mPhoneState %d",
          usage, config, mPhoneState);

    bool forceVolumeReeval = false;
    switch(usage) {
    case AudioSystem::FOR_COMMUNICATION:
        if (config != AudioSystem::FORCE_SPEAKER &&
            config != AudioSystem::FORCE_BT_SCO &&
            config != AudioSystem::FORCE_NONE) {
            ALOGW("setForceUse() invalid config %d for FOR_COMMUNICATION",
                  config);
            return;
        }
        forceVolumeReeval = true;
        mForceUse[usage] = config;
        break;
    case AudioSystem::FOR_MEDIA:
        if (config != AudioSystem::FORCE_HEADPHONES &&
            config != AudioSystem::FORCE_BT_A2DP &&
            config != AudioSystem::FORCE_WIRED_ACCESSORY &&
            config != AudioSystem::FORCE_ANALOG_DOCK &&
            config != AudioSystem::FORCE_DIGITAL_DOCK &&
            config != AudioSystem::FORCE_NONE &&
            config != AudioSystem::FORCE_NO_BT_A2DP) {
            ALOGW("setForceUse() invalid config %d for FOR_MEDIA", config);
            return;
        }
        mForceUse[usage] = config;
        break;
    case AudioSystem::FOR_RECORD:
        if (config != AudioSystem::FORCE_BT_SCO &&
            config != AudioSystem::FORCE_WIRED_ACCESSORY &&
            config != AudioSystem::FORCE_NONE) {
            ALOGW("setForceUse() invalid config %d for FOR_RECORD", config);
            return;
        }
        mForceUse[usage] = config;
        break;
    case AudioSystem::FOR_DOCK:
        if (config != AudioSystem::FORCE_NONE &&
            config != AudioSystem::FORCE_BT_CAR_DOCK &&
            config != AudioSystem::FORCE_BT_DESK_DOCK &&
            config != AudioSystem::FORCE_WIRED_ACCESSORY &&
            config != AudioSystem::FORCE_ANALOG_DOCK &&
            config != AudioSystem::FORCE_DIGITAL_DOCK) {
            ALOGW("setForceUse() invalid config %d for FOR_DOCK", config);
        }
        forceVolumeReeval = true;
        mForceUse[usage] = config;
        break;
    case AudioSystem::FOR_SYSTEM:
        if (config != AudioSystem::FORCE_NONE &&
            config != AudioSystem::FORCE_SYSTEM_ENFORCED) {
            ALOGW("setForceUse() invalid config %d for FOR_SYSTEM", config);
        }
        forceVolumeReeval = true;
        mForceUse[usage] = config;
        break;
    default:
        ALOGW("setForceUse() invalid usage %d", usage);
        break;
    }

    // Force use and force configs are not scalable for multiple outputs in
    // automotive environments:
    // - FOR_COMMUNICATION: phone call is routed through BT SCO to cabin
    //   speakers. The existing force configs don't cause any route update
    // - FOR_MEDIA: there may be multiple outputs playing media streams, so
    //   the force config cannot be fairly associated with one of them.
    //   It's currently used only for FORCE_NONE and FORCE_NO_BT_A2DP.
    //   The latter will be handled at setDeviceConnectionState() when
    //   BT A2DP is disconnected.
    // - FOR_DOCK: It's not applicable either as it refers to the Android
    //   device being docked
    // - FOR_SYSTEM: Used for camera sound (shutter).
    //
    // The logic is kept not to disturb AudioService, but there won't be
    // any actions taken.
}

AudioSystem::forced_config AudioPolicyManager::getForceUse(AudioSystem::force_use usage)
{
    return mForceUse[usage];
}

void AudioPolicyManager::setSystemProperty(const char* property, const char* value)
{
    ALOGV("setSystemProperty() property %s, value %s", property, value);
}

status_t AudioPolicyManager::setZoneDevices_l(audio_zones_t zone,
                                              audio_devices_t devices)
{
    if (AudioSystem::popCount(zone) > 1) {
        ALOGE("setZoneDevices() cannot set multiple zones simultaneously");
        return BAD_VALUE;
    }

    if (devices == AUDIO_DEVICE_NONE) {
        ALOGE("setZoneDevices() at least one device must be set per zone");
        return BAD_VALUE;
    }

    // Devices must be owned by only one zone
    for (size_t i = 0; i < mZoneDevices.size(); i++) {
        if (zone != mZoneDevices.keyAt(i)) {
            audio_devices_t common = devices & mZoneDevices.valueAt(i);
            if (common) {
                ALOGE("setZoneDevices_l() devices 0x%08x are already set in zone 0x%x",
                      common, mZoneDevices.keyAt(i));
                return INVALID_OPERATION;
            }
        }
    }

    ALOGI("setZoneDevices_l() zone 0x%x set to devices 0x%08x", zone, devices);
    mZoneDevices.replaceValueFor(zone, devices);

    return NO_ERROR;
}

status_t AudioPolicyManager::setZoneDevices(audio_zones_t zone,
                                            audio_devices_t devices)
{
    ALOGI("setZoneDevices() zone 0x%x devices 0x%08x", zone, devices);

    // Attached devices are assigned to a zone and cannot be moved
    audio_devices_t tmpDevices = getZoneDevices(zone) & ~devices;
    if (tmpDevices & mAttachedOutputDevices) {
        ALOGE("setZoneDevices() attached devices 0x%08x cannot be removed",
              tmpDevices & mAttachedOutputDevices);
        return INVALID_OPERATION;
    }

    // Reject devices that are not supported by the zone, as defined
    // in the zone_affinity section of the audio_policy.conf file
    tmpDevices = devices & ~getZoneSupportedDevices(zone);
    if (tmpDevices) {
        ALOGE("setZoneDevices() devices 0x%08x not supported in zone 0x%x",
              tmpDevices, zone);
        return INVALID_OPERATION;
    }

    // Update the devices in the listening zone descriptor
    status_t status;
    status = setZoneDevices_l(zone, devices);
    if (status != NO_ERROR) {
        return status;
    }

    // Sessions using the zone being set must update their device too
    for (size_t i = 0; i < mSessions.size(); i++) {
        SessionDescriptor *sessionDesc = mSessions.valueAt(i);
        if (sessionDesc->zones() & zone) {
            ALOGI("setZoneDevices() update session %u with new zone devices 0x%08x",
                  sessionDesc->sessionId(), devices);
            setSessionZones(sessionDesc->sessionId(), sessionDesc->zones());
        }
    }

    return NO_ERROR;
}

audio_devices_t AudioPolicyManager::getZoneDevices(audio_zones_t zones)
{
    audio_devices_t devices = AUDIO_DEVICE_NONE;

    for (size_t i = 0; i < mZoneDevices.size(); i++) {
        if (zones & mZoneDevices.keyAt(i)) {
            devices |= mZoneDevices.valueAt(i);
        }
    }

    ALOGV("getZoneDevices() zones 0x%x devices 0x%08x", zones, devices);

    return devices;
}

status_t AudioPolicyManager::setSessionZones(int session, audio_zones_t zones)
{
    ALOGI("setSessionZones() session %d zones 0x%x", session, zones);

    SessionDescriptor *sessionDesc;
    ssize_t index = mSessions.indexOfKey(session);
    if (index < 0) {
        // Create an session desc entry with default output handle
        ALOGV("setSessionZones() session %d hasn't been used yet", session);
        sessionDesc = new SessionDescriptor(session, 0, zones, getZoneDevices(zones));
        mSessions.add(session, sessionDesc);
    } else {
        ALOGV("setSessionZones() session %d zones updated", session);
        sessionDesc = mSessions.valueAt(index);
        audio_devices_t prevDevices = sessionDesc->devices();
        sessionDesc->mZones = zones;
        sessionDesc->mDevices = getZoneDevices(zones);

        // Move tracks from previous to new devices if the output is already open
        moveTracks(sessionDesc->mId, prevDevices, sessionDesc->devices());
    }

    return NO_ERROR;
}

audio_zones_t AudioPolicyManager::getSessionZones(int session)
{
    audio_zones_t zones;

    ssize_t index = mSessions.indexOfKey(session);
    if (index < 0) {
        ALOGV("getSessionZones() session %d is not active", session);
        zones = AUDIO_ZONE_NONE;
    } else {
        SessionDescriptor *sessionDesc = mSessions.valueAt(index);
        zones = sessionDesc->zones();
        ALOGV("getSessionZones() session %d zones 0x%x", session, zones);
    }

    return zones;
}

status_t AudioPolicyManager::setZoneVolume(audio_zones_t zone, float volume)
{
    ALOGI("setZoneVolume() zone 0x%x volume %f", zone, volume);

    if (AudioSystem::popCount(zone) > 1) {
        ALOGE("setZoneVolume() cannot set multiple zones simultaneously");
        return BAD_VALUE;
    }

    if (!audio_is_output_zone(zone)) {
        ALOGE("setZoneVolume() zone 0x%x is invalid", zone);
        return BAD_VALUE;
    }

    if (volume < 0.0f || volume > 1.0f) {
        ALOGE("setZoneVolume() volume %f is out of limits", volume);
        return BAD_VALUE;
    }

    ssize_t index = mZoneVolume.indexOfKey(zone);
    if (index < 0) {
        ALOGE("setZoneVolume() zone 0x%x is not supported", zone);
        return BAD_VALUE;
    }

    mZoneVolume.replaceValueAt(index, volume);

    // Update the volumes of all sessions playing on the listening zone
    // whose volume just changed
    for (size_t i = 0; i < mSessions.size(); i++) {
        SessionDescriptor *sessionDesc = mSessions.valueAt(i);
        int session = sessionDesc->sessionId();

        if (getSessionZones(session) & zone) {
            float sessionVolume = sessionDesc->mVolume.valueFor(zone);
            if (setSessionVolume_l(session, zone, sessionVolume) != NO_ERROR) {
                ALOGE("setZoneVolume() couldn't to set volume for session %d on zone 0x%x",
                      session, zone);
            }
        }
    }

    return NO_ERROR;
}

float AudioPolicyManager::getZoneVolume(audio_zones_t zone)
{
    float zoneVolume = -1.0f;

    if (AudioSystem::popCount(zone) > 1) {
        ALOGE("getZoneVolume() cannot get multiple zones simultaneously");
        return zoneVolume;
    }

    if (!audio_is_output_zone(zone)) {
        ALOGE("getZoneVolume() zone 0x%x is invalid", zone);
        return zoneVolume;
    }

    ssize_t index = mZoneVolume.indexOfKey(zone);
    if (index < 0) {
        ALOGE("getZoneVolume() zone 0x%x is not supported", zone);
        return zoneVolume;
    }

    zoneVolume = mZoneVolume.valueAt(index);

    ALOGV("getZoneVolume() zone 0x%x volume %f", zone, zoneVolume);

    return zoneVolume;
}

status_t AudioPolicyManager::setSessionVolume_l(int session,
                                                audio_zones_t zone,
                                                float volume)
{
    SessionDescriptor *sessionDesc = mSessions.valueFor(session);
    audio_io_handle_t output = sessionDesc->mId;
    audio_zones_t activeZones = getSessionZones(session);

    if (zone & activeZones) {
        float zoneVolume = volume * mZoneVolume.valueFor(zone);

        // Duplicating output sets the volume to the output(s) that is actually
        // handling that zone
        SortedVector<audio_io_handle_t> zoneOutputs;
        findOutputsInZone(output, zone, zoneOutputs);
        for (size_t i = 0; i < zoneOutputs.size(); i++) {
            mpClientInterface->setZoneVolume(zoneOutputs.itemAt(i), session, zoneVolume);
        }
    }

    return NO_ERROR;
}

status_t AudioPolicyManager::setSessionVolume(int session,
                                              audio_zones_t zones,
                                              float volume)
{
    ALOGI("setSessionVolume() session %d zones 0x%x volume %f",
          session, zones, volume);

    ssize_t index = mSessions.indexOfKey(session);
    if (index < 0) {
        ALOGE("setSessionVolume() session %d is not active", session);
        return BAD_VALUE;
    }

    if (!audio_is_output_zones(zones)) {
        ALOGE("setSessionVolume() zones 0x%x are not valid", zones);
        return BAD_VALUE;
    }

    if (volume < 0.0f || volume > 1.0f) {
        ALOGE("setSessionVolume() volume %f is out of limits", volume);
        return BAD_VALUE;
    }

    SessionDescriptor *sessionDesc = mSessions.valueAt(index);

    for (audio_zones_t zone = AUDIO_ZONE_LAST; zone & AUDIO_ZONE_ALL; zone >>= 1) {
        audio_zones_t cur = zone & zones;
        if (cur != AUDIO_ZONE_NONE) {
            // The volume for the session takes into account the coarse per-zone
            // volume as well
            sessionDesc->mVolume.replaceValueFor(cur, volume);
            float zoneVolume = mZoneVolume.valueFor(cur);

            // Apply the calculated volume
            ALOGV("setSessionVolume() session %d zone-vol %f session-vol %f final-vol %f",
                  session, zoneVolume, volume, zoneVolume * volume);
            setSessionVolume_l(session, cur, volume);
        }
    }

    return NO_ERROR;
}

float AudioPolicyManager::getSessionVolume(int session, audio_zones_t zone)
{
    float sessionVolume = -1.0f;

    ssize_t index = mSessions.indexOfKey(session);
    if (index < 0) {
        ALOGE("getSessionVolume() session %d is not active", session);
        return sessionVolume;
    }

    if (AudioSystem::popCount(zone) > 1) {
        ALOGE("getSessionVolume() cannot get multiple zones simultaneously");
        return sessionVolume;
    }

    if (!audio_is_output_zone(zone)) {
        ALOGE("getSessionVolume() zone 0x%x is not valid", zone);
        return sessionVolume;
    }

    SessionDescriptor *sessionDesc = mSessions.valueAt(index);
    sessionVolume = sessionDesc->mVolume.valueFor(zone);

    ALOGV("getSessionVolume() session %d volume %f", session, sessionVolume);

    return sessionVolume;
}

audio_io_handle_t AudioPolicyManager::getOutput(AudioSystem::stream_type stream,
                                                uint32_t samplingRate,
                                                uint32_t format,
                                                uint32_t channelMask,
                                                AudioSystem::output_flags flags,
                                                int session)
{
    ALOGI("getOutput() stream %d rate %d format %d channelMask %x flags %x session %d",
          stream, samplingRate, format, channelMask, flags, session);

    // Find the best zone for this session, based on explicit requests or
    // predefined policy rules
    selectZones(session, stream);

    // selectZones() must have returned valid zone(s) and device(s) since the
    // last selection mechanism is a fixed listening zone
    ssize_t index = mSessions.indexOfKey(session);
    if (index < 0) {
        ALOGE("getOutput() policy failed to find a zone for session %d", session);
        return 0;
    }

    audio_devices_t devices = getSessionDevices(session);
    if (!devices) {
        ALOGE("getOutput() unexpected null device for session %d", session);
        return 0;
    }

    SessionDescriptor *sessionDesc = mSessions.valueAt(index);
    audio_io_handle_t output = sessionDesc->mId;
    AudioOutputDescriptor *outputDesc;

    // Multiple devices could be handled by different outputs, populate a vector
    // with the needed outputs for all requested devices
    SortedVector<audio_io_handle_t> outputs;
    while (devices) {
        ALOGV("getOutput() search output(s) for devices 0x%08x flags 0x%08x", devices, flags);
        if (needsDirectOuput(stream, samplingRate, format, channelMask, flags, devices)) {
            output = openOutput(devices, samplingRate, format, channelMask,
                                AUDIO_OUTPUT_FLAG_DIRECT);
        } else {
            output = openOutput(devices, samplingRate, format, channelMask,
                                (audio_output_flags_t)flags);
        }
        if (output) {
            outputs.add(output);
            outputDesc = mOutputs.valueFor(output);
            devices &= ~outputDesc->device();

            // Try opening a direct output for multichannel tracks. One of the two
            // outputs for their associated device(s) will be closed when refining
            // outputs (see refineOutputs())
            if (!outputDesc->isDirectOutput() &&
                (AudioSystem::popCount(channelMask) > 2) &&
                (outputDesc->mChannelMask != channelMask)) {
                ALOGV("getOutput() output %d requires downmix, try direct output",
                      output);
                output = openOutput(outputDesc->device(),
                                    samplingRate, format, channelMask,
                                    AUDIO_OUTPUT_FLAG_DIRECT);
                if (output)
                    outputs.add(output);
            }
        } else {
            ALOGW("getOutput() no output found for devices 0x%08x", devices);
            break;
        }
    }

    if (outputs.isEmpty()) {
        ALOGE("getOutput() session %d no output found", session);
        clearSession(session);
        return 0;
    }

    // Refine the list of outputs where audio data is going to be duplicated:
    // either exclude all direct outputs (they're incompatible with duplicated
    // output) or just take one direct output (no duplication will be possible)
    if (outputs.size() > 1)
        refineOutputs(outputs);

    // There might be need of output duplication if the requested devices are not
    // supported by a single output
    if (outputs.size() > 1) {
        // Create a duplicating output connected to all individual outputs that
        // render to the requested devices
        output = openDuplicateOutput(outputs, samplingRate, format, channelMask, session);
        if (!output) {
            ALOGE("getOutput() could not get duplicate output");
            for (size_t i = 0; i < outputs.size(); i++) {
                closeOutput(outputs.itemAt(i));
            }
            return 0;
        }
        ALOGV("getOutput() session %d got duplicate output %d", session, output);
    } else {
        // A single output is required to satisfy parameters and devices
        output = outputs.itemAt(0);
        outputDesc = mOutputs.valueFor(output);
        ALOGV("getOutput() session %d got %s output %d", session,
              outputDesc->isDirectOutput() ? "direct" : "mixer", output);
    }

    sessionDesc->mId = output;

    // Session must be ref counted to maintain the current routes after re-routing
    ALOGV("getOutput() session %d users %d->%d",
          session, sessionDesc->mUsers, sessionDesc->mUsers+1);
    sessionDesc->mUsers++;

    initSessionVolume(session);

    return output;
}

status_t AudioPolicyManager::startOutput(audio_io_handle_t output,
                                         AudioSystem::stream_type stream,
                                         int session)
{
    ALOGI("startOutput() output %d session %d stream %d", output, session, stream);

    ssize_t index = mOutputs.indexOfKey(output);
    if (index < 0) {
        ALOGW("startOutput() unknow output %d", output);
        return BAD_VALUE;
    }

    AudioOutputDescriptor *outputDesc = mOutputs.valueAt(index);

    // Increment usage count of this stream on the output
    outputDesc->changeRefCount(stream, 1);
    if (outputDesc->mRefCount[stream] == 1) {
        audio_devices_t devices = getSessionDevices(session);
        if (devices == AUDIO_DEVICE_NONE) {
            ALOGW("startOutput() no device for session %d, using the cabin",
                  session);
            setSessionZones(session, AUDIO_ZONE_CABIN);
        }

        setOutputDevice(output, devices, false);

        // Apply volume rules for current stream and device if necessary
        setVolume(stream, output);
    }

    return NO_ERROR;
}

status_t AudioPolicyManager::stopOutput(audio_io_handle_t output,
                                        AudioSystem::stream_type stream,
                                        int session)
{
    ALOGI("stopOutput() output %d session %d stream %d", output, session, stream);

    ssize_t index = mOutputs.indexOfKey(output);
    if (index < 0) {
        ALOGE("stopOutput() unknown output %d", output);
        return BAD_VALUE;
    }

    AudioOutputDescriptor *outputDesc = mOutputs.valueAt(index);

    if (outputDesc->mRefCount[stream] == 0) {
        ALOGE("stopOutput() refcount is already 0 for output %d", output);
        return INVALID_OPERATION;
    }

    // Decrement usage count of this stream on the output
    outputDesc->changeRefCount(stream, -1);

    // Store time at which the stream was stopped - see isStreamActive()
    if (outputDesc->mRefCount[stream] == 0) {
        outputDesc->mStopTime[stream] = systemTime();
        audio_devices_t devices = getSessionDevices(session);

        // Delay the device switch by twice the latency because stopOutput() is
        // executed when the track stop() command is received and at that time
        // the audio track buffer can still contain data that needs to be drained.
        // The latency only covers the audio HAL and kernel buffers. Also the
        // latency does not always include additional delay in the audio path
        // (audio DSP, CODEC ...)
        setOutputDevice(output, devices, false, outputDesc->mLatency*2);
    }

    return NO_ERROR;
}

void AudioPolicyManager::releaseOutput(audio_io_handle_t output, int session)
{
    ALOGI("releaseOutput() output %d session %d", output, session);

    ssize_t index = mOutputs.indexOfKey(output);
    if (index < 0) {
        ALOGE("releaseOutput() unknown output %d", output);
        return;
    }

    index = mSessions.indexOfKey(session);
    if (index < 0) {
        ALOGW("releaseOutput() unknown session %d", session);
    } else {
        SessionDescriptor *sessionDesc = mSessions.valueAt(index);
        ALOGV("releaseOutput() session %d users %d->%d",
              session, sessionDesc->mUsers, sessionDesc->mUsers-1);
        if (!--sessionDesc->mUsers)
            clearSession(session);
    }

    closeOutput(output);
}

audio_io_handle_t AudioPolicyManager::getInput(int inputSource,
                                               uint32_t samplingRate,
                                               uint32_t format,
                                               uint32_t channelMask,
                                               AudioSystem::audio_in_acoustics acoustics,
                                               int session)
{
    audio_io_handle_t input = 0;
    audio_devices_t device = getDeviceForInputSource(inputSource);

    ALOGI("getInput() inputSource %d rate %d format %d channelMask %04x acoustics %x",
          inputSource, samplingRate, format, channelMask, acoustics);

    if (device == AUDIO_DEVICE_NONE) {
        ALOGW("getInput() could not find device for inputSource %d", inputSource);
        return 0;
    }

    // adapt channel selection to input source
    switch(inputSource) {
    case AUDIO_SOURCE_VOICE_UPLINK:
        channelMask = AudioSystem::CHANNEL_IN_VOICE_UPLINK;
        break;
    case AUDIO_SOURCE_VOICE_DOWNLINK:
        channelMask = AudioSystem::CHANNEL_IN_VOICE_DNLINK;
        break;
    case AUDIO_SOURCE_VOICE_CALL:
        channelMask = (AudioSystem::CHANNEL_IN_VOICE_UPLINK |
                       AudioSystem::CHANNEL_IN_VOICE_DNLINK);
        break;
    default:
        break;
    }

    IOProfile *profile = getInputProfile(device,
                                         samplingRate,
                                         format,
                                         channelMask);
    if (profile == NULL) {
        ALOGE("getInput() could not find profile");
        return 0;
    }

    if (profile->mModule->mHandle == 0) {
        ALOGE("getInput(): HW module %s not opened", profile->mModule->mName);
        return 0;
    }

    AudioInputDescriptor *inputDesc = new AudioInputDescriptor(profile);

    inputDesc->mInputSource = inputSource;
    inputDesc->mDevice = device;
    inputDesc->mSamplingRate = samplingRate;
    inputDesc->mFormat = (audio_format_t)format;
    inputDesc->mChannelMask = (audio_channel_mask_t)channelMask;
    inputDesc->mUsers = 0;
    input = mpClientInterface->openInput(profile->mModule->mHandle,
                                         &inputDesc->mDevice,
                                         &inputDesc->mSamplingRate,
                                         &inputDesc->mFormat,
                                         &inputDesc->mChannelMask);

    // only accept input with the exact requested set of parameters
    if (input == 0 ||
        (samplingRate != inputDesc->mSamplingRate) ||
        (format != inputDesc->mFormat) ||
        (channelMask != inputDesc->mChannelMask)) {
        ALOGV("getInput() failed opening input: rate %d format %d channelMask %d",
                samplingRate, format, channelMask);
        if (input != 0) {
            mpClientInterface->closeInput(input);
        }
        delete inputDesc;
        return 0;
    }
    mInputs.add(input, inputDesc);
    return input;
}

status_t AudioPolicyManager::startInput(audio_io_handle_t input)
{
    ALOGI("startInput() input %d", input);

    ssize_t index = mInputs.indexOfKey(input);
    if (index < 0) {
        ALOGW("startInput() unknow input %d", input);
        return BAD_VALUE;
    }

    AudioInputDescriptor *inputDesc = mInputs.valueAt(index);

    // Refuse 2 active AudioRecord clients at the same time
    if (getActiveInput() != 0) {
        ALOGE("startInput() input %d failed: other input already started", input);
        return INVALID_OPERATION;
    }

    audio_devices_t newDevice = getDeviceForInputSource(inputDesc->mInputSource);
    if ((newDevice != AUDIO_DEVICE_NONE) && (newDevice != inputDesc->mDevice)) {
        inputDesc->mDevice = newDevice;
    }

    AudioParameter param = AudioParameter();
    param.addInt(String8(AudioParameter::keyRouting), (int)inputDesc->mDevice);
    param.addInt(String8(AudioParameter::keyInputSource), (int)inputDesc->mInputSource);
    ALOGV("startInput() input source %d", inputDesc->mInputSource);
    mpClientInterface->setParameters(input, param.toString());

    inputDesc->mUsers = 1;

    return NO_ERROR;
}

status_t AudioPolicyManager::stopInput(audio_io_handle_t input)
{
    ALOGI("stopInput() input %d", input);

    ssize_t index = mInputs.indexOfKey(input);
    if (index < 0) {
        ALOGW("stopInput() unknow input %d", input);
        return BAD_VALUE;
    }
    AudioInputDescriptor *inputDesc = mInputs.valueAt(index);

    if (inputDesc->mUsers == 0) {
        ALOGW("stopInput() input %d already stopped", input);
        return INVALID_OPERATION;
    } else {
        AudioParameter param = AudioParameter();
        param.addInt(String8(AudioParameter::keyRouting), 0);
        mpClientInterface->setParameters(input, param.toString());
        inputDesc->mUsers = 0;
        return NO_ERROR;
    }
}

void AudioPolicyManager::releaseInput(audio_io_handle_t input, int session)
{
    ALOGI("releaseInput() %d", input);

    ssize_t index = mInputs.indexOfKey(input);
    if (index < 0) {
        ALOGW("releaseInput() unknown input %d", input);
        return;
    }
    mpClientInterface->closeInput(input);
    delete mInputs.valueAt(index);
    mInputs.removeItem(input);
}

void AudioPolicyManager::initStreamVolume(AudioSystem::stream_type stream,
                                          int indexMin,
                                          int indexMax)
{
    ALOGV("initStreamVolume() stream %d min %d max %d", stream, indexMin, indexMax);

    if (indexMin < 0 || indexMin >= indexMax) {
        ALOGW("initStreamVolume() invalid index limits for stream %d min %d max %d",
              stream, indexMin, indexMax);
        return;
    }

    mStreams[stream].mIndexMin = indexMin;
    mStreams[stream].mIndexMax = indexMax;
}

status_t AudioPolicyManager::setStreamVolumeIndex(AudioSystem::stream_type stream,
                                                  int index,
                                                  audio_devices_t device)
{
    if ((index < mStreams[stream].mIndexMin) || (index > mStreams[stream].mIndexMax)) {
        ALOGE("setStreamVolumeIndex() invalid index %d, should be within %d,%d",
              index, mStreams[stream].mIndexMin, mStreams[stream].mIndexMax);
        return BAD_VALUE;
    }

    if (!audio_is_output_device(device)) {
        ALOGE("setStreamVolumeIndex() devices 0x%08x is invalid", device);
        return BAD_VALUE;
    }

    // Force max volume if stream cannot be muted
    if (!mStreams[stream].mCanBeMuted)
        index = mStreams[stream].mIndexMax;

    ALOGV("setStreamVolumeIndex() stream %d device 0x%08x index %d",
          stream, device, index);

    mStreams[stream].mIndexCur = index;

    // Compute and apply stream volume on all outputs
    status_t status = NO_ERROR;
    for (size_t i = 0; i < mOutputs.size(); i++) {
        status_t volStatus = setVolume(stream, mOutputs.keyAt(i));
        if (volStatus != NO_ERROR)
            status = volStatus;
    }

    return status;
}

status_t AudioPolicyManager::getStreamVolumeIndex(AudioSystem::stream_type stream,
                                                  int *index,
                                                  audio_devices_t device)
{
    if (index == NULL)
        return BAD_VALUE;

    if (!audio_is_output_device(device)) {
        ALOGE("getStreamVolumeIndex() devices 0x%08x is invalid", device);
        return BAD_VALUE;
    }

    *index = mStreams[stream].mIndexCur;

    ALOGV("getStreamVolumeIndex() stream %d device 0x%08x index %d",
          stream, device, *index);

    return NO_ERROR;
}

uint32_t AudioPolicyManager::getStrategyForStream(AudioSystem::stream_type stream)
{
    return (uint32_t)getStrategy(stream);
}

audio_devices_t AudioPolicyManager::getDevicesForStream(AudioSystem::stream_type stream)
{
    if (stream < (AudioSystem::stream_type) 0 ||
        stream >= AudioSystem::NUM_STREAM_TYPES) {
        return AUDIO_DEVICE_NONE;
    }

    // Same stream type on different listening zones may produce devices
    // that are anyways discarded by AudioService, since it gives priority
    // to speaker. Use main cabin device refined for the stream type as
    // reference for stream type volume in the system.
    return getRefinedZoneDevices(AUDIO_ZONE_CABIN, stream);
}

audio_io_handle_t AudioPolicyManager::getOutputForEffect(const effect_descriptor_t *desc)
{
    audio_devices_t device = getRefinedZoneDevices(AUDIO_ZONE_CABIN,
                                                   AudioSystem::MUSIC);
    audio_io_handle_t output = mPrimaryOutput;
    for (size_t i = 0; i < mOutputs.size(); i++) {
        AudioOutputDescriptor *outputDesc = mOutputs.valueAt(i);
        if ((device & outputDesc->supportedDevices()) == device) {
            output = mOutputs.keyAt(i);
            if (outputDesc->mFlags & AUDIO_OUTPUT_FLAG_DEEP_BUFFER)
                break;
        }
    }

    ALOGV("getOutputForEffect() output %d", output);

    return output;
}

status_t AudioPolicyManager::registerEffect(const effect_descriptor_t *desc,
                                            audio_io_handle_t io,
                                            uint32_t strategy,
                                            int session,
                                            int id)
{
    ssize_t index = mOutputs.indexOfKey(io);
    if (index < 0) {
        index = mInputs.indexOfKey(io);
        if (index < 0) {
            ALOGW("registerEffect() unknown io %d", io);
            return INVALID_OPERATION;
        }
    }

    if (mTotalEffectsMemory + desc->memoryUsage > getMaxEffectsMemory()) {
        ALOGW("registerEffect() memory limit exceeded for Fx %s, Memory %d KB",
              desc->name, desc->memoryUsage);
        return INVALID_OPERATION;
    }
    mTotalEffectsMemory += desc->memoryUsage;
    ALOGV("registerEffect() effect %s, io %d, strategy %d session %d id %d",
          desc->name, io, strategy, session, id);
    ALOGV("registerEffect() memory %d, total memory %d",
          desc->memoryUsage, mTotalEffectsMemory);

    EffectDescriptor *pDesc = new EffectDescriptor();
    memcpy (&pDesc->mDesc, desc, sizeof(effect_descriptor_t));
    pDesc->mIo = io;
    pDesc->mStrategy = (routing_strategy)strategy;
    pDesc->mSession = session;
    pDesc->mEnabled = false;

    mEffects.add(id, pDesc);

    return NO_ERROR;
}

status_t AudioPolicyManager::unregisterEffect(int id)
{
    ssize_t index = mEffects.indexOfKey(id);
    if (index < 0) {
        ALOGW("unregisterEffect() unknown effect ID %d", id);
        return INVALID_OPERATION;
    }

    EffectDescriptor *pDesc = mEffects.valueAt(index);

    setEffectEnabled(pDesc, false);

    if (mTotalEffectsMemory < pDesc->mDesc.memoryUsage) {
        ALOGW("unregisterEffect() memory %d too big for total %d",
              pDesc->mDesc.memoryUsage, mTotalEffectsMemory);
        pDesc->mDesc.memoryUsage = mTotalEffectsMemory;
    }
    mTotalEffectsMemory -= pDesc->mDesc.memoryUsage;
    ALOGV("unregisterEffect() effect %s, ID %d, memory %d total memory %d",
          pDesc->mDesc.name, id, pDesc->mDesc.memoryUsage, mTotalEffectsMemory);

    mEffects.removeItem(id);
    delete pDesc;

    return NO_ERROR;
}

status_t AudioPolicyManager::setEffectEnabled(int id, bool enabled)
{
    ssize_t index = mEffects.indexOfKey(id);
    if (index < 0) {
        ALOGW("unregisterEffect() unknown effect ID %d", id);
        return INVALID_OPERATION;
    }

    return setEffectEnabled(mEffects.valueAt(index), enabled);
}

status_t AudioPolicyManager::setEffectEnabled(EffectDescriptor *pDesc, bool enabled)
{
    if (enabled == pDesc->mEnabled) {
        ALOGV("setEffectEnabled(%s) effect already %s",
              enabled?"true":"false", enabled?"enabled":"disabled");
        return INVALID_OPERATION;
    }

    if (enabled) {
        if (mTotalEffectsCpuLoad + pDesc->mDesc.cpuLoad > getMaxEffectsCpuLoad()) {
            ALOGW("setEffectEnabled(true) CPU Load limit exceeded for Fx %s, CPU %f MIPS",
                  pDesc->mDesc.name, (float)pDesc->mDesc.cpuLoad/10);
            return INVALID_OPERATION;
        }
        mTotalEffectsCpuLoad += pDesc->mDesc.cpuLoad;
        ALOGV("setEffectEnabled(true) total CPU %d", mTotalEffectsCpuLoad);
    } else {
        if (mTotalEffectsCpuLoad < pDesc->mDesc.cpuLoad) {
            ALOGW("setEffectEnabled(false) CPU load %d too high for total %d",
                  pDesc->mDesc.cpuLoad, mTotalEffectsCpuLoad);
            pDesc->mDesc.cpuLoad = mTotalEffectsCpuLoad;
        }
        mTotalEffectsCpuLoad -= pDesc->mDesc.cpuLoad;
        ALOGV("setEffectEnabled(false) total CPU %d", mTotalEffectsCpuLoad);
    }
    pDesc->mEnabled = enabled;
    return NO_ERROR;
}

bool AudioPolicyManager::isStreamActive(int stream, uint32_t inPastMs) const
{
    nsecs_t sysTime = systemTime();
    for (size_t i = 0; i < mOutputs.size(); i++) {
        const AudioOutputDescriptor *outputDesc = mOutputs.valueAt(i);
        if (outputDesc->isStreamActive((AudioSystem::stream_type)stream,
                                       inPastMs, sysTime)) {
            return true;
        }
    }
    return false;
}

bool AudioPolicyManager::isStreamActiveRemotely(int stream, uint32_t inPastMs) const
{
    nsecs_t sysTime = systemTime();
    for (size_t i = 0; i < mOutputs.size(); i++) {
        const AudioOutputDescriptor *outputDesc = mOutputs.valueAt(i);
        if (((outputDesc->device() & APM_AUDIO_OUT_DEVICE_REMOTE_ALL) != 0) &&
                outputDesc->isStreamActive((AudioSystem::stream_type)stream,
                                           inPastMs, sysTime)) {
            return true;
        }
    }
    return false;
}

bool AudioPolicyManager::isSourceActive(audio_source_t source) const
{
    for (size_t i = 0; i < mInputs.size(); i++) {
        const AudioInputDescriptor * inputDescriptor = mInputs.valueAt(i);
        if ((inputDescriptor->mInputSource == (int) source)
            && (inputDescriptor->mUsers > 0)) {
            return true;
        }
    }
    return false;
}

status_t AudioPolicyManager::dump(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, "\nAudioPolicyManager Dump: %p\n", this);
    result.append(buffer);

    snprintf(buffer, SIZE, " Primary Output: %d\n", mPrimaryOutput);
    result.append(buffer);
    snprintf(buffer, SIZE, " A2DP device address: %s\n", mA2dpDeviceAddress.string());
    result.append(buffer);
    snprintf(buffer, SIZE, " SCO device address: %s\n", mScoDeviceAddress.string());
    result.append(buffer);
    snprintf(buffer, SIZE, " USB audio ALSA %s\n", mUsbCardAndDevice.string());
    result.append(buffer);
    snprintf(buffer, SIZE, " Output devices: %08x\n", mAvailableOutputDevices);
    result.append(buffer);
    snprintf(buffer, SIZE, " Input devices: %08x\n", mAvailableInputDevices);
    result.append(buffer);
    snprintf(buffer, SIZE, " Phone state: %d\n", mPhoneState);
    result.append(buffer);
    snprintf(buffer, SIZE, " Force use for communications %d\n", mForceUse[AudioSystem::FOR_COMMUNICATION]);
    result.append(buffer);
    snprintf(buffer, SIZE, " Force use for media %d\n", mForceUse[AudioSystem::FOR_MEDIA]);
    result.append(buffer);
    snprintf(buffer, SIZE, " Force use for record %d\n", mForceUse[AudioSystem::FOR_RECORD]);
    result.append(buffer);
    snprintf(buffer, SIZE, " Force use for dock %d\n", mForceUse[AudioSystem::FOR_DOCK]);
    result.append(buffer);
    snprintf(buffer, SIZE, " Force use for system %d\n", mForceUse[AudioSystem::FOR_SYSTEM]);
    result.append(buffer);
    write(fd, result.string(), result.size());


    snprintf(buffer, SIZE, "\nHW Modules dump:\n");
    write(fd, buffer, strlen(buffer));
    for (size_t i = 0; i < mHwModules.size(); i++) {
        snprintf(buffer, SIZE, "- HW Module %d:\n", i + 1);
        write(fd, buffer, strlen(buffer));
        mHwModules[i]->dump(fd);
    }

    snprintf(buffer, SIZE, "\nOutputs dump:\n");
    write(fd, buffer, strlen(buffer));
    for (size_t i = 0; i < mOutputs.size(); i++) {
        snprintf(buffer, SIZE, "- Output %d dump:\n", mOutputs.keyAt(i));
        write(fd, buffer, strlen(buffer));
        mOutputs.valueAt(i)->dump(fd);
    }

    snprintf(buffer, SIZE, "\nInputs dump:\n");
    write(fd, buffer, strlen(buffer));
    for (size_t i = 0; i < mInputs.size(); i++) {
        snprintf(buffer, SIZE, "- Input %d dump:\n", mInputs.keyAt(i));
        write(fd, buffer, strlen(buffer));
        mInputs.valueAt(i)->dump(fd);
    }

    snprintf(buffer, SIZE, "\nStreams dump:\n");
    write(fd, buffer, strlen(buffer));
    snprintf(buffer, SIZE,
             " Stream  Can be muted  Index Min  Index Max  Index Cur\n");
    write(fd, buffer, strlen(buffer));
    for (size_t i = 0; i < AudioSystem::NUM_STREAM_TYPES; i++) {
        snprintf(buffer, SIZE, " %02d      ", i);
        write(fd, buffer, strlen(buffer));
        mStreams[i].dump(fd);
    }

    snprintf(buffer, SIZE, "\nTotal Effects CPU: %f MIPS, Total Effects memory: %d KB\n",
            (float)mTotalEffectsCpuLoad/10, mTotalEffectsMemory);
    write(fd, buffer, strlen(buffer));

    snprintf(buffer, SIZE, "Registered effects:\n");
    write(fd, buffer, strlen(buffer));
    for (size_t i = 0; i < mEffects.size(); i++) {
        snprintf(buffer, SIZE, "- Effect %d dump:\n", mEffects.keyAt(i));
        write(fd, buffer, strlen(buffer));
        mEffects.valueAt(i)->dump(fd);
    }

    snprintf(buffer, SIZE, "\nSessions:\n");
    write(fd, buffer, strlen(buffer));
    for (size_t i = 0; i < mSessions.size(); i++) {
        snprintf(buffer, SIZE, "- Session %d dump:\n", mSessions.keyAt(i));
        write(fd, buffer, strlen(buffer));
        mSessions.valueAt(i)->dump(fd);
    }

    snprintf(buffer, SIZE, "\nZone Affinity:\n");
    write(fd, buffer, strlen(buffer));
    snprintf(buffer, SIZE, " Zone   Devices\n");
    write(fd, buffer, strlen(buffer));
    for (size_t i = 0; i < mZoneAffinity.size(); i++) {
        snprintf(buffer, SIZE, "  0x%x  0x%08x\n",
                 mZoneAffinity.keyAt(i), mZoneAffinity.valueAt(i));
        write(fd, buffer, strlen(buffer));
    }

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
// AudioPolicyManager
// ----------------------------------------------------------------------------

AudioPolicyManager::AudioPolicyManager(AudioPolicyClientInterface *clientInterface)
    : mPrimaryOutput((audio_io_handle_t)0),
      mAvailableOutputDevices(AUDIO_DEVICE_NONE),
      mA2dpSuspended(false),
      mHasA2dp(false),
      mHasUsb(false),
      mHasRemoteSubmix(false),
      mPhoneState(AudioSystem::MODE_NORMAL),
      mLastVoiceVolume(-1.0f),
      mTotalEffectsCpuLoad(0),
      mTotalEffectsMemory(0),
      mParser(mHwModules, mZoneAffinity)
{
    mpClientInterface = clientInterface;

    for (int i = 0; i < AudioSystem::NUM_FORCE_USE; i++) {
        mForceUse[i] = AudioSystem::FORCE_NONE;
    }

    initializeVolumeCurves();

    mA2dpDeviceAddress = String8("");
    mScoDeviceAddress = String8("");
    mUsbCardAndDevice = String8("");

    if (mParser.loadAudioPolicyConfig(AUDIO_POLICY_VENDOR_CONFIG_FILE) != NO_ERROR) {
        if (mParser.loadAudioPolicyConfig(AUDIO_POLICY_CONFIG_FILE) != NO_ERROR) {
            ALOGE("could not load audio policy configuration file, setting defaults");
            mParser.defaultAudioPolicyConfig();
        }
    }

    mAvailableInputDevices = mParser.getAttachedInputDevices();
    mAttachedOutputDevices = mParser.getAttachedOutputDevices();
    mDefaultOutputDevice = mParser.getDefaultOutputDevice();
    mHasA2dp = mParser.supportsA2DP();
    mHasUsb = mParser.supportsUSB();
    mHasRemoteSubmix = mParser.supportsRemoteSubmix();

    if (mZoneAffinity.indexOfKey(AUDIO_ZONE_CABIN) < 0) {
        ALOGW("No affinity set for cabin, using default");
        mZoneAffinity.add(AUDIO_ZONE_CABIN, AUDIO_DEVICE_OUT_SPEAKER);
    }

    if (mZoneAffinity.indexOfKey(AUDIO_ZONE_BACKSEAT1) < 0) {
        ALOGW("No affinity set for back-seat 1, using default");
        mZoneAffinity.add(AUDIO_ZONE_BACKSEAT1, AUDIO_DEVICE_OUT_WIRED_HEADPHONE);
    }

    if (mZoneAffinity.indexOfKey(AUDIO_ZONE_BACKSEAT2) < 0) {
        ALOGW("No affinity set for back-seat 2, using default");
        mZoneAffinity.add(AUDIO_ZONE_BACKSEAT2, AUDIO_DEVICE_OUT_WIRED_HEADPHONE2);
    }

    ALOGI("Initialize per-listening zone volumes");
    for (audio_zones_t zone = AUDIO_ZONE_LAST; zone & AUDIO_ZONE_ALL; zone >>= 1) {
        mZoneVolume.add(zone, 1.0f);
    }

    ALOGI("Initialize listening zones routes");
    setZoneDevices(AUDIO_ZONE_CABIN, mDefaultOutputDevice |
                   (mZoneAffinity.valueFor(AUDIO_ZONE_CABIN) & mAttachedOutputDevices));
    setZoneDevices(AUDIO_ZONE_BACKSEAT1,
                   mZoneAffinity.valueFor(AUDIO_ZONE_BACKSEAT1) & mAttachedOutputDevices);
    setZoneDevices(AUDIO_ZONE_BACKSEAT2,
                   mZoneAffinity.valueFor(AUDIO_ZONE_BACKSEAT2) & mAttachedOutputDevices);

    // Open all output streams needed to access attached devices
    for (size_t i = 0; i < mHwModules.size(); i++) {
        mHwModules[i]->mHandle = mpClientInterface->loadHwModule(mHwModules[i]->mName);
        if (mHwModules[i]->mHandle == 0) {
            ALOGW("could not open HW module %s", mHwModules[i]->mName);
            continue;
        }
        // Open all output streams needed to access attached devices
        for (size_t j = 0; j < mHwModules[i]->mOutputProfiles.size(); j++) {
            IOProfile *profile = mHwModules[i]->mOutputProfiles[j];
            audio_devices_t devices = profile->mSupportedDevices & mAttachedOutputDevices;

            // Skip profiles that don't support attached output devices
            if (devices == AUDIO_DEVICE_NONE)
                continue;

            // Open the output with flags defined in the config file
            audio_io_handle_t output = openOutput_l(profile, devices, profile->mFlags);
            if (output) {
                AudioOutputDescriptor *desc = mOutputs.valueFor(output);

                ALOGV("output %d users %u->%u",
                      output, desc->mUsers, desc->mUsers+1);
                desc->mUsers++;

                mAvailableOutputDevices |= devices;
                if (!mPrimaryOutput &&
                    (profile->mFlags & AUDIO_OUTPUT_FLAG_PRIMARY)) {
                    mPrimaryOutput = output;
                }
                setOutputDevice(output, devices, true);
            }
        }
    }

    ALOGE_IF((mAttachedOutputDevices & ~mAvailableOutputDevices),
             "No output found for attached devices 0x%08x",
             (mAttachedOutputDevices & ~mAvailableOutputDevices));

    ALOGE_IF((mPrimaryOutput == 0), "Failed to open primary output");
}

AudioPolicyManager::~AudioPolicyManager()
{
   for (size_t i = 0; i < mOutputs.size(); i++) {
        mpClientInterface->closeOutput(mOutputs.keyAt(i));
        delete mOutputs.valueAt(i);
   }
   for (size_t i = 0; i < mSessions.size(); i++) {
       delete mSessions.valueAt(i);
   }
   for (size_t i = 0; i < mInputs.size(); i++) {
        mpClientInterface->closeInput(mInputs.keyAt(i));
        delete mInputs.valueAt(i);
   }
}

status_t AudioPolicyManager::initCheck()
{
    return (mPrimaryOutput == 0) ? NO_INIT : NO_ERROR;
}

// ---

AudioPolicyManager::routing_strategy
AudioPolicyManager::getStrategy(AudioSystem::stream_type stream)
{
    // stream to strategy mapping
    switch (stream) {
    case AudioSystem::VOICE_CALL:
    case AudioSystem::BLUETOOTH_SCO:
        return STRATEGY_PHONE;
    case AudioSystem::RING:
    case AudioSystem::ALARM:
        return STRATEGY_SONIFICATION;
    case AudioSystem::NOTIFICATION:
        return STRATEGY_SONIFICATION_RESPECTFUL;
    case AudioSystem::DTMF:
        return STRATEGY_DTMF;
    default:
        ALOGE("unknown stream type");
    case AudioSystem::SYSTEM:
        // NOTE: SYSTEM stream uses MEDIA strategy because muting music and switching outputs
        // while key clicks are played produces a poor result
    case AudioSystem::TTS:
    case AudioSystem::MUSIC:
        return STRATEGY_MEDIA;
    case AudioSystem::ENFORCED_AUDIBLE:
        return STRATEGY_ENFORCED_AUDIBLE;
    }
}

AudioPolicyManager::device_category
AudioPolicyManager::getDeviceCategory(audio_devices_t device)
{
    switch(getDeviceForVolume(device)) {
    case AUDIO_DEVICE_OUT_EARPIECE:
        return DEVICE_CATEGORY_EARPIECE;
    case AUDIO_DEVICE_OUT_WIRED_HEADSET:
    case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
    case AUDIO_DEVICE_OUT_WIRED_HEADPHONE2:
    case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
    case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
    case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP:
    case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES:
        return DEVICE_CATEGORY_HEADSET;
    case AUDIO_DEVICE_OUT_SPEAKER:
    case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
    case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER:
    case AUDIO_DEVICE_OUT_AUX_DIGITAL:
    case AUDIO_DEVICE_OUT_USB_ACCESSORY:
    case AUDIO_DEVICE_OUT_USB_DEVICE:
    case AUDIO_DEVICE_OUT_REMOTE_SUBMIX:
    default:
        return DEVICE_CATEGORY_SPEAKER;
    }
}

audio_devices_t AudioPolicyManager::getDeviceForVolume(audio_devices_t device)
{
    if (device == AUDIO_DEVICE_NONE) {
        // this happens when forcing a route update and no track is active on an output.
        // In this case the returned category is not important.
        device =  AUDIO_DEVICE_OUT_SPEAKER;
    } else if (AudioSystem::popCount(device) > 1) {
        // Multiple device selection is either:
        //  - speaker + one other device: give priority to speaker in this case.
        //  - headphone(s) + other devices: give priority to headphones (back-seat)
        //  - one A2DP device + another device: happens with duplicated output.
        //    In this case retain the device on the A2DP output as the other must not
        //    correspond to an active selection if not the speaker.
        if (device & AUDIO_DEVICE_OUT_SPEAKER) {
            device = AUDIO_DEVICE_OUT_SPEAKER;
        } else if (device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
            device = AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
        } else if (device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE2) {
            device = AUDIO_DEVICE_OUT_WIRED_HEADPHONE2;
        } else {
            device = (audio_devices_t)(device & AUDIO_DEVICE_OUT_ALL_A2DP);
        }
    }

    ALOGW_IF(AudioSystem::popCount(device) != 1,
            "getDeviceForVolume() invalid device combination: 0x%08x", device);

    return device;
}

bool AudioPolicyManager::needsDirectOuput(AudioSystem::stream_type stream,
                                          uint32_t samplingRate,
                                          uint32_t format,
                                          uint32_t channelMask,
                                          AudioSystem::output_flags flags,
                                          audio_devices_t device)
{
   return ((flags & AudioSystem::OUTPUT_FLAG_DIRECT) ||
           (format != 0 && !AudioSystem::isLinearPCM(format)));
}

void AudioPolicyManager::queryOutputParameters(audio_io_handle_t output,
                                               IOProfile *profile)
{
    String8 reply;
    char *rates = NULL;
    char *formats = NULL;
    char *channels = NULL;

    if (profile->mSamplingRates[0] == 0) {
        reply = mpClientInterface->getParameters(output,
                        String8(AUDIO_PARAMETER_STREAM_SUP_SAMPLING_RATES));
        ALOGV("queryOutputParameters() direct output sup sampling rates %s",
              reply.string());
        rates = (char *)reply.string();
    }

    if (profile->mFormats[0] == 0) {
        reply = mpClientInterface->getParameters(output,
                        String8(AUDIO_PARAMETER_STREAM_SUP_FORMATS));
        ALOGV("queryOutputParameters() direct output sup formats %s",
              reply.string());
        formats = (char *)reply.string();
    }

    if (profile->mChannelMasks[0] == 0) {
        reply = mpClientInterface->getParameters(output,
                        String8(AUDIO_PARAMETER_STREAM_SUP_CHANNELS));
        ALOGV("queryOutputParameters() direct output sup channel masks %s",
              reply.string());
        channels = (char *)reply.string();
    }

    mParser.fillProfile(profile, rates, formats, channels);
}

void AudioPolicyManager::getProfilesForDevices(audio_devices_t devices,
                                               SortedVector<IOProfile*> &profiles)
{
    for (size_t i = 0; i < mHwModules.size(); i++) {
        if (mHwModules[i]->mHandle == 0)
            continue;

        for (size_t j = 0; j < mHwModules[i]->mOutputProfiles.size(); j++) {
            IOProfile *profile = mHwModules[i]->mOutputProfiles[j];
            if (profile->mSupportedDevices & devices)
                profiles.add(profile);
        }
    }
}

void AudioPolicyManager::setDeviceAddress(audio_io_handle_t output,
                                          audio_devices_t device)
{
    String8 paramStr;

    if (mHasA2dp && audio_is_a2dp_device(device)) {
        AudioParameter param;
        param.add(String8(AUDIO_PARAMETER_A2DP_SINK_ADDRESS), mA2dpDeviceAddress);
        paramStr = param.toString();
    } else if (mHasUsb && audio_is_usb_device(device)) {
        paramStr = mUsbCardAndDevice;
    }

    // not currently handling multiple simultaneous submixes: ignoring remote submix
    // case and address
    if (!paramStr.isEmpty()) {
        ALOGV("setDeviceAddress() set device address '%s'", paramStr.string());
        mpClientInterface->setParameters(output, paramStr);
    }
}

void AudioPolicyManager::refineOutputs(SortedVector<audio_io_handle_t> &outputs)
{
    SortedVector<audio_io_handle_t> direct;
    SortedVector<audio_io_handle_t> discarded;
    bool useDirectOutput;
    char value[PROPERTY_VALUE_MAX];

    // AudioFlinger's duplicating output doesn't support direct outputs, so there
    // are two possible choices:
    // - Keep mixer outputs, discard all direct outputs (useDirectOutput = false)
    // - Use a single direct output, discard the rest (useDirectOutput = true)
    if ((property_get("multizone_audio.use_direct", value, NULL) == 0) ||
        !strcmp(value, "0") || !strcasecmp(value, "false")) {
        ALOGV("refineOutputs() keep mixer outputs, discard all direct outputs");
        useDirectOutput = false;
    } else {
        ALOGV("refineOutputs() keep a single direct output");
        useDirectOutput = true;
    }

    // Find all direct outputs in the candidates for duplication
    for (size_t i = 0; i < outputs.size(); i++) {
        AudioOutputDescriptor *desc = mOutputs.valueFor(outputs.itemAt(i));
        if (desc->isDirectOutput())
            direct.add(outputs.itemAt(i));
    }

    // If all outputs are direct, the only option is using one of them to
    // prevent duplication of direct outputs
    useDirectOutput |= (direct.size() == outputs.size());

    if (direct.isEmpty()) {
        // Nothing to refine when no direct outputs were found
        ALOGVV("refineOutputs() no direct outputs found");
    } else if (useDirectOutput) {
        // Use the first direct output and discard the rest
        audio_io_handle_t out = direct.itemAt(0);
        discarded = outputs;
        discarded.remove(out);
        outputs.clear();
        outputs.add(out);
    } else {
        // Discard direct outputs, keep mixer outputs
        for (size_t i = 0; i < direct.size(); i++)
            outputs.remove(direct.itemAt(i));
        discarded = direct;
    }

    ALOGI_IF(!discarded.isEmpty(), "refineOutputs() discard %d %s outputs",
             discarded.size(), useDirectOutput ? "mixer" : "direct");

    for (size_t i = 0; i < discarded.size(); i++)
        closeOutput(discarded.itemAt(i));
}

audio_io_handle_t AudioPolicyManager::openOutput_l(IOProfile *profile,
                                                   audio_devices_t devices,
                                                   audio_output_flags_t flags,
                                                   uint32_t samplingRate,
                                                   uint32_t format,
                                                   uint32_t channelMask)
{
    ALOGV("openOutput_l() opening output for devices 0x%08x",
          devices & profile->mSupportedDevices);

    AudioOutputDescriptor *desc = new AudioOutputDescriptor(profile);
    audio_io_handle_t output = 0;

    // Direct outputs must be opened with the exact requested parameters
    if (flags & AUDIO_OUTPUT_FLAG_DIRECT) {
        if (samplingRate)
            desc->mSamplingRate = samplingRate;
        if (format)
            desc->mFormat = (audio_format_t)format;
        if (channelMask)
            desc->mChannelMask = channelMask;
    }

    // AudioTrack queries the sampling rate to check if default resampler
    // implementation supports it. Resampler limits the input sampling rate
    // to 2x the output rate, so the best output sampling rate in the profile
    // is the highest
    if (!desc->mSamplingRate) {
        uint32_t maxRate = 0;
        for (size_t i = 0; i < profile->mSamplingRates.size(); i++) {
            if (profile->mSamplingRates[i] > maxRate)
                maxRate = profile->mSamplingRates[i];
        }
        desc->mSamplingRate = maxRate;
    }

    // Skip profiles that don't support the requested flags (direct or none)
    if ((flags & AUDIO_OUTPUT_FLAG_DIRECT) !=
        (desc->mFlags & AUDIO_OUTPUT_FLAG_DIRECT)) {
        ALOGV("openOutput_l() flags mismatch req 0x%08x desc 0x%08x",
              flags, desc->mFlags);
        delete desc;
        return 0;
    }

    desc->mDevice = devices & profile->mSupportedDevices;

    // It's hardware module task to restrict the number of outputs that
    // can be opened
    output = mpClientInterface->openOutput(profile->mModule->mHandle,
                                           &desc->mDevice,
                                           &desc->mSamplingRate,
                                           &desc->mFormat,
                                           &desc->mChannelMask,
                                           &desc->mLatency,
                                           desc->mFlags);
    if (!output) {
        ALOGW("openOutput_l() could not open output for device 0x%08x",
              devices);
        delete desc;
        return 0;
    }

    if (desc->isDirectOutput()) {
        if (profile->hasDynamicParams()) {
            // It's important to keep the original profile intact because it has
            // information about what parameters are dynamic
            IOProfile dynamicProfile = *profile;
            queryOutputParameters(output, &dynamicProfile);
            devices &= profile->mSupportedDevices;
            // Close and re-open the port with new parameters if its profile
            // supports the requested parameters
            if (dynamicProfile.isCompatibleProfile(devices, samplingRate, format,
                                                   channelMask, flags)) {
                mpClientInterface->closeOutput(output);
                desc->mDevice = devices;
                desc->mSamplingRate = samplingRate;
                desc->mFormat = (audio_format_t)format;
                desc->mChannelMask = channelMask;
                desc->mLatency = 0;
                output = mpClientInterface->openOutput(dynamicProfile.mModule->mHandle,
                                                       &desc->mDevice,
                                                       &desc->mSamplingRate,
                                                       &desc->mFormat,
                                                       &desc->mChannelMask,
                                                       &desc->mLatency,
                                                       desc->mFlags);
                if (!output) {
                    ALOGE("openOutput_l() could not re-open direct output");
                    delete desc;
                    return 0;
                }
            }
        }
        if ((samplingRate && (samplingRate != desc->mSamplingRate)) ||
            (format && (format != desc->mFormat)) ||
            (channelMask && (channelMask != desc->mChannelMask))) {
            ALOGV("openOutput_l() params mismatch");
            mpClientInterface->closeOutput(output);
            output = 0;
            delete desc;
        }
    }

    if (output) {
        ALOGV("openOutput_l() adding output %d", output);
        addOutput(output, desc);
        setDeviceAddress(output, desc->device());
    }

    return output;
}

audio_io_handle_t AudioPolicyManager::openOutput(audio_devices_t devices,
                                                 uint32_t samplingRate,
                                                 uint32_t format,
                                                 uint32_t channelMask,
                                                 audio_output_flags_t flags)
{
    ALOGVV("openOutput() devices 0x%08x", devices);

    if (devices == AUDIO_DEVICE_NONE) {
        ALOGE("openOutput() invalid device 0x%08x", devices);
        return BAD_VALUE;
    }

    audio_io_handle_t output = 0;
    AudioOutputDescriptor *desc;

    // First look in the active outputs
    for (size_t i = 0; i < mOutputs.size(); i++) {
        desc = mOutputs.valueAt(i);

        // Skip duplicating outputs
        if (desc->isDuplicated())
            continue;

        // Skip outputs that don't match the requested flags
        if ((flags & AUDIO_OUTPUT_FLAG_DIRECT) !=
            (desc->mFlags & AUDIO_OUTPUT_FLAG_DIRECT))
            continue;

        // Direct outputs cannot mix the new stream, but can be moved to
        // a different track
        if (desc->isDirectOutput() && (desc->mUsers > 1))
            continue;

        if (devices & desc->supportedDevices()) {
            output = mOutputs.keyAt(i);
            // Direct outputs can be reassigned only if requested parameters
            // are the same than those of the open output
            if (desc->isDirectOutput() &&
                (samplingRate == desc->mSamplingRate) &&
                (channelMask == desc->mChannelMask) &&
                (format == desc->mFormat)) {
                ALOGV("openOutput() direct output %d will be reused", output);
                break;
            }
        }
    }

    // Look in other profiles that can route to the requested devices
    if (!output) {
        SortedVector<IOProfile*> profiles;
        getProfilesForDevices(devices, profiles);
        if (profiles.isEmpty()) {
            ALOGE("openOutput() no profile available for devices 0x%08x", devices);
            return 0;
        }

        for (size_t i = 0; i < profiles.size(); i++) {
            IOProfile *profile = profiles.itemAt(i);
            output = openOutput_l(profile, devices, flags,
                                  samplingRate, format, channelMask);
            if (output)
                break;
        }
    }

    // Increase use count if a valid output was found
    if (output) {
        desc = mOutputs.valueFor(output);
        ALOGV("openOutput() output %d users %u->%u",
              output, desc->mUsers, desc->mUsers+1);
        desc->mUsers++;
    }

    ALOGV("openOutput() got output %d", output);

    return output;
}

audio_io_handle_t AudioPolicyManager::openDuplicateOutput(SortedVector<audio_io_handle_t> &outputs,
                                                          uint32_t samplingRate,
                                                          uint32_t format,
                                                          uint32_t channelMask,
                                                          int session)
{
    audio_io_handle_t dupOutput;
    audio_io_handle_t outs[outputs.size()];
    uint32_t idx = 0;

    ALOGV("openDuplicateOutput() open output to duplicate to %d more outputs",
          outputs.size());

    for (size_t i = 0; i < outputs.size(); i++) {
        outs[idx++] = outputs.itemAt(i);
        ALOGV("openDuplicateOutput() duplicate to output %d", outputs.itemAt(i));
    }

    dupOutput = mpClientInterface->openDuplicateOutput(outs, outputs.size());
    if (!dupOutput) {
        ALOGE("openDuplicateOutput() could not open duplicated output");
        return 0;
    }

    // Add duplicated output descriptor
    AudioOutputDescriptor *dupDesc = new AudioOutputDescriptor(NULL);
    for (size_t i = 0; i < outputs.size(); i++) {
        dupDesc->mDupOutputs.add(mOutputs.valueFor(outputs.itemAt(i)));
    }
    dupDesc->mSamplingRate = samplingRate;
    dupDesc->mFormat = (audio_format_t)format;
    dupDesc->mChannelMask = channelMask;

    addOutput(dupOutput, dupDesc);

    ALOGV("openDuplicateOutput() output %d users %u->%u",
          dupOutput, dupDesc->mUsers, dupDesc->mUsers+1);
    dupDesc->mUsers++;

    ALOGV("openDuplicateOutput() got output %d", dupOutput);

    return dupOutput;
}

void AudioPolicyManager::addOutput(audio_io_handle_t id,
                                   AudioOutputDescriptor *outputDesc)
{
    outputDesc->mId = id;
    mOutputs.add(id, outputDesc);
}

void AudioPolicyManager::closeOutput(audio_io_handle_t output)
{
    ALOGV("closeOutput() output %d", output);

    AudioOutputDescriptor *desc = mOutputs.valueFor(output);
    if (!desc) {
        ALOGE("closeOutput() unknown output %d", output);
        return;
    }

    // Output appears to be already closed
    if (!desc->mUsers) {
        ALOGW("closeOutput() invalid count %d for output %d",
              desc->mUsers, output);
        return;
    }

    // Look for duplicated outputs connected to the output being removed
    if (desc->isDuplicated() && (desc->mUsers == 1)) {
        for (size_t i = 0; i < desc->mDupOutputs.size(); i++) {
            audio_io_handle_t dupOutput = desc->mDupOutputs.itemAt(i)->mId;
            if (dupOutput)
                closeOutput(dupOutput);
            else
                ALOGW("closeOutput() unexpected duplicated output %d", dupOutput);
        }
    }

    ALOGV("closeOutput() output %d users %u->%u",
          output, desc->mUsers, desc->mUsers-1);
    desc->mUsers--;

    if (!desc->mUsers) {
        AudioParameter param;
        param.add(String8("closing"), String8("true"));
        mpClientInterface->setParameters(output, param.toString());
        mpClientInterface->closeOutput(output);
        delete desc;
        mOutputs.removeItem(output);
    }
}

void AudioPolicyManager::setOutputDevice(audio_io_handle_t output,
                                         audio_devices_t device,
                                         bool force,
                                         int delayMs)
{
    AudioOutputDescriptor *outputDesc = mOutputs.valueFor(output);
    AudioParameter param;

    ALOGV("setOutputDevice() output %d device 0x%08x delayMs %d",
          output, device, delayMs);

    // The duplicating output passes the new device selection to the
    // connected outputs since they are the ones actually connected
    // to the audio hardware
    if (outputDesc->isDuplicated()) {
        for (size_t i = 0; i < outputDesc->mDupOutputs.size(); i++) {
            AudioOutputDescriptor *dupDesc = outputDesc->mDupOutputs.itemAt(i);
            if (dupDesc)
                setOutputDevice(dupDesc->mId, dupDesc->mDevice, force, delayMs);
        }
        return;
    }

    // No need to proceed if new device is valid but not supported by the
    // current output profile
    if (device && !(device & outputDesc->mProfile->mSupportedDevices))
        return;

    // filter devices according to output selected
    device = (audio_devices_t)(device & outputDesc->mProfile->mSupportedDevices);

    audio_devices_t prevDevice = outputDesc->device();

    // Do not change the routing if:
    //  - the requested device is AUDIO_DEVICE_NONE
    //  - the requested device is the same as current device and force is not specified.
    // Doing this check here allows the caller to call setOutputDevice() without conditions
    if (((device == AUDIO_DEVICE_NONE) || (device == prevDevice)) && !force) {
        ALOGV("setOutputDevice() setting same device 0x%08x for output %d",
              device, output);
        return;
    }

    outputDesc->mDevice = device;

    // Mute while transitioning to the new selected devices prevents volume bursts
    // when volume for the old device(s) and new device(s) are different.
    // The volume that is restored may have changed due if the new devices have
    // different volume curve.
    if (getDeviceForVolume(prevDevice) != getDeviceForVolume(device)) {
        // Mute takes place immediately
        muteOutput(output, true, 0);
        // Unmute after device has been changed
        muteOutput(output, false, delayMs * 2);
    }

    // Do the routing
    ALOGI("setOutputDevice() changing device to 0x%08x", device);
    param.addInt(String8(AudioParameter::keyRouting), (int)device);
    mpClientInterface->setParameters(output, param.toString(), delayMs);
}

void AudioPolicyManager::muteOutput(audio_io_handle_t output,
                                    bool mute,
                                    int delayMs)
{
    AudioOutputDescriptor *desc = mOutputs.valueFor(output);
    audio_devices_t devices = desc->device();

    ALOGV("muteOutput() %s output %d devices 0x%08x",
          mute ? "mute" : "unmute", output, devices);

    // Nothing to do
    if (devices == AUDIO_DEVICE_NONE)
        return;

    // Caller must ensure muteOutput() is called on each connected output
    // in order to have more granular volume per output
    if (desc->isDuplicated()) {
        ALOGW("muteOutput() cannot mute duplicating outputs");
        return;
    }

    // Volume after unmuting is selected based on the most relevant device
    // for volume settings
    devices = getDeviceForVolume(devices);

    for (int stream = 0; stream < AudioSystem::NUM_STREAM_TYPES; stream++) {
        setStreamMute(stream, mute, output, mute ? 0 : delayMs, devices);
    }
}

status_t AudioPolicyManager::moveTracks(audio_io_handle_t output,
                                        audio_devices_t oldDevices,
                                        audio_devices_t newDevices)
{
    // Move tracks if the output is open and devices have changed
    if (output && (oldDevices != newDevices)) {
        ALOGV("moveTracks() invalidate tracks on output %d", output);
        // Invalidate all tracks associated with this output
        for (int i = 0; i < (int)AudioSystem::NUM_STREAM_TYPES; i++) {
            mpClientInterface->setStreamOutput((AudioSystem::stream_type)i,
                                               output);
        }
    }

    return NO_ERROR;
}

audio_devices_t AudioPolicyManager::getZoneSupportedDevices(audio_zones_t zone)
{
    if (!audio_is_output_zone(zone)) {
        ALOGE("getZoneSupportedDevices() zone 0x%x is invalid", zone);
        return AUDIO_DEVICE_NONE;
    }

    ssize_t index = mZoneAffinity.indexOfKey(zone);
    if (index < 0) {
        ALOGE("getZoneSupportedDevices() zone 0x%x has affinity information", zone);
        return AUDIO_DEVICE_NONE;
    }

    audio_devices_t devices = mZoneAffinity.valueAt(index);

    ALOGV("getZoneSupportedDevices() zone 0x%x supports devices 0x%08x", zone, devices);

    return devices;
}

audio_devices_t AudioPolicyManager::getPrimaryDevices()
{
    return mAttachedOutputDevices;
}

audio_devices_t AudioPolicyManager::getRefinedZoneDevices(audio_zones_t zones,
                                                   AudioSystem::stream_type stream)
{
    audio_devices_t devices = getZoneDevices(zones);

    // Refine zone devices based on the stream type, in case it's not required
    // to be routed to all its devices (e.g. voice call is restricted to cabin
    // speakers).
    switch (stream) {
    case AudioSystem::DTMF:
    case AudioSystem::BLUETOOTH_SCO:
    case AudioSystem::VOICE_CALL:
        if (devices & AUDIO_DEVICE_OUT_SPEAKER)
            devices = AUDIO_DEVICE_OUT_SPEAKER;
        break;
    default:
        break;
    }

    return devices;
}

audio_devices_t AudioPolicyManager::getSessionDevices(int session)
{
    audio_devices_t devices;

    ssize_t index = mSessions.indexOfKey(session);
    if (index < 0) {
        ALOGV("getSessionDevices() session %d is not active", session);
        devices = AUDIO_DEVICE_NONE;
    } else {
        SessionDescriptor *sessionDesc = mSessions.valueAt(index);
        devices = sessionDesc->devices();
        ALOGV("getSessionDevices() session %d devices 0x%08x", session, devices);
    }

    return devices;
}

audio_io_handle_t AudioPolicyManager::findOutput(audio_devices_t device)
{
    audio_io_handle_t output = 0;

    for (size_t i = 0; i < mOutputs.size(); i++) {
        AudioOutputDescriptor *outputDesc = mOutputs.valueAt(i);
        if (!outputDesc->isDuplicated() &&
            (outputDesc->device() & device)) {
            output = outputDesc->mId;
            break;
        }
    }

    return output;
}

audio_zones_t AudioPolicyManager::findZone(audio_devices_t device)
{
    audio_zones_t zone = AUDIO_ZONE_NONE;

    if (popcount(device) > 1) {
        ALOGE("findZone() expected only one device, got 0x%08x", device);
        return AUDIO_ZONE_NONE;
    }

    for (size_t i = 0; i < mZoneDevices.size(); i++) {
        if (mZoneDevices.valueAt(i) & device) {
            zone = mZoneDevices.keyAt(i);
            break;
        }
    }

    return zone;
}

void AudioPolicyManager::findOutputsInZone(audio_io_handle_t output,
                                           audio_zones_t zone,
                                           SortedVector<audio_io_handle_t> &outputs)
{
    AudioOutputDescriptor *desc = mOutputs.valueFor(output);

    if (AudioSystem::popCount(zone) > 1) {
        ALOGE("findOutputsInZone() cannot find multiple zones simultaneously");
        return;
    }

    if (desc->isDuplicated()) {
        audio_devices_t devices = getZoneDevices(zone);

        for (size_t i = 0; i < desc->mDupOutputs.size(); i++) {
            AudioOutputDescriptor *dupDesc = desc->mDupOutputs.itemAt(i);
            if (devices & dupDesc->device()) {
                outputs.add(dupDesc->mId);
            }
        }
    } else {
        outputs.add(output);
    }

    ALOGV("findOutputsInZone() found %u outputs in zone 0x%x", outputs.size(), zone);
}

void AudioPolicyManager::clearSession(int session)
{
    ALOGV("clearSession() session %d", session);

    ssize_t index = mSessions.indexOfKey(session);
    if (index < 0) {
        ALOGW("clearSession() session %d is not registered", session);
    } else {
        // Remove session routes and volumes
        SessionDescriptor *sessionDesc = mSessions.valueAt(index);
        delete sessionDesc;
        mSessions.removeItem(session);
    }
}

status_t AudioPolicyManager::disconnectDevice(audio_devices_t device)
{
    // Find the listening zone affected by the device disconnection
    audio_zones_t zone = findZone(device);

    ALOGI("disconnectDevice() zone 0x%x device 0x%08x", zone, device);

    // The disconnected device was not assigned to a listening zone
    if (zone == AUDIO_ZONE_NONE) {
        ALOGW("disconnectDevice() device 0x%08x is not assigned to a zone, "
              "cannot disconnect", device);
        return INVALID_OPERATION;
    }

    if (device & mAttachedOutputDevices) {
        ALOGE("disconnectDevice() attached devices 0x%08x cannot be disconnected",
              device & mAttachedOutputDevices);
        return INVALID_OPERATION;
    }

    // Update the devices of the listening zone
    setZoneDevices_l(zone, getZoneDevices(zone) & ~device);

    // Find the audio sessions affected by the device disconnection
    for (size_t i = 0; i < mSessions.size(); i++) {
        SessionDescriptor *sessionDesc = mSessions.valueAt(i);

        // Skip sessions not rendering to the disconnected device
        if (!(sessionDesc->devices() & device))
            continue;

        ALOGV("disconnectDevice() session %d device 0x%08x",
              sessionDesc->sessionId(), device);

        sessionDesc->mDevices &= ~device;

        // Move all tracks in the output
        moveTracks(sessionDesc->mId, 0, sessionDesc->devices());
    }

    return NO_ERROR;
}

void AudioPolicyManager::selectZoneForStrategy(routing_strategy strategy,
                                               audio_zones_t &zones,
                                               audio_devices_t &devices)
{
    zones = AUDIO_ZONE_NONE;
    devices = AUDIO_DEVICE_NONE;

    switch (strategy) {
    case STRATEGY_MEDIA:
        break;

    case STRATEGY_DTMF:
    case STRATEGY_SONIFICATION:
    case STRATEGY_SONIFICATION_RESPECTFUL:
        zones = AUDIO_ZONE_CABIN;
        devices = getZoneDevices(zones);
        break;

    case STRATEGY_PHONE:
        // Calls go strictly to speaker, not all cabin zone devices
        if (mAttachedOutputDevices & AUDIO_DEVICE_OUT_SPEAKER) {
            devices = AUDIO_DEVICE_OUT_SPEAKER;
            zones = findZone(devices);
        }
        break;

    case STRATEGY_ENFORCED_AUDIBLE:
        zones = AUDIO_ZONE_ALL;
        devices = getZoneDevices(zones);
        break;

    default:
        break;
    }

    ALOGV("selectZoneForStrategy() zones 0x%x devices 0x%08x", zones, devices);
}

void AudioPolicyManager::selectZoneForStream(AudioSystem::stream_type stream,
                                             audio_zones_t &zones,
                                             audio_devices_t &devices)
{
    zones = AUDIO_ZONE_NONE;
    devices = AUDIO_DEVICE_NONE;

    switch (stream) {
    case AudioSystem::ENFORCED_AUDIBLE:
        zones = AUDIO_ZONE_ALL;
        devices = getZoneDevices(zones);
        break;

    case AudioSystem::DTMF:
    case AudioSystem::TTS:
    case AudioSystem::SYSTEM:
    case AudioSystem::RING:
    case AudioSystem::ALARM:
    case AudioSystem::NOTIFICATION:
        zones = AUDIO_ZONE_CABIN;
        devices = getZoneDevices(zones);
        break;

    case AudioSystem::BLUETOOTH_SCO:
    case AudioSystem::VOICE_CALL:
        // Calls go strictly to speaker, not all cabin zone devices
        zones = AUDIO_ZONE_CABIN;
        devices = getZoneDevices(zones);
        if (devices & AUDIO_DEVICE_OUT_SPEAKER)
            devices = AUDIO_DEVICE_OUT_SPEAKER;
        break;

    case AudioSystem::MUSIC:
    default:
        // Any listening zone is equally valid for music streams,
        // none is chosen to let the next selection mechanism execute
        break;
    }

    ALOGV("selectZoneForStream() zones 0x%x devices 0x%08x", zones, devices);
}

void AudioPolicyManager::selectZones(int session,
                                     AudioSystem::stream_type stream)
{
    audio_zones_t zones = AUDIO_ZONE_NONE;
    audio_devices_t devices = AUDIO_DEVICE_NONE;

    // First option: zones explicitly requested
    ssize_t index = mSessions.indexOfKey(session);
    if (index >= 0) {
        // Zones and devices are already populated
        zones = getSessionZones(session);
        ALOGV_IF(zones != AUDIO_ZONE_NONE,
                 "selectZones() session %d got zone from descriptor", session);
        return;
    }

    // Second option: best zones for strategy
    if (zones == AUDIO_ZONE_NONE) {
        selectZoneForStrategy(getStrategy(stream), zones, devices);
        ALOGV_IF(zones != AUDIO_ZONE_NONE,
                 "selectZones() session %d got zone from strategy", session);
    }

    // Third option: best zones for stream type
    if (zones == AUDIO_ZONE_NONE) {
        selectZoneForStream(stream, zones, devices);
        ALOGV_IF(zones != AUDIO_ZONE_NONE,
                 "selectZones() session %d got zone from stream type", session);
    }

    // Fourth option: cabin
    if (zones == AUDIO_ZONE_NONE) {
        zones = AUDIO_ZONE_CABIN;
        devices = getZoneDevices(zones);
        ALOGV("selectZones() session %d got default zone", session);
    }

    ALOGV("selectZones() session %d got zones 0x%x devices 0x%08x",
          session, zones, devices);

    // Create an session descriptor with the selected zone(s) and device(s)
    mSessions.add(session, new SessionDescriptor(session, 0, zones, devices));
}

const AudioPolicyManager::VolumeCurvePoint
    AudioPolicyManager::sDefaultVolumeCurve[AudioPolicyManager::VOLCNT] = {
    {1, -49.5f}, {33, -33.5f}, {66, -17.0f}, {100, 0.0f}
};

const AudioPolicyManager::VolumeCurvePoint
    AudioPolicyManager::sDefaultMediaVolumeCurve[AudioPolicyManager::VOLCNT] = {
    {1, -58.0f}, {20, -40.0f}, {60, -17.0f}, {100, 0.0f}
};

const AudioPolicyManager::VolumeCurvePoint
    AudioPolicyManager::sSpeakerMediaVolumeCurve[AudioPolicyManager::VOLCNT] = {
    {1, -56.0f}, {20, -34.0f}, {60, -11.0f}, {100, 0.0f}
};

const AudioPolicyManager::VolumeCurvePoint
    AudioPolicyManager::sSpeakerSonificationVolumeCurve[AudioPolicyManager::VOLCNT] = {
    {1, -29.7f}, {33, -20.1f}, {66, -10.2f}, {100, 0.0f}
};

// AUDIO_STREAM_SYSTEM, AUDIO_STREAM_ENFORCED_AUDIBLE and AUDIO_STREAM_DTMF volume tracks
// AUDIO_STREAM_RING on phones and AUDIO_STREAM_MUSIC on tablets (See AudioService.java).
// The range is constrained between -24dB and -6dB over speaker and -30dB and -18dB over headset.
const AudioPolicyManager::VolumeCurvePoint
    AudioPolicyManager::sDefaultSystemVolumeCurve[AudioPolicyManager::VOLCNT] = {
    {1, -24.0f}, {33, -18.0f}, {66, -12.0f}, {100, -6.0f}
};

const AudioPolicyManager::VolumeCurvePoint
    AudioPolicyManager::sHeadsetSystemVolumeCurve[AudioPolicyManager::VOLCNT] = {
    {1, -30.0f}, {33, -26.0f}, {66, -22.0f}, {100, -18.0f}
};

const AudioPolicyManager::VolumeCurvePoint
    AudioPolicyManager::sDefaultVoiceVolumeCurve[AudioPolicyManager::VOLCNT] = {
    {0, -42.0f}, {33, -28.0f}, {66, -14.0f}, {100, 0.0f}
};

const AudioPolicyManager::VolumeCurvePoint
    AudioPolicyManager::sSpeakerVoiceVolumeCurve[AudioPolicyManager::VOLCNT] = {
    {0, -24.0f}, {33, -16.0f}, {66, -8.0f}, {100, 0.0f}
};

const AudioPolicyManager::VolumeCurvePoint
*AudioPolicyManager::sVolumeProfiles[AUDIO_STREAM_CNT]
                                    [AudioPolicyManager::DEVICE_CATEGORY_CNT] = {
    { // AUDIO_STREAM_VOICE_CALL
        sDefaultVoiceVolumeCurve,        // DEVICE_CATEGORY_HEADSET
        sSpeakerVoiceVolumeCurve,        // DEVICE_CATEGORY_SPEAKER
        sDefaultVoiceVolumeCurve         // DEVICE_CATEGORY_EARPIECE
    },
    { // AUDIO_STREAM_SYSTEM
        sHeadsetSystemVolumeCurve,       // DEVICE_CATEGORY_HEADSET
        sDefaultSystemVolumeCurve,       // DEVICE_CATEGORY_SPEAKER
        sDefaultSystemVolumeCurve        // DEVICE_CATEGORY_EARPIECE
    },
    { // AUDIO_STREAM_RING
        sDefaultVolumeCurve,             // DEVICE_CATEGORY_HEADSET
        sSpeakerSonificationVolumeCurve, // DEVICE_CATEGORY_SPEAKER
        sDefaultVolumeCurve              // DEVICE_CATEGORY_EARPIECE
    },
    { // AUDIO_STREAM_MUSIC
        sDefaultMediaVolumeCurve,        // DEVICE_CATEGORY_HEADSET
        sSpeakerMediaVolumeCurve,        // DEVICE_CATEGORY_SPEAKER
        sDefaultMediaVolumeCurve         // DEVICE_CATEGORY_EARPIECE
    },
    { // AUDIO_STREAM_ALARM
        sDefaultVolumeCurve,             // DEVICE_CATEGORY_HEADSET
        sSpeakerSonificationVolumeCurve, // DEVICE_CATEGORY_SPEAKER
        sDefaultVolumeCurve              // DEVICE_CATEGORY_EARPIECE
    },
    { // AUDIO_STREAM_NOTIFICATION
        sDefaultVolumeCurve,             // DEVICE_CATEGORY_HEADSET
        sSpeakerSonificationVolumeCurve, // DEVICE_CATEGORY_SPEAKER
        sDefaultVolumeCurve              // DEVICE_CATEGORY_EARPIECE
    },
    { // AUDIO_STREAM_BLUETOOTH_SCO
        sDefaultVoiceVolumeCurve,        // DEVICE_CATEGORY_HEADSET
        sSpeakerVoiceVolumeCurve,        // DEVICE_CATEGORY_SPEAKER
        sDefaultVoiceVolumeCurve         // DEVICE_CATEGORY_EARPIECE
    },
    { // AUDIO_STREAM_ENFORCED_AUDIBLE
        sHeadsetSystemVolumeCurve,       // DEVICE_CATEGORY_HEADSET
        sDefaultSystemVolumeCurve,       // DEVICE_CATEGORY_SPEAKER
        sDefaultSystemVolumeCurve        // DEVICE_CATEGORY_EARPIECE
    },
    {  // AUDIO_STREAM_DTMF
        sHeadsetSystemVolumeCurve,       // DEVICE_CATEGORY_HEADSET
        sDefaultSystemVolumeCurve,       // DEVICE_CATEGORY_SPEAKER
        sDefaultSystemVolumeCurve        // DEVICE_CATEGORY_EARPIECE
    },
    { // AUDIO_STREAM_TTS
        sDefaultMediaVolumeCurve,        // DEVICE_CATEGORY_HEADSET
        sSpeakerMediaVolumeCurve,        // DEVICE_CATEGORY_SPEAKER
        sDefaultMediaVolumeCurve         // DEVICE_CATEGORY_EARPIECE
    },
};

void AudioPolicyManager::initializeVolumeCurves()
{
    for (int i = 0; i < AUDIO_STREAM_CNT; i++) {
        for (int j = 0; j < DEVICE_CATEGORY_CNT; j++) {
            mStreams[i].mVolumeCurve[j] = sVolumeProfiles[i][j];
        }
    }
}

float AudioPolicyManager::computeVolume(int stream,
                                        int index,
                                        audio_io_handle_t output,
                                        audio_devices_t device)
{
    float volume = 1.0;
    AudioOutputDescriptor *outputDesc = mOutputs.valueFor(output);
    StreamDescriptor &streamDesc = mStreams[stream];

    if (device == AUDIO_DEVICE_NONE)
        device = outputDesc->device();

    // if volume is not 0 (not muted), force media volume to max on digital output
    if (stream == AudioSystem::MUSIC &&
        index != mStreams[stream].mIndexMin &&
        (device == AUDIO_DEVICE_OUT_AUX_DIGITAL ||
         device == AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET ||
         device == AUDIO_DEVICE_OUT_USB_ACCESSORY ||
         device == AUDIO_DEVICE_OUT_USB_DEVICE)) {
        return 1.0;
    }

    volume = volIndexToAmpl(device, streamDesc, index);

    // Always attenuate ringtones and notifications volume by 6dB with a minimum threshold
    // at -36d so that notification is always perceived
    const routing_strategy stream_strategy = getStrategy((AudioSystem::stream_type)stream);
    if ((device & (AUDIO_DEVICE_OUT_BLUETOOTH_A2DP |
                   AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES |
                   AUDIO_DEVICE_OUT_WIRED_HEADSET |
                   AUDIO_DEVICE_OUT_WIRED_HEADPHONE |
                   AUDIO_DEVICE_OUT_WIRED_HEADPHONE2)) &&
        ((stream_strategy == STRATEGY_SONIFICATION) ||
         (stream_strategy == STRATEGY_SONIFICATION_RESPECTFUL) ||
         (stream == AudioSystem::SYSTEM) ||
         ((stream_strategy == STRATEGY_ENFORCED_AUDIBLE) &&
          (mForceUse[AudioSystem::FOR_SYSTEM] == AudioSystem::FORCE_NONE))) &&
        streamDesc.mCanBeMuted) {
        volume *= SONIFICATION_HEADSET_VOLUME_FACTOR;
        float minVol = (volume > SONIFICATION_HEADSET_VOLUME_MIN) ?
            volume : SONIFICATION_HEADSET_VOLUME_MIN;
        if (volume > minVol) {
            volume = minVol;
            ALOGV("computeVolume() limiting volume to %f (was %f)", minVol, volume);
        }
    }

    ALOGV("computeVolume() output %d stream %d index %d devices 0x%08x volume %f",
          output, stream, index, device, volume);

    return volume;
}

float AudioPolicyManager::volIndexToAmpl(audio_devices_t device,
                                         const StreamDescriptor& streamDesc,
                                         int indexInUi)
{
    device_category deviceCategory = getDeviceCategory(device);
    const VolumeCurvePoint *curve = streamDesc.mVolumeCurve[deviceCategory];

    // The volume index in the UI is relative to the min and max volume
    // indices for this stream type
    int nbSteps = 1 + curve[VOLMAX].mIndex - curve[VOLMIN].mIndex;
    int volIdx = (nbSteps * (indexInUi - streamDesc.mIndexMin)) /
            (streamDesc.mIndexMax - streamDesc.mIndexMin);

    // Find what part of the curve this index volume belongs to or if
    // it's out of bounds
    int segment = 0;
    if (volIdx < curve[VOLMIN].mIndex) {   // out of bounds
        return 0.0f;
    } else if (volIdx < curve[VOLKNEE1].mIndex) {
        segment = 0;
    } else if (volIdx < curve[VOLKNEE2].mIndex) {
        segment = 1;
    } else if (volIdx <= curve[VOLMAX].mIndex) {
        segment = 2;
    } else {                               // out of bounds
        return 1.0f;
    }

    // Linear interpolation in the attenuation table in dB
    float decibels = curve[segment].mDBAttenuation +
            ((float)(volIdx - curve[segment].mIndex)) *
                ( (curve[segment+1].mDBAttenuation -
                        curve[segment].mDBAttenuation) /
                    ((float)(curve[segment+1].mIndex -
                            curve[segment].mIndex)) );

    float amplification = exp( decibels * 0.115129f); // exp( dB * ln(10) / 20 )

    ALOGVV("VOLUME vol index=[%d %d %d], dB=[%.1f %.1f %.1f] ampl=%.5f",
           curve[segment].mIndex, volIdx,
           curve[segment+1].mIndex,
           curve[segment].mDBAttenuation,
           decibels,
           curve[segment+1].mDBAttenuation,
           amplification);

    return amplification;
}

status_t AudioPolicyManager::setVolume(int stream,
                                       audio_io_handle_t output,
                                       int delayMs)
{
    return setVolume_l(stream, output, false, delayMs);
}

status_t AudioPolicyManager::muteVolume(int stream,
                                        audio_io_handle_t output,
                                        int delayMs)
{
    return setVolume_l(stream, output, true, delayMs);
}

status_t AudioPolicyManager::setVolume_l(int stream,
                                         audio_io_handle_t output,
                                         bool mute,
                                         int delayMs)
{
    AudioOutputDescriptor *desc = mOutputs.valueFor(output);
    status_t status;

    ALOGV("setVolume_l() output %d stream %d mute %d delayMs %d",
          output, stream, mute, delayMs);

    if (desc->isDuplicated())
        status = setDuplicatingVolume(stream, output, mute, delayMs);
    else
        status = setStreamVolume(stream, output, mute, delayMs);

    return status;
}

status_t AudioPolicyManager::setStreamVolume(int stream,
                                             audio_io_handle_t output,
                                             bool mute,
                                             int delayMs)
{
    AudioOutputDescriptor *desc = mOutputs.valueFor(output);

    // Select the most relevant device for volume computation
    audio_devices_t device =  getDeviceForVolume(desc->device());

    int index = mute ? 0 : mStreams[stream].mIndexCur;

    ALOGV("setStreamVolume() output %d stream %d index %d delayMs %d",
          output, stream, index, delayMs);

    // Do not change actual stream volume if the stream is muted
    if (desc->mMuteCount[stream] != 0) {
        ALOGV("setStreamVolume() stream %d muted count %d",
              stream, desc->mMuteCount[stream]);
        return NO_ERROR;
    }

    float volume = computeVolume(stream, index, output, device);
    if (volume != desc->mCurVolume[stream]) {
        desc->mCurVolume[stream] = volume;

        ALOGV("setStreamVolume() for output %d stream %d, volume %f, delay %d",
              output, stream, volume, delayMs);

        // Force VOICE_CALL to track BLUETOOTH_SCO stream volume when
        // bluetooth audio is enabled
        if (stream == AudioSystem::BLUETOOTH_SCO) {
            mpClientInterface->setStreamVolume(AudioSystem::VOICE_CALL,
                                               volume, output, delayMs);
        }
        mpClientInterface->setStreamVolume((AudioSystem::stream_type)stream,
                                           volume, output, delayMs);
    }

    if (stream == AudioSystem::VOICE_CALL ||
        stream == AudioSystem::BLUETOOTH_SCO) {
        float voiceVolume = (float)index/(float)mStreams[stream].mIndexMax;
        if (voiceVolume != mLastVoiceVolume && output == mPrimaryOutput) {
            mpClientInterface->setVoiceVolume(voiceVolume, delayMs);
            mLastVoiceVolume = voiceVolume;
        }
    }

    return NO_ERROR;
}

status_t AudioPolicyManager::setDuplicatingVolume(int stream,
                                                  audio_io_handle_t output,
                                                  bool mute,
                                                  int delayMs)
{
    AudioOutputDescriptor *desc = mOutputs.valueFor(output);

    ALOGV("setDuplicatingVolume() output %d stream %d delayMs %d",
          output, stream, delayMs);

    if (!desc->isDuplicated()) {
        ALOGE("setDuplicatingVolume() output %d is not duplicate", output);
        return INVALID_OPERATION;
    }

    // Set the volume for all outputs that are receiving the duplicated data.
    // The volume to be set is in the OutputTrack, which only exists for
    // duplicating threads
    for (size_t i = 0; i < desc->mDupOutputs.size(); i++) {
        AudioOutputDescriptor *dupDesc = desc->mDupOutputs.itemAt(i);
        if (dupDesc) {
            // Select the most relevant device for volume computation
            ALOGE("setDuplicatingVolume() output %d device 0x%08x forvol 0x%08x",
                  dupDesc->mId, dupDesc->device(), getDeviceForVolume(dupDesc->device()));
            audio_devices_t device = getDeviceForVolume(dupDesc->device());
            int index = mute ? 0 : mStreams[stream].mIndexCur;
            float volume;
            if (!dupDesc->mMuteCount[stream])
                volume = computeVolume(stream, index, output, device);
            else
                volume = 0;

            ALOGV("setDuplicatingVolume() dup output %d devices 0x%08x "
                  "index %d volume %f", dupDesc->mId, device, index, volume);
            mpClientInterface->setDuplicatingVolume(output, dupDesc->mId, volume);
        }
    }

    // The volume for the duplicating output is set to 0dB as its OutputTrack's
    // already took care of per-device category volume
    for (int i = 0; i < AudioSystem::NUM_STREAM_TYPES; i++) {
        mpClientInterface->setStreamVolume((AudioSystem::stream_type)i,
                                           1.0f, output, delayMs);
    }

    return NO_ERROR;
}

void AudioPolicyManager::setStreamMute(int stream,
                                       bool on,
                                       audio_io_handle_t output,
                                       int delayMs,
                                       audio_devices_t device)
{
    StreamDescriptor &streamDesc = mStreams[stream];
    AudioOutputDescriptor *outputDesc = mOutputs.valueFor(output);

    if (device == AUDIO_DEVICE_NONE)
        device = outputDesc->device();

    ALOGV("setStreamMute() stream %d mute %d output %d mMuteCount %d device 0x%08x",
          stream, on, output, outputDesc->mMuteCount[stream], device);

    if (on) {
        if (outputDesc->mMuteCount[stream] == 0) {
            if (streamDesc.mCanBeMuted &&
                ((stream != AudioSystem::ENFORCED_AUDIBLE) ||
                 (mForceUse[AudioSystem::FOR_SYSTEM] == AudioSystem::FORCE_NONE))) {
                muteVolume(stream, output, delayMs);
            }
        }
        // Increment mMuteCount after calling muteVolume() so that volume change
        // is not ignored
        ALOGV("setStreamMute() output %d stream %d mute %d->%d", output, stream,
              outputDesc->mMuteCount[stream], outputDesc->mMuteCount[stream]+1);
        outputDesc->mMuteCount[stream]++;
    } else {
        if (outputDesc->mMuteCount[stream] == 0) {
            ALOGW("setStreamMute() unmuting non muted stream!");
            return;
        }
        ALOGV("setStreamMute() output %d stream %d mute %d->%d", output, stream,
              outputDesc->mMuteCount[stream], outputDesc->mMuteCount[stream]-1);
        if (--outputDesc->mMuteCount[stream] == 0)
            setVolume(stream, output, delayMs);
    }
}

void AudioPolicyManager::initSessionVolume(int session)
{
    // Session 0 is a special case where the output is used only to query
    // hardware parameters, no need for volume updates
    if (!session)
        return;

    SessionDescriptor *sessionDesc = mSessions.valueFor(session);
    audio_zones_t zones = getSessionZones(session);

    for (audio_zones_t zone = AUDIO_ZONE_LAST; zone & AUDIO_ZONE_ALL; zone >>= 1) {
        audio_zones_t cur = zone & zones;
        if (cur != AUDIO_ZONE_NONE) {
            float zoneVolume = mZoneVolume.valueFor(cur);
            float sessionVolume = sessionDesc->mVolume.valueFor(cur);
            float volume = zoneVolume * sessionVolume;

            ALOGI("initSessionVolume() session %d zone 0x%0x "
                  "zone-vol %f session-vol %f final-vol %f",
                  session, cur, zoneVolume, sessionVolume, volume);

            setSessionVolume_l(session, cur, volume);
        }
    }
}

bool AudioPolicyManager::isInCall()
{
    return isStateInCall(mPhoneState);
}

bool AudioPolicyManager::isStateInCall(int state) {
    return ((state == AudioSystem::MODE_IN_CALL) ||
            (state == AudioSystem::MODE_IN_COMMUNICATION));
}



void AudioPolicyManager::handleIncallSonification(int stream, bool starting, bool stateChange)
{
    // if the stream pertains to sonification strategy and we are in call we must
    // mute the stream if it is low visibility. If it is high visibility, we must play a tone
    // in the device used for phone strategy and play the tone if the selected device does not
    // interfere with the device used for phone strategy
    // if stateChange is true, we are called from setPhoneState() and we must mute or unmute as
    // many times as there are active tracks on the output

    const routing_strategy stream_strategy = getStrategy((AudioSystem::stream_type)stream);

    if ((stream_strategy != STRATEGY_SONIFICATION) &&
        (stream_strategy != STRATEGY_SONIFICATION_RESPECTFUL)) {
        return;
    }

    AudioOutputDescriptor *outputDesc = mOutputs.valueFor(mPrimaryOutput);

    ALOGV("handleIncallSonification() stream %d starting %d device %x stateChange %d",
          stream, starting, outputDesc->mDevice, stateChange);

    // nothing to do if output is inactive
    if (!outputDesc->mRefCount[stream]) {
        return;
    }

    int muteCount = 1;
    if (stateChange) {
        muteCount = outputDesc->mRefCount[stream];
    }
    if (AudioSystem::isLowVisibility((AudioSystem::stream_type)stream)) {
        ALOGV("handleIncallSonification() low visibility, muteCount %d", muteCount);
        for (int i = 0; i < muteCount; i++) {
            setStreamMute(stream, starting, mPrimaryOutput);
        }
    } else {
        ALOGV("handleIncallSonification() high visibility");
        if (outputDesc->device() &
            getRefinedZoneDevices(AUDIO_ZONE_CABIN, AudioSystem::VOICE_CALL)) {
            ALOGV("handleIncallSonification() high visibility muted, muteCount %d",
                  muteCount);
            for (int i = 0; i < muteCount; i++) {
                setStreamMute(stream, starting, mPrimaryOutput);
            }
        }
        if (starting) {
            mpClientInterface->startTone(ToneGenerator::TONE_SUP_CALL_WAITING,
                                         AudioSystem::VOICE_CALL);
        } else {
            mpClientInterface->stopTone();
        }
    }
}

void AudioPolicyManager::checkA2dpSuspend()
{
    if (!mHasA2dp) {
        return;
    }
    audio_io_handle_t a2dpOutput = getA2dpOutput();
    if (a2dpOutput == 0) {
        return;
    }

    // suspend A2DP output if:
    //      (NOT already suspended) &&
    //      ((SCO device is connected &&
    //       (forced usage for communication || for record is SCO))) ||
    //      (phone state is ringing || in call)
    //
    // restore A2DP output if:
    //      (Already suspended) &&
    //      ((SCO device is NOT connected ||
    //       (forced usage NOT for communication && NOT for record is SCO))) &&
    //      (phone state is NOT ringing && NOT in call)
    //
    if (mA2dpSuspended) {
        if (((mScoDeviceAddress == "") ||
             ((mForceUse[AudioSystem::FOR_COMMUNICATION] != AudioSystem::FORCE_BT_SCO) &&
              (mForceUse[AudioSystem::FOR_RECORD] != AudioSystem::FORCE_BT_SCO))) &&
            ((mPhoneState != AudioSystem::MODE_IN_CALL) &&
             (mPhoneState != AudioSystem::MODE_RINGTONE))) {
            mpClientInterface->restoreOutput(a2dpOutput);
            mA2dpSuspended = false;
        }
    } else {
        if (((mScoDeviceAddress != "") &&
             ((mForceUse[AudioSystem::FOR_COMMUNICATION] == AudioSystem::FORCE_BT_SCO) ||
              (mForceUse[AudioSystem::FOR_RECORD] == AudioSystem::FORCE_BT_SCO))) ||
            ((mPhoneState == AudioSystem::MODE_IN_CALL) ||
             (mPhoneState == AudioSystem::MODE_RINGTONE))) {

            mpClientInterface->suspendOutput(a2dpOutput);
            mA2dpSuspended = true;
        }
    }
}

audio_io_handle_t AudioPolicyManager::getA2dpOutput()
{
    if (!mHasA2dp)
        return 0;

    for (size_t i = 0; i < mOutputs.size(); i++) {
        AudioOutputDescriptor *outputDesc = mOutputs.valueAt(i);
        if (!outputDesc->isDuplicated() && outputDesc->device() & AUDIO_DEVICE_OUT_ALL_A2DP) {
            return mOutputs.keyAt(i);
        }
    }

    return 0;
}

uint32_t AudioPolicyManager::getMaxEffectsCpuLoad()
{
    return MAX_EFFECTS_CPU_LOAD;
}

uint32_t AudioPolicyManager::getMaxEffectsMemory()
{
    return MAX_EFFECTS_MEMORY;
}

IOProfile *AudioPolicyManager::getInputProfile(audio_devices_t device,
                                               uint32_t samplingRate,
                                               uint32_t format,
                                               uint32_t channelMask)
{
    // Choose an input profile based on the requested capture parameters:
    // select the first available profile supporting all requested parameters
    for (size_t i = 0; i < mHwModules.size(); i++) {
        if (mHwModules[i]->mHandle == 0) {
            continue;
        }
        for (size_t j = 0; j < mHwModules[i]->mInputProfiles.size(); j++) {
            IOProfile *profile = mHwModules[i]->mInputProfiles[j];
            if (profile->isCompatibleProfile(device, samplingRate,
                                             format, channelMask,
                                             (audio_output_flags_t)0)) {
                return profile;
            }
        }
    }
    return NULL;
}

audio_devices_t AudioPolicyManager::getDeviceForInputSource(int inputSource)
{
    uint32_t device = AUDIO_DEVICE_NONE;

    switch (inputSource) {
    case AUDIO_SOURCE_VOICE_UPLINK:
      if (mAvailableInputDevices & AUDIO_DEVICE_IN_VOICE_CALL) {
          device = AUDIO_DEVICE_IN_VOICE_CALL;
          break;
      }
      // FALL THROUGH

    case AUDIO_SOURCE_DEFAULT:
    case AUDIO_SOURCE_MIC:
    case AUDIO_SOURCE_VOICE_RECOGNITION:
    case AUDIO_SOURCE_VOICE_COMMUNICATION:
        if (mForceUse[AudioSystem::FOR_RECORD] == AudioSystem::FORCE_BT_SCO &&
            mAvailableInputDevices & AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET) {
            device = AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET;
        } else if (mAvailableInputDevices & AUDIO_DEVICE_IN_WIRED_HEADSET) {
            device = AUDIO_DEVICE_IN_WIRED_HEADSET;
        } else if (mAvailableInputDevices & AUDIO_DEVICE_IN_BUILTIN_MIC) {
            device = AUDIO_DEVICE_IN_BUILTIN_MIC;
        }
        break;
    case AUDIO_SOURCE_CAMCORDER:
        if (mAvailableInputDevices & AUDIO_DEVICE_IN_BACK_MIC) {
            device = AUDIO_DEVICE_IN_BACK_MIC;
        } else if (mAvailableInputDevices & AUDIO_DEVICE_IN_BUILTIN_MIC) {
            device = AUDIO_DEVICE_IN_BUILTIN_MIC;
        }
        break;
    case AUDIO_SOURCE_VOICE_DOWNLINK:
    case AUDIO_SOURCE_VOICE_CALL:
        if (mAvailableInputDevices & AUDIO_DEVICE_IN_VOICE_CALL) {
            device = AUDIO_DEVICE_IN_VOICE_CALL;
        }
        break;
    case AUDIO_SOURCE_REMOTE_SUBMIX:
        if (mAvailableInputDevices & AUDIO_DEVICE_IN_REMOTE_SUBMIX) {
            device = AUDIO_DEVICE_IN_REMOTE_SUBMIX;
        }
        break;
    case AUDIO_SOURCE_AUXILIARY_INPUT:
        if (mAvailableInputDevices & AUDIO_DEVICE_IN_ANLG_DOCK_HEADSET) {
            device = AUDIO_DEVICE_IN_ANLG_DOCK_HEADSET;
        }
        break;
    default:
        ALOGW("getDeviceForInputSource() invalid input source %d", inputSource);
        break;
    }

    ALOGV("getDeviceForInputSource()input source %d, device %08x", inputSource, device);
    return device;
}

bool AudioPolicyManager::isVirtualInputDevice(audio_devices_t device)
{
    if ((device & AUDIO_DEVICE_BIT_IN) != 0) {
        device &= ~AUDIO_DEVICE_BIT_IN;
        if ((popcount(device) == 1) && ((device & ~APM_AUDIO_IN_DEVICE_VIRTUAL_ALL) == 0))
            return true;
    }
    return false;
}

audio_io_handle_t AudioPolicyManager::getActiveInput(bool ignoreVirtualInputs)
{
    for (size_t i = 0; i < mInputs.size(); i++) {
        const AudioInputDescriptor *inputDesc = mInputs.valueAt(i);
        if ((inputDesc->mUsers > 0) &&
            (!ignoreVirtualInputs || !isVirtualInputDevice(inputDesc->mDevice))) {
            return mInputs.keyAt(i);
        }
    }
    return 0;
}

// --- AudioOutputDescriptor class implementation

AudioPolicyManager::AudioOutputDescriptor::AudioOutputDescriptor(const IOProfile *profile)
    : mId(0), mSamplingRate(0), mFormat((audio_format_t)0),
      mChannelMask((audio_channel_mask_t)0), mLatency(0),
      mFlags((audio_output_flags_t)0), mDevice(AUDIO_DEVICE_NONE),
      mUsers(0), mProfile(profile)
{
    // clear usage count for all stream types
    for (int i = 0; i < AudioSystem::NUM_STREAM_TYPES; i++) {
        mRefCount[i] = 0;
        mCurVolume[i] = -1.0;
        mMuteCount[i] = 0;
        mStopTime[i] = 0;
    }
    mDupOutputs.clear();
    if (profile != NULL) {
        mSamplingRate = profile->mSamplingRates[0];
        mFormat = profile->mFormats[0];
        mChannelMask = profile->mChannelMasks[0];
        mFlags = profile->mFlags;
    }
}

audio_devices_t AudioPolicyManager::AudioOutputDescriptor::device() const
{
    audio_devices_t devices = AUDIO_DEVICE_NONE;

    if (isDuplicated()) {
        // All devices supported for each individual duplicated output
        for (size_t i = 0; i < mDupOutputs.size(); i++) {
            AudioOutputDescriptor *dupDesc = mDupOutputs.itemAt(i);
            if (dupDesc)
                devices |= dupDesc->device();
        }
    } else {
        devices = mDevice;
    }

    return devices;
}

uint32_t AudioPolicyManager::AudioOutputDescriptor::latency()
{
    uint32_t latency = 0;

    if (isDuplicated()) {
        // Highest latency in all duplicated outputs
        for (size_t i = 0; i < mDupOutputs.size(); i++) {
            AudioOutputDescriptor *dupDesc = mDupOutputs.itemAt(i);
            if (dupDesc) {
                latency = (latency > dupDesc->mLatency) ?
                                 latency : dupDesc->mLatency;
            }
        }
    } else {
        latency = mLatency;
    }

    return latency;
}

void AudioPolicyManager::AudioOutputDescriptor::changeRefCount(AudioSystem::stream_type stream,
                                                               int delta)
{
    // Forward usage count change to attached outputs
    if (isDuplicated()) {
        for (size_t i = 0; i < mDupOutputs.size(); i++) {
            AudioOutputDescriptor *dupDesc = mDupOutputs.itemAt(i);
            if (dupDesc) {
                dupDesc->changeRefCount(stream, delta);
            }
        }
    }
    if ((delta + (int)mRefCount[stream]) < 0) {
        ALOGW("changeRefCount() invalid delta %d for stream %d refCount %d",
              delta, stream, mRefCount[stream]);
        mRefCount[stream] = 0;
        return;
    }
    mRefCount[stream] += delta;
    ALOGVV("changeRefCount() stream %d count %d", stream, mRefCount[stream]);
}

bool AudioPolicyManager::AudioOutputDescriptor::isDuplicated() const
{
    return !mDupOutputs.isEmpty();
}

bool AudioPolicyManager::AudioOutputDescriptor::isDirectOutput() const
{
    return !!(mFlags & AUDIO_OUTPUT_FLAG_DIRECT);
}

audio_devices_t AudioPolicyManager::AudioOutputDescriptor::supportedDevices()
{
    audio_devices_t devices = AUDIO_DEVICE_NONE;

    if (isDuplicated()) {
        for (size_t i = 0; i < mDupOutputs.size(); i++) {
            AudioOutputDescriptor *dupDesc = mDupOutputs.itemAt(i);
            if (dupDesc)
                devices |= dupDesc->supportedDevices();
        }
    } else {
        devices = mProfile->mSupportedDevices;
    }

    return devices;
}

bool AudioPolicyManager::AudioOutputDescriptor::isStreamActive(AudioSystem::stream_type stream,
                                                               uint32_t inPastMs,
                                                               nsecs_t sysTime) const
{
    if (mRefCount[stream] != 0)
        return true;

    if (inPastMs == 0)
        return false;

    if (sysTime == 0)
        sysTime = systemTime();

    if (ns2ms(sysTime - mStopTime[stream]) < inPastMs)
        return true;

    return false;
}

status_t AudioPolicyManager::AudioOutputDescriptor::dump(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, " Sampling rate: %d\n", mSamplingRate);
    result.append(buffer);
    snprintf(buffer, SIZE, " Format: %d\n", mFormat);
    result.append(buffer);
    snprintf(buffer, SIZE, " Channels: %08x\n", mChannelMask);
    result.append(buffer);
    snprintf(buffer, SIZE, " Latency: %d\n", mLatency);
    result.append(buffer);
    snprintf(buffer, SIZE, " Flags: 0x%08x\n", mFlags);
    result.append(buffer);
    snprintf(buffer, SIZE, " Devices: 0x%08x\n", device());
    result.append(buffer);
    snprintf(buffer, SIZE, " Stream volume refCount muteCount\n");
    result.append(buffer);
    for (int i = 0; i < AudioSystem::NUM_STREAM_TYPES; i++) {
        snprintf(buffer, SIZE, " %02d     %.03f     %02d       %02d\n",
                 i, mCurVolume[i], mRefCount[i], mMuteCount[i]);
        result.append(buffer);
    }
    write(fd, result.string(), result.size());

    return NO_ERROR;
}

// --- AudioInputDescriptor class implementation

AudioPolicyManager::AudioInputDescriptor::AudioInputDescriptor(const IOProfile *profile)
    : mSamplingRate(0), mFormat((audio_format_t)0),
      mChannelMask((audio_channel_mask_t)0),
      mDevice(AUDIO_DEVICE_NONE), mUsers(0),
      mInputSource(0), mProfile(profile)
{
}

status_t AudioPolicyManager::AudioInputDescriptor::dump(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, " Sampling rate: %d\n", mSamplingRate);
    result.append(buffer);
    snprintf(buffer, SIZE, " Format: %d\n", mFormat);
    result.append(buffer);
    snprintf(buffer, SIZE, " Channels: %08x\n", mChannelMask);
    result.append(buffer);
    snprintf(buffer, SIZE, " Devices: 0x%08x\n", mDevice);
    result.append(buffer);
    snprintf(buffer, SIZE, " Ref Count: %d\n", mUsers);
    result.append(buffer);
    write(fd, result.string(), result.size());

    return NO_ERROR;
}

// --- StreamDescriptor class implementation

AudioPolicyManager::StreamDescriptor::StreamDescriptor()
    : mIndexMin(0), mIndexMax(1), mIndexCur(0), mCanBeMuted(true)
{
}

void AudioPolicyManager::StreamDescriptor::dump(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, "%s         %02d         %02d         %02d\n",
             mCanBeMuted ? "true " : "false", mIndexMin, mIndexMax, mIndexCur);
    result.append(buffer);
    write(fd, result.string(), result.size());
}

// --- EffectDescriptor class implementation

status_t AudioPolicyManager::EffectDescriptor::dump(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, " I/O: %d\n", mIo);
    result.append(buffer);
    snprintf(buffer, SIZE, " Strategy: %d\n", mStrategy);
    result.append(buffer);
    snprintf(buffer, SIZE, " Session: %d\n", mSession);
    result.append(buffer);
    snprintf(buffer, SIZE, " Name: %s\n",  mDesc.name);
    result.append(buffer);
    snprintf(buffer, SIZE, " %s\n",  mEnabled ? "Enabled" : "Disabled");
    result.append(buffer);
    write(fd, result.string(), result.size());

    return NO_ERROR;
}

// --- SessionDescriptor class implementation

AudioPolicyManager::SessionDescriptor::SessionDescriptor(int session,
                                                         audio_io_handle_t output,
                                                         audio_zones_t zones,
                                                         audio_devices_t devices)
    : mSession(session), mId(output), mZones(zones), mDevices(devices), mUsers(0)
{
    for (audio_zones_t zone = AUDIO_ZONE_LAST; zone & AUDIO_ZONE_ALL; zone >>= 1) {
        mVolume.add(zone, 1.0f);
    }
}

void AudioPolicyManager::SessionDescriptor::dump(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, " AudioSessionId: %d\n", mSession);
    result.append(buffer);
    snprintf(buffer, SIZE, " Output: %d\n", mId);
    result.append(buffer);
    snprintf(buffer, SIZE, " Zones: 0x%x\n", mZones);
    result.append(buffer);
    snprintf(buffer, SIZE, " Devices: 0x%08x\n",  mDevices);
    result.append(buffer);
    for (size_t i = 0; i < mVolume.size(); i++) {
        snprintf(buffer, SIZE, " Zone 0x%x volume: %f\n",
                 mVolume.keyAt(i), mVolume.valueAt(i));
        result.append(buffer);
    }
    snprintf(buffer, SIZE, " Users: %d\n",  mUsers);
    result.append(buffer);
    write(fd, result.string(), result.size());
}

} // namespace android
