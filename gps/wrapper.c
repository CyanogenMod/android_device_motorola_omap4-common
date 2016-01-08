/*
 * Copyright (c) 2015 The CyanogenMod Project
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

#define LOG_TAG "moto_gps_wrapper"
/* #define LOG_NDEBUG 0 */

#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>

#include <cutils/log.h>

#include <hardware/hardware.h>
#include <hardware/gps.h>

static struct gps_device_t *moto_gps_device;

static const GpsInterface *moto_gps_interface;
static GpsInterface my_gps_interface;

static GpsCallbacks *real_gps_callbacks;
static GpsCallbacks my_gps_callbacks;

static const GpsXtraInterface *moto_xtra_interface;
static GpsXtraInterface my_xtra_interface;

static GpsXtraCallbacks *real_xtra_callbacks;
static GpsXtraCallbacks my_xtra_callbacks;

pthread_t wrapper_gps_create_thread(const char* name, void (*start)(void *), void* arg)
{
    pthread_t ret;
    ALOGI("wrapper_gps_create_thread: %s", name);

    ret = real_gps_callbacks->create_thread_cb(name, start, arg);
    ALOGI("create ret: 0x%lx", ret);

    if (ret != 0) {
        /* Moto does a signed compare > 0... */
        ret = 0x42;
        ALOGI("modified ret: 0x%lx", ret);
    }

    return ret;
}

static int wrapper_xtra_init(GpsXtraCallbacks* callbacks)
{
    ALOGI("wrapper_xtra_init");

    real_xtra_callbacks = callbacks;
    memcpy(&my_xtra_callbacks, callbacks, sizeof(GpsXtraCallbacks));
    my_xtra_callbacks.create_thread_cb = wrapper_gps_create_thread;

    return moto_xtra_interface->init(&my_xtra_callbacks);
}

static const void* wrapper_get_extension(const char* name)
{
    const void *ret;

    ALOGI("wrapper_get_extension: %s", name);

    ret = moto_gps_interface->get_extension(name);

    if (!strcmp(name, GPS_XTRA_INTERFACE) && ret) {
        moto_xtra_interface = ret;

        memcpy(&my_xtra_interface, moto_xtra_interface, sizeof(GpsXtraInterface));
        my_xtra_interface.init = wrapper_xtra_init;

        ret = &my_xtra_interface;
    }

    return ret;
}

static int wrapper_init(GpsCallbacks* callbacks)
{
    ALOGI("wrapper_init");

    real_gps_callbacks = callbacks;
    memcpy(&my_gps_callbacks, callbacks, sizeof(GpsCallbacks));
    my_gps_callbacks.create_thread_cb = wrapper_gps_create_thread;

    return moto_gps_interface->init(&my_gps_callbacks);
}

static const GpsInterface* wrapper_get_gps_interface(struct gps_device_t* dev)
{
    ALOGI("wrapper_get_gps_interface");

    moto_gps_interface = moto_gps_device->get_gps_interface(dev);

    memcpy(&my_gps_interface, moto_gps_interface, sizeof(GpsInterface));
    my_gps_interface.init = wrapper_init;
    my_gps_interface.get_extension = wrapper_get_extension;

    return &my_gps_interface;
}

static int wrapper_open(__attribute__((unused)) const hw_module_t* module,
                        __attribute__((unused)) const char* name,
                        hw_device_t** device)
{
    static void *dso_handle = NULL;
    struct hw_module_t *hmi;
    struct gps_device_t *my_gps_device;
    int ret;
    GpsInterface* sGpsInterface = NULL;

    ALOGI("Initializing wrapper for Motorola's GPS-HAL");

    my_gps_device = malloc(sizeof(struct gps_device_t));
    if (!my_gps_device) {
        ALOGE("wrapper_open: couldn't malloc");
        return -EINVAL;
    }

    dso_handle = dlopen("/system/lib/hw/gps.vendor.so", RTLD_NOW);
    if (dso_handle == NULL) {
        char const *err_str = dlerror();
        ALOGE("wrapper_open: %s", err_str ? err_str : "unknown");
        return -EINVAL;
    }

    const char *sym = HAL_MODULE_INFO_SYM_AS_STR;
    hmi = (struct hw_module_t *)dlsym(dso_handle, sym);
    if (hmi == NULL) {
        ALOGE("wrapper_open: couldn't find symbol %s", sym);
        dlclose(dso_handle);
        dso_handle = NULL;
        return -EINVAL;
    }

    hmi->dso = dso_handle;

    ret = hmi->methods->open(hmi, GPS_HARDWARE_MODULE_ID, (hw_device_t**)&moto_gps_device);
    if (ret != 0) {
        ALOGE("wrapper_open: couldn't open");
        dlclose(dso_handle);
        dso_handle = NULL;
        return -EINVAL;
    }

    memcpy(my_gps_device, moto_gps_device, sizeof(struct gps_device_t));

    my_gps_device->get_gps_interface = wrapper_get_gps_interface;

    *device = (hw_device_t*)my_gps_device;

    return ret;
}

static struct hw_module_methods_t wrapper_module_methods = {
    .open = wrapper_open,
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = GPS_HARDWARE_MODULE_ID,
    .name = "Motorola GPS-HAL wrapper",
    .author = "The CyanogenMod Project (Michael Gernoth)",
    .methods = &wrapper_module_methods,
};
