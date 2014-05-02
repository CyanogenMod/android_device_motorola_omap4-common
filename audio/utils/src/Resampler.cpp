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
#include <speex/speex_resampler.h>

#include <tiaudioutils/Log.h>
#include <tiaudioutils/Resampler.h>

namespace tiaudioutils {

Resampler::Resampler(const PcmParams &params, uint32_t quality)
    : mInParams(params), mOutParams(params), mQuality(quality),
      mAvail(0), mBufferFrames(0), mRatioNum(0), mRatioDen(1)
{
    createResampler();

    /* initial buffer to hold the equivalent frames of input buffer size */
    uint32_t frames = (mInParams.frameCount * mRatioNum) / mRatioDen;
    reallocateBuffer(frames);
}

Resampler::Resampler(const PcmParams &inParams,
                     const PcmParams &outParams,
                     uint32_t quality)
    : mInParams(inParams), mOutParams(outParams), mQuality(quality),
      mAvail(0), mBufferFrames(0), mRatioNum(0), mRatioDen(1)
{
    createResampler();

    /* initial buffer to hold the equivalent frames of input buffer size */
    uint32_t frames = (mInParams.frameCount * mRatioNum) / mRatioDen;
    reallocateBuffer(frames);
}

Resampler::~Resampler()
{
    if (mBuffer.i16)
        delete [] mBuffer.i16;

    if (mSpeexRsmp)
        speex_resampler_destroy(mSpeexRsmp);
}

bool Resampler::initCheck() const
{
    if ((mInParams.sampleBits != 16) || (mOutParams.sampleBits != 16)) {
        ALOGE("Resampler: %u bits/sample is not supported",
              mInParams.sampleBits);
        return false;
    }

    if (mInParams.channels != mOutParams.channels) {
        ALOGE("Resampler: channel count mismatch, in %u out %u",
              mInParams.channels, mOutParams.channels);
        return false;
    }

    if (!mSpeexRsmp || !mBuffer.raw)
        return false;

    return true;
}

int Resampler::setInSampleRate(uint32_t rate)
{
    AutoMutex lock(mLock);

    if (rate == mInParams.sampleRate)
        return 0;

    ALOGV("Resampler: set new input sample rate %u", rate);

    int ret = speex_resampler_set_rate(mSpeexRsmp,
                                       rate,
                                       mOutParams.sampleRate);
    if (ret) {
        ALOGE("Resampler: failed to set new input sample rate: %s",
              speex_resampler_strerror(ret));
        return ret;
    }

    mInParams.sampleRate = rate;
    speex_resampler_get_ratio(mSpeexRsmp, &mRatioNum, &mRatioDen);

    uint32_t frames = (mInParams.frameCount * mRatioNum) / mRatioDen;
    if (frames > mBufferFrames)
        reallocateBuffer(frames);

    return 0;
}

int Resampler::setOutSampleRate(uint32_t rate)
{
    AutoMutex lock(mLock);

    if (rate == mOutParams.sampleRate)
        return 0;

    ALOGV("Resampler: set new output sample rate %u", rate);

    int ret = speex_resampler_set_rate(mSpeexRsmp,
                                       mInParams.sampleRate,
                                       rate);
    if (ret) {
        ALOGE("Resampler: failed to set new output sample rate: %s",
              speex_resampler_strerror(ret));
        return ret;
    }

    mOutParams.sampleRate = rate;
    speex_resampler_get_ratio(mSpeexRsmp, &mRatioNum, &mRatioDen);

    uint32_t frames = (mInParams.frameCount * mRatioNum) / mRatioDen;
    if (frames > mBuffer.frameCount)
        reallocateBuffer(frames);

    return 0;
}

void Resampler::getRatio(uint32_t &num, uint32_t &den) const
{
    num = mRatioNum;
    den = mRatioDen;
}

int Resampler::resample(BufferProvider &provider,
                        void *outBuffer,
                        uint32_t outFrames)
{
    if (!outBuffer)
        return -EINVAL;

    if (!outFrames)
        return 0;

    AutoMutex lock(mLock);

    /*
     * Calculate the number of frames required on the input side to
     * produce the requested output frames. Intermediate buffer is
     * resized accordingly.
     */
    uint32_t reqInFrames = (outFrames * mRatioNum) / mRatioDen + 1;
    if (reqInFrames > mBuffer.frameCount)
        reallocateBuffer(reqInFrames);

    uint32_t written = 0;
    while (written < outFrames) {
        if (mAvail < reqInFrames) {
            BufferProvider::Buffer buf;

            buf.frameCount = reqInFrames - mAvail;

            int ret = provider.getNextBuffer(&buf);
            if (ret) {
                ALOGE("Resampler: failed to get next buffer %d", ret);
                return ret;
            }

            /* append new buffer to existing frames in local buffer */
            memcpy(mBuffer.i8 + mOutParams.framesToBytes(mAvail),
                   buf.i8,
                   mOutParams.framesToBytes(buf.frameCount));
            mAvail += buf.frameCount;

            provider.releaseBuffer(&buf);
        }

        uint32_t framesIn = mAvail;
        uint32_t framesOut = outFrames - written;
        int16_t *bufferIn = mBuffer.i16;
        int16_t *bufferOut = (int16_t *)outBuffer + (written * mOutParams.channels);

        /* resample */
        if (mOutParams.channels == 1) {
            speex_resampler_process_int(mSpeexRsmp,
                                        0,
                                        bufferIn,
                                        &framesIn,
                                        bufferOut,
                                        &framesOut);
        } else {
            speex_resampler_process_interleaved_int(mSpeexRsmp,
                                        bufferIn,
                                        &framesIn,
                                        bufferOut,
                                        &framesOut);
        }
        written += framesOut;
        mAvail -= framesIn;

        /* move samples left to the beginning of the local buffer */
        if (mAvail) {
            memmove(mBuffer.raw,
                    mBuffer.i8 + mOutParams.framesToBytes(framesIn),
                    mOutParams.framesToBytes(mAvail));
        }
    }

    ALOGW_IF(written != outFrames,
             "Resampler: frame count mismatch, req %u written %u", outFrames, written);

    return 0;
}

int Resampler::resample(const void *inBuffer, uint32_t &inFrames,
                        void *outBuffer, uint32_t &outFrames)
{
    AutoMutex lock(mLock);

    /* resample */
    if (mOutParams.channels == 1) {
        speex_resampler_process_int(mSpeexRsmp,
                                    0,
                                    (int16_t *)inBuffer,
                                    &inFrames,
                                    (int16_t *)outBuffer,
                                    &outFrames);
    } else {
        speex_resampler_process_interleaved_int(mSpeexRsmp,
                                    (int16_t *)inBuffer,
                                    &inFrames,
                                    (int16_t *)outBuffer,
                                    &outFrames);
    }

    return 0;
}

void Resampler::createResampler()
{
    ALOGV("Resampler: create speex resampler %u to %u Hz",
          mInParams.sampleRate, mOutParams.sampleRate);

    if (mQuality > SPEEX_RESAMPLER_QUALITY_MAX)
        mQuality = SPEEX_RESAMPLER_QUALITY_MAX;

    int ret;
    mSpeexRsmp = speex_resampler_init(mInParams.channels,
                                      mInParams.sampleRate,
                                      mOutParams.sampleRate,
                                      mQuality,
                                      &ret);
    if (!mSpeexRsmp) {
        ALOGE("Resampler: failed to create Speex resampler: %s",
              speex_resampler_strerror(ret));
        return;
    }

    speex_resampler_reset_mem(mSpeexRsmp);
    speex_resampler_get_ratio(mSpeexRsmp, &mRatioNum, &mRatioDen);
}

void Resampler::reallocateBuffer(uint32_t frames)
{
    /* current buffer is large enough */
    if (frames < mBufferFrames)
        return;

    int16_t *oldBuf = mBuffer.i16;
    mBufferFrames = frames;

    /* keep the frame count with the headroom frames under the hood */
    mBuffer.frameCount = mBufferFrames + kHeadRoomFrames;
    mBuffer.i16 = new int16_t[mBuffer.frameCount * mInParams.channels];

    /* copy frames in the old buffer to the new larger buffer */
    if (mAvail)
        memcpy(mBuffer.raw, oldBuf, mInParams.framesToBytes(mAvail));

    if (oldBuf)
        delete [] oldBuf;
}

} /* namespace tiaudioutils */
