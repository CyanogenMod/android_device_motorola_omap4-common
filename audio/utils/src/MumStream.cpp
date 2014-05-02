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

#include <tiaudioutils/Log.h>
#include <tiaudioutils/MumStream.h>
#include <tiaudioutils/Stream.h>

namespace tiaudioutils {

Merge::Merge(const PcmParams &params)
    : mParams(params), mDstMask(0)
{
    mContainerBytes = mParams.sampleBits / 8;

    /* 24-bit samples (3 bytes) are actually in 32-bit containers (4 bytes) */
    mContainerBytes = (mContainerBytes == 3) ? 4 : mContainerBytes;
}

Merge::~Merge()
{
    for (StreamSet::iterator i = mStreams.begin(); i != mStreams.end(); ++i) {
        sp<OutStream> stream = (*i).promote();
        if (stream == 0)
            continue;
        ALOGW("Merge: automatically un-registering stream %p during destroy",
              stream.get());
        unregisterStream(stream);
    }
}

bool Merge::initCheck() const
{
    if (!mParams.isValid()) {
        ALOGE("Merge: params are invalid");
        return false;
    }

    if (!mParams.frameCount) {
        ALOGE("Merge: params.frameCount is invalid");
        return false;
    }

    /* Only support 16, 24, and 32-bit samples */
    if (mParams.sampleBits != 16 &&
        mParams.sampleBits != 24 &&
        mParams.sampleBits != 32) {
        ALOGE("Merge: unsupported bits/sample");
        return false;
    }

    return true;
}

int Merge::registerStream(const sp<OutStream>& stream)
{
    if (stream == NULL) {
        ALOGE("Merge: stream is invalid, cannot register");
        return -EINVAL;
    }

    const PcmParams &params = stream->getParams();
    const SlotMap &map = stream->getSlotMap();

    ALOGV("Merge: register stream %p src 0x%04x dst 0x%04x",
          stream.get(), map.getSrcMask(), map.getDstMask());

    if (params.sampleBits != mParams.sampleBits) {
        ALOGE("Merge: stream has incompatible sample size");
        return -EINVAL;
    }

    if (!stream->canResample() && (params.sampleRate != mParams.sampleRate)) {
        ALOGE("Merge: stream has incompatible sample rate");
        return -EINVAL;
    }

    if (!stream->canResample() && (params.frameCount != mParams.frameCount)) {
        ALOGE("Merge: stream has incompatible frame count");
        return -EINVAL;
    }

    /*
     * sanity check that defined dest channels fall within the defined number
     * of channels for the output
     */
    if (map.getDstMask() >= (1U << mParams.channels)) {
        ALOGE("Merge: stream's dest mask 0x%x requests channels not present in"
              " the output (%u channel output)",
              map.getDstMask(), mParams.channels);
        return -EINVAL;
    }

    AutoMutex lock(mLock);

    /* check if dst channels overlap with already registered dst channels */
    if (map.getDstMask() & mDstMask) {
        ALOGE("Merge: stream's dst mask overlaps already registered streams");
        return -EINVAL;
    }

    if (mStreams.find(stream) != mStreams.end()) {
        ALOGE("Merge: stream is already registered");
        return -EINVAL;
    }

    if (mStreams.size() == mParams.channels) {
        ALOGE("Merge: max number of streams registered");
        return -ENOMEM;
    }

    mStreams.insert(stream);

    mDstMask |= map.getDstMask();

    return 0;
}

void Merge::unregisterStream(sp<OutStream>& stream)
{
    if (stream == NULL) {
        ALOGE("Merge: stream is invalid, cannot unregister");
        return;
    }

    const SlotMap &map = stream->getSlotMap();

    ALOGV("Merge: unregister stream %p src 0x%04x dst 0x%04x",
          stream.get(), map.getSrcMask(), map.getDstMask());

    AutoMutex lock(mLock);

    if (mStreams.find(stream) != mStreams.end()) {
        mDstMask &= ~map.getDstMask();
        mStreams.erase(stream);
    } else {
        ALOGE("Merge: stream is already un-registered");
    }
}

void Merge::merge(sp<OutStream> stream,
                  BufferProvider::Buffer &inBuffer,
                  BufferProvider::Buffer &outBuffer)
{
    int16_t *in16 = inBuffer.i16;
    int16_t *out16 = outBuffer.i16;
    int32_t *in32 = inBuffer.i32;
    int32_t *out32 = outBuffer.i32;
    uint32_t inOffset = stream->getParams().channels;
    uint32_t outOffset = mParams.channels;

    for (size_t i = 0; i < outBuffer.frameCount && i < inBuffer.frameCount; i++) {
        SlotMap::const_iterator j = stream->getSlotMap().begin();

        if (mContainerBytes == 2) {
            for ( ; j != stream->getSlotMap().end(); ++j)
                out16[j->first] = in16[j->second];

            in16 += inOffset;
            out16 += outOffset;
        } else if (mContainerBytes == 4) {
            for ( ; j != stream->getSlotMap().end(); ++j)
                out32[j->first] = in32[j->second];

            in32 += inOffset;
            out32 += outOffset;
        }
    }
}

int Merge::process(BufferProvider::Buffer &outBuffer)
{
    int ret = 0;

    if (!outBuffer.raw) {
        ALOGE("Merge: cannot process invalid audio buffer");
        return -EINVAL;
    }

    if (outBuffer.frameCount > mParams.frameCount)
        outBuffer.frameCount = mParams.frameCount;

    memset(outBuffer.raw, 0x0,
           outBuffer.frameCount * mParams.channels * mContainerBytes);

    AutoMutex lock(mLock);

    for (StreamSet::iterator i = mStreams.begin(); i != mStreams.end(); ++i) {
        sp<OutStream> stream = (*i).promote();
        BufferProvider::Buffer inBuffer;

        inBuffer.frameCount = outBuffer.frameCount;

        if (stream == 0) {
            ALOGW("Merge: registered stream is no longer valid, skipping");
            continue;
        }

        ret = stream->getNextBuffer(&inBuffer);
        if (ret) {
            ALOGW("Merge: failed to get buffer from stream %d", ret);
            continue;
        }

        if (!outBuffer.frameCount || !inBuffer.frameCount) {
            ALOGE("Merge: cannot merge 0 frame stream");
            /* Set error here, so can be propogated to the user */
            inBuffer.i32 = (int32_t*)(-EINVAL);
            inBuffer.frameCount = 0;
        }
        else if (outBuffer.frameCount != inBuffer.frameCount) {
            ALOGE("Merge: unable to process whole stream, not enough frames."
                  "Expected %d, Received %d. Performing partial process. ",
                  outBuffer.frameCount, inBuffer.frameCount);
            if (outBuffer.frameCount < inBuffer.frameCount) {
                inBuffer.frameCount = outBuffer.frameCount;
            }
        }

        merge(stream, inBuffer, outBuffer);

        stream->releaseBuffer(&inBuffer);
    }

    return 0;
}

/* ---------------------------------------------------------------------------------------- */

UnMerge::UnMerge(const PcmParams &params)
    : mParams(params), mSrcMask(0)
{
    mContainerBytes = mParams.sampleBits / 8;

    /* 24-bit samples (3 bytes) are actually in 32-bit containers (4 bytes) */
    mContainerBytes = (mContainerBytes == 3) ? 4 : mContainerBytes;
}

UnMerge::~UnMerge()
{
    for (StreamSet::iterator i = mStreams.begin(); i != mStreams.end(); ++i) {
        sp<InStream> stream = (*i).promote();
        ALOGW("UnMerge: automatically un-registering stream %p during destroy",
              stream.get());
        unregisterStream(stream);
    }
}

bool UnMerge::initCheck() const
{
    if (!mParams.isValid()) {
        ALOGE("UnMerge: params are invalid");
        return false;
    }

    if (!mParams.frameCount) {
        ALOGE("UnMerge: params.frameCount is invalid");
        return false;
    }

    /* Only support 16, 24, and 32-bit samples */
    if (mParams.sampleBits != 16 &&
        mParams.sampleBits != 24 &&
        mParams.sampleBits != 32) {
        ALOGE("UnMerge: unsupported bits/sample");
        return false;
    }

    return true;
}

int UnMerge::registerStream(const sp<InStream>& stream)
{
    if (stream == NULL) {
        ALOGE("UnMerge: stream is invalid, cannot register");
        return -EINVAL;
    }

    const PcmParams &params = stream->getParams();
    const SlotMap &map = stream->getSlotMap();

    ALOGV("UnMerge: register stream %p src 0x%04x dst 0x%04x",
          stream.get(), map.getSrcMask(), map.getDstMask());

    if (params.sampleBits != mParams.sampleBits) {
        ALOGE("UnMerge: stream has incompatible sample size");
        return -EINVAL;
    }

    if (!stream->canResample() && (params.sampleRate != mParams.sampleRate)) {
        ALOGE("UnMerge: stream has incompatible sample rate");
        return -EINVAL;
    }

    if (!stream->canResample() && (params.frameCount != mParams.frameCount)) {
        ALOGE("UnMerge: stream has incompatible frame count");
        return -EINVAL;
    }

    /*
     * sanity check that defined src channels fall within the defined number
     * of channels for the stream
     */
    if (map.getSrcMask() >= (1U << mParams.channels)) {
        ALOGE("UnMerge: stream's src mask 0x%x requests channels not present in"
              " the input (%u channel input)",
              map.getSrcMask(), mParams.channels);
        return -EINVAL;
    }

    AutoMutex lock(mLock);

    if (mStreams.find(stream) != mStreams.end()) {
        ALOGE("UnMerge: stream is already registered");
        return -EINVAL;
    }

    if (mStreams.size() == mParams.channels) {
        ALOGE("UnMerge: max number of streams registered");
        return -ENOMEM;
    }

    mStreams.insert(stream);

    mSrcMask |= map.getSrcMask();

    return 0;
}

void UnMerge::unregisterStream(sp<InStream>& stream)
{
    if (stream == NULL) {
        ALOGE("UnMerge: stream is invalid, cannot unregister");
        return;
    }

    const SlotMap &map = stream->getSlotMap();

    ALOGV("UnMerge: unregister stream %p src 0x%04x dst 0x%04x",
          stream.get(), map.getSrcMask(), map.getDstMask());

    AutoMutex lock(mLock);

    if (mStreams.find(stream) != mStreams.end()) {
        mSrcMask &= ~map.getSrcMask();
        mStreams.erase(stream);
    } else {
        ALOGE("UnMerge: stream is already un-registered");
    }
}

void UnMerge::unmerge(sp<InStream> stream,
                      BufferProvider::Buffer &inBuffer,
                      BufferProvider::Buffer &outBuffer)
{
    int16_t *in16 = inBuffer.i16;
    int16_t *out16 = outBuffer.i16;
    int32_t *in32 = inBuffer.i32;
    int32_t *out32 = outBuffer.i32;
    uint32_t inOffset = mParams.channels;
    uint32_t outOffset = stream->getParams().channels;

    for (size_t i = 0; i < outBuffer.frameCount && i < inBuffer.frameCount; i++) {
        SlotMap::const_iterator j = stream->getSlotMap().begin();

        if (mContainerBytes == 2) {
            for ( ; j != stream->getSlotMap().end(); ++j)
                out16[j->first] = in16[j->second];

            in16 += inOffset;
            out16 += outOffset;
        } else if (mContainerBytes == 4) {
            for ( ; j != stream->getSlotMap().end(); ++j)
                out32[j->first] = in32[j->second];

            in32 += inOffset;
            out32 += outOffset;
        }
    }
}

int UnMerge::process(BufferProvider::Buffer &inBuffer)
{
    int ret = 0;

    if (!inBuffer.raw) {
        ALOGE("UnMerge: cannot process invalid audio buffer");
        return -EINVAL;
    }

    if (inBuffer.frameCount > mParams.frameCount)
        inBuffer.frameCount = mParams.frameCount;

    AutoMutex lock(mLock);

    for (StreamSet::iterator i = mStreams.begin(); i != mStreams.end(); ++i) {
        sp<InStream> stream = (*i).promote();
        BufferProvider::Buffer outBuffer;

        outBuffer.frameCount = inBuffer.frameCount;

        if (stream == 0) {
            ALOGW("UnMerge: registered stream is no longer valid, skipping");
            continue;
        }

        ret = stream->getNextBuffer(&outBuffer);
        if (ret) {
            ALOGW("UnMerge: failed to get buffer from stream %d", ret);
            continue;
        }

        if (!inBuffer.frameCount || !outBuffer.frameCount) {
            ALOGE("UnMerge: cannot unmerge 0 frames");
            /* Set error here, so can be propogated to the user */
            outBuffer.i32 = (int32_t*)(-EINVAL);
            outBuffer.frameCount = 0;
        }
        else if (inBuffer.frameCount > outBuffer.frameCount) {
            ALOGE("UnMerge: unable to process whole stream, not enough frames."
                  "Expected %d, Received %d. Performing partial process. ",
                  inBuffer.frameCount, outBuffer.frameCount);
            if (inBuffer.frameCount < outBuffer.frameCount) {
                outBuffer.frameCount = inBuffer.frameCount;
            }
        }

        unmerge(stream, inBuffer, outBuffer);

        stream->releaseBuffer(&outBuffer);
    }

    return 0;
}

} /* namespace tiaudioutils */
