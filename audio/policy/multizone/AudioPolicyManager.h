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

#ifndef _POLICY_MULTIZONE_AUDIOPOLICYMANAGER_H_
#define _POLICY_MULTIZONE_AUDIOPOLICYMANAGER_H_

#include <stdint.h>
#include <sys/types.h>
#include <cutils/config_utils.h>
#include <cutils/misc.h>
#include <utils/Timers.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/SortedVector.h>

#include <hardware_legacy/AudioPolicyInterface.h>
#include "AudioPolicyParser.h"

namespace android_audio_legacy {
    using android::KeyedVector;
    using android::DefaultKeyedVector;
    using android::SortedVector;

// ----------------------------------------------------------------------------

#define MAX_DEVICE_ADDRESS_LEN 20

// Attenuation applied to STRATEGY_SONIFICATION streams when a headset is connected: 6dB
#define SONIFICATION_HEADSET_VOLUME_FACTOR 0.5

// Min volume for STRATEGY_SONIFICATION streams when limited by music volume: -36dB
#define SONIFICATION_HEADSET_VOLUME_MIN  0.016

// Time in milliseconds during witch some streams are muted while the audio path
// is switched
#define MUTE_TIME_MS 2000

#define NUM_VOL_CURVE_KNEES 2

class AudioPolicyManager: public AudioPolicyInterface
{

public:
    AudioPolicyManager(AudioPolicyClientInterface *clientInterface);

    virtual ~AudioPolicyManager();

    // AudioPolicyInterface

    // Set the connection state of a hot-pluggable audio device.
    virtual status_t setDeviceConnectionState(audio_devices_t device,
                                              AudioSystem::device_connection_state state,
                                              const char *device_address);

    // Get the connection state of an audio device.
    virtual AudioSystem::device_connection_state getDeviceConnectionState(audio_devices_t device,
                                                                          const char *device_address);

    // Set the phone state: normal, ringong, in-call, communication.
    virtual void setPhoneState(int state);

    // Set the config for a force use. The multizone routing APIs replace it.
    virtual void setForceUse(AudioSystem::force_use usage,
                             AudioSystem::forced_config config);

    // Get the current config used for a force use.
    virtual AudioSystem::forced_config getForceUse(AudioSystem::force_use usage);

    virtual void setSystemProperty(const char* property, const char* value);

    virtual status_t initCheck();

    // === Multizone Routing ===

    // Get the devices that cannot be reassigned to different zones. These are
    // the default devices for each listening zone.
    virtual audio_devices_t getPrimaryDevices();

    // Get the devices that can be assigned to a listening zone. It's obtained
    // from the "zone_affinity" section in the audio_policy.conf file.
    virtual audio_devices_t getZoneSupportedDevices(audio_zones_t zone);

    // Assign devices to a listening zone, tracks playing on that zone will be
    // re-routed to the new devices.
    virtual status_t setZoneDevices(audio_zones_t zone,
                                    audio_devices_t devices);

    // Get the current devices assigned to a listening zone.
    virtual audio_devices_t getZoneDevices(audio_zones_t zones);

    // Set the listening zones where an audio track plays. The zones can be
    // set pre stream start or mid-stream, the latter causes re-routing.
    virtual status_t setSessionZones(int session,
                                     audio_zones_t zones);

    // Get the listening zones where an audio track is playing (or will play if
    // not playing yet).
    virtual audio_zones_t getSessionZones(int session);

    // === Multizone Volumes ===

    // Set the volume for a listening zone. Tracks playing on that zone will be
    // updated to the new volume.
    virtual status_t setZoneVolume(audio_zones_t zone,
                                   float volume);

    // Get the current volume of a listening zone.
    virtual float getZoneVolume(audio_zones_t zone);

    // Set the volume for a listening zone of a specific track. The actual
    // volume of the track takes into account the listening zone volume
    // (set through setZoneVolume()).
    virtual status_t setSessionVolume(int session,
                                      audio_zones_t zones,
                                      float volume);

    // Get the volume of a listening zone of a specific session.
    virtual float getSessionVolume(int session, audio_zones_t zone);

    // === Outputs ===

    // Get a new output for an audio track. It can be called multiple times with
    // session 0 to query hardware parameters.
    virtual audio_io_handle_t getOutput(AudioSystem::stream_type stream,
                                        uint32_t samplingRate = 0,
                                        uint32_t format = AudioSystem::FORMAT_DEFAULT,
                                        uint32_t channels = 0,
                                        AudioSystem::output_flags flags =
                                          AudioSystem::OUTPUT_FLAG_INDIRECT,
                                        int session = 0);

    // Start the output. Indicates to the Audio Policy Manager that the output
    // starts being used.
    virtual status_t startOutput(audio_io_handle_t output,
                                 AudioSystem::stream_type stream,
                                 int session = 0);

    // Stop the output. Indicates to the Audio Policy Manager that the output
    // stops being used.
    virtual status_t stopOutput(audio_io_handle_t output,
                                AudioSystem::stream_type stream,
                                int session = 0);

    // Release the output. Hardware output may be closed if it's not used
    // by other audio track (depends on the output type, e.g. mixer, direct).
    virtual void releaseOutput(audio_io_handle_t output,
                               int session = 0);

    // == Inputs ==

    // Get a new input for an audio record.
    virtual audio_io_handle_t getInput(int inputSource,
                                       uint32_t samplingRate,
                                       uint32_t format,
                                       uint32_t channels,
                                       AudioSystem::audio_in_acoustics acoustics,
                                       int session = 0);

    // Start the input. Indicates to the Audio Policy Manager that the input
    // starts being used.
    virtual status_t startInput(audio_io_handle_t input);

    // Stop the output. Indicates to the Audio Policy Manager that the input
    // stops being used.
    virtual status_t stopInput(audio_io_handle_t input);

    // Releae the input.
    virtual void releaseInput(audio_io_handle_t input,
                              int session = 0);

    // == Stream-type Volume ==

    // Initialize per-stream volumes.
    virtual void initStreamVolume(AudioSystem::stream_type stream,
                                  int indexMin,
                                  int indexMax);

    // Set the volume to the equivalent of the given index. Volume computation
    // involves device and stream type dependent curves.
    virtual status_t setStreamVolumeIndex(AudioSystem::stream_type stream,
                                          int index,
                                          audio_devices_t device);

    // Get the current index of a per-stream volume.
    virtual status_t getStreamVolumeIndex(AudioSystem::stream_type stream,
                                          int *index,
                                          audio_devices_t device);

    // Return the strategy corresponding to a given stream type.
    virtual uint32_t getStrategyForStream(AudioSystem::stream_type stream);

    // Return the enabled output devices for the given stream type.
    virtual audio_devices_t getDevicesForStream(AudioSystem::stream_type stream);

    // === Effects ===

    // Get the most accurate output for effects.
    virtual audio_io_handle_t getOutputForEffect(const effect_descriptor_t *desc);

    // Register an audio effect.
    virtual status_t registerEffect(const effect_descriptor_t *desc,
                                    audio_io_handle_t io,
                                    uint32_t strategy,
                                    int session,
                                    int id);

    // Unregister an audio effect.
    virtual status_t unregisterEffect(int id);

    // Enable an audio effect.
    virtual status_t setEffectEnabled(int id, bool enabled);

    // Check if an stream type has been active in the past milliseconds.
    virtual bool isStreamActive(int stream, uint32_t inPastMs = 0) const;

    // Return whether a stream is playing remotely, override to change the
    // definition of local/remote playback, used for instance by notification
    // manager to not make media players lose audio focus when not playing
    // locally.
    virtual bool isStreamActiveRemotely(int stream, uint32_t inPastMs = 0) const;

    // Check if an audio source type is active.
    virtual bool isSourceActive(audio_source_t source) const;

    // Dump the state of the Audio Policy Manager.
    virtual status_t dump(int fd);

protected:

    // Routing strategy that groups stream types
    enum routing_strategy {
        STRATEGY_MEDIA,
        STRATEGY_PHONE,
        STRATEGY_SONIFICATION,
        STRATEGY_SONIFICATION_RESPECTFUL,
        STRATEGY_DTMF,
        STRATEGY_ENFORCED_AUDIBLE,
        NUM_STRATEGIES
    };

    // 4 points to define the volume attenuation curve, each characterized by
    // the volume index (from 0 to 100) at which they apply, and the attenuation
    // in dB at that index. We use 100 steps to avoid rounding errors when
    // computing the volume in volIndexToAmpl().
    enum { VOLMIN = 0, VOLKNEE1 = 1, VOLKNEE2 = 2, VOLMAX = 3, VOLCNT = 4};

    class VolumeCurvePoint {
    public:
        int mIndex;
        float mDBAttenuation;
    };

    // Device categories used for volume curve management.
    enum device_category {
        DEVICE_CATEGORY_HEADSET,
        DEVICE_CATEGORY_SPEAKER,
        DEVICE_CATEGORY_EARPIECE,
        DEVICE_CATEGORY_CNT
    };

    // Default volume curve.
    static const VolumeCurvePoint sDefaultVolumeCurve[AudioPolicyManager::VOLCNT];

    // Default volume curve for media strategy.
    static const VolumeCurvePoint sDefaultMediaVolumeCurve[AudioPolicyManager::VOLCNT];

    // Volume curve for media strategy on speakers.
    static const VolumeCurvePoint sSpeakerMediaVolumeCurve[AudioPolicyManager::VOLCNT];

    // Volume curve for sonification strategy on speakers.
    static const VolumeCurvePoint sSpeakerSonificationVolumeCurve[AudioPolicyManager::VOLCNT];
    static const VolumeCurvePoint sDefaultSystemVolumeCurve[AudioPolicyManager::VOLCNT];
    static const VolumeCurvePoint sHeadsetSystemVolumeCurve[AudioPolicyManager::VOLCNT];
    static const VolumeCurvePoint sDefaultVoiceVolumeCurve[AudioPolicyManager::VOLCNT];
    static const VolumeCurvePoint sSpeakerVoiceVolumeCurve[AudioPolicyManager::VOLCNT];

    // Default volume curves per stream and device category. See initializeVolumeCurves().
    static const VolumeCurvePoint *sVolumeProfiles[AUDIO_STREAM_CNT][DEVICE_CATEGORY_CNT];

    // Descriptor for audio outputs.
    // Used to maintain current configuration of each opened audio output
    // and keep track of the usage of this output by each audio stream type.
    class AudioOutputDescriptor {
    public:
        AudioOutputDescriptor(const IOProfile *profile);
        status_t dump(int fd);

        // Get the current devices active on the output
        audio_devices_t device() const;

        // Set per-stream ref count
        void changeRefCount(AudioSystem::stream_type stream, int delta);

        // Check if the output uses a duplicating thread
        bool isDuplicated() const;

        // Check if the output uses a direct thread
        bool isDirectOutput() const;

        // Devices supported by this port or any linked duplicate output
        audio_devices_t supportedDevices();

        // Hardware latency of this output
        uint32_t latency();

        // Check if a stream type has been active on the output
        bool isStreamActive(AudioSystem::stream_type stream,
                            uint32_t inPastMs = 0,
                            nsecs_t sysTime = 0) const;

        audio_io_handle_t mId;             // Output handle
        uint32_t mSamplingRate;            // Sample rate
        audio_format_t mFormat;            // Data format
        audio_channel_mask_t mChannelMask; // Channels
        uint32_t mLatency;                 // Port latency
        audio_output_flags_t mFlags;       // Flags: deep-buffer, direct, etc
        audio_devices_t mDevice;           // Current device(s) the output is routed to
        uint32_t mUsers;                   // Number of AudioTrack clients using this output

        // Number of streams of each type using this output
        uint32_t mRefCount[AudioSystem::NUM_STREAM_TYPES];

        // Timestamp of the stream stop event
        nsecs_t mStopTime[AudioSystem::NUM_STREAM_TYPES];

        // Descriptors of the duplicated outputs connected to this output
        SortedVector<AudioOutputDescriptor*> mDupOutputs;

        // Current stream volume
        float mCurVolume[AudioSystem::NUM_STREAM_TYPES];

        // Mute request counter
        int mMuteCount[AudioSystem::NUM_STREAM_TYPES];

        // I/O profile this output derives from
        const IOProfile *mProfile;
    };

    // Descriptor for audio inputs.
    // Used to maintain current configuration of each opened audio input
    // and keep track of the usage of this input.
    class AudioInputDescriptor {
    public:
        AudioInputDescriptor(const IOProfile *profile);
        status_t dump(int fd);

        audio_io_handle_t mId;             // Output handle
        uint32_t mSamplingRate;            // Sampling rate
        audio_format_t mFormat;            // Data format
        audio_channel_mask_t mChannelMask; // Channels
        audio_devices_t mDevice;           // Current device this input is routed to
        uint32_t mUsers;                   // Number of AudioRecord clients using this input
        int mInputSource;                  // Input source selected by application (mediarecorder.h)
        const IOProfile *mProfile;         // I/O profile this output derives from
    };

    // Stream descriptor used for volume control.
    class StreamDescriptor {
    public:
        StreamDescriptor();
        void dump(int fd);

        int mIndexMin;                     // Min volume index
        int mIndexMax;                     // Max volume index
        int mIndexCur;                     // Current volume index
        bool mCanBeMuted;                  // true if the stream can be muted
        const VolumeCurvePoint *mVolumeCurve[DEVICE_CATEGORY_CNT];
    };

    // Audio effect descriptor.
    class EffectDescriptor {
    public:
        status_t dump(int fd);

        int mIo;                           // io the effect is attached to
        routing_strategy mStrategy;        // Routing strategy the effect is associated to
        int mSession;                      // Audio session the effect is on
        effect_descriptor_t mDesc;         // Effect descriptor
        bool mEnabled;                     // Enabled state: CPU load being used or not
    };

    // Descriptor of the audio session.
    // Used to hold information about the current listening zones and devices
    // assigned to an audio track.
    class SessionDescriptor {
    public:
        SessionDescriptor(int session,
                          audio_io_handle_t output,
                          audio_zones_t zones,
                          audio_devices_t devices);
        int sessionId() const { return mSession; }
        audio_devices_t devices() const { return mDevices; }
        audio_zones_t zones() const { return mZones; }
        void dump(int fd);

        int mSession;                      // The audio session id
        audio_io_handle_t mId;             // Output handle (can be 0 if not started)
        audio_zones_t mZones;              // Listening zones where to play
        audio_devices_t mDevices;          // Devices where the track is playing
        KeyedVector<audio_zones_t, float> mVolume; // Per-zone volume
        int mUsers;                        // Ref count
    };

    // Return the strategy corresponding to a given stream type.
    static routing_strategy getStrategy(AudioSystem::stream_type stream);

    // Return the category the device belongs to with regard to volume curve
    // management.
    static device_category getDeviceCategory(audio_devices_t device);

    // Extract one device relevant for volume control from multiple device
    // selection.
    static audio_devices_t getDeviceForVolume(audio_devices_t device);

    // === Output management helper methods ===

    // true if current platform requires a specific output to be opened for this
    // particular set of parameters.
    bool needsDirectOuput(AudioSystem::stream_type stream,
                          uint32_t samplingRate,
                          uint32_t format,
                          uint32_t channelMask,
                          AudioSystem::output_flags flags,
                          audio_devices_t device);

    // Query the output parameters dynamically. It's needed for profiles that
    // declare their params as 'dynamic' in the audio_policy.conf file
    // (typically direct outputs).
    void queryOutputParameters(audio_io_handle_t output,
                               IOProfile *profile);

    // Fill the vector with all IOProfiles that support any of the requested
    // devices.
    void getProfilesForDevices(audio_devices_t devices,
                               SortedVector<IOProfile*> &profiles);

    // Set the device address parameters to the output. Currently, this is
    // applicable only to USB and A2DP.
    void setDeviceAddress(audio_io_handle_t output,
                          audio_devices_t device);

    // Refine the outputs by removing all direct outputs or keeping a single
    // direct output. Removing all direct outputs is useful to have a list of
    // outputs for duplication (since AudioFlinger's duplicating output requires
    // mixer outputs). Keeping a single direct output avoids using duplication
    // later but allows rendering with parameters natively supported (e.g.
    // multichannel without down-mixing).
    void refineOutputs(SortedVector<audio_io_handle_t> &outputs);

    // Open an output associated with a given IOProfile using the AudioFlinger.
    // The sampling rate, format and channel mask is only used for direct
    // outputs as the output must match the stream parameters. For mixer
    // outputs, AudioFlinger can use its resampler or downmixer.
    audio_io_handle_t openOutput_l(IOProfile *profile,
                                   audio_devices_t devices,
                                   audio_output_flags_t flags =
                                     AUDIO_OUTPUT_FLAG_NONE,
                                   uint32_t samplingRate = 0,
                                   uint32_t format = 0,
                                   uint32_t channelMask = 0);

    // Open an output (if available) for the given devices and flags. First, it
    // looks into the open outputs and then in the profiles of other registered
    // outputs.
    audio_io_handle_t openOutput(audio_devices_t devices,
                                 uint32_t samplingRate,
                                 uint32_t format,
                                 uint32_t channelMask,
                                 audio_output_flags_t flags);

    // Get an output that duplicates audio data to the requested outputs.
    audio_io_handle_t openDuplicateOutput(SortedVector<audio_io_handle_t> &outputs,
                                          uint32_t samplingRate,
                                          uint32_t format,
                                          uint32_t channelMask,
                                          int session);

    // Add the output and its descriptor to the list of open outputs.
    void addOutput(audio_io_handle_t id, AudioOutputDescriptor *outputDesc);

    // Close an output and its companion duplicated outputs if needed.
    void closeOutput(audio_io_handle_t output);

    // Set the new device for an output. The output has been started before,
    // possibly with different device selection, so the new device is
    // communicated to the audioHAL through set_parameters() call.
    // The delay helps to prevent rendering the tail of the closing streams
    // (buffers that are still not completely flushed) into the new device.
    void setOutputDevice(audio_io_handle_t output,
                         audio_devices_t device,
                         bool force = false,
                         int delayMs = 0);

    // Mute or unmute all stream type volumes on a given output. The volume to be
    // applied upon unmute depends on the device selection (most relevant device
    // for volume).
    void muteOutput(audio_io_handle_t output,
                    bool mute,
                    int delayMs = 0);

    // Move tracks that are playing on the given output. The actual action is the
    // invalidation of those tracks which are re-created by AudioTrack.
    status_t moveTracks(audio_io_handle_t output,
                        audio_devices_t oldDevices,
                        audio_devices_t newDevices);

    // === Multizone routing helper methods ===

    // Set the devices for the listening zone, but doesn't perform any update
    // on sessions that are active on the updated zone as setZoneDevices() do.
    status_t setZoneDevices_l(audio_zones_t zone, audio_devices_t devices);

    // Get a refined selection of devices in a given zone based on the stream
    // type. This is applicable for cases only a subset of the zones devices
    // are required by the stream type (e.g. voice call in main cabin only).
    audio_devices_t getRefinedZoneDevices(audio_zones_t zones,
                                          AudioSystem::stream_type stream);

    // Get the active devices in an output session, that includes devices from
    // all the zones the output is rendering to.
    audio_devices_t getSessionDevices(int session);

    // Find the output (if any) that is rendering to the device.
    audio_io_handle_t findOutput(audio_devices_t device);

    // Find the listening zone (if any) that the device is assigned to.
    audio_zones_t findZone(audio_devices_t device);

    // Find the outputs in a listening zone. For duplicating outputs, it finds
    // all outputs connected to it (the ones that receive the duplicated data).
    void findOutputsInZone(audio_io_handle_t output,
                           audio_zones_t zone,
                           SortedVector<audio_io_handle_t> &outputs);

    // Remove an output session descriptor.
    void clearSession(int session);

    // Handle the disconnection of an output device.
    status_t disconnectDevice(audio_devices_t device);

    // Find a suitable listening zone for a routing strategy.
    // Setting AUDIO_ZONE_NONE to the zones argument passed by reference means
    // that there was no policy rule for that stream type, next zone selection
    // mechanism will kick in.
    virtual void selectZoneForStrategy(routing_strategy strategy,
                                       audio_zones_t &zones,
                                       audio_devices_t &devices);

    // Find a suitable listening zone for a stream type.
    // Setting AUDIO_ZONE_NONE to the zones argument passed by reference means
    // that there was no policy rule for that stream type, next zone selection
    // mechanism will kick in.
    virtual void selectZoneForStream(AudioSystem::stream_type stream,
                                     audio_zones_t &zones,
                                     audio_devices_t &devices);

    // Select the listening zone for an audio stream:
    // 1. Use the explicit zone requested via setSessionZones()
    // 2. Based on the routing strategy (see selectZoneForStrategy())
    // 3. Based on the stream type (see selectZoneForStream())
    // 4. Main cabin
    virtual void selectZones(int session, AudioSystem::stream_type stream);

    // === Volume helper methods ===

    // Initialize volume curves for each strategy and device category.
    void initializeVolumeCurves();

    // Compute the stream type volume for a given output according to the
    // requested index. For a given stream type, the volume depends on the
    // rendering device, there are different volume curves for each volume
    // category.
    virtual float computeVolume(int stream,
                                int index,
                                audio_io_handle_t output,
                                audio_devices_t device);

    // Set the stream type volume of an output.
    status_t setVolume(int stream,
                       audio_io_handle_t output,
                       int delayMs = 0);

    // Mute the stream type volume of an output.
    status_t muteVolume(int stream,
                        audio_io_handle_t output,
                        int delayMs = 0);

    // Calculate and set the stream type volume on a given output.
    // The type of output defines how the volume is applied: mixer/direct
    // outputs use setStreamVolume(), duplicating outputs use
    // setDuplicatingVolume().
    status_t setVolume_l(int stream,
                         audio_io_handle_t output,
                         bool mute,
                         int delayMs = 0);

    // Calculate and set the stream type volume on a given output.
    // If the output is rendering audio to multiple devices, then the most
    // relevant device is selected and its volume is calculated and used.
    status_t setStreamVolume(int stream,
                             audio_io_handle_t output,
                             bool mute,
                             int delayMs);

    // Calculate and set the volume for a duplicating output.
    // The volume is not directly applied to the duplicating output but
    // calculated and set for each output that is connected to it.
    // That allows more granularity in volume calculation as the most relevant
    // device for each connected output is a subset of those in the duplicating
    // output itself.
    status_t setDuplicatingVolume(int stream,
                                  audio_io_handle_t output,
                                  bool mute,
                                  int delayMs);

    // Mute or unmute the stream on the specified output.
    void setStreamMute(int stream,
                       bool on,
                       audio_io_handle_t output,
                       int delayMs = 0,
                       audio_devices_t device = AUDIO_DEVICE_NONE);

    // === Multizone volume helper methods ===

    // Initialize the session volumes for all the listening zones that the
    // session is connected to.
    void initSessionVolume(int session);

    // Set the per-session zone volume. Performs the additional handling
    // required for duplicating outputs where the volume is applied only to the
    // output that is actually rendering on the given zone. This method requires
    // a single zone passed.
    status_t setSessionVolume_l(int session, audio_zones_t zone, float volume);

    // === Voice call helper methods ===

    // true if device is in a telephony or VoIP call
    bool isInCall();

    // true if given state represents a device in a telephony or VoIP call.
    bool isStateInCall(int state);

    // Handle special cases for sonification strategy while in call: mute
    // streams or replace by a special tone in the device used for communication.
    void handleIncallSonification(int stream,
                                  bool starting,
                                  bool stateChange);

    // === Bluetooth helper methods ===

    // Manages A2DP output suspend/restore according to phone state and BT SCO
    // usage.
    void checkA2dpSuspend();

    // === Effects helper methods ===

    // Returns the A2DP output handle if it is open or 0 otherwise.
    audio_io_handle_t getA2dpOutput();

    virtual uint32_t getMaxEffectsCpuLoad();
    virtual uint32_t getMaxEffectsMemory();

    status_t setEffectEnabled(EffectDescriptor *pDesc, bool enabled);

    // === Input helper methods ===

    IOProfile *getInputProfile(audio_devices_t device,
                               uint32_t samplingRate,
                               uint32_t format,
                               uint32_t channelMask);

    // Select input device corresponding to requested audio source.
    audio_devices_t getDeviceForInputSource(int inputSource);

    // Return io handle of active input or 0 if no input is active
    // Only considers inputs from physical devices (e.g. main mic, headset mic)
    // when ignoreVirtualInputs is true.
    audio_io_handle_t getActiveInput(bool ignoreVirtualInputs = true);

    AudioPolicyClientInterface *mpClientInterface; // Audio policy client interface
    audio_io_handle_t mPrimaryOutput;              // Primary output handle

    audio_devices_t mAvailableOutputDevices;  // Bit field of all available output devices
    audio_devices_t mAvailableInputDevices;   // Bit field of all available input devices
                                              // without AUDIO_DEVICE_BIT_IN to allow direct bit
                                              // field comparisons
    audio_devices_t mAttachedOutputDevices;   // Output devices always available on the platform
    audio_devices_t mDefaultOutputDevice;     // Output device selected by default at boot time
                                              // (must be in mAttachedOutputDevices)

    bool mA2dpSuspended;                      // true if A2DP output is suspended
    bool mHasA2dp;                            // true on platforms with support for BT A2DP
    bool mHasUsb;                             // true on platforms with support for USB audio
    bool mHasRemoteSubmix;                    // true on platforms with support for remote presentation
                                              // of a submix

    int mPhoneState;                          // Current phone state
    String8 mA2dpDeviceAddress;               // A2DP device MAC address
    String8 mScoDeviceAddress;                // SCO device MAC address
    String8 mUsbCardAndDevice;                // USB audio ALSA card and device numbers:
                                              // card=<card_number>;device=<><device_number>
    float mLastVoiceVolume;                   // Last voice volume value sent to audio HAL

    // Listening zone to output device mappings
    KeyedVector<audio_zones_t, audio_devices_t> mZoneDevices;

    // Listening zone volumes
    KeyedVector<audio_zones_t, float> mZoneVolume;

    // Descriptors for sessions currently active
    KeyedVector<int, SessionDescriptor*> mSessions;

    // Descriptors for outputs currently opened
    DefaultKeyedVector<audio_io_handle_t, AudioOutputDescriptor *> mOutputs;

    // Descriptors for inputs currently opened
    DefaultKeyedVector<audio_io_handle_t, AudioInputDescriptor *> mInputs;

    // Current forced use configuration
    AudioSystem::forced_config mForceUse[AudioSystem::NUM_FORCE_USE];

    // Stream descriptors for volume control
    StreamDescriptor mStreams[AudioSystem::NUM_STREAM_TYPES];

    // Maximum CPU load allocated to audio effects in 0.1 MIPS (ARMv5TE, 0 WS memory) units
    static const uint32_t MAX_EFFECTS_CPU_LOAD = 1000;

    // Maximum memory allocated to audio effects in KB
    static const uint32_t MAX_EFFECTS_MEMORY = 512;

    // Current CPU load used by effects
    uint32_t mTotalEffectsCpuLoad;

    // Current memory used by effects
    uint32_t mTotalEffectsMemory;

    // List of registered audio effects
    KeyedVector<int, EffectDescriptor *> mEffects;

    // Audio hardware modules registered in the system
    Vector <HwModule *> mHwModules;

    // Zone affinity: allowed devices for a listening zone
    KeyedVector<audio_zones_t, audio_devices_t> mZoneAffinity;

    AudioPolicyParser mParser;

 private:
    // Calculate the amplification corresponding to a volume UI index.
    // The amplification and index are for a specific stream type whose
    // descriptor is received.
    static float volIndexToAmpl(audio_devices_t device,
                                const StreamDescriptor& streamDesc,
                                int indexInUi);

    // updates device caching and output for streams that can influence the
    // routing of notifications
    void handleNotificationRoutingForStream(AudioSystem::stream_type stream);

    static bool isVirtualInputDevice(audio_devices_t device);
};

}

#endif // _POLICY_MULTIZONE_AUDIOPOLICYMANAGER_H_
