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

/* Input */
struct wrapper_in_stream {
    struct audio_stream_in *stream_in;
    struct audio_stream_in *copy_stream_in;
};

static struct wrapper_in_stream *in_streams = NULL;
static int n_in_streams = 0;
static pthread_mutex_t in_streams_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Output */
struct wrapper_out_stream {
    struct audio_stream_out *stream_out;
    struct audio_stream_out *copy_stream_out;
};

static struct wrapper_out_stream *out_streams = NULL;
static int n_out_streams = 0;
static pthread_mutex_t out_streams_mutex = PTHREAD_MUTEX_INITIALIZER;

/* HAL */
static struct audio_hw_device *copy_hw_dev = NULL;
static void *dso_handle = NULL;
static int in_use = 0;
static pthread_mutex_t in_use_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t in_use_cond = PTHREAD_COND_INITIALIZER;

/* ICS Voice blob */
#ifdef ICS_VOICE_BLOB
static struct audio_hw_device *ics_hw_dev = NULL;
static void *ics_dso_handle = NULL;
#endif

#define INCREMENT_IN_USE() do { pthread_mutex_lock(&in_use_mutex); \
                                in_use++; \
                                pthread_mutex_unlock(&in_use_mutex); } while(0)

#define DECREMENT_IN_USE() do { pthread_mutex_lock(&in_use_mutex); \
                                in_use--; \
                                pthread_cond_signal(&in_use_cond); \
                                pthread_mutex_unlock(&in_use_mutex); } while(0)

#define WAIT_FOR_FREE() do { pthread_mutex_lock(&in_use_mutex); \
                             while (in_use) { \
                                 pthread_cond_wait(&in_use_cond, &in_use_mutex); \
                             } } while(0)

#define UNLOCK_FREE() do { pthread_cond_signal(&in_use_cond); \
                           pthread_mutex_unlock(&in_use_mutex); } while (0)

/* Generic wrappers for streams */
#define _WRAP_STREAM_LOCKED(name, function, direction, prototype, parameters, log) \
    static int wrapper_ ## direction ## _ ## name  prototype \
    { \
        int ret = -ENODEV; \
        int i; \
    \
        ALOGI log; \
        pthread_mutex_lock(& direction ## _streams_mutex); \
        for (i = 0; i < n_ ## direction ## _streams; i++) { \
            if (direction ## _streams[i].stream_ ## direction == (struct audio_stream_ ## direction*)stream) { \
                WAIT_FOR_FREE(); \
                ret = direction ## _streams[i].copy_stream_ ## direction ->function parameters; \
                UNLOCK_FREE(); \
                break; \
            } \
        } \
        pthread_mutex_unlock(& direction ## _streams_mutex); \
    \
        return ret; \
    }

#define WRAP_STREAM_LOCKED(name, direction, prototype, parameters, log) \
        _WRAP_STREAM_LOCKED(name, name, direction, prototype, parameters, log)

#define WRAP_STREAM_LOCKED_COMMON(name, direction, prototype, parameters, log) \
        _WRAP_STREAM_LOCKED(name, common.name, direction, prototype, parameters, log)

/* Generic wrappers for HAL */
#define _WRAP_HAL_LOCKED(name, function, prototype, parameters, log) \
    static int wrapper_ ## name  prototype \
    { \
        int ret; \
        int i; \
    \
        ALOGI log; \
        pthread_mutex_lock(&out_streams_mutex); \
        pthread_mutex_lock(&in_streams_mutex); \
    \
        WAIT_FOR_FREE(); \
        ret = copy_hw_dev->function parameters; \
        UNLOCK_FREE(); \
    \
        pthread_mutex_unlock(&in_streams_mutex); \
        pthread_mutex_unlock(&out_streams_mutex); \
    \
        return ret; \
    }

#define WRAP_HAL_LOCKED(name, prototype, parameters, log) \
        _WRAP_HAL_LOCKED(name, name, prototype, parameters, log)
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
            INCREMENT_IN_USE();
            break;
        }
    }
    pthread_mutex_unlock(&in_streams_mutex);

    if (found) {
        ret = wrapped_stream.copy_stream_in->read(stream, buffer, bytes);
#if 0
        if ((ret > 0) && (ret != (ssize_t)bytes)) {
            if (wrapper_hal_is_resampling(bytes, ret))
                ret = bytes;
        }
#endif
        DECREMENT_IN_USE();
    }

    return ret;
}

WRAP_STREAM_LOCKED(set_gain, in, (struct audio_stream_in *stream, float gain),
            (stream, gain), ("in_set_gain: %f", gain))

WRAP_STREAM_LOCKED_COMMON(standby, in, (struct audio_stream *stream),
            (stream), ("in_standby"))

WRAP_STREAM_LOCKED_COMMON(set_parameters, in, (struct audio_stream *stream, const char *kv_pairs),
            (stream, kv_pairs), ("in_set_parameters: %s", kv_pairs))

static void wrapper_close_input_stream(struct audio_hw_device *dev,
                                       struct audio_stream_in *stream_in)
{
    int i;

    pthread_mutex_lock(&in_streams_mutex);
    for (i = 0; i < n_in_streams; i++) {
        if (in_streams[i].stream_in == stream_in) {
            free(in_streams[i].copy_stream_in);
            n_in_streams--;
            memmove(in_streams + i,
                    in_streams + i + 1,
                    sizeof(struct wrapper_in_stream) * (n_in_streams - i));
            in_streams = realloc(in_streams,
                                  sizeof(struct wrapper_in_stream) * n_in_streams);
            ALOGI("Closed wrapped input stream");
            break;
        }
    }
    WAIT_FOR_FREE();
    copy_hw_dev->close_input_stream(dev, stream_in);
    UNLOCK_FREE();

    pthread_mutex_unlock(&in_streams_mutex);
}

static int wrapper_open_input_stream(struct audio_hw_device *dev,
                                     audio_io_handle_t handle,
                                     audio_devices_t devices,
                                     struct audio_config *config,
                                     struct audio_stream_in **stream_in)
{
    int ret;

    pthread_mutex_lock(&in_streams_mutex);

    WAIT_FOR_FREE();
    ret = copy_hw_dev->open_input_stream(dev, handle, devices,
                                         config, stream_in);
    UNLOCK_FREE();

    if (ret == 0) {
        struct wrapper_in_stream *new_in_streams;

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
        in_streams[n_in_streams].copy_stream_in = malloc(sizeof(struct audio_stream_in));
        if (!in_streams[n_in_streams].copy_stream_in) {
            ALOGE("Can't allocate memory for copy_stream_in!");
            pthread_mutex_unlock(&in_streams_mutex);
            return ret;
        }
        memcpy(in_streams[n_in_streams].copy_stream_in, *stream_in,
               sizeof(struct audio_stream_in));

        (*stream_in)->read = wrapper_read;
        (*stream_in)->set_gain = wrapper_in_set_gain;
        (*stream_in)->common.set_parameters = wrapper_in_set_parameters;
        (*stream_in)->common.standby = wrapper_in_standby;

        ALOGI("Wrapped an input stream: rate %d, channel_mask: %x, format: %d",
              config->sample_rate, config->channel_mask, config->format);

        n_in_streams++;
    }
    pthread_mutex_unlock(&in_streams_mutex);

    return ret;
}

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
            INCREMENT_IN_USE();
            break;
        }
    }
    pthread_mutex_unlock(&out_streams_mutex);

    if (found) {
        written = wrapped_stream.copy_stream_out->write(stream, buffer, bytes);
        ret = written;
        if ((ret > 0) && (written != bytes)) {
            if (wrapper_hal_is_resampling(bytes, written))
                ret = bytes;
        }
        DECREMENT_IN_USE();
    }

    return ret;
}

WRAP_STREAM_LOCKED(set_volume, out, (struct audio_stream_out *stream, float left, float right),
            (stream, left, right), ("set_out_volume: %f/%f", left, right))

WRAP_STREAM_LOCKED_COMMON(standby, out, (struct audio_stream *stream),
            (stream), ("out_standby"))

WRAP_STREAM_LOCKED_COMMON(set_parameters, out, (struct audio_stream *stream, const char *kv_pairs),
            (stream, kv_pairs), ("out_set_parameters: %s", kv_pairs))

void wrapper_close_output_stream(struct audio_hw_device *dev,
                            struct audio_stream_out* stream_out)
{
    int i;

    pthread_mutex_lock(&out_streams_mutex);
    for (i = 0; i < n_out_streams; i++) {
        if (out_streams[i].stream_out == stream_out) {
            free(out_streams[i].copy_stream_out);
            n_out_streams--;
            memmove(out_streams + i,
                    out_streams + i + 1,
                    sizeof(struct wrapper_out_stream) * (n_out_streams - i));
            out_streams = realloc(out_streams,
                                  sizeof(struct wrapper_out_stream) * n_out_streams);
            ALOGI("Closed wrapped output stream");
            break;
        }
    }

    WAIT_FOR_FREE();
    copy_hw_dev->close_output_stream(dev, stream_out);
    UNLOCK_FREE();

    pthread_mutex_unlock(&out_streams_mutex);
}

static int wrapper_open_output_stream(struct audio_hw_device *dev,
                                      audio_io_handle_t handle,
                                      audio_devices_t devices,
                                      audio_output_flags_t flags,
                                      struct audio_config *config,
                                      struct audio_stream_out **stream_out)
{
    int ret;

    pthread_mutex_lock(&out_streams_mutex);

    WAIT_FOR_FREE();
    ret = copy_hw_dev->open_output_stream(dev, handle, devices,
                                          flags, config, stream_out);
    UNLOCK_FREE();

    if (ret == 0) {
        struct wrapper_out_stream *new_out_streams;

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
        out_streams[n_out_streams].copy_stream_out = malloc(sizeof(struct audio_stream_out));
        if (!out_streams[n_out_streams].copy_stream_out) {
            ALOGE("Can't allocate memory for copy_stream_out!");
            pthread_mutex_unlock(&out_streams_mutex);
            return ret;
        }
        memcpy(out_streams[n_out_streams].copy_stream_out, *stream_out,
               sizeof(struct audio_stream_out));

        (*stream_out)->write = wrapper_write;
        (*stream_out)->set_volume = wrapper_out_set_volume;
        (*stream_out)->common.set_parameters = wrapper_out_set_parameters;
        (*stream_out)->common.standby = wrapper_out_standby;

        ALOGI("Wrapped an output stream: rate %d, channel_mask: %x, format: %d",
              config->sample_rate, config->channel_mask, config->format);

        n_out_streams++;
    }
    pthread_mutex_unlock(&out_streams_mutex);

    return ret;
}

/* Generic HAL */

WRAP_HAL_LOCKED(set_master_volume, (struct audio_hw_device *dev, float volume),
                (dev, volume), ("set_master_volume: %f", volume))

#ifndef ICS_VOICE_BLOB
WRAP_HAL_LOCKED(set_voice_volume, (struct audio_hw_device *dev, float volume),
                (dev, volume), ("set_voice_volume: %f", volume))
#else
static int wrapper_set_voice_volume(struct audio_hw_device *dev, float volume)
{
    ALOGI("ICS: set_voice_volume: %f", volume);

    return ics_hw_dev->set_voice_volume(ics_hw_dev, volume);
}
#endif


WRAP_HAL_LOCKED(set_mic_mute, (struct audio_hw_device *dev, bool state),
                (dev, state), ("set_mic_mute: %d", state))

WRAP_HAL_LOCKED(set_mode, (struct audio_hw_device *dev, audio_mode_t mode),
                (dev, mode), ("set_mode: %d", mode))

WRAP_HAL_LOCKED(set_parameters, (struct audio_hw_device *dev, const char *kv_pairs),
                (dev, kv_pairs), ("set_parameters: %s", kv_pairs))

#ifdef ICS_VOICE_BLOB
static int wrapper_set_mode_ics(struct audio_hw_device *dev, audio_mode_t mode)
{
    ALOGI("ICS: set_mode: %d", mode);

    ics_hw_dev->set_voice_volume(ics_hw_dev, mode);

    return wrapper_set_mode(dev, mode);
}

static int wrapper_set_parameters_ics(struct audio_hw_device *dev, const char *kv_pairs)
{
    ALOGI("ICS: set_parameters: %s", kv_pairs);

    ics_hw_dev->set_parameters(ics_hw_dev, kv_pairs);

    return wrapper_set_parameters(dev, kv_pairs);
}
#endif

static int wrapper_close(hw_device_t *device)
{
    int ret;

    pthread_mutex_lock(&out_streams_mutex);
    pthread_mutex_lock(&in_streams_mutex);

    WAIT_FOR_FREE();

    ret = copy_hw_dev->common.close(device);

    dlclose(dso_handle);
    dso_handle = NULL;
    free(copy_hw_dev);
    copy_hw_dev = NULL;

    if (out_streams) {
        free(out_streams);
        out_streams = NULL;
        n_out_streams = 0;
    }

    if (in_streams) {
        free(in_streams);
        in_streams = NULL;
        n_in_streams = 0;
    }

#ifdef ICS_VOICE_BLOB
    ics_hw_dev->common.close((hw_device_t*)ics_hw_dev);
    ics_hw_dev = NULL;
    dlclose(ics_dso_handle);
    ics_dso_handle = NULL;
#endif

    UNLOCK_FREE();
    pthread_mutex_unlock(&in_streams_mutex);
    pthread_mutex_unlock(&out_streams_mutex);

    return ret;
}

static int wrapper_open(const hw_module_t* module,
                             const char* name,
                             hw_device_t** device)
{
    struct hw_module_t *hmi;
    struct audio_hw_device *adev;
    int ret;

    ALOGI("Initializing wrapper for Motorola's audio-HAL");
    if (copy_hw_dev) {
        ALOGE("Audio HAL already opened!");
        return -ENODEV;
    }

    dso_handle = dlopen("/system/lib/hw/audio.primary.omap4.so", RTLD_NOW);
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

    ret = audio_hw_device_open(hmi, &adev);
    ALOGE_IF(ret, "%s couldn't open audio module in %s. (%s)", __func__,
                 AUDIO_HARDWARE_MODULE_ID, strerror(-ret));
    if (ret) {
        dlclose(dso_handle);
        dso_handle = NULL;
        return ret;
    }

    *device = (hw_device_t*)adev;

#ifdef ICS_VOICE_BLOB
    ALOGI("Loading ICS blob for voice-volume");
    ics_dso_handle = dlopen("/system/lib/hw/audio.primary.ics.so", RTLD_NOW);
    if (ics_dso_handle == NULL) {
        char const *err_str = dlerror();
        ALOGE("wrapper_open: %s", err_str ? err_str : "unknown");
        return -EINVAL;
    }

    sym = HAL_MODULE_INFO_SYM_AS_STR;
    hmi = (struct hw_module_t *)dlsym(ics_dso_handle, sym);
    if (hmi == NULL) {
        ALOGE("wrapper_open: couldn't find symbol %s", sym);
        dlclose(ics_dso_handle);
        ics_dso_handle = NULL;
        return -EINVAL;
    }

    hmi->dso = ics_dso_handle;

    ret = audio_hw_device_open(hmi, &ics_hw_dev);
    ALOGE_IF(ret, "%s couldn't open ICS audio module in %s. (%s)", __func__,
                 AUDIO_HARDWARE_MODULE_ID, strerror(-ret));
    if (ret) {
        dlclose(ics_dso_handle);
        ics_dso_handle = NULL;
        return ret;
    }
#endif

    copy_hw_dev = malloc(sizeof(struct audio_hw_device));
    if (!copy_hw_dev) {
        ALOGE("Can't allocate memory for copy_hw_dev, continuing unwrapped...");
        return 0;
    }

    memcpy(copy_hw_dev, *device, sizeof(struct audio_hw_device));

    /* HAL */
    adev->common.close = wrapper_close;
    adev->set_master_volume = wrapper_set_master_volume;
    adev->set_voice_volume = wrapper_set_voice_volume;
    adev->set_mic_mute = wrapper_set_mic_mute;
    /* set_master_mute not supported by MR0_AUDIO_BLOB */
#ifdef ICS_VOICE_BLOB
    adev->set_mode = wrapper_set_mode_ics;
    adev->set_parameters = wrapper_set_parameters_ics;
#else
    adev->set_mode = wrapper_set_mode;
    adev->set_parameters = wrapper_set_parameters;
#endif

    /* Output */
    adev->open_output_stream = wrapper_open_output_stream;
    adev->close_output_stream = wrapper_close_output_stream;

    /* Input */
    adev->open_input_stream = wrapper_open_input_stream;
    adev->close_input_stream = wrapper_close_input_stream;

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
        .name = "Motorola AUDIO-HAL wrapper",
        .author = "The CyanogenMod Project (Michael Gernoth)",
        .methods = &wrapper_module_methods,
    },
};
