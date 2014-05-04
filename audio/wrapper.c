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

#define LOG_TAG "moto_audio_wrapper"
/* #define LOG_NDEBUG 0 */

#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include <fcntl.h>

#include <cutils/log.h>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <hardware/audio.h>

/* #define WRAP_INPUT */

/* Input */
#ifdef WRAP_INPUT
struct wrapper_in_stream {
    struct audio_stream_in *stream_in;
    ssize_t (*read)(struct audio_stream_in *stream, void* buffer,
                    size_t bytes);
};

static struct wrapper_in_stream *in_streams = NULL;
static int n_in_streams = 0;
static pthread_mutex_t in_streams_mutex = PTHREAD_MUTEX_INITIALIZER;

static int (*real_open_input_stream)(struct audio_hw_device *dev,
                                     audio_io_handle_t handle,
                                     audio_devices_t devices,
                                     struct audio_config *config,
                                     struct audio_stream_in **stream_in) = NULL;
static void (*real_close_input_stream)(struct audio_hw_device *dev,
                                       struct audio_stream_in *stream_in) = NULL;
#endif

/* Output */
struct wrapper_out_stream {
    struct audio_stream_out *stream_out;
    ssize_t (*write)(struct audio_stream_out *stream, const void* buffer,
                     size_t bytes);
};

static struct wrapper_out_stream *out_streams = NULL;
static int n_out_streams = 0;
static pthread_mutex_t out_streams_mutex = PTHREAD_MUTEX_INITIALIZER;

static int (*real_open_output_stream)(struct audio_hw_device *dev,
                                      audio_io_handle_t handle,
                                      audio_devices_t devices,
                                      audio_output_flags_t flags,
                                      struct audio_config *config,
                                      struct audio_stream_out **stream_out) = NULL;
static void (*real_close_output_stream)(struct audio_hw_device *dev,
                                        struct audio_stream_out* stream_out) = NULL;

/* HAL */
static struct audio_hw_device *hw_dev = NULL;
static void *dso_handle = NULL;

static int (*real_close)(hw_device_t *device) = NULL;


/*
 * When Motorola's HAL is internally resampling and downmixing a
 * signal it does not return the real number of bytes written
 * but the arbitrary number 192.
 *
 * 192 seems not to represent the written bytes at all, as it
 * is always returned regardless of the number of bytes written.
 *
 * In fact, all data has been written when 192 is returned.
 */
static int wrapper_hal_is_resampling(size_t expected, size_t real)
{
    int ret = 0;

    ALOGV("expected: %u, real: %u", expected, real);

    if (real == 192)
        ret = 1;

    return ret;
}

/* Input stream */

#ifdef WRAP_INPUT
static ssize_t wrapper_read(struct audio_stream_in *stream, void* buffer,
                            size_t bytes)
{
    ssize_t ret = -ENODEV;
    int i;
    int found = 0;
    struct wrapper_in_stream wrapped_stream;

    pthread_mutex_lock(&in_streams_mutex);
    for (i = 0; i < n_in_streams; i++) {
        if (in_streams[i].stream_in == stream) {
            wrapped_stream = in_streams[i];
            found = 1;
            break;
        }
    }
    pthread_mutex_unlock(&in_streams_mutex);

    if (found) {
        ret = wrapped_stream.read(stream, buffer, bytes);
        if ((ret > 0) && (ret != (ssize_t)bytes)) {
            if (wrapper_hal_is_resampling(bytes, ret))
                ret = bytes;
        }
    }

    return ret;
}

static void wrapper_close_input_stream(struct audio_hw_device *dev,
                                       struct audio_stream_in *stream_in)
{
    int i;

    ALOGI("Closing wrapped input stream");
    pthread_mutex_lock(&in_streams_mutex);
    for (i = 0; i < n_in_streams; i++) {
        if (in_streams[i].stream_in == stream_in) {
            n_in_streams--;
            memmove(in_streams + i,
                    in_streams + i + 1,
                    sizeof(struct wrapper_in_stream) * (n_in_streams - i));
            in_streams = realloc(in_streams,
                                  sizeof(struct wrapper_in_stream) * n_in_streams);
            ALOGI("Closed an wrapped input stream");
            break;
        }
    }
    pthread_mutex_unlock(&in_streams_mutex);

    real_close_input_stream(dev, stream_in);
}

static int wrapper_open_input_stream(struct audio_hw_device *dev,
                                     audio_io_handle_t handle,
                                     audio_devices_t devices,
                                     struct audio_config *config,
                                     struct audio_stream_in **stream_in)
{
    int ret;

    ALOGI("Wrapping an input stream");
    ret = real_open_input_stream(dev, handle, devices,
                                 config, stream_in);
    if (ret == 0) {
        struct wrapper_in_stream *new_in_streams;

        pthread_mutex_lock(&in_streams_mutex);
        new_in_streams = realloc(in_streams,
                              sizeof(struct wrapper_in_stream) * (n_in_streams + 1));
        if (!new_in_streams) {
            ALOGE("Can't allocate memory for wrapped stream, not touching original!");
            pthread_mutex_unlock(&in_streams_mutex);
            return ret;
        }
        in_streams = new_in_streams;
        memset(&in_streams[n_in_streams], 0, sizeof(struct wrapper_in_stream));

        in_streams[n_in_streams].stream_in = *stream_in;
        in_streams[n_in_streams].read = (*stream_in)->read;
        (*stream_in)->read = wrapper_read;
        ALOGI("Wrapped an input stream");

        n_in_streams++;
        pthread_mutex_unlock(&in_streams_mutex);
    }

    return ret;
}
#endif

/* Output stream */

static int wrapper_write(struct audio_stream_out *stream, const void* buffer,
                         size_t bytes)
{
    int ret = -ENODEV;
    int i;
    int found = 0;
    struct wrapper_out_stream wrapped_stream;
    size_t written;

    pthread_mutex_lock(&out_streams_mutex);
    for (i = 0; i < n_out_streams; i++) {
        if (out_streams[i].stream_out == stream) {
            wrapped_stream = out_streams[i];
            found = 1;
            break;
        }
    }
    pthread_mutex_unlock(&out_streams_mutex);

    if (found) {
        written = wrapped_stream.write(stream, buffer, bytes);
        ret = written;
        if ((ret > 0) && (written != bytes)) {
            if (wrapper_hal_is_resampling(bytes, written))
                ret = bytes;
        }
    }

    return ret;
}

void wrapper_close_output_stream(struct audio_hw_device *dev,
                            struct audio_stream_out* stream_out)
{
    int i;

    ALOGI("Closing wrapped output stream");
    pthread_mutex_lock(&out_streams_mutex);
    for (i = 0; i < n_out_streams; i++) {
        if (out_streams[i].stream_out == stream_out) {
            n_out_streams--;
            memmove(out_streams + i,
                    out_streams + i + 1,
                    sizeof(struct wrapper_out_stream) * (n_out_streams - i));
            out_streams = realloc(out_streams,
                                  sizeof(struct wrapper_out_stream) * n_out_streams);
            ALOGI("Closed an wrapped output stream");
            break;
        }
    }
    pthread_mutex_unlock(&out_streams_mutex);

    real_close_output_stream(dev, stream_out);
}

static int wrapper_open_output_stream(struct audio_hw_device *dev,
                                      audio_io_handle_t handle,
                                      audio_devices_t devices,
                                      audio_output_flags_t flags,
                                      struct audio_config *config,
                                      struct audio_stream_out **stream_out)
{
    int ret;

    ALOGI("Wrapping an output stream");
    ret = real_open_output_stream(dev, handle, devices,
                                  flags, config, stream_out);
    if (ret == 0) {
        struct wrapper_out_stream *new_out_streams;

        pthread_mutex_lock(&out_streams_mutex);
        new_out_streams = realloc(out_streams,
                              sizeof(struct wrapper_out_stream) * (n_out_streams + 1));
        if (!new_out_streams) {
            ALOGE("Can't allocate memory for wrapped stream, not touching original!");
            pthread_mutex_unlock(&out_streams_mutex);
            return ret;
        }
        out_streams = new_out_streams;
        memset(&out_streams[n_out_streams], 0, sizeof(struct wrapper_out_stream));

        out_streams[n_out_streams].stream_out = *stream_out;
        out_streams[n_out_streams].write = (*stream_out)->write;
        (*stream_out)->write = wrapper_write;
        ALOGI("Wrapped an output stream");

        n_out_streams++;
        pthread_mutex_unlock(&out_streams_mutex);
    }

    return ret;
}

/* Generic HAL */

static int wrapper_close(hw_device_t *device)
{
    int ret;

    pthread_mutex_lock(&out_streams_mutex);
#ifdef WRAP_INPUT
    pthread_mutex_lock(&in_streams_mutex);
#endif

    ret = real_close(device);

    dlclose(dso_handle);
    dso_handle = NULL;
    hw_dev = NULL;

    if (out_streams) {
        free(out_streams);
        out_streams = NULL;
        n_out_streams = 0;
    }

#ifdef WRAP_INPUT
    if (in_streams) {
        free(in_streams);
        in_streams = NULL;
        n_in_streams = 0;
    }

    pthread_mutex_unlock(&in_streams_mutex);
#endif
    pthread_mutex_unlock(&out_streams_mutex);

    return ret;
}

static int wrapper_open(const hw_module_t* module,
                             const char* name,
                             hw_device_t** device)
{
    struct hw_module_t *hmi;
    int rc;

    if (hw_dev) {
        *device = (hw_device_t*)hw_dev;
        return 0;
    }

    if (!dso_handle) {
        dso_handle = dlopen("/system/lib/hw/audio.primary.omap4.so", RTLD_NOW);
        if (dso_handle == NULL) {
            char const *err_str = dlerror();
            ALOGE("wrapper_open:%s", err_str?err_str:"unknown");
            return -EINVAL;
        }
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

    rc = audio_hw_device_open(hmi, (struct audio_hw_device**)device);
    ALOGE_IF(rc, "%s couldn't open audio module in %s. (%s)", __func__,
                 AUDIO_HARDWARE_MODULE_ID, strerror(-rc));
    if (rc) {
        dlclose(dso_handle);
        dso_handle = NULL;
        return rc;
    }

    hw_dev = *((struct audio_hw_device**)device);

    real_close = hw_dev->common.close;
    hw_dev->common.close = wrapper_close;

    /* Output */
    real_open_output_stream = hw_dev->open_output_stream;
    hw_dev->open_output_stream = wrapper_open_output_stream;

    real_close_output_stream = hw_dev->close_output_stream;
    hw_dev->close_output_stream = wrapper_close_output_stream;

    /* Input */
#ifdef WRAP_INPUT
    real_open_input_stream = hw_dev->open_input_stream;
    hw_dev->open_input_stream = wrapper_open_input_stream;

    real_close_input_stream = hw_dev->close_input_stream;
    hw_dev->close_input_stream = wrapper_close_input_stream;
#endif

    return 0;
}

static struct hw_module_methods_t wrapper_module_methods = {
    .open = wrapper_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "Motorla AUDIO-HAL wrapper",
        .author = "The CyanogenMod Project (Michael Gernoth)",
        .methods = &wrapper_module_methods,
    },
};
