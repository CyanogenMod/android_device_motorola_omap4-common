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

#include <tinyalsa/asoundlib.h>

#include <tiaudioutils/Pcm.h>

namespace tiaudioutils {

PcmParams::PcmParams()
    : channels(0), sampleBits(0), sampleRate(0), frameCount(0)
{
}

PcmParams::PcmParams(uint32_t chan, uint32_t bits, uint32_t rate, uint32_t frames)
    : channels(chan), sampleBits(bits), sampleRate(rate), frameCount(frames)
{
}

PcmParams::PcmParams(const struct pcm_config &config)
    : channels(config.channels),
      sampleRate(config.rate),
      frameCount(config.period_size)
{
    switch (config.format) {
    case PCM_FORMAT_S32_LE:
        sampleBits = 32;
        break;
    case PCM_FORMAT_S16_LE:
    default:
        sampleBits = 16;
        break;
    }
}

PcmParams::PcmParams(const struct audio_config &config, uint32_t frames)
    : channels(popcount(config.channel_mask)),
      sampleRate(config.sample_rate),
      frameCount(frames)
{
    switch (config.format) {
    case AUDIO_FORMAT_PCM_8_BIT:
        sampleBits = 8;
        break;
    case AUDIO_FORMAT_PCM_8_24_BIT:
    case AUDIO_FORMAT_PCM_32_BIT:
        sampleBits = 32;
        break;
    case AUDIO_FORMAT_PCM_16_BIT:
    default:
        sampleBits = 16;
        break;
    }
}

bool PcmParams::isValid() const
{
    return (channels && sampleRate && sampleBits);
}

uint32_t PcmParams::framesToBytes(uint32_t frames) const
{
    return (frames * frameSize());
}

uint32_t PcmParams::bytesToFrames(uint32_t bytes) const
{
    return (bytes / frameSize());
}

void PcmParams::toPcmConfig(struct pcm_config &config) const
{
    config.channels = channels;
    config.rate = sampleRate;
    config.period_size = frameCount;

    switch (sampleBits) {
    case 32:
        config.format = PCM_FORMAT_S32_LE;
        break;
    case 16:
    default:
        config.format = PCM_FORMAT_S16_LE;
        break;
    }
}

void PcmParams::toAudioConfig(struct audio_config &config, StreamDirection dir) const
{
    config.sample_rate = sampleRate;

    if (dir == CAPTURE) {
        config.channel_mask = audio_channel_in_mask_from_count(channels);
    } else {
        config.channel_mask = audio_channel_out_mask_from_count(channels);
    }

    switch (sampleBits) {
    case 32:
        config.format = AUDIO_FORMAT_PCM_32_BIT;
        break;
    case 16:
    default:
        config.format = AUDIO_FORMAT_PCM_16_BIT;
        break;
    }
}

} /* namespace tiaudioutils */
