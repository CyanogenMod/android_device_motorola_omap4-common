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
#include <tiaudioutils/Stream.h>
#include <tiaudioutils/Resampler.h>

namespace tiaudioutils {

BufferAdaptor::BufferAdaptor()
    : mStatus(0)
{
    sem_init(&mFill, 0, 0);
    sem_init(&mEmpty, 0, 1);
    sem_init(&mDone, 0, 0);
    mUnderRuns = 0;
}

BufferAdaptor::~BufferAdaptor()
{
    sem_destroy(&mFill);
    sem_destroy(&mEmpty);
    sem_destroy(&mDone);
}

int BufferAdaptor::read(void * const buffer, size_t frames)
{
    sem_wait(&mEmpty);

    mMutex.lock();
    mBuffer = buffer;
    mFrames = frames;
    mMutex.unlock();

    sem_post(&mFill);

    sem_wait(&mDone);

    return mStatus;
}

int BufferAdaptor::write(const void *buffer, size_t frames)
{
    sem_wait(&mEmpty);

    mMutex.lock();
    mBuffer = buffer;
    mFrames = frames;
    mMutex.unlock();

    sem_post(&mFill);

    sem_wait(&mDone);

    return mStatus;
}

int BufferAdaptor::getNextBuffer(BufferProvider::Buffer* buffer)
{
    timespec ts;

    /*
     * Set a 200ms timeout for the semaphore wait call. Waiting indefinitely
     * may result in a deadlock condition, and waiting for too short a period
     * can result in glitches in the audio. A value is chosen that is large
     * enough to give sufficient time to receive the next read/write call and
     * yet not so large as to noticably block the user in case of error. A
     * timeout of 200ms should be sufficient to receive the next audio buffer
     * during normal playback and record.
     */
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += (200 * 1000000); /* 200ms timeout */
    if (ts.tv_nsec > 1000000000) { /* check for wraparound */
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }

    int ret = sem_timedwait(&mFill, &ts);

    mMutex.lock();
    if (ret) {
        ALOGW("getNextBuffer: failed to get buffer [%s]", strerror(errno));
        buffer->raw = NULL;
        buffer->frameCount = 0;
        mStatus = ret;
        mUnderRuns++;
    }
    else {
        buffer->raw = const_cast<void *>(mBuffer);
        if (buffer->frameCount > mFrames)
            buffer->frameCount = mFrames;
    }
    mMutex.unlock();

    return ret;
}

void BufferAdaptor::releaseBuffer(BufferProvider::Buffer* buffer)
{
    sem_post(&mEmpty);

    /*
     * Pass the number of read/written frames in case of success,
     * or the error code otherwise.
     *
     * NOTE: The rule is that the buffer's frame count doesn't change
     * between getNextBuffer() and releaseBuffer().
     * The exception to that rule is when the frame count is not known
     * at the time the buffer is acquired.
     * Specifically, that's the case of resampling with fractional ratio
     * where the "actual" resampled frame count might vary from one
     * iteration to another. So, if the resampler produced less frames
     * than we expected (and got the buffer for), simply update the amount
     * of frames here.
     * This is not fair for all providers (particularly write providers)
     * that would otherwise have to "rewind" to update the actual number
     * of frames. Read providers only provide a new empty buffer to be filled,
     * so the "rewind" operation has no side effect.
     */
    if (buffer->frameCount)
        mStatus = buffer->frameCount;
    else
        mStatus = (int)buffer->i32;

    sem_post(&mDone);
}

/* ---------------------------------------------------------------------------------------- */

InStream::InStream(const PcmParams &params,
                   BufferProvider *provider)
    : mBufferProvider(provider),
      mParams(params),
      mMap((1U << params.channels) - 1),
      mReader(NULL),
      mResampler(NULL),
      mStarted(false)
{
}

InStream::InStream(const PcmParams &params,
                   const SlotMap &map,
                   BufferProvider *provider)
    : mBufferProvider(provider),
      mParams(params),
      mMap(map),
      mReader(NULL),
      mResampler(NULL),
      mStarted(false)
{
}

InStream::~InStream()
{
    if (mReader) {
        ALOGW("InStream: automatically unregistered");
        if (isStarted())
            stop();
        sp<InStream> stream = this;
        mReader->unregisterStream(stream);
    }
}

bool InStream::initCheck() const
{
    if (mBufferProvider == NULL) {
        ALOGE("InStream: buffer provider is invalid");
        return false;
    }

    if (!mMap.isValid()) {
        ALOGE("InStream: slot map is invalid");
        return false;
    }

    if (!mParams.isValid()) {
        ALOGE("InStream: params are invalid");
        return false;
    }

    if (!mParams.frameCount) {
        ALOGE("InStream: params.frameCount is invalid");
        return false;
    }

    if (mMap.getDstMask() >= (1U << mParams.channels)) {
        ALOGE("InStream: dest mask is invalid (too many channels %u)",
              mMap.getChannelCount());
        return false;
    }

    return true;
}

int InStream::start()
{
    if (!mReader) {
        ALOGE("InStream: not registered to reader, cannot start");
        return -EINVAL;
    }

    AutoMutex lock(mLock);
    if (mStarted) {
        ALOGE("InStream: stream is already started");
        return -EBUSY;
    }

    int ret = mReader->open();
    if (!ret)
        mStarted = true;
    else
        ALOGE("InStream: failed to open %d", ret);

    return ret;
}

void InStream::stop()
{
    if (!mReader) {
        ALOGE("InStream: not registered to reader, cannot stop");
        return;
    }

    AutoMutex lock(mLock);
    if (!mStarted) {
        ALOGE("InStream: stream is already stopped");
        return;
    }

    mStarted = false;
    mReader->close();
}

bool InStream::isStarted() const
{
    AutoMutex lock(mLock);
    return mStarted;
}

int InStream::read(void *buffer, size_t frames)
{
    ALOGE("InStream: not supported by the stream type");
    return -ENOTSUP;
}

int InStream::getNextBuffer(BufferProvider::Buffer *buffer)
{
    int ret;

    if (canResample())
        ret = getNextBufferForResample(buffer);
    else
        ret = mBufferProvider->getNextBuffer(buffer);

    return ret;
}

void InStream::releaseBuffer(BufferProvider::Buffer *buffer)
{
    if (canResample())
        releaseResampledBuffer(buffer);
    else
        mBufferProvider->releaseBuffer(buffer);
}

int InStream::getNextBufferForResample(BufferProvider::Buffer *buffer)
{
    /*
     * The buffer requested to this method is in the native sample rate
     * (at which PcmReader or UnMerge operate). We pass our internal
     * temporary buffer from which we resample after it has been filled up.
     */
    buffer->raw = mRsmpBuffer.raw;
    buffer->frameCount = mRsmpBuffer.frameCount;

    return 0;
}

void InStream::releaseResampledBuffer(BufferProvider::Buffer *buffer)
{
    BufferProvider::Buffer outBuffer;
    int8_t *curBuffer = buffer->i8;
    int32_t pending = buffer->frameCount;
    uint32_t ratioNum;
    uint32_t ratioDen;

    mResampler->getRatio(ratioNum, ratioDen);

    /* resample all frames in the released output buffer */
    while (pending > 0) {
        /*
         * Request a buffer to our internal provider, whose size is proportional
         * to the resampling ratio.
         */
        outBuffer.frameCount = (pending * ratioDen) / ratioNum + 1;
        int ret = mBufferProvider->getNextBuffer(&outBuffer);
        if (ret) {
            ALOGE("InStream: failed to get buffer from provider %d", ret);
            return;
        }

        uint32_t inFrames = pending;
        uint32_t outFrames = outBuffer.frameCount;
        ret = mResampler->resample(curBuffer, inFrames, outBuffer.raw, outFrames);
        if (ret) {
            ALOGW("InStream: failed to resample %d", ret);
            outBuffer.i32 = (int32_t*)ret;
            outBuffer.frameCount = 0;
        } else {
            /*
             * NOTE: Consciously change the frame count of the acquired buffer.
             * This is an exception for read providers, otherwise we would have
             * to resample in a temporary buffer and once the "produced" number
             * of samples is known, acquire the buffer and copy the audio frames.
             */
            outBuffer.frameCount = outFrames;
            pending -= inFrames;
            curBuffer += mParams.framesToBytes(inFrames);
        }

        mBufferProvider->releaseBuffer(&outBuffer);
    }

    ALOGW_IF(pending, "InStream: unexpected pending frame count %d", pending);
}

/* ---------------------------------------------------------------------------------------- */

AdaptedInStream::AdaptedInStream(const PcmParams &params)
    : InStream(params, &mAdaptor)
{
}

AdaptedInStream::AdaptedInStream(const PcmParams &params,
                                 const SlotMap &map)
    : InStream(params, map, &mAdaptor)
{
}

int AdaptedInStream::read(void *buffer, size_t frames)
{
    AutoMutex lock(mLock);
    if (!mStarted) {
        ALOGE("AdaptedInStream: stream is not started, cannot read");
        return -EPERM;
    }

    return mAdaptor.read(buffer, frames);
}

/* ---------------------------------------------------------------------------------------- */

BufferedInStream::BufferedInStream(const PcmParams &params,
                                   uint32_t frames)
    : InStream(params), mPipe(params, frames), mPipeWriter(&mPipe)
{
    mBufferProvider = &mPipeWriter;
}

BufferedInStream::BufferedInStream(const PcmParams &params,
                                   const SlotMap &map,
                                   uint32_t frames)
    : InStream(params, map), mPipe(params, frames), mPipeWriter(&mPipe)
{
    mBufferProvider = &mPipeWriter;
}

bool BufferedInStream::initCheck() const
{
    return (InStream::initCheck() &&
            mPipe.initCheck() &&
            mPipeWriter.initCheck());
}

int BufferedInStream::read(void *buffer, size_t frames)
{
    AutoMutex lock(mLock);
    if (!mStarted) {
        ALOGE("BufferedInStream: stream is not started, cannot read");
        return -EPERM;
    }

    return mPipe.read(buffer, frames);
}

/* ---------------------------------------------------------------------------------------- */

OutStream::OutStream(const PcmParams &params,
                     BufferProvider *provider)
    : mBufferProvider(provider),
      mParams(params),
      mMap((1U << params.channels) - 1),
      mWriter(NULL),
      mResampler(NULL),
      mStarted(false)
{
}

OutStream::OutStream(const PcmParams &params,
                     const SlotMap &map,
                     BufferProvider *provider)
    : mBufferProvider(provider),
      mParams(params),
      mMap(map),
      mWriter(NULL),
      mResampler(NULL),
      mStarted(false)
{
}

OutStream::~OutStream()
{
    if (mWriter) {
        ALOGW("OutStream: automatically unregistered");
        if (isStarted())
            stop();
        sp<OutStream> stream = this;
        mWriter->unregisterStream(stream);
    }
}

bool OutStream::initCheck() const
{
    if (mBufferProvider == NULL) {
        ALOGE("OutStream: buffer provider is invalid");
        return false;
    }

    if (!mMap.isValid()) {
        ALOGE("OutStream: slot map is invalid");
        return false;
    }

    if (!mParams.isValid()) {
        ALOGE("OutStream: params are invalid");
        return false;
    }

    if (!mParams.frameCount) {
        ALOGE("OutStream: params.frameCount is invalid");
        return false;
    }

    if (mMap.getSrcMask() >= (1U << mParams.channels)) {
        ALOGE("OutStream: source mask is invalid (too many channels %u)",
              mMap.getChannelCount());
        return false;
    }

    return true;
}

int OutStream::start()
{
    if (!mWriter) {
        ALOGE("OutStream: not registered to writer, cannot start");
        return -EINVAL;
    }

    AutoMutex lock(mLock);
    if (mStarted) {
        ALOGE("OutStream: stream is already started");
        return -EBUSY;
    }

    int ret = mWriter->open();
    if (!ret)
        mStarted = true;
    else
        ALOGE("OutStream: failed to open %d", ret);

    return ret;
}

void OutStream::stop()
{
    if (!mWriter) {
        ALOGE("OutStream: not registered to writer, cannot stop");
        return;
    }

    AutoMutex lock(mLock);
    if (!mStarted) {
        ALOGE("OutStream: stream is already stopped");
        return;
    }

    mStarted = false;
    mWriter->close();
}

bool OutStream::isStarted() const
{
    AutoMutex lock(mLock);
    return mStarted;
}

int OutStream::write(const void *buffer, size_t frames)
{
    ALOGE("OutStream: not supported by the stream type");
    return -ENOTSUP;
}

int OutStream::getNextBuffer(BufferProvider::Buffer *buffer)
{
    int ret;

    if (canResample())
        ret = getNextResampledBuffer(buffer);
    else
        ret = mBufferProvider->getNextBuffer(buffer);

    return ret;
}

void OutStream::releaseBuffer(BufferProvider::Buffer *buffer)
{
    if (!canResample())
        mBufferProvider->releaseBuffer(buffer);
}

int OutStream::getNextResampledBuffer(BufferProvider::Buffer *buffer)
{
    int ret;

    if (buffer->frameCount > mRsmpBuffer.frameCount)
        buffer->frameCount = mRsmpBuffer.frameCount;

    ret = mResampler->resample(*mBufferProvider,
                               mRsmpBuffer.raw,
                               mRsmpBuffer.frameCount);
    if (ret) {
        ALOGE("OutStream: failed to resample %d", ret);
        return ret;
    }

    buffer->raw = mRsmpBuffer.raw;
    buffer->frameCount = mRsmpBuffer.frameCount;

    return ret;
}

/* ---------------------------------------------------------------------------------------- */

AdaptedOutStream::AdaptedOutStream(const PcmParams &params)
    : OutStream(params, &mAdaptor)
{
}

AdaptedOutStream::AdaptedOutStream(const PcmParams &params,
                                   const SlotMap &map)
    : OutStream(params, map, &mAdaptor)
{
}

int AdaptedOutStream::write(const void *buffer, size_t frames)
{
    AutoMutex lock(mLock);
    if (!mStarted) {
        ALOGE("AdaptedOutStream: stream is not started, cannot write");
        return -EPERM;
    }

    return mAdaptor.write(buffer, frames);
}

/* ---------------------------------------------------------------------------------------- */

BufferedOutStream::BufferedOutStream(const PcmParams &params,
                                     uint32_t frames)
    : OutStream(params), mPipe(params, frames), mPipeReader(&mPipe)
{
    mBufferProvider = &mPipeReader;
}

BufferedOutStream::BufferedOutStream(const PcmParams &params,
                                     const SlotMap &map,
                                     uint32_t frames)
    : OutStream(params, map), mPipe(params, frames), mPipeReader(&mPipe)
{
    mBufferProvider = &mPipeReader;
}

bool BufferedOutStream::initCheck() const
{
    return (OutStream::initCheck() &&
            mPipe.initCheck() &&
            mPipeReader.initCheck());
}

int BufferedOutStream::write(const void *buffer, size_t frames)
{
    AutoMutex lock(mLock);
    if (!mStarted) {
        ALOGE("BufferedOutStream: stream is not started, cannot write");
        return -EPERM;
    }

    return mPipe.write(buffer, frames);
}

/* ---------------------------------------------------------------------------------------- */

PcmReader::PcmReader(PcmInPort *port, const PcmParams &params)
    : ThreadBase("PcmReader"),
      mPort(port),
      mParams(params),
      mUnMerge(params),
      mUsers(0)
{
    if (mPort)
        setName(string("Reader-") + port->getName());

    mBuffer.i8 = new int8_t[mParams.bufferSize()];
    mBuffer.frameCount = mParams.frameCount;
}

PcmReader::~PcmReader()
{
    for (StreamSet::iterator i = mStreams.begin(); i != mStreams.end(); ++i) {
        sp<InStream> stream = (*i).promote();
        if (stream == 0)
            continue;
        ALOGW("PcmReader: automatically unregistering stream %p", stream.get());
        unregisterStream(stream);
    }

    if (mPort && mPort->isOpen())
        mPort->close();

    if (mBuffer.i8) {
        delete [] mBuffer.i8;
        mBuffer.i8 = NULL;
    }
}

bool PcmReader::initCheck() const
{
    if (mPort == NULL) {
        ALOGE("PcmReader: invalid PCM input port");
        return false;
    }

    if (!mParams.isValid() || !mParams.frameCount) {
        ALOGE("PcmReader: %s: params are not valid", mPort->getName());
        return false;
    }

    if (mBuffer.raw == NULL) {
        ALOGE("PcmReader: %s: intermediate buffer allocation failed", mPort->getName());
        return false;
    }

    if (!mUnMerge.initCheck()) {
        ALOGE("PcmReader: %s: un-merge failed to initialize", mPort->getName());
        return false;
    }

    return true;
}

int PcmReader::registerStream(sp<InStream>& stream)
{
    if (stream == NULL) {
        ALOGE("PcmReader: %s: stream is invalid, cannot register", mPort->getName());
        return -EINVAL;
    }

    const PcmParams &outParams = stream->getParams();
    if (!outParams.isValid() || !outParams.frameCount) {
        ALOGE("PcmReader: %s: stream has invalid params", mPort->getName());
        return -EINVAL;
    }

    ALOGI("PcmReader: %s: register stream %p src 0x%04x dst 0x%04x",
          mPort->getName(), stream.get(),
          stream->mMap.getSrcMask(), stream->mMap.getDstMask());

    AutoMutex lock(mLock);
    if (mStreams.find(stream) != mStreams.end()) {
        ALOGE("PcmReader: %s: stream is already registered", mPort->getName());
        return -EINVAL;
    }

    /*
     * Hardware parameters are not known when the stream is created. It's not
     * until stream registration that we have enough information to determine
     * if the stream needs resampling, and if so, what the resampling parameters
     * are. Speex-based resampler is supported only for 16-bits/sample.
     */
    if ((mParams.sampleRate != outParams.sampleRate) && (outParams.sampleBits == 16)) {
        int ret = attachResampler(stream.get());
        if (ret) {
            ALOGE("PcmReader: %s: failed to create and attached resampler",
                  mPort->getName());
            return ret;
        }
    } else if ((mParams.sampleBits != outParams.sampleBits) ||
               (mParams.sampleRate != outParams.sampleRate)) {
        /* Channels of the reader and stream are different if un-merge is used */
        ALOGE("PcmReader: %s: reader doesn't support stream's params", mPort->getName());
        return -EINVAL;
    }

    int ret = mUnMerge.registerStream(stream);
    if (ret) {
        ALOGE("PcmReader: %s: failed to register stream %d", mPort->getName(), ret);
        detachResampler(stream.get());
        return ret;
    }

    stream->mReader = this;

    mStreams.insert(stream);

    return 0;
}

void PcmReader::unregisterStream(sp<InStream>& stream)
{
    if (stream == NULL) {
        ALOGE("PcmReader: %s: stream is invalid, cannot unregister", mPort->getName());
        return;
    }

    if (stream->isStarted()) {
        ALOGE("PcmReader: %s: stream %p is not stopped, cannot unregister",
              mPort->getName(), stream.get());
        return;
    }

    ALOGI("PcmReader: %s: unregister stream %p src 0x%04x dst 0x%04x",
          mPort->getName(), stream.get(),
          stream->mMap.getSrcMask(), stream->mMap.getDstMask());

    AutoMutex lock(mLock);
    if (mStreams.find(stream) != mStreams.end()) {
        mUnMerge.unregisterStream(stream);
        detachResampler(stream.get());
        stream->mReader = NULL;
        mStreams.erase(stream);
    } else {
        ALOGE("PcmReader: %s: stream is not registered or already unregistered",
              mPort->getName());
    }
}

bool PcmReader::isStreamRegistered(sp<InStream>& stream)
{
    AutoMutex lock(mLock);
    return (mStreams.find(stream) != mStreams.end());
}

int PcmReader::open()
{
    AutoMutex lock(mLock);

    ALOGV("PcmReader: %s: users %d->%d", mPort->getName(), mUsers, mUsers+1);
    if (mUsers++)
        return 0;

    ALOGV("PcmReader: %s: open PCM port and start reader thread", mPort->getName());
    int ret = mPort->open(mParams);
    if (ret) {
        ALOGE("PcmReader: %s: failed to open PCM port %d", mPort->getName(), ret);
        return ret;
    }

    /* Start the reader thread */
    ret = run();
    if (ret)
        ALOGE("PcmReader: %s: failed to start reader thread %d", mPort->getName(), ret);

    return ret;
}

void PcmReader::close()
{
    AutoMutex lock(mLock);

    ALOGV("PcmReader: %s: users %d->%d", mPort->getName(), mUsers, mUsers-1);
    ALOG_ASSERT(mUsers);
    if (--mUsers)
        return;

    ALOGV("PcmReader: %s: stop reader thread and close PCM port", mPort->getName());
    stop();
    mPort->close();
}

int PcmReader::threadFunc()
{
    int ret = mPort->read(mBuffer.raw, mParams.frameCount);
    if (ret < 0) {
        ALOGE("PcmReader: %s: failed to read PCM data %d", mPort->getName(), ret);
        /*
         * Set frameCount to 0, this will result in an error being returned to
         * the buffer adaptor read call after the unmerge process call
         */
        mBuffer.frameCount = 0;
    }
    else {
        if (ret != (int)mBuffer.frameCount) {
            ALOGW("PcmReader: %s: read fewer PCM frames than requested %d",
                  mPort->getName(), ret);
        }
        /* Set the frame count to the actual amount read */
        mBuffer.frameCount = (size_t)ret;
    }

    ret = mUnMerge.process(mBuffer);
    if (ret) {
        ALOGE("PcmReader: %s: unmerge failed %d", mPort->getName(), ret);
    }

    return 0;
}

int PcmReader::attachResampler(InStream *stream)
{
    PcmParams inParams = mParams;
    const PcmParams &outParams = stream->getParams();

    inParams.channels = outParams.channels;

    /* Intermediate buffer for the frames to be resampled */
    stream->mRsmpBuffer.frameCount = inParams.frameCount;
    stream->mRsmpBuffer.i8 = new int8_t[inParams.frameCount * inParams.frameSize()];
    if (!stream->mRsmpBuffer.i8)
        return -ENOMEM;

    stream->mResampler = new Resampler(inParams, outParams);
    if (!stream->mResampler || !stream->mResampler->initCheck()) {
        detachResampler(stream);
        return -ENODEV;
    }

    return 0;
}

void PcmReader::detachResampler(InStream *stream)
{
    if (stream->mResampler) {
        delete stream->mResampler;
        stream->mResampler = NULL;
    }

    if (stream->mRsmpBuffer.i8) {
        delete [] stream->mRsmpBuffer.i8;
        stream->mRsmpBuffer.i8 = NULL;
    }
}

/* ---------------------------------------------------------------------------------------- */

PcmWriter::PcmWriter(PcmOutPort *port, const PcmParams &params)
    : ThreadBase("PcmWriter"),
      mPort(port),
      mParams(params),
      mMerge(params),
      mUsers(0)
{
    if (mPort)
        setName(string("Writer-") + port->getName());

    mBuffer.i8 = new int8_t[mParams.bufferSize()];
    mBuffer.frameCount = mParams.frameCount;
}

PcmWriter::~PcmWriter()
{
    for (StreamSet::iterator i = mStreams.begin(); i != mStreams.end(); ++i) {
        sp<OutStream> stream = (*i).promote();
        if (stream == 0)
            continue;
        ALOGW("PcmWriter: automatically unregistering stream %p", stream.get());
        unregisterStream(stream);
    }

    if (mPort && mPort->isOpen())
        mPort->close();

    if (mBuffer.i8) {
        delete [] mBuffer.i8;
        mBuffer.i8 = NULL;
    }
}

bool PcmWriter::initCheck() const
{
    if (mPort == NULL) {
        ALOGE("PcmWriter: invalid PCM input port");
        return false;
    }

    if (!mParams.isValid() || !mParams.frameCount) {
        ALOGE("PcmWriter: %s: params are not valid", mPort->getName());
        return false;
    }

    if (mBuffer.raw == NULL) {
        ALOGE("PcmWriter: %s: intermediate buffer allocation failed", mPort->getName());
        return false;
    }

    if (!mMerge.initCheck()) {
        ALOGE("PcmWriter: %s: merge failed to initialize", mPort->getName());
        return false;
    }

    return true;
}

int PcmWriter::registerStream(sp<OutStream>& stream)
{
    if (stream == NULL) {
        ALOGE("PcmWriter: %s: stream is invalid, cannot register", mPort->getName());
        return -EINVAL;
    }

    const PcmParams &inParams = stream->getParams();
    if (!inParams.isValid() || !inParams.frameCount) {
        ALOGE("PcmWriter: %s: stream has invalid params", mPort->getName());
        return -EINVAL;
    }

    ALOGI("PcmWriter: %s: register stream %p src 0x%04x dst 0x%04x",
          mPort->getName(), stream.get(),
          stream->mMap.getSrcMask(), stream->mMap.getDstMask());

    AutoMutex lock(mLock);
    if (mStreams.find(stream) != mStreams.end()) {
        ALOGE("PcmWriter: %s: stream is already registered", mPort->getName());
        return -EINVAL;
    }

    /*
     * Hardware parameters are not known when the stream is created. It's not
     * until stream registration that we have enough information to determine if
     * the stream needs resampling, and if so, what the resampling parameters
     * are. Speex-based resampler is supported only for 16-bits/sample.
     */
    if ((mParams.sampleRate != inParams.sampleRate) && (inParams.sampleBits == 16)) {
        int ret = attachResampler(stream.get());
        if (ret) {
            ALOGE("PcmWriter: %s: failed to create and attached resampler",
                  mPort->getName());
            return ret;
        }
    } else if ((mParams.sampleBits != inParams.sampleBits) ||
               (mParams.sampleRate != inParams.sampleRate)) {
        /* Channels of the writer and stream are different if merge is used */
        ALOGE("PcmWriter: %s: writer doesn't support stream's params", mPort->getName());
        return -EINVAL;
    }

    int ret = mMerge.registerStream(stream);
    if (ret) {
        ALOGE("PcmWriter: %s: failed to register stream %d", mPort->getName(), ret);
        detachResampler(stream.get());
        return ret;
    }

    stream->mWriter = this;

    mStreams.insert(stream);

    return 0;
}

void PcmWriter::unregisterStream(sp<OutStream>& stream)
{
    if (stream == NULL) {
        ALOGE("PcmWriter: %s: stream is invalid, cannot unregister", mPort->getName());
        return;
    }

    if (stream->isStarted()) {
        ALOGE("PcmWriter: %s: stream %p is not stopped, cannot unregister",
              mPort->getName(), stream.get());
        return;
    }

    ALOGI("PcmWriter: %s: unregister stream %p src 0x%04x dst 0x%04x",
          mPort->getName(), stream.get(),
          stream->mMap.getSrcMask(), stream->mMap.getDstMask());

    AutoMutex lock(mLock);
    if (mStreams.find(stream) != mStreams.end()) {
        mMerge.unregisterStream(stream);
        detachResampler(stream.get());
        stream->mWriter = NULL;
        mStreams.erase(stream);
    } else {
        ALOGE("PcmWriter: %s: stream is not registered or already unregistered",
              mPort->getName());
    }
}

bool PcmWriter::isStreamRegistered(sp<OutStream>& stream)
{
    AutoMutex lock(mLock);
    return (mStreams.find(stream) != mStreams.end());
}

int PcmWriter::open()
{
    AutoMutex lock(mLock);

    ALOGV("PcmWriter: %s: users %d->%d", mPort->getName(), mUsers, mUsers+1);
    if (mUsers++)
        return 0;

    ALOGV("PcmWriter: %s: open PCM port and start writer thread", mPort->getName());
    int ret = mPort->open(mParams);
    if (ret) {
        ALOGE("PcmWriter: %s: failed to open PCM port %d", mPort->getName(), ret);
        return ret;
    }

    /* Start the writer thread */
    ret = run();
    if (ret)
        ALOGE("PcmWriter: %s: failed to start writer thread %d", mPort->getName(), ret);

    return ret;
}

void PcmWriter::close()
{
    AutoMutex lock(mLock);

    ALOGV("PcmWriter: %s: users %d->%d", mPort->getName(), mUsers, mUsers-1);
    ALOG_ASSERT(mUsers);
    if (--mUsers)
        return;

    ALOGV("PcmWriter: %s: stop writer thread and close PCM port", mPort->getName());
    stop();
    mPort->close();
}

int PcmWriter::threadFunc()
{
    int ret = mMerge.process(mBuffer);
    if (ret) {
        ALOGE("PcmWriter: %s: error processing data from provider %d",
              mPort->getName(), ret);
    }

    ret = mPort->write(mBuffer.raw, mBuffer.frameCount);
    if (ret < 0) {
        ALOGE("PcmWriter: %s: failed to write PCM data %d", mPort->getName(), ret);
        /*
         * mPort->write errors will not propagate to the user through the buffer
         * adaptor. So, perform the sleep here to lessen the number of errors.
         */
        uint32_t usecs = (mBuffer.frameCount * 1000) / mParams.sampleRate;
        usleep(usecs);
    }

    return 0;
}

int PcmWriter::attachResampler(OutStream *stream)
{
    const PcmParams &inParams = stream->getParams();
    PcmParams outParams = mParams;

    outParams.channels = inParams.channels;

    /* Intermediate buffer for the resampled frames */
    stream->mRsmpBuffer.frameCount = outParams.frameCount;
    stream->mRsmpBuffer.i8 = new int8_t[outParams.frameCount * outParams.frameSize()];
    if (!stream->mRsmpBuffer.i8)
        return -ENOMEM;

    stream->mResampler = new Resampler(inParams, outParams);
    if (!stream->mResampler || !stream->mResampler->initCheck()) {
        detachResampler(stream);
        return -ENODEV;
    }

    return 0;
}

void PcmWriter::detachResampler(OutStream *stream)
{
    if (stream->mResampler) {
        delete stream->mResampler;
        stream->mResampler = NULL;
    }

    if (stream->mRsmpBuffer.i8) {
        delete [] stream->mRsmpBuffer.i8;
        stream->mRsmpBuffer.i8 = NULL;
    }
}

} /* namespace tiaudioutils */
