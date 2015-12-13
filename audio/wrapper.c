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

#include <tinyalsa/asoundlib.h>

#include "wrapper.h"

/* Direct manipulation of mixer controls (in-call muting) */
#define ALSA_CARD	0
#define MUTE_CTL	"Analog Right Capture Route"
#define MUTE_VALUE	"Off"

/* Input */
struct wrapper_in_stream {
    struct audio_stream_in *stream_in;
    struct jb_audio_stream_in *jb_stream_in;
};

static struct wrapper_in_stream *in_streams = NULL;
static int n_in_streams = 0;
static pthread_mutex_t in_streams_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Output */
struct wrapper_out_stream {
    struct audio_stream_out *stream_out;
    struct jb_audio_stream_out *jb_stream_out;
};

static struct wrapper_out_stream *out_streams = NULL;
static int n_out_streams = 0;
static pthread_mutex_t out_streams_mutex = PTHREAD_MUTEX_INITIALIZER;

/* HAL */
static struct jb_audio_hw_device *jb_hw_dev = NULL;
static void *dso_handle = NULL;
static int in_use = 0;
static int last_mute_ctl_value = -1;
static int hal_audio_mode = AUDIO_MODE_NORMAL;
static pthread_mutex_t in_use_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t in_use_cond = PTHREAD_COND_INITIALIZER;

static int alsa_set_mic_mute(bool state);

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
#define _WRAP_STREAM_LOCKED(name, function, direction, rettype, err, prototype, parameters, log, pre_fn, post_fn) \
    static rettype wrapper_ ## direction ## _ ## name  prototype \
    { \
        rettype ret = err; \
        struct jb_audio_stream *jbstream; \
        struct jb_audio_stream_ ## direction *jbstream_ ## direction; \
        int i; \
    \
        if (log) ALOGI log; \
        pthread_mutex_lock(& direction ## _streams_mutex); \
        for (i = 0; i < n_ ## direction ## _streams; i++) { \
            if (direction ## _streams[i].stream_ ## direction == (struct audio_stream_ ## direction*)stream) { \
                WAIT_FOR_FREE(); \
                jbstream = (struct jb_audio_stream *)direction ## _streams[i].jb_stream_ ## direction; \
                jbstream_ ## direction = direction ## _streams[i].jb_stream_ ## direction; \
                pre_fn; \
                ret = jbstream_ ## direction ->function parameters; \
                post_fn; \
                UNLOCK_FREE(); \
                break; \
            } \
        } \
        pthread_mutex_unlock(& direction ## _streams_mutex); \
    \
        return ret; \
    }

#define WRAP_STREAM_LOCKED(name, direction, rettype, err, prototype, parameters, log) \
        _WRAP_STREAM_LOCKED(name, name, direction, rettype, err, prototype, parameters, log, do{}while(0), do{}while(0))

#define WRAP_STREAM_LOCKED_FN(name, direction, rettype, err, prototype, parameters, log, pre_fn, post_fn) \
        _WRAP_STREAM_LOCKED(name, name, direction, rettype, err, prototype, parameters, log, pre_fn, post_fn)

#define WRAP_STREAM_LOCKED_COMMON(name, direction, rettype, err, prototype, parameters, log) \
        _WRAP_STREAM_LOCKED(name, common.name, direction, rettype, err, prototype, parameters, log, do{}while(0), do{}while(0))

#define WRAP_STREAM_LOCKED_COMMON_FN(name, direction, rettype, err, prototype, parameters, log, pre_fn, post_fn) \
        _WRAP_STREAM_LOCKED(name, common.name, direction, rettype, err, prototype, parameters, log, pre_fn, post_fn)

/* Generic wrappers for HAL */
#define _WRAP_HAL_LOCKED(name, function, prototype, parameters, log) \
    static int wrapper_ ## name  prototype \
    { \
        int ret; \
    \
        if (log) ALOGI log; \
        pthread_mutex_lock(&out_streams_mutex); \
        pthread_mutex_lock(&in_streams_mutex); \
    \
        WAIT_FOR_FREE(); \
        ret = jb_hw_dev->function parameters; \
        UNLOCK_FREE(); \
    \
        pthread_mutex_unlock(&in_streams_mutex); \
        pthread_mutex_unlock(&out_streams_mutex); \
    \
        return ret; \
    }

#define WRAP_HAL_LOCKED(name, prototype, parameters, log) \
        _WRAP_HAL_LOCKED(name, name, prototype, parameters, log)

#define _WRAP_HAL(name, function, rettype, prototype, parameters, log) \
    static rettype wrapper_ ## name  prototype \
    { \
        ALOGI log; \
    \
        return jb_hw_dev->function parameters; \
    }

#define WRAP_HAL(name, rettype, prototype, parameters, log) \
        _WRAP_HAL(name, name, rettype, prototype, parameters, log)

/* Unused parameters */
#define unused_audio_hw_device	__attribute__((unused)) struct audio_hw_device

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
    struct jb_audio_stream_in *wrapped_stream = NULL;

    pthread_mutex_lock(&in_streams_mutex);
    for (i = 0; i < n_in_streams; i++) {
        if (in_streams[i].stream_in == stream) {
            wrapped_stream = in_streams[i].jb_stream_in;
            INCREMENT_IN_USE();
            break;
        }
    }
    pthread_mutex_unlock(&in_streams_mutex);

    if (wrapped_stream) {
        ret = wrapped_stream->read(wrapped_stream, buffer, bytes);
#if 0
        if ((ret > 0) && (ret != (ssize_t)bytes)) {
            if (wrapper_hal_is_resampling(bytes, ret))
                ret = bytes;
        }
#endif
        if (ret != (ssize_t)bytes) {
            ALOGE("read %u bytes instead of %u", (unsigned int)ret, (unsigned int)bytes);
        }
        DECREMENT_IN_USE();
    } else {
            ALOGE("read on non-wrapped stream!");
    }

    return ret;
}

WRAP_STREAM_LOCKED(set_gain, in, int, -ENODEV, (struct audio_stream_in *stream, float gain),
            (jbstream_in, gain), ("in_set_gain: %f", gain))

WRAP_STREAM_LOCKED_COMMON(standby, in, int, -ENODEV, (struct audio_stream *stream),
            (jbstream), ("in_standby"))

WRAP_STREAM_LOCKED_COMMON_FN(set_parameters, in, int, -ENODEV, (struct audio_stream *stream, const char *kv_pairs),
            (jbstream, kv_pairs), ("in_set_parameters: %s", kv_pairs), do{}while(0), do{if (ret) {ALOGI("ret: %d", ret);}}while(0))

WRAP_STREAM_LOCKED_COMMON(get_sample_rate, in, uint32_t, 0, (const struct audio_stream *stream),
            (jbstream), ("in_get_sample_rate"))

WRAP_STREAM_LOCKED_COMMON(set_sample_rate, in, int, -ENODEV, (struct audio_stream *stream, uint32_t rate),
            (jbstream, rate), ("in_set_sample_rate: %u", rate))

WRAP_STREAM_LOCKED_COMMON(get_buffer_size, in, size_t, 0, (const struct audio_stream *stream),
            (jbstream), ("in_get_buffer_size"))

WRAP_STREAM_LOCKED_COMMON(get_channels, in, audio_channel_mask_t, 0, (const struct audio_stream *stream),
            (jbstream), ("in_get_channels"))

WRAP_STREAM_LOCKED_COMMON(get_format, in, audio_format_t, 0, (const struct audio_stream *stream),
            (jbstream), ("in_get_format"))

WRAP_STREAM_LOCKED_COMMON(set_format, in, int, -ENODEV, (struct audio_stream *stream, audio_format_t format),
            (jbstream, format), ("in_set_format: %u", format))

WRAP_STREAM_LOCKED_COMMON(dump, in, int, -ENODEV, (const struct audio_stream *stream, int fd),
            (jbstream, fd), ("in_dump: %d", fd))

WRAP_STREAM_LOCKED_COMMON(get_device, in, audio_devices_t, 0, (const struct audio_stream *stream),
            (jbstream), ("in_get_device"))

WRAP_STREAM_LOCKED_COMMON(set_device, in, int, -ENODEV, (struct audio_stream *stream, audio_devices_t device),
            (jbstream, device), ("in_set_device: %d", device))

WRAP_STREAM_LOCKED_COMMON(get_parameters, in, char*, NULL, (const struct audio_stream *stream, const char *keys),
            (jbstream, keys), ("in_get_parameters: %s", keys))

WRAP_STREAM_LOCKED_COMMON(add_audio_effect, in, int, -ENODEV, (const struct audio_stream *stream, effect_handle_t effect),
            (jbstream, effect), ("in_add_audio_effect"))

WRAP_STREAM_LOCKED_COMMON(remove_audio_effect, in, int, -ENODEV, (const struct audio_stream *stream, effect_handle_t effect),
            (jbstream, effect), ("in_remove_audio_effect"))

static void wrapper_close_input_stream(unused_audio_hw_device *dev,
                                       struct audio_stream_in *stream_in)
{
    struct jb_audio_stream_in *jb_stream_in = NULL;
    int i;

    pthread_mutex_lock(&in_streams_mutex);
    for (i = 0; i < n_in_streams; i++) {
        if (in_streams[i].stream_in == stream_in) {
            jb_stream_in = in_streams[i].jb_stream_in;
            free(in_streams[i].stream_in);
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
    if (jb_stream_in) {
        WAIT_FOR_FREE();
        jb_hw_dev->close_input_stream(jb_hw_dev, jb_stream_in);
        UNLOCK_FREE();
    }

    pthread_mutex_unlock(&in_streams_mutex);
}

uint32_t wrapper_get_input_frames_lost(__attribute__((unused))struct audio_stream_in *stream)
{
        return 0;
}

static int wrapper_open_input_stream(unused_audio_hw_device *dev,
                                     audio_io_handle_t handle,
                                     audio_devices_t devices,
                                     struct audio_config *config,
                                     struct audio_stream_in **stream_in,
                                     __attribute__((unused)) audio_input_flags_t flags,
                                     __attribute__((unused)) const char *address,
                                     __attribute__((unused)) audio_source_t source)
{
    struct jb_audio_stream_in *jb_stream_in;
    int ret;

    pthread_mutex_lock(&in_streams_mutex);

    /* Convert to JB_MR0 devices */
    if (devices & 0x80000000) {
        devices &= ~0x80000000;
        if (devices < 0x2000) {
                devices *= 0x10000;
        } else {
                devices |= 0x10000;
        }
    }

    WAIT_FOR_FREE();
    ret = jb_hw_dev->open_input_stream(jb_hw_dev, handle, devices,
                                         config, &jb_stream_in);
    UNLOCK_FREE();

    if (ret == 0) {
        struct wrapper_in_stream *new_in_streams;

        new_in_streams = realloc(in_streams,
                              sizeof(struct wrapper_in_stream) * (n_in_streams + 1));
        if (!new_in_streams) {
            ALOGE("Can't allocate memory for wrapped stream, not touching original!");
            pthread_mutex_unlock(&in_streams_mutex);
            return -ENOMEM;
        }
        in_streams = new_in_streams;
        memset(&in_streams[n_in_streams], 0, sizeof(struct wrapper_in_stream));

        in_streams[n_in_streams].jb_stream_in = jb_stream_in;
        in_streams[n_in_streams].stream_in = malloc(sizeof(struct audio_stream_in));
        if (!in_streams[n_in_streams].stream_in) {
            ALOGE("Can't allocate memory for stream_in!");
            pthread_mutex_unlock(&in_streams_mutex);
            return -ENOMEM;
        }
        memset(in_streams[n_in_streams].stream_in, 0, sizeof(struct audio_stream_in));
        *stream_in = in_streams[n_in_streams].stream_in;

        (*stream_in)->common.get_sample_rate = wrapper_in_get_sample_rate;
        (*stream_in)->common.set_sample_rate = wrapper_in_set_sample_rate;
        (*stream_in)->common.get_buffer_size = wrapper_in_get_buffer_size;
        (*stream_in)->common.get_channels = wrapper_in_get_channels;
        (*stream_in)->common.get_format = wrapper_in_get_format;
        (*stream_in)->common.set_format = wrapper_in_set_format;
        (*stream_in)->common.standby = wrapper_in_standby;
        (*stream_in)->common.dump = wrapper_in_dump;
        (*stream_in)->common.get_device = wrapper_in_get_device;
        (*stream_in)->common.set_device = wrapper_in_set_device;
        (*stream_in)->common.set_parameters = wrapper_in_set_parameters;
        (*stream_in)->common.get_parameters = wrapper_in_get_parameters;
        (*stream_in)->common.add_audio_effect = wrapper_in_add_audio_effect;
        (*stream_in)->common.remove_audio_effect = wrapper_in_remove_audio_effect;

        (*stream_in)->set_gain = wrapper_in_set_gain;
        (*stream_in)->read = wrapper_read;
        (*stream_in)->get_input_frames_lost = wrapper_get_input_frames_lost;

        ALOGI("Wrapped an input stream: rate %d, channel_mask: %x, format: %d, addr: %p/%p",
              config->sample_rate, config->channel_mask, config->format, *stream_in, jb_stream_in);

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
    struct jb_audio_stream_out *wrapped_stream = NULL;
    size_t written;

    pthread_mutex_lock(&out_streams_mutex);
    for (i = 0; i < n_out_streams; i++) {
        if (out_streams[i].stream_out == stream) {
            wrapped_stream = out_streams[i].jb_stream_out;
            INCREMENT_IN_USE();
            break;
        }
    }
    pthread_mutex_unlock(&out_streams_mutex);

    if (wrapped_stream) {
        written = wrapped_stream->write(wrapped_stream, buffer, bytes);
        ret = written;
        if ((ret > 0) && (written != bytes)) {
            if (wrapper_hal_is_resampling(bytes, written))
                ret = bytes;
        }
        DECREMENT_IN_USE();
    } else {
            ALOGE("write on non-wrapped stream!");
    }

    return ret;
}

WRAP_STREAM_LOCKED(set_volume, out, int, -ENODEV, (struct audio_stream_out *stream, float left, float right),
            (jbstream_out, left, right), ("set_out_volume: %f/%f", left, right))

WRAP_STREAM_LOCKED_COMMON(standby, out, int, -ENODEV, (struct audio_stream *stream),
            (jbstream), ("out_standby"))

static void restore_mute(void)
{
        if (hal_audio_mode == AUDIO_MODE_IN_CALL) {
                if (last_mute_ctl_value != -1)
                        alsa_set_mic_mute(1);
        }
}

WRAP_STREAM_LOCKED_COMMON_FN(set_parameters, out, int, -ENODEV, (struct audio_stream *stream, const char *kv_pairs),
            (jbstream, kv_pairs), ("out_set_parameters: %s", kv_pairs), do{}while(0), do{if (ret) {ALOGI("ret: %d", ret);} restore_mute();}while(0))

WRAP_STREAM_LOCKED_COMMON(get_sample_rate, out, uint32_t, 0, (const struct audio_stream *stream),
            (jbstream), ("out_get_sample_rate"))

WRAP_STREAM_LOCKED_COMMON(set_sample_rate, out, int, -ENODEV, (struct audio_stream *stream, uint32_t rate),
            (jbstream, rate), ("out_set_sample_rate: %u", rate))

WRAP_STREAM_LOCKED_COMMON(get_buffer_size, out, size_t, 0, (const struct audio_stream *stream),
            (jbstream), ("out_get_buffer_size"))

WRAP_STREAM_LOCKED_COMMON(get_channels, out, audio_channel_mask_t, 0, (const struct audio_stream *stream),
            (jbstream), ("out_get_channels"))

WRAP_STREAM_LOCKED_COMMON(get_format, out, audio_format_t, 0, (const struct audio_stream *stream),
            (jbstream), ("out_get_format"))

WRAP_STREAM_LOCKED_COMMON(set_format, out, int, -ENODEV, (struct audio_stream *stream, audio_format_t format),
            (jbstream, format), ("out_set_format: %u", format))

WRAP_STREAM_LOCKED_COMMON(dump, out, int, -ENODEV, (const struct audio_stream *stream, int fd),
            (jbstream, fd), ("out_dump: %d", fd))

WRAP_STREAM_LOCKED_COMMON(get_device, out, audio_devices_t, 0, (const struct audio_stream *stream),
            (jbstream), ("out_get_device"))

WRAP_STREAM_LOCKED_COMMON(set_device, out, int, -ENODEV, (struct audio_stream *stream, audio_devices_t device),
            (jbstream, device), ("out_set_device: %d", device))

WRAP_STREAM_LOCKED_COMMON(get_parameters, out, char*, NULL, (const struct audio_stream *stream, const char *keys),
            (jbstream, keys), ("out_get_parameters: %s", keys))

WRAP_STREAM_LOCKED_COMMON(add_audio_effect, out, int, -ENODEV, (const struct audio_stream *stream, effect_handle_t effect),
            (jbstream, effect), ("out_add_audio_effect"))

WRAP_STREAM_LOCKED_COMMON(remove_audio_effect, out, int, -ENODEV, (const struct audio_stream *stream, effect_handle_t effect),
            (jbstream, effect), ("out_remove_audio_effect"))

WRAP_STREAM_LOCKED(get_latency, out, uint32_t, 0, (const struct audio_stream_out *stream),
            (jbstream_out), ("out_get_latency"))

WRAP_STREAM_LOCKED(get_render_position, out, int, -ENODEV, (const struct audio_stream_out *stream, uint32_t *dsp_frames),
            (jbstream_out, dsp_frames), ("out_get_render_position"))

WRAP_STREAM_LOCKED(get_next_write_timestamp, out, int, -ENODEV, (const struct audio_stream_out *stream, int64_t *timestamp),
            (jbstream_out, timestamp), NULL)

int wrapper_get_presentation_position(__attribute__((unused)) const struct audio_stream_out *stream,
                __attribute__((unused)) uint64_t *frames, __attribute__((unused)) struct timespec *timestamp)
{
        return -1;
}

static void wrapper_close_output_stream(unused_audio_hw_device *dev,
                            struct audio_stream_out* stream_out)
{
    struct jb_audio_stream_out *jb_stream_out = NULL;
    int i;

    pthread_mutex_lock(&out_streams_mutex);
    for (i = 0; i < n_out_streams; i++) {
        if (out_streams[i].stream_out == stream_out) {
            jb_stream_out = out_streams[i].jb_stream_out;
            free(out_streams[i].stream_out);
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

    if (jb_stream_out) {
        WAIT_FOR_FREE();
        jb_hw_dev->close_output_stream(jb_hw_dev, jb_stream_out);
        UNLOCK_FREE();
    }

    pthread_mutex_unlock(&out_streams_mutex);
}

static int wrapper_open_output_stream(unused_audio_hw_device *dev,
                                      audio_io_handle_t handle,
                                      audio_devices_t devices,
                                      audio_output_flags_t flags,
                                      struct audio_config *config,
                                      struct audio_stream_out **stream_out,
                                      __attribute__((unused)) const char *address)
{
    struct jb_audio_stream_out *jb_stream_out;
    int ret;

    pthread_mutex_lock(&out_streams_mutex);

    WAIT_FOR_FREE();
    ret = jb_hw_dev->open_output_stream(jb_hw_dev, handle, devices,
                                          flags, config, &jb_stream_out);
    UNLOCK_FREE();

    if (ret == 0) {
        struct wrapper_out_stream *new_out_streams;

        new_out_streams = realloc(out_streams,
                              sizeof(struct wrapper_out_stream) * (n_out_streams + 1));
        if (!new_out_streams) {
            ALOGE("Can't allocate memory for wrapped stream, not touching original!");
            pthread_mutex_unlock(&out_streams_mutex);
            return -ENOMEM;
        }
        out_streams = new_out_streams;
        memset(&out_streams[n_out_streams], 0, sizeof(struct wrapper_out_stream));

        out_streams[n_out_streams].jb_stream_out = jb_stream_out;
        out_streams[n_out_streams].stream_out = malloc(sizeof(struct audio_stream_out));
        if (!out_streams[n_out_streams].stream_out) {
            ALOGE("Can't allocate memory for stream_out!");
            pthread_mutex_unlock(&out_streams_mutex);
            return -ENOMEM;
        }
        memset(out_streams[n_out_streams].stream_out, 0, sizeof(struct audio_stream_out));
        *stream_out = out_streams[n_out_streams].stream_out;

        (*stream_out)->common.get_sample_rate = wrapper_out_get_sample_rate;
        (*stream_out)->common.set_sample_rate = wrapper_out_set_sample_rate;
        (*stream_out)->common.get_buffer_size = wrapper_out_get_buffer_size;
        (*stream_out)->common.get_channels = wrapper_out_get_channels;
        (*stream_out)->common.get_format = wrapper_out_get_format;
        (*stream_out)->common.set_format = wrapper_out_set_format;
        (*stream_out)->common.standby = wrapper_out_standby;
        (*stream_out)->common.dump = wrapper_out_dump;
        (*stream_out)->common.get_device = wrapper_out_get_device;
        (*stream_out)->common.set_device = wrapper_out_set_device;
        (*stream_out)->common.set_parameters = wrapper_out_set_parameters;
        (*stream_out)->common.get_parameters = wrapper_out_get_parameters;
        (*stream_out)->common.add_audio_effect = wrapper_out_add_audio_effect;
        (*stream_out)->common.remove_audio_effect = wrapper_out_remove_audio_effect;

        (*stream_out)->get_latency = wrapper_out_get_latency;
        (*stream_out)->set_volume = wrapper_out_set_volume;
        (*stream_out)->write = wrapper_write;
        (*stream_out)->get_render_position = wrapper_out_get_render_position;
        (*stream_out)->get_next_write_timestamp = wrapper_out_get_next_write_timestamp;
        (*stream_out)->set_callback = NULL;
        (*stream_out)->pause = NULL;
        (*stream_out)->resume = NULL;
        (*stream_out)->drain = NULL;
        (*stream_out)->flush = NULL;
        (*stream_out)->get_presentation_position = wrapper_get_presentation_position;

        ALOGI("Wrapped an output stream: rate %d, channel_mask: %x, format: %d, addr: %p/%p",
              config->sample_rate, config->channel_mask, config->format, *stream_out, jb_stream_out);

        n_out_streams++;
    }
    pthread_mutex_unlock(&out_streams_mutex);

    return ret;
}

/* Generic HAL */

WRAP_HAL_LOCKED(set_master_volume, (unused_audio_hw_device *dev, float volume),
                (jb_hw_dev, volume), ("set_master_volume: %f", volume))

#ifndef ICS_VOICE_BLOB
WRAP_HAL_LOCKED(set_voice_volume, (unused_audio_hw_device *dev, float volume),
                (jb_hw_dev, volume), ("set_voice_volume: %f", volume))
#else
static int wrapper_set_voice_volume(unused_audio_hw_device *dev, float volume)
{
    ALOGI("ICS: set_voice_volume: %f", volume);

    return ics_hw_dev->set_voice_volume(ics_hw_dev, volume);
}
#endif

static int alsa_set_mic_mute(bool state)
{
	struct mixer *mixer;
	struct mixer_ctl *ctl;
	int ret = 1;

	if ((!state) && (last_mute_ctl_value == -1)) {
		return 0;
	}

	mixer = mixer_open(ALSA_CARD);
	if (!mixer) {
		ALOGW("Can't open alsa mixer!");
		return 0;
	}

	ctl = mixer_get_ctl_by_name(mixer, MUTE_CTL);
	if (!ctl) {
		ALOGW("Can't find mixer control " MUTE_CTL "!");
		mixer_close(mixer);
		return 0;
	}

	if (state) {
		if (last_mute_ctl_value == -1)
			last_mute_ctl_value = mixer_ctl_get_value(ctl, 0);

		ALOGI("Setting " MUTE_CTL " to " MUTE_VALUE);
		if (mixer_ctl_set_enum_by_string(ctl, MUTE_VALUE)) {
			ALOGW("Can't set control!");
			ret = 0;
		}
	} else {
		ALOGI("Setting " MUTE_CTL " to %d", last_mute_ctl_value);
		if (mixer_ctl_set_value(ctl, 0, last_mute_ctl_value)) {
			ALOGW("Can't set control!");
			ret = 0;
		}
		last_mute_ctl_value = -1;
	}


	mixer_close(mixer);

	return ret;
}

static int wrapper_set_mic_mute(unused_audio_hw_device *dev, bool state)
{
	int ret;

	ALOGI("set_mic_mute: %d", state);

	pthread_mutex_lock(&out_streams_mutex);
	pthread_mutex_lock(&in_streams_mutex);

	WAIT_FOR_FREE();
	ret = jb_hw_dev->set_mic_mute(jb_hw_dev, state);
	if (hal_audio_mode == AUDIO_MODE_IN_CALL) {
		alsa_set_mic_mute(state);
	}
	UNLOCK_FREE();

	pthread_mutex_unlock(&in_streams_mutex);
	pthread_mutex_unlock(&out_streams_mutex);

	return ret;
}

static int wrapper_set_mode(unused_audio_hw_device *dev, audio_mode_t mode)
{
	int ret;

	ALOGI("set_mode: %d", mode);

	pthread_mutex_lock(&out_streams_mutex);
	pthread_mutex_lock(&in_streams_mutex);

	WAIT_FOR_FREE();
	ret = jb_hw_dev->set_mode(jb_hw_dev, mode);
	if ((hal_audio_mode == AUDIO_MODE_IN_CALL) && (mode != AUDIO_MODE_IN_CALL)) {
		if (last_mute_ctl_value != -1) {
			alsa_set_mic_mute(0);
			last_mute_ctl_value = -1;
		}
	}
	hal_audio_mode = mode;
	UNLOCK_FREE();

	pthread_mutex_unlock(&in_streams_mutex);
	pthread_mutex_unlock(&out_streams_mutex);

	return ret;
}

WRAP_HAL_LOCKED(set_parameters, (unused_audio_hw_device *dev, const char *kv_pairs),
                (jb_hw_dev, kv_pairs), ("set_parameters: %s", kv_pairs))

WRAP_HAL(get_supported_devices, uint32_t, (const unused_audio_hw_device *dev),
		(jb_hw_dev), ("get_supported_devices"))

WRAP_HAL(init_check, int, (const unused_audio_hw_device *dev),
		(jb_hw_dev), ("init_check"))

WRAP_HAL(get_master_volume, int, (unused_audio_hw_device *dev, float *volume),
		(jb_hw_dev, volume), ("get_master_volume"))

WRAP_HAL(get_mic_mute, int, (const unused_audio_hw_device *dev, bool *state),
		(jb_hw_dev, state), ("get_mic_mute"))

WRAP_HAL(get_parameters, char*, (const unused_audio_hw_device *dev, const char *keys),
		(jb_hw_dev, keys), ("get_parameters: %s", keys))

WRAP_HAL(get_input_buffer_size, size_t, (const unused_audio_hw_device *dev, const struct audio_config *config),
		(jb_hw_dev, config), ("get_input_buffer_size"))

WRAP_HAL(dump, int, (const unused_audio_hw_device *dev, int fd),
		(jb_hw_dev, fd), ("dump"))

#ifdef ICS_VOICE_BLOB
static int wrapper_set_mode_ics(unused_audio_hw_device *dev, audio_mode_t mode)
{
    ALOGI("ICS: set_mode: %d", mode);

    ics_hw_dev->set_mode(ics_hw_dev, mode);

    return wrapper_set_mode(dev, mode);
}

static int wrapper_set_parameters_ics(unused_audio_hw_device *dev, const char *kv_pairs)
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

    ret = jb_hw_dev->common.close(device);

    dlclose(dso_handle);
    dso_handle = NULL;
    free(jb_hw_dev);
    jb_hw_dev = NULL;

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

extern const char * _ZN7android14AudioParameter10keyRoutingE;

static int wrapper_open(__attribute__((unused)) const hw_module_t* module,
                             __attribute__((unused)) const char* name,
                             hw_device_t** device)
{
    struct hw_module_t *hmi;
    struct audio_hw_device *adev;
    int ret;

    ALOGI("Initializing wrapper for Motorola's audio-HAL");
    if (jb_hw_dev) {
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

    ret = audio_hw_device_open(hmi, (struct audio_hw_device**)&jb_hw_dev);
    ALOGE_IF(ret, "%s couldn't open audio module in %s. (%s)", __func__,
                 AUDIO_HARDWARE_MODULE_ID, strerror(-ret));
    if (ret) {
        dlclose(dso_handle);
        dso_handle = NULL;
        return ret;
    }

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

    *device = malloc(sizeof(struct audio_hw_device));
    if (!*device) {
        ALOGE("Can't allocate memory for device, aborting...");
        dlclose(dso_handle);
        dso_handle = NULL;
#ifdef ICS_VOICE_BLOB
        dlclose(ics_dso_handle);
        ics_dso_handle = NULL;
#endif
        return -ENOMEM;
    }

    memset(*device, 0, sizeof(struct audio_hw_device));

    adev = (struct audio_hw_device*)*device;

    /* HAL */
    adev->common.tag = HARDWARE_DEVICE_TAG;
    adev->common.version = AUDIO_DEVICE_API_VERSION_MIN;
    adev->common.module = (struct hw_module_t *) module;
    adev->common.close = wrapper_close;

    adev->get_supported_devices = wrapper_get_supported_devices;
    adev->init_check = wrapper_init_check;
    adev->set_voice_volume = wrapper_set_voice_volume;
    adev->set_master_volume = wrapper_set_master_volume;
    adev->get_master_volume = wrapper_get_master_volume;
    adev->set_mic_mute = wrapper_set_mic_mute;
    adev->get_mic_mute = wrapper_get_mic_mute;
    adev->get_parameters = wrapper_get_parameters;
    adev->get_input_buffer_size = wrapper_get_input_buffer_size;
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

    adev->dump = wrapper_dump;
    adev->set_master_mute = NULL;
    adev->get_master_mute = NULL;
    adev->create_audio_patch = NULL;
    adev->release_audio_patch = NULL;
    adev->get_audio_port = NULL;
    adev->set_audio_port_config = NULL;

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
