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
 */

#ifndef _POLICY_MULTIZONE_AUDIOPOLICYPARSER_H_
#define _POLICY_MULTIZONE_AUDIOPOLICYPARSER_H_

#include <stdint.h>
#include <sys/types.h>
#include <cutils/config_utils.h>
#include <cutils/misc.h>
#include <utils/Timers.h>
#include <utils/Errors.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>

#include <hardware_legacy/AudioPolicyInterface.h>

namespace android_audio_legacy {
    using android::KeyedVector;

class IOProfile;

class HwModule {
 public:
    HwModule(const char *name);
    ~HwModule();

    void dump(int fd);

    const char *const mName; // base name of the audio HW module (primary, a2dp ...)
    audio_module_handle_t mHandle;
    Vector <IOProfile *> mOutputProfiles; // output profiles exposed by this module
    Vector <IOProfile *> mInputProfiles;  // input profiles exposed by this module
};

// the IOProfile class describes the capabilities of an output or input stream.
// It is currently assumed that all combination of listed parameters are supported.
// It is used by the policy manager to determine if an output or input is suitable for
// a given use case,  open/close it accordingly and connect/disconnect audio tracks
// to/from it.
class IOProfile
{
 public:
    IOProfile(HwModule *module);
    ~IOProfile();

    bool isCompatibleProfile(audio_devices_t device,
                             uint32_t samplingRate,
                             uint32_t format,
                             uint32_t channelMask,
                             audio_output_flags_t flags) const;

    bool hasDynamicParams() const;
    bool isValid() const;

    void dump(int fd);

    // by convention, "0' in the first entry in mSamplingRates, mChannelMasks or mFormats
    // indicates the supported parameters should be read from the output stream
    // after it is opened for the first time
    Vector <uint32_t> mSamplingRates; // supported sampling rates
    Vector <audio_channel_mask_t> mChannelMasks; // supported channel masks
    Vector <audio_format_t> mFormats; // supported audio formats
    audio_devices_t mSupportedDevices; // supported devices (devices this output can be
                                       // routed to)
    audio_output_flags_t mFlags; // attribute flags (e.g primary output,
                                 // direct output...). For outputs only.
    HwModule *mModule;           // audio HW module exposing this I/O stream
};

class AudioPolicyParser {
 public:
    //
    // Audio policy configuration file parsing (audio_policy.conf)
    //
    AudioPolicyParser(Vector<HwModule *> &modules,
                      KeyedVector<audio_zones_t, audio_devices_t> &zoneAffinity);
    virtual ~AudioPolicyParser();

    status_t loadAudioPolicyConfig(const char *path);
    void defaultAudioPolicyConfig();

    void fillProfile(IOProfile *profile, const char *rates,
                     const char *formats, const char *channels);

    audio_devices_t getAttachedOutputDevices() const { return mAttachedOutputDevices; }
    audio_devices_t getDefaultOutputDevice() const { return mDefaultOutputDevice; }
    audio_devices_t getAttachedInputDevices() const { return mAttachedInputDevices; }

    bool supportsA2DP() const { return mHasA2dp; }
    bool supportsUSB() const { return mHasUsb; }
    bool supportsRemoteSubmix() const { return mHasRemoteSubmix; }

 protected:
    static uint32_t stringToEnum(const struct StringToEnum *table,
                                 size_t size, const char *name);
    static audio_output_flags_t parseFlagNames(char *name);
    static audio_devices_t parseDeviceNames(char *name);
    void loadSamplingRates(char *name, IOProfile *profile);
    void loadFormats(char *name, IOProfile *profile);
    void loadOutChannels(char *name, IOProfile *profile);
    void loadInChannels(char *name, IOProfile *profile);
    status_t loadOutput(cnode *root,  HwModule *module);
    status_t loadInput(cnode *root,  HwModule *module);
    void loadHwModule(cnode *root);
    void loadHwModules(cnode *root);
    void loadGlobalConfig(cnode *root);
    void loadZoneAffinity(cnode *root);

    Vector <HwModule *> &mHwModules;
    KeyedVector<audio_zones_t, audio_devices_t> &mZoneAffinity;

    audio_devices_t mAttachedOutputDevices;  // field "attached_output_devices"
    audio_devices_t mDefaultOutputDevice;    // field "default_output_device"
    audio_devices_t mAttachedInputDevices;   // field "attached_input_devices"
    bool mHasA2dp;                           // Support for bluetooth A2DP
    bool mHasUsb;                            // Support for USB audio
    bool mHasRemoteSubmix;                   // Supportt for remote presentation of a submix
};

}

#endif // _POLICY_MULTIZONE_AUDIOPOLICYPARSER_H_
