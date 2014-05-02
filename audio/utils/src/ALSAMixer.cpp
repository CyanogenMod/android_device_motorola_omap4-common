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

// #define LOG_NDEBUG 0
// #define VERY_VERBOSE_LOGGING

#include <errno.h>
#include <system/audio.h>
#include <audio_route/audio_route.h>
#include <tinyalsa/asoundlib.h>

#include <tiaudioutils/Log.h>
#include <tiaudioutils/ALSAMixer.h>

namespace tiaudioutils {

ALSAControl::ALSAControl(string &name, int val)
    : mName(name), mStrVal(""), mIntVal(val), mType(TYPE_INT)
{
}

ALSAControl::ALSAControl(string &name, string &val)
    : mName(name), mStrVal(val), mIntVal(-1), mType(TYPE_STR)
{
}

ALSAControl::ALSAControl(const char *name, int val)
    : mName(name), mStrVal(""), mIntVal(val), mType(TYPE_INT)
{
}

ALSAControl::ALSAControl(const char *name, const char *val)
    : mName(name), mStrVal(val), mIntVal(-1), mType(TYPE_STR)
{
}

bool ALSAControl::isIntegerType() const
{
    return (mType == TYPE_INT);
}

bool ALSAControl::isStringType() const
{
    return (mType == TYPE_STR);
}

/* ---------------------------------------------------------------------------------------- */

const string ALSAMixer::kAudioRoutePrefix = "/system/etc/";
const string ALSAMixer::kAudioRouteSuffix = "_paths.xml";

ALSAMixer::ALSAMixer(uint32_t card)
    : mCard(card), mRouter(NULL)
{
    ALOGI("ALSAMixer: open mixer for hw:%u", card);
    mMixer = mixer_open(card);

    initDeviceMap();

    char name[256] = "default";
    /* FIXME read card name from procfs since tinyalsa doesn't support it natively */
#ifdef OMAP_ENHANCEMENT
    int ret = mixer_get_card_name(card, name, sizeof(name));
#else
    int ret = 0;
#endif
    if (!ret) {
        string xmlRouteFile = kAudioRoutePrefix + name + kAudioRouteSuffix;

        mRouter = audio_route_init(card, xmlRouteFile.c_str());
        if (mRouter)
            ALOGI("ALSAMixer: using audio paths file: %s", xmlRouteFile.c_str());
        else
            ALOGI("ALSAMixer: no audio paths file found for card hw:%u", mCard);
    }
}

ALSAMixer::~ALSAMixer()
{
    if (mMixer) {
        ALOGI("ALSAMixer: close mixer for hw:%u", mCard);
        mixer_close(mMixer);
    }

    if (mRouter)
        audio_route_free(mRouter);
}

bool ALSAMixer::initCheck() const
{
    return (mMixer != NULL);
}

int ALSAMixer::set(const ALSAControl &control, bool on)
{
    struct mixer_ctl *ctl;

    AutoMutex lock(mLock);

    ctl = mixer_get_ctl_by_name(mMixer, control.name().c_str());
    if (!ctl) {
        ALOGE("ALSAMixer: failed to get control '%s'", control.name().c_str());
        return -EINVAL;
    }

    if (control.isIntegerType()) {
        int val = on ? control.intVal() : 0;

        ALOGV("ALSAMixer: %s name='%s' val=%d",
              on ? "set" : "clear", control.name().c_str(), val);

        for (uint32_t i = 0; i < mixer_ctl_get_num_values(ctl); i++)
            mixer_ctl_set_value(ctl, i, val);
    } else {
        const char *val = on ? control.strVal().c_str() : "Off";

        ALOGV("ALSAMixer: set name='%s' val='%s'",
              control.name().c_str(), control.strVal().c_str());

        mixer_ctl_set_enum_by_string(ctl, val);
    }

    return 0;
}

int ALSAMixer::set(const ALSAControlList &controls, bool on)
{
    for (ALSAControlList::const_iterator i = controls.begin(); i != controls.end(); ++i) {
        ALOGV("ALSAMixer: %s control '%s'",
              on ? "set" : "clear", i->name().c_str());

        int ret = set(*i, on);
        if (ret) {
            ALOGE("ALSAMixer: failed to %s control '%s'",
                  on ? "set" : "clear", i->name().c_str());
            return ret;
        }
    }

    return 0;
}

int ALSAMixer::initRoutes()
{
    if (!mRouter) {
        ALOGW("ALSAMixer: no audio router available for card hw:%u", mCard);
        return 0;
    }

    ALOGV("ALSAMixer: set card default routes");

    audio_route_reset(mRouter);
    int ret = audio_route_update_mixer(mRouter);
    if (ret)
        ALOGE("ALSAMixer: failed to set card default routes %d", ret);

    return ret;
}

int ALSAMixer::setPath(const char *path, bool on)
{
    if (!mRouter) {
        ALOGW("ALSAMixer: no audio router available for card hw:%u", mCard);
        return 0;
    }

    if (path == NULL)
        return -EINVAL;

    ALOGI("ALSAMixer: %s path for '%s'", on ? "apply" : "reset", path);

    int ret;
    if (on)
        ret = audio_route_apply_path(mRouter, path);
    else
        ret = audio_route_reset_path(mRouter, path);

    if (ret) {
        ALOGE("ALSAMixer: path '%s' not found", path);
        return -EINVAL;
    }

    ret = audio_route_update_mixer(mRouter);
    if (ret)
        ALOGE("ALSAMixer: failed to update mixer for '%s'", path);

    return ret;
}

int ALSAMixer::setPath(audio_devices_t devices, bool on)
{
    if (!mRouter) {
        ALOGW("ALSAMixer: no audio router available for card hw:%u", mCard);
        return 0;
    }

    ALOGV("ALSAMixer: %s path for devices 0x%08x",
          on ? "apply" : "reset", devices);

    int ret;
    for (audio_devices_t mask = 1; mask != AUDIO_DEVICE_BIT_DEFAULT; mask <<= 1) {
        audio_devices_t dev = devices;

        bool found;
        if (audio_is_input_device(dev)) {
            dev &= (mask | AUDIO_DEVICE_BIT_IN);
            found = (dev != AUDIO_DEVICE_BIT_IN);
        } else {
            dev &= mask;
            found = (dev != 0);
        }

        if (!found)
            continue;

        DeviceMap::iterator i = mDevMap.find(dev);
        if (i == mDevMap.end()) {
            ALOGE("ALSAMixer: no map entry for device 0x%08x", dev);
            return -EINVAL;
        }

        const char *path = mDevMap[dev].c_str();
        ALOGV("ALSAMixer: %s path '%s' for device 0x%08x",
              on ? "apply" : "reset", path, dev);
        if (on)
            ret = audio_route_apply_path(mRouter, path);
        else
            ret = audio_route_reset_path(mRouter, path);

        if (ret) {
            ALOGW("ALSAMixer: path '%s' not found", path);
            continue;
        }
    }

    ret = audio_route_update_mixer(mRouter);
    if (ret)
        ALOGE("ALSAMixer: failed to update mixer with new paths %d", ret);

    return ret;
}

int ALSAMixer::updatePath(audio_devices_t oldDevices, audio_devices_t newDevices)
{
    if (!mRouter) {
        ALOGW("ALSAMixer: no audio router available for card hw:%u", mCard);
        return 0;
    }

    audio_devices_t inactive = oldDevices & ~newDevices;
    audio_devices_t active = newDevices & ~oldDevices;

    ALOGV("ALSAMixer: update paths for devices from 0x%08x to 0x%08x",
          oldDevices, newDevices);

    /* First reset the paths for inactive devices */
    int ret = setPath(inactive, false);
    if (ret) {
        ALOGE("ALSAMixer: failed to reset path for inactive devices 0x%08x",
              inactive);
        return ret;
    }

    /* Now apply the path for new active devices */
    ret = setPath(active, true);
    if (ret)
        ALOGE("ALSAMixer: failed to apply path for new active devices 0x%08x",
              active);

    return ret;
}

#define addToDevMap(device)    mDevMap[device] = #device

void ALSAMixer::initDeviceMap()
{
    /* input devices */
    addToDevMap(AUDIO_DEVICE_IN_COMMUNICATION);
    addToDevMap(AUDIO_DEVICE_IN_AMBIENT);
    addToDevMap(AUDIO_DEVICE_IN_BUILTIN_MIC);
    addToDevMap(AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET);
    addToDevMap(AUDIO_DEVICE_IN_WIRED_HEADSET);
    addToDevMap(AUDIO_DEVICE_IN_AUX_DIGITAL);
    addToDevMap(AUDIO_DEVICE_IN_VOICE_CALL);
    addToDevMap(AUDIO_DEVICE_IN_BACK_MIC);
    addToDevMap(AUDIO_DEVICE_IN_REMOTE_SUBMIX);
    addToDevMap(AUDIO_DEVICE_IN_ANLG_DOCK_HEADSET);
    addToDevMap(AUDIO_DEVICE_IN_DGTL_DOCK_HEADSET);
    addToDevMap(AUDIO_DEVICE_IN_USB_ACCESSORY);
    addToDevMap(AUDIO_DEVICE_IN_USB_DEVICE);
    addToDevMap(AUDIO_DEVICE_IN_DEFAULT);

    /* output devices */
    addToDevMap(AUDIO_DEVICE_OUT_EARPIECE);
    addToDevMap(AUDIO_DEVICE_OUT_SPEAKER);
    addToDevMap(AUDIO_DEVICE_OUT_WIRED_HEADSET);
    addToDevMap(AUDIO_DEVICE_OUT_WIRED_HEADPHONE);
    addToDevMap(AUDIO_DEVICE_OUT_BLUETOOTH_SCO);
    addToDevMap(AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET);
    addToDevMap(AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT);
    addToDevMap(AUDIO_DEVICE_OUT_BLUETOOTH_A2DP);
    addToDevMap(AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES);
    addToDevMap(AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER);
    addToDevMap(AUDIO_DEVICE_OUT_AUX_DIGITAL);
    addToDevMap(AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET);
    addToDevMap(AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET);
    addToDevMap(AUDIO_DEVICE_OUT_USB_ACCESSORY);
    addToDevMap(AUDIO_DEVICE_OUT_USB_DEVICE);
    addToDevMap(AUDIO_DEVICE_OUT_REMOTE_SUBMIX);
    addToDevMap(AUDIO_DEVICE_OUT_DEFAULT);

#ifdef OMAP_ENHANCEMENT
    addToDevMap(AUDIO_DEVICE_IN_USB_HEADSET);
    addToDevMap(AUDIO_DEVICE_OUT_WIRED_HEADPHONE2);
#endif
}

} /* namespace tiaudioutils */
