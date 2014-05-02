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
#include <tiaudioutils/SimpleStream.h>
#include <tiaudioutils/Resampler.h>

namespace tiaudioutils {

SimpleInStream::SimpleInStream(const PcmParams &params)
    : mParams(params), mReader(NULL), mStarted(false)
{
}

SimpleInStream::~SimpleInStream()
{
    if (mReader) {
        ALOGW("SimpleInStream: automatically unregistered");
        mReader->unregisterStream(this);
    }
}

bool SimpleInStream::initCheck() const
{
    if (!mParams.isValid()) {
        ALOGE("SimpleInStream: params are invalid");
        return false;
    }

    if (!mParams.frameCount) {
        ALOGE("SimpleInStream: params.frameCount is invalid");
        return false;
    }

    return true;
}

int SimpleInStream::start()
{
    if (!mReader) {
        ALOGE("SimpleInStream: not registered to reader, cannot start");
        return -EINVAL;
    }

    AutoMutex lock(mLock);
    if (mStarted) {
        ALOGE("SimpleInStream: stream is already started");
        return -EBUSY;
    }

    int ret = mReader->open();
    if (!ret)
        mStarted = true;
    else
        ALOGE("SimpleInStream: failed to open %d", ret);

    return ret;
}

void SimpleInStream::stop()
{
    if (!mReader) {
        ALOGE("SimpleInStream: not registered to reader, cannot stop");
        return;
    }

    AutoMutex lock(mLock);
    if (!mStarted) {
        ALOGE("SimpleInStream: stream is already stopped");
        return;
    }

    mStarted = false;
    mReader->close();
}

bool SimpleInStream::isStarted() const
{
    AutoMutex lock(mLock);
    return mStarted;
}

int SimpleInStream::read(void *buffer, size_t frames)
{
    if (!mReader) {
        ALOGE("SimpleInStream: not registered to reader, cannot read");
        return -EINVAL;
    }

    AutoMutex lock(mLock);
    if (!mStarted) {
        ALOGE("SimpleInStream: stream is not started, cannot read");
        return -EPERM;
    }

    return mReader->read(buffer, frames);
}

/* ---------------------------------------------------------------------------------------- */

SimpleOutStream::SimpleOutStream(const PcmParams &params)
    : mParams(params), mWriter(NULL), mStarted(false)
{
}

SimpleOutStream::~SimpleOutStream()
{
    if (mWriter) {
        ALOGW("SimpleOutStream: automatically unregistered");
        mWriter->unregisterStream(this);
    }
}

bool SimpleOutStream::initCheck() const
{
    if (!mParams.isValid()) {
        ALOGE("SimpleOutStream: params are invalid");
        return false;
    }

    if (!mParams.frameCount) {
        ALOGE("SimpleOutStream: params.frameCount is invalid");
        return false;
    }

    return true;
}

int SimpleOutStream::start()
{
    if (!mWriter) {
        ALOGE("SimpleOutStream: not registered to writer, cannot start");
        return -EINVAL;
    }

    AutoMutex lock(mLock);
    if (mStarted) {
        ALOGE("SimpleOutStream: stream is already started");
        return -EBUSY;
    }

    int ret = mWriter->open();
    if (!ret)
        mStarted = true;
    else
        ALOGE("SimpleOutStream: failed to open %d", ret);

    return ret;
}

void SimpleOutStream::stop()
{
    if (!mWriter) {
        ALOGE("SimpleOutStream: not registered to writer, cannot stop");
        return;
    }

    AutoMutex lock(mLock);
    if (!mStarted) {
        ALOGE("SimpleOutStream: stream is already stopped");
        return;
    }

    mStarted = false;
    mWriter->close();
}

bool SimpleOutStream::isStarted() const
{
    AutoMutex lock(mLock);
    return mStarted;
}

int SimpleOutStream::write(const void *buffer, size_t frames)
{
    if (!mWriter) {
        ALOGE("SimpleOutStream: not registered to writer, cannot write");
        return -EINVAL;
    }

    AutoMutex lock(mLock);
    if (!mStarted) {
        ALOGE("SimpleOutStream: stream is not started, cannot write");
        return -EPERM;
    }

    return mWriter->write(buffer, frames);
}

/* ---------------------------------------------------------------------------------------- */

SimpleReader::SimpleReader(PcmInPort *port, const PcmParams &params)
    : mPort(port),
      mParams(params),
      mStream(NULL),
      mResampler(NULL),
      mBufferProvider(*this)
{
    /* Intermediate buffer for the frames to be resampled */
    mBuffer.frameCount = mParams.frameCount;
    mBuffer.i8 = new int8_t[mParams.bufferSize()];
}

SimpleReader::~SimpleReader()
{
    if (mBuffer.i8)
        delete [] mBuffer.i8;
}

bool SimpleReader::initCheck() const
{
    if (mPort == NULL) {
        ALOGE("SimpleReader: invalid PCM input port");
        return false;
    }

    if (!mParams.isValid() || !mParams.frameCount) {
        ALOGE("SimpleReader: params are not valid");
        return false;
    }

    if (mBuffer.raw == NULL) {
        ALOGE("SimpleReader: intermediate buffer allocation failed");
        return false;
    }

    return true;
}

int SimpleReader::registerStream(SimpleInStream *stream)
{
    if (!stream) {
        ALOGE("SimpleReader: stream is invalid, cannot register");
        return -EINVAL;
    }

    const PcmParams &outParams = stream->getParams();
    if (!outParams.isValid() || !outParams.frameCount) {
        ALOGE("SimpleReader: stream has invalid params");
        return -EINVAL;
    }

    ALOGI("SimpleReader: register stream %p", stream);

    AutoMutex lock(mLock);
    if (mStream) {
        ALOGE("SimpleReader: reader allows only one stream");
        return -ENOTSUP;
    }

    /*
     * Hardware parameters are not known when the stream is created. It's until
     * stream registration that we have enough information to determine if the
     * stream needs resampling, and if so, what the resampling parameters are.
     * Speex-based resampler is supported only for 16-bits/sample.
     */
    if ((mParams.sampleRate != outParams.sampleRate) && (outParams.sampleBits == 16)) {
        mResampler = new Resampler(mParams, outParams);
        if (!mResampler) {
            ALOGE("SimpleReader: failed to create resampler");
            return -ENODEV;
        }
        if (!mResampler->initCheck()) {
            ALOGE("SimpleReader: failed to initialize resampler");
            delete mResampler;
            mResampler = NULL;
            return -EINVAL;
        }
    } else if ((mParams.sampleBits != outParams.sampleBits) ||
               (mParams.channels != outParams.channels) ||
               (mParams.sampleRate != outParams.sampleRate)) {
        ALOGE("SimpleReader: reader doesn't support stream's params");
        return -EINVAL;
    }

    stream->mReader = this;
    mStream = stream;

    return 0;
}

void SimpleReader::unregisterStream(SimpleInStream *stream)
{
    if (!stream) {
        ALOGE("SimpleReader: stream is invalid, cannot unregister");
        return;
    }

    if (stream->isStarted()) {
        ALOGE("SimpleReader: stream %p is not stopped, cannot unregister", stream);
        return;
    }

    ALOGI("SimpleReader: unregister stream %p", stream);

    AutoMutex lock(mLock);
    if (mStream != stream) {
        ALOGE("SimpleReader: stream is already unregistered");
        return;
    }

    if (mResampler) {
        delete mResampler;
        mResampler = NULL;
    }

    stream->mReader = NULL;
    mStream = NULL;
}

bool SimpleReader::isStreamRegistered(SimpleInStream *stream)
{
    AutoMutex lock(mLock);
    return (mStream == stream);
}

int SimpleReader::open()
{
    ALOGI("SimpleReader: open PCM port");

    AutoMutex lock(mLock);
    if (mPort->isOpen()) {
        ALOGW("SimpleReader: port is already open");
        return 0;
    }

    int ret = mPort->open(mParams);
    if (ret)
        ALOGE("SimpleReader: failed to open PCM port %d", ret);

    return ret;
}

void SimpleReader::close()
{
    ALOGI("SimpleReader: close PCM port");

    AutoMutex lock(mLock);
    if (mPort->isOpen())
        mPort->close();
    else
        ALOGW("SimpleReader: PCM port is already closed");
}


int SimpleReader::read(void *buffer, size_t frames)
{
    AutoMutex lock(mLock);
    int ret;

    if (!mPort->isOpen()) {
        ALOGE("SimpleReader: PCM port is not open");
        return -ENODEV;
    }

    if (mResampler) {
        ret = mResampler->resample(mBufferProvider,
                                   buffer, frames);
        if (ret) {
            ALOGE("SimpleReader: failed to resample %d", ret);
            return ret;
        }
        ret = frames;
    } else {
        ret = mPort->read(buffer, frames);
        if (ret < 0)
            ALOGE("SimpleReader: failed to read PCM data %d", ret);
    }

    return ret;
}

/* ---------------------------------------------------------------------------------------- */

int SimpleReader::ReadProvider::getNextBuffer(BufferProvider::Buffer *buffer)
{
    int ret = 0;

    /* resize buffer if needed */
    if (buffer->frameCount > mReader.mBuffer.frameCount) {
        delete [] mReader.mBuffer.i8;
        mReader.mBuffer.i8 = new int8_t[buffer->frameCount * mReader.mParams.frameSize()];
        if (mReader.mBuffer.i8) {
            ALOGE("SimpleReader: failed to resize internal buffer");
            buffer->frameCount = 0;
            return -ENOMEM;
        }
        mReader.mBuffer.frameCount = buffer->frameCount;
    }

    int8_t *curBuffer = mReader.mBuffer.i8;
    uint32_t pending = buffer->frameCount;
    while (pending > 0) {
        int read = mReader.mPort->read(curBuffer, pending);
        if (read < 0) {
            ALOGE("SimpleReader: failed to read PCM data %d", read);
            break;
        }
        pending -= read;
        curBuffer += read * mReader.mParams.frameSize();
    }

    ALOGW_IF(pending, "SimpleReader: could not read %d frames", pending);

    buffer->raw = mReader.mBuffer.raw;
    buffer->frameCount -= pending;

    if (!buffer->frameCount)
        ret = -EAGAIN;

    return ret;
}

void SimpleReader::ReadProvider::releaseBuffer(BufferProvider::Buffer *buffer)
{
    /* Nothing to do to release the buffer, but must be implemented */
}

/* ---------------------------------------------------------------------------------------- */

SimpleWriter::SimpleWriter(PcmOutPort *port, const PcmParams &params)
    : mPort(port),
      mParams(params),
      mStream(NULL),
      mResampler(NULL)
{
    /* Intermediate buffer for the resampled frames */
    mBuffer.frameCount = mParams.frameCount;
    mBuffer.i8 = new int8_t[mParams.bufferSize()];
}

SimpleWriter::~SimpleWriter()
{
    if (mBuffer.i8)
        delete [] mBuffer.i8;
}

bool SimpleWriter::initCheck() const
{
    if (mPort == NULL) {
        ALOGE("SimpleWriter: invalid PCM output port");
        return false;
    }

    if (!mParams.isValid() || !mParams.frameCount) {
        ALOGE("SimpleWriter: params are not valid");
        return false;
    }

    if (mBuffer.raw == NULL) {
        ALOGE("SimpleWriter: intermediate buffer allocation failed");
        return false;
    }

    return true;
}

int SimpleWriter::registerStream(SimpleOutStream *stream)
{
    if (!stream) {
        ALOGE("SimpleWriter: stream is invalid, cannot register");
        return -EINVAL;
    }

    const PcmParams &inParams = stream->getParams();
    if (!inParams.isValid() || !inParams.frameCount) {
        ALOGE("SimpleWriter: stream has invalid params");
        return -EINVAL;
    }

    ALOGI("SimpleWriter: register stream %p", stream);

    AutoMutex lock(mLock);
    if (mStream) {
        ALOGE("SimpleWriter: writer allows only one stream");
        return -ENOTSUP;
    }

    /*
     * Hardware parameters are not known when the stream is created. It's until
     * stream registration that we have enough information to determine if the
     * stream needs resampling, and if so, what the resampling parameters are.
     * Speex-based resampler is supported only for 16-bits/sample.
     */
    if ((mParams.sampleRate != inParams.sampleRate) && (inParams.sampleBits == 16)) {
        mResampler = new Resampler(inParams, mParams);
        if (!mResampler) {
            ALOGE("SimpleWriter: failed to create resampler");
            return -ENODEV;
        }
        if (!mResampler->initCheck()) {
            ALOGE("SimpleWriter: failed to initialize resampler");
            delete mResampler;
            mResampler = NULL;
            return -EINVAL;
        }
    } else if ((mParams.sampleBits != inParams.sampleBits) ||
               (mParams.channels != inParams.channels) ||
               (mParams.sampleRate != inParams.sampleRate)) {
        ALOGE("SimpleWriter: writer doesn't support stream's params");
        return -EINVAL;
    }

    stream->mWriter = this;
    mStream = stream;

    return 0;
}

void SimpleWriter::unregisterStream(SimpleOutStream *stream)
{
    if (!stream) {
        ALOGE("SimpleWriter: stream is invalid, cannot unregister");
        return;
    }

    if (stream->isStarted()) {
        ALOGE("SimpleWriter: stream %p is not stopped, cannot unregister", stream);
        return;
    }

    ALOGI("SimpleWriter: unregister stream %p", stream);

    AutoMutex lock(mLock);
    if (mStream != stream) {
        ALOGE("SimpleWriter: stream is already unregistered");
        return;
    }

    if (mResampler) {
        delete mResampler;
        mResampler = NULL;
    }

    stream->mWriter = NULL;
    mStream = NULL;
}

bool SimpleWriter::isStreamRegistered(SimpleOutStream *stream)
{
    AutoMutex lock(mLock);
    return (mStream == stream);
}

int SimpleWriter::open()
{
    ALOGI("SimpleWriter: open PCM port");

    AutoMutex lock(mLock);
    if (mPort->isOpen()) {
        ALOGW("SimpleWriter: port is already open");
        return 0;
    }

    int ret = mPort->open(mParams);
    if (ret)
        ALOGE("SimpleWriter: failed to open PCM port %d", ret);

    return ret;
}

void SimpleWriter::close()
{
    ALOGI("SimpleWriter: close PCM port");

    AutoMutex lock(mLock);
    if (mPort->isOpen())
        mPort->close();
    else
        ALOGW("SimpleWriter: PCM port is already closed");
}


int SimpleWriter::write(const void *buffer, size_t frames)
{
    AutoMutex lock(mLock);

    if (!mPort->isOpen()) {
        ALOGE("SimpleWriter: PCM port is not open");
        return -ENODEV;
    }

    const void *outBuffer;
    uint32_t outFrames;
    int ret;

    if (mResampler) {
        outBuffer = mBuffer.raw;
        outFrames = mBuffer.frameCount;
        ret = mResampler->resample(buffer, frames, mBuffer.raw, outFrames);
        if (ret) {
            ALOGE("SimpleWriter: failed to resample %d", ret);
            return ret;
        }
    } else {
        outBuffer = buffer;
        outFrames = frames;
    }

    ret = mPort->write(outBuffer, outFrames);
    if (ret < 0) {
        ALOGE("SimpleWriter: failed to write PCM data %d", ret);
        return ret;
    }

    return frames;
}

} /* namespace tiaudioutils */
