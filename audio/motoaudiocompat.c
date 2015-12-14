/*
 * Copyright (c) 2014 The CyanogenMod Project
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

#include <hardware/audio_policy.h>

/*
 * Inject symbol for getDeviceConnectionState for old blob
 * This file is used by alsa-lib to satisfy dependencies of
 * audio.primary.omap4.so, which links against libasound.so
 */
audio_policy_dev_state_t _ZN7android11AudioSystem24getDeviceConnectionStateEjPKc(unsigned int device,
                                                  const char *device_address);

audio_policy_dev_state_t _ZN7android11AudioSystem24getDeviceConnectionStateE15audio_devices_tPKc(audio_devices_t device,
                                                  const char *device_address)
{
    return _ZN7android11AudioSystem24getDeviceConnectionStateEjPKc(device, device_address);
}
