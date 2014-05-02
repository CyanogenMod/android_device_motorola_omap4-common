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

/**
 * \file MonoPipe.h
 * \brief Classes for writing and reading to/from a mono pipe
 *
 * Contains the MonoPipe and its helper buffer provider MonoReader and
 * MonoWriter that can be used to seamlessly connect the pipe with
 * stream writers/readers like PcmWriter and PcmReader.
 */

#ifndef _TIAUDIOUTILS_MONOPIPE_H_
#define _TIAUDIOUTILS_MONOPIPE_H_

#include <tiaudioutils/Base.h>
#include <tiaudioutils/Pcm.h>

namespace android {
    class MonoPipe;
    class MonoPipeReader;
}

namespace tiaudioutils {

/**
 * \class MonoPipe
 * \brief Mono pipe
 *
 * Blocking mono pipe, only one reader and one writer are possible, although
 * that constraint is not enforced by the class.
 * The pipe can be accessed directly through read() and write() methods, but
 * it's also possible through the buffer providers PipeWriter and PipeReader.
 * The latter approach allows to connect the providers with PCM streams (e.g.
 * InStream, OutStream) to read and write to PCM ports.
 */
class MonoPipe {
 public:
    /**
     * \brief Mono pipe constructor
     *
     * Constructs a mono pipe for the specified PCM parameters: sample rate,
     * sample size and channels. This mono pipe is implemented on top of
     * NBAIO's mono pipe, so the same limitations apply:
     * - Sample rate: 44100 and 48000
     * - Sample size: 16-bits
     * - Channels: mono and stereo
     *
     * \param params PCM params for the pipe as described above
     * \param frames The size of pipe in frames, forced to be at least to 3x
     *               the frameCount of passed PCM params
     */
    MonoPipe(const PcmParams &params, uint32_t frames = 0);

    /**
     * \brief Mono pipe destructor
     *
     * Destroys the mono pipe and its underlying NBAIO mono pipe.
     */
    virtual ~MonoPipe();

    /**
     * \brief Check the result of constructing the mono pipe
     *
     * Result of constructing the mono pipe. It must be checked before using
     * any methods. Result is undefined if the pipe is used when in an
     * unitialized state.
     *
     * \return true if pipe construction is correct, false otherwise
     */
    virtual bool initCheck() const;

    /**
     * \brief Get the parameters of the pipe
     *
     * Get the native PcmParams used in the mono pipe.
     *
     * \return Reference to the PCM params of the pipe
     */
    const PcmParams& getParams() const { return mParams; }

    /**
     * \brief Read data from the pipe
     *
     * Reads data from the mono pipe. The call blocks if the pipe doesn't
     * have the requested frames, but only for equivalent time to the
     * missing frames.
     *
     * \param buffer Pointer to the destination buffer
     * \param frames Number of frames to be read
     * \return Number of frames read, otherwise negative error code
     */
    virtual int read(void *buffer, uint32_t frames);

    /**
     * \brief Check the result of constructing the mono pipe
     *
     * Writes data to the mono pipe. The call blocks if the pipe is full.
     *
     * \param buffer Pointer to the source buffer
     * \param frames Number of frames to be written
     * \return Number of frames written, otherwise negative error code
     */
    virtual int write(const void *buffer, uint32_t frames);

    /**
     * \brief Query the number of frames in the pipe
     *
     * Queries the number of frames available in the pipe. A subsequent
     * read() call for that many frames should succeed.
     *
     * \return The number of available frames to be read, negative error
     *         code if the pipe is in bad state
     */
    virtual int availableToRead() const;

    /**
     * \brief Query the empty frame locations in the pipe
     *
     * Queries the empty frame locations in the pipe. A subsequent write()
     * call for that many frames should succeed.
     *
     * \return The number of available frames to be write, negative error
     *         code if the pipe is in bad state
     */
    virtual int availableToWrite() const;

    /**
     * \brief Query the size of the pipe
     *
     * Queries the size of the pipe. It might be different to the requested
     * number of frames if the pipe implementation relies does size rounding.
     *
     * \return Size of the pipe (in frames)
     */
    virtual size_t size() const;

    /**
     * \brief Flush the pipe
     *
     * Flushes all frames present in the pipe. It should be called when the
     * pipe is in shutdown state to ensure no new data is written to the
     * pipe.
     *
     * \return 0 on success, otherwise negative error code
     */
    virtual int flush();

    /**
     * \brief Shut down the pipe
     *
     * Shuts down the pipe, causing any blocking write() calls to unblock
     * and return.
     *
     * \param state true if the pipe is to be shutdown, false otherwise
     */
    virtual void shutdown(bool state);

    /**
     *
     * \brief Test if the pipe is shutdown
     *
     * Tests if the pipe is currently shutdown.
     *
     * \return true if the pipe is shutdown, false otherwise
     */
    virtual bool isShutdown();

 protected:
    /** Ratio between pipe frame count and params frame count */
    static const uint32_t kPipeSizeFactor = 3;

    android::MonoPipe *mSink;         /**< Mono pipe */
    android::MonoPipeReader *mSource; /**< Reader for the mono pipe */
    PcmParams mParams;                /**< PCM parameters of the pipe */
};

/**
 * \class PipeReader
 * \brief Mono pipe reader
 *
 * Buffer provider that can be used to read data from the MonoPipe. This
 * provider can be used along with OutStream and PcmWriter to read data
 * from the pipe and write it to a PCM port.
 */
class PipeReader : public BufferProvider {
 public:
    /**
     * \brief Pipe reader constructor
     *
     * Constructs a buffer provider that is capable of reading data from
     * the specified mono pipe.
     *
     * \param pipe The mono pipe to read data from
     */
    PipeReader(MonoPipe *pipe);

    /**
     * \brief Pipe reader destructor
     *
     * Destroys a pipe reader object.
     */
    virtual ~PipeReader();

    /**
     * \brief Check the result of constructing the mono pipe reader
     *
     * Result of constructing the mono pipe reader. It must be checked before
     * using any methods. Result is undefined if the writer is used when in an
     * unitialized state.
     *
     * \return true if pipe construction is correct, false otherwise
     */
    bool initCheck() const { return mPipe != NULL; }

    /**
     * \brief Get the next buffer filled with data from the pipe
     *
     * Gets the next audio buffer filled with PCM data read from the pipe.
     * The buffer can be filled with zeroes if there are no frames
     * available to read in the pipe, see MonoPipe read() for more
     * details.
     *
     * \param buffer Pointer to the buffer to fill with next buffer's info
     * \return 0 on success, otherwise negative error code
     */
    int getNextBuffer(BufferProvider::Buffer *buffer);

    /**
     * \brief Release the buffer from the pipe
     *
     * Release the audio buffer previously obtained through getNextBuffer().
     * It's a logical error to release a buffer not previously obtained,
     * result is undefined.
     *
     * \param buffer Pointer to the buffer to be released
     */
    void releaseBuffer(BufferProvider::Buffer *buffer);

 protected:
    MonoPipe *mPipe;                 /**< The mono pipe used by this writer */
    BufferProvider::Buffer mBuffer;  /**< Buffer with data to be written to the pipe */
};

/**
 * \class PipeWriter
 * \brief Mono pipe writer
 *
 * Buffer provider that can be used to write data to the MonoPipe. This
 * provider can be used along with InStream and PcmReader to read
 * data from a PCM port and write it to the pipe.
 */
class PipeWriter : public BufferProvider {
 public:
    /**
     * \brief Pipe writer constructor
     *
     * Constructs a buffer provider that is capable of writing data to
     * the specified mono pipe.
     *
     * \param pipe The mono pipe to write data to
     */
    PipeWriter(MonoPipe *pipe);

    /**
     * \brief Pipe writer destructor
     *
     * Destroys a pipe writer object.
     */
    virtual ~PipeWriter();

    /**
     * \brief Check the result of constructing the mono pipe writer
     *
     * Result of constructing the mono pipe writer. It must be checked before
     * using any methods. Result is undefined if the writer is used when in an
     * unitialized state.
     *
     * \return true if pipe construction is correct, false otherwise
     */
    bool initCheck() const { return mPipe != NULL; }

    /**
     * \brief Get the next buffer that will be written to the pipe
     *
     * Gets the next audio buffer that can be filled with PCM data for the
     * mono pipe.
     *
     * \param buffer Pointer to the buffer to fill with next buffer's info
     * \return 0 on success, otherwise negative error code
     */
    int getNextBuffer(BufferProvider::Buffer *buffer);

    /**
     * \brief Release the buffer from the pipe
     *
     * Release the audio buffer previously obtained through getNextBuffer().
     * It's a logical error to release a buffer not previously obtained,
     * result is undefined.
     *
     * \param buffer Pointer to the buffer to be released
     */
    void releaseBuffer(BufferProvider::Buffer *buffer);

 protected:
    MonoPipe *mPipe;                 /**< The mono pipe used by this writer */
    BufferProvider::Buffer mBuffer;  /**< Buffer with data to be written to the pipe */
};

} /* namespace tiaudioutils */

#endif /* _TIAUDIOUTILS_MONOPIPE_H_ */
