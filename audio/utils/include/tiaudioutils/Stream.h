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
 * \file Stream.h
 * \brief Classes for writing and reading data to/from PCM ports
 *
 * Contains the PcmReader, PcmWriter and their corresponding streams.
 * The reader is capable of unmerging the incoming buffers for the
 * PCM port into multiple streams. Similarly, the writer is capable of
 * merging multiple streams into a single one that goes to the PCM port.
 * The underlying PCM ports are not tied to any
 * specific card or port type, as long as they implement interfaces defined
 * in Pcm.h.
 */

#ifndef _TIAUDIOUTILS_STREAM_H_
#define _TIAUDIOUTILS_STREAM_H_

#include <semaphore.h>
#include <set>

#include <tiaudioutils/Base.h>
#include <tiaudioutils/Pcm.h>
#include <tiaudioutils/MumStream.h>
#include <tiaudioutils/MonoPipe.h>

namespace tiaudioutils {

using std::set;
using android::RefBase;

class PcmWriter;
class PcmReader;
class Resampler;

/**
 * \class BufferAdaptor
 * \brief Buffer Adaptor
 *
 * Single producer, single consumer buffer adaptor that converts push-type
 * calls (write()) to pull-type ones (getNextBuffer(), releaseBuffer()).
 * There is no data copied during the conversion.
 */
class BufferAdaptor : public BufferProvider {
 public:
    /**
     * \brief Buffer adaptor constructor
     *
     * Constructs a single producer, single consumer buffer adaptor.
     */
    BufferAdaptor();

    /**
     * \brief Buffer adaptor destructor
     *
     * Destroys a buffer adaptor object.
     */
    virtual ~BufferAdaptor();

    /**
     * \brief Check the result of constructing a buffer adaptor.
     *
     * Result of constructing a buffer adaptor. It must be checked before
     * using any buffer adaptor methods. Result is undefined otherwise.
     *
     * \return true if buffer adaptor construction is correct, false otherwise
     */
    virtual bool initCheck() const { return true; }

    /**
     * \brief Push side of the read adaptor
     *
     * Audio streams using the push model can use this method as they would
     * normally do to read data from a buffer. The call blocks until the other
     * side of the adaptor calls getNextBuffer() and releaseBuffer().
     *
     * \param buffer Pointer to the audio buffer
     * \param frames Number of frames to be read
     * \return Number of frames read, otherwise negative error code
     */
    virtual int read(void * const buffer, size_t frames);

    /**
     * \brief Push side of the write adaptor
     *
     * Audio streams using the push model can use this method as they would
     * normally do to write data to a buffer. The call blocks until the other
     * side of the adaptor calls getNextBuffer() and releaseBuffer().
     *
     * \param buffer Pointer to the audio buffer
     * \param frames Number of frames to be written
     * \return Number of frames written, otherwise negative error code
     */
    virtual int write(const void *buffer, size_t frames);

    /**
     * \brief Pull side side of the adaptor (acquire part)
     *
     * Audio outputs using the pull model can use this method as they would
     * normally do to get the next available buffer. The call blocks until
     * the other side of the adaptor calls write().
     *
     * \param buffer Pointer to the Buffer to be filled up
     * \return 0 on success, otherwise negative error code
     */
    virtual int getNextBuffer(BufferProvider::Buffer* buffer);

    /**
     * \brief Pull side side of the adaptor (release part)
     *
     * Audio outputs using the pull model can use this method as they would
     * normally do to release a buffer previously acquired through
     * getNextBuffer().
     * The call unblocks write().
     *
     * \param buffer Pointer to the Buffer to be released
     */
    virtual void releaseBuffer(BufferProvider::Buffer* buffer);

    /**
     * \brief Retrieve the number of times that getNextBuffer did not get a
     *        buffer
     *
     * The getNextBuffer call performs a sem_trywait() instead of a sem_wait()
     * in order to avoid blocking forever in case no buffers are given.
     * When sem_trywait() returns without getting a next buffer, the event is
     * recorded. The number of times this happens can be retrieved by using
     * getNumUnderruns().
     */
    virtual uint32_t getNumUnderruns() const { return mUnderRuns; }

 protected:
    const void *mBuffer;  /**< Buffer pointer to be passed pull side */
    size_t mFrames;       /**< Number of frames to be passed to pull side */
    sem_t mFill;          /**< Semaphore to indicate a new buffer has been
                               pushed */
    sem_t mEmpty;         /**< Semaphore to indicate readiness for a new buffer
                               to be pushed */
    sem_t mDone;          /**< Semaphore to indicate the pushed buffer has been
                               processed */
    int mStatus;          /**< Return value for read() / write() */
    Mutex mMutex;         /**< Local mutex protection */
    uint32_t mUnderRuns;  /**< Number of times getNextBuffer( did not get a
                               buffer */
};

/**
 * \class InStream
 * \brief PCM reader stream
 *
 * The stream with PCM data for capture.
 */
class InStream : public RefBase, public BufferProvider {
 public:
    /**
     * \brief PCM input stream constructor from parameters
     *
     * Constructs a PCM stream for the given parameters. Slot map is
     * created based on the number of channels in those parameters,
     * assuming that the N channels are located in the first N slots.
     *
     * \param params PCM params for the stream
     * \param provider The buffer provider associated with the stream.
     */
    InStream(const PcmParams &params,
             BufferProvider *provider = NULL);

    /**
     * \brief PCM input stream constructor with specific slot map
     *
     * Constructs a PCM stream with requested parameters and using a
     * specific slot map.
     *
     * \param params PCM params for the stream
     * \param map Slot map to be used when registered to the reader
     * \param provider The buffer provider associated with the stream.
     */
    InStream(const PcmParams &params,
             const SlotMap &map,
             BufferProvider *provider = NULL);

    /**
     * \brief Destructor for a PCM input stream
     *
     * Destroys a PCM reader stream object.
     */
    virtual ~InStream();

    /**
     * \brief Check the result of constructing an input stream
     *
     * Result of constructing an input stream. It must be checked before
     * using any stream methods. Result is undefined otherwise.
     *
     * \return true if stream construction is correct, false otherwise
     */
    virtual bool initCheck() const;

    /**
     * \brief Get the PcmParams of the stream
     *
     * Get the PcmParams used in this stream.
     *
     * \return Reference to the PCM params of the stream
     */
    const PcmParams& getParams() const { return mParams; }

    /**
     * \brief Get the slot map of the stream
     *
     * Get the SlotMap used in this stream.
     *
     * \return Reference to the slot map of the stream
     */
    const SlotMap& getSlotMap() const { return mMap; }

    /**
     * \brief Test if the stream can resample
     *
     * Tests if this stream is capable of sample rate conversion.
     *
     * \return true if stream can resample, false otherwise
     */
    virtual bool canResample() const { return mResampler != NULL; }

    /**
     * \brief Get the next buffer from the stream
     *
     * Gets the next audio buffer from the buffer provider associated
     * with the stream, it can be a provider set during construction or
     * a buffer adaptor.
     *
     * \param buffer Pointer to the buffer to fill with next buffer's info
     * \return 0 on success, otherwise negative error code
     */
    int getNextBuffer(BufferProvider::Buffer *buffer);

    /**
     * \brief Release the buffer from the stream
     *
     * Release the audio buffer previously obtained through getNextBuffer().
     * It's a logical error to release a buffer not previously obtained,
     * result is undefined.
     *
     * \param buffer Pointer to the buffer to be released
     */
    void releaseBuffer(BufferProvider::Buffer *buffer);

    /**
     * \brief Start the stream
     *
     * Starts the stream for use. If this stream is the first active stream
     * in the PCM reader, the PCM reader will open the underlying port.
     *
     * \return 0 on success, otherwise negative error code
     */
    virtual int start();

    /**
     * \brief Stop the stream
     *
     * Stops the stream. If this stream is the last active stream in the
     * PCM reader, the PCM reader will close the underlying port.
     */
    virtual void stop();

    /**
     * \brief Test if the stream is started
     *
     * Tests whether the stream has been started or not.
     *
     * \return true if the stream is started, false if it's stopped
     */
    virtual bool isStarted() const;

    /**
     * \brief Read data from the stream
     *
     * The base PCM input stream does not support read method.
     *
     * \param buffer Pointer to the destination buffer
     * \param frames Number of frames to be read
     * \return Returns -ENOTSUP
     */
    virtual int read(void *buffer, size_t frames);

    friend class PcmReader;

 private:
    InStream(const InStream &stream);
    InStream& operator=(const InStream &stream);

 protected:
    /**
     * \brief Get next audio buffer to be resampled
     *
     * Gets the next audio buffer to be resampled. This method is used
     * only when the stream has a resampled attached.
     *
     * \param buffer Pointer to the buffer to fill with next buffer's info
     * \return 0 on success, otherwise negative error code
     */
    int getNextBufferForResample(BufferProvider::Buffer *buffer);

    /**
     * \brief Release the buffer with resampled data
     *
     * Release the audio buffer with resampled data. The buffer must be
     * previously obtained through getNextBufferForResample().
     * It's a logical error to release a buffer not previously obtained,
     * result is undefined.
     *
     * \param buffer Pointer to the buffer to be released
     */
    void releaseResampledBuffer(BufferProvider::Buffer *buffer);


    BufferProvider *mBufferProvider;    /**< Buffer provider */
    PcmParams mParams;                  /**< PCM parameters used by this stream */
    SlotMap mMap;                       /**< Slot map for the channels of this stream */
    PcmReader *mReader;                 /**< PCM reader that the stream is registered to */
    Resampler *mResampler;              /**< Resampler set by the reader when stream is registered */
    BufferProvider::Buffer mRsmpBuffer; /**< Intermediate buffer with data to be resampled */
    bool mStarted;                      /**< Whether the stream is started or not */
    mutable Mutex mLock;                /**< Synchronize stream start/stop */
};

/**
 * \class AdaptedInStream
 * \brief PCM reader adapted stream
 *
 * The stream with PCM data for capture. The stream uses a buffer adaptor
 * so it can accept read() calls on the consumer side and seamlessly
 * connect with the PcmReader that expects getNextBuffer() and releaseBuffer().
 */
class AdaptedInStream : public InStream {
 public:
    /**
     * \brief Adapted PCM input stream constructor from parameters
     *
     * Constructs an adapted PCM stream for the given parameters. Slot map
     * is created based on the number of channels in those parameters,
     * assuming that the N channels are located in the first N slots.
     *
     * \param params PCM params for the stream
     */
    AdaptedInStream(const PcmParams &params);

    /**
     * \brief Adapted PCM input stream constructor with specific slot map
     *
     * Constructs an adapted PCM stream with requested parameters and using
     * a specific slot map.
     *
     * \param params PCM params for the stream
     * \param map Slot map to be used when registered to the reader
     */
    AdaptedInStream(const PcmParams &params,
                    const SlotMap &map);

    /**
     * \brief Destructor for an adapted PCM input stream
     *
     * Destroys a PCM reader adapted stream object.
     */
    virtual ~AdaptedInStream() {}

    /**
     * \brief Read data from the stream using a buffer adaptor
     *
     * Reads data from the stream using a buffer adaptor. The owner of
     * the stream can use this method while the PcmReader or UnMerge can
     * use the other side of the adaptor that exposes getNextBuffer()
     * and releaseBuffer() methods.
     *
     * \param buffer Pointer to the destination buffer
     * \param frames Number of frames to be read
     * \return Number of frames read, otherwise negative error code.
     */
    int read(void *buffer, size_t frames);

 protected:
    BufferAdaptor mAdaptor;             /**< Buffer adaptor needed for read() */
};

/**
 * \class BufferedInStream
 * \brief PCM reader buffered stream
 *
 * The stream with PCM data for capture. The stream uses a pipe so it
 * can accept read() calls on the consumer side and seamlessly connect
 * with the PcmReader that expects getNextBuffer() and releaseBuffer().
 */
class BufferedInStream : public InStream {
 public:
    /**
     * \brief Buffered PCM input stream constructor from parameters
     *
     * Constructs a buffered PCM stream for the given parameters. Slot map
     * is created based on the number of channels in those parameters,
     * assuming that the N channels are located in the first N slots.
     *
     * \param params PCM params for the stream
     * \param frames Size of the buffer in frames. If not specified, uses
     *               the MonoPipe default size which is 3x the frame count
     *               in the PCM params
     */
    BufferedInStream(const PcmParams &params,
                     uint32_t frames = 0);

    /**
     * \brief Buffered PCM input stream constructor with specific slot map
     *
     * Constructs a buffered PCM stream with requested parameters and using
     * a specific slot map.
     *
     * \param params PCM params for the stream
     * \param map Slot map to be used when registered to the reader
     * \param frames Size of the buffer in frames. If not specified, uses
     *               the MonoPipe default size which is 3x the frame count
     *               in the PCM params
     */
    BufferedInStream(const PcmParams &params,
                     const SlotMap &map,
                     uint32_t frames = 0);

    /**
     * \brief Destructor for an buffered PCM input stream
     *
     * Destroys a PCM reader buffered stream object.
     */
    virtual ~BufferedInStream() {}

    /**
     * \brief Check the result of constructing an input stream
     *
     * Result of constructing a buffered input stream. It must be checked
     * before using any stream methods. Result is undefined otherwise.
     *
     * \return true if stream construction is correct, false otherwise
     */
    bool initCheck() const;

    /**
     * \brief Read data from the stream using a pipe
     *
     * Reads data from the stream using a mono pipe. The owner of the
     * stream can use this method while the PcmReader or UnMerge can
     * write to the pipe using a PipeWriter.
     *
     * \param buffer Pointer to the destination buffer
     * \param frames Number of frames to be read
     * \return Number of frames read, otherwise negative error code.
     */
    int read(void *buffer, size_t frames);

 protected:
    MonoPipe mPipe;                /**< Mono pipe between producer and consumer */
    PipeWriter mPipeWriter;        /**< BufferProvider used to write to the pipe */
};

/**
 * \class OutStream
 * \brief PCM writer stream
 *
 * The stream with PCM data for playback.
 */
class OutStream : public RefBase, public BufferProvider {
 public:
    /**
     * \brief PCM output stream constructor from parameters
     *
     * Constructs a PCM stream for the given parameters. Slot map is
     * created based on the number of channels in those parameters,
     * assuming that the N channels are located in the first N slots.
     *
     * \param params PCM params for the stream
     * \param provider The buffer provider associated with the stream.
     *                 If none is specified, it will use a BufferAdaptor.
     */
    OutStream(const PcmParams &params,
              BufferProvider *provider = NULL);

    /**
     * \brief PCM output stream constructor with specific slot map
     *
     * Constructs a PCM stream with requested parameters and using a
     * specific slot map.
     *
     * \param params PCM params for the stream
     * \param map Slot map to be used when registered to the writer
     * \param provider The buffer provider associated with the stream.
     *                 If none is specified, it will use a BufferAdaptor.
     */
    OutStream(const PcmParams &params,
              const SlotMap &map,
              BufferProvider *provider = NULL);

    /**
     * \brief Destructor for a PCM output stream
     *
     * Destroys a PCM writer stream object.
     */
    virtual ~OutStream();

    /**
     * \brief Check the result of constructing the stream
     *
     * Result of constructing an OutStream. It must be checked before using
     * any stream methods. Result is undefined otherwise.
     *
     * \return true if stream construction is correct, false otherwise
     */
    virtual bool initCheck() const;

    /**
     * \brief Get the PcmParams of the stream
     *
     * Get the PcmParams used in this stream.
     *
     * \return Reference to the PCM params of the stream
     */
    const PcmParams& getParams() const { return mParams; }

    /**
     * \brief Get the slot map of the stream
     *
     * Get the SlotMap used in this stream.
     *
     * \return Reference to the slot map of the stream
     */
    const SlotMap& getSlotMap() const { return mMap; }

    /**
     * \brief Test if the stream can resample
     *
     * Tests if this stream is capable of sample rate conversion.
     *
     * \return true if stream can resample, false otherwise
     */
    virtual bool canResample() const { return mResampler != NULL; }

    /**
     * \brief Get the next buffer from the stream
     *
     * Gets the next audio buffer from the buffer provider associated
     * with the stream, it can be a provider set during construction or
     * a buffer adaptor.
     *
     * \param buffer Pointer to the buffer to fill with next buffer's info
     * \return 0 on success, otherwise negative error code
     */
    int getNextBuffer(BufferProvider::Buffer *buffer);

    /**
     * \brief Release the buffer from the stream
     *
     * Release the audio buffer previously obtained through getNextBuffer().
     * It's a logical error to release a buffer not previously obtained,
     * result is undefined.
     *
     * \param buffer Pointer to the buffer to be released
     */
    void releaseBuffer(BufferProvider::Buffer *buffer);

    /**
     * \brief Start the stream
     *
     * Starts the stream for use. If this stream is the first active stream
     * in the PCM writer, the PCM writer will open the underlying port.
     *
     * \return 0 on success, otherwise negative error code
     */
    virtual int start();

    /**
     * \brief Stop the stream
     *
     * Stops the stream. If this stream is the last active stream in the
     * PCM writer, the PCM writer will close the underlying port.
     */
    virtual void stop();

    /**
     * \brief Test if the stream is started
     *
     * Tests whether the stream has been started or not.
     *
     * \return true if the stream is started, false if it's stopped
     */
    virtual bool isStarted() const;

    /**
     * \brief Write data to the stream
     *
     * The base PCM output stream does not support write method.
     *
     * \param buffer Pointer to the source buffer
     * \param frames Number of frames to be written
     * \return Returns -ENOTSUP
     */
    virtual int write(const void *buffer, size_t frames);

    friend class PcmWriter;

 private:
    OutStream(const OutStream &stream);
    OutStream& operator=(const OutStream &stream);

 protected:
    /**
     * \brief Get the next buffer from the stream and resample
     *
     * Gets the next audio buffer from the buffer provider associated
     * with the stream and resamples it. Regular releaseBuffer() call
     * can be used to release the resampled buffer.
     *
     * \param buffer Pointer to the buffer to fill with next buffer's info
     * \return 0 on success, otherwise negative error code
     */
    int getNextResampledBuffer(BufferProvider::Buffer *buffer);

    BufferProvider *mBufferProvider;    /**< Buffer provider */
    PcmParams mParams;                  /**< PCM parameters used by this stream */
    SlotMap mMap;                       /**< Slot map for the channels of this stream */
    PcmWriter *mWriter;                 /**< PCM writer that the stream is registered to */
    Resampler *mResampler;              /**< Resampler set by the writer when stream is registered */
    BufferProvider::Buffer mRsmpBuffer; /**< Intermediate buffer with resampled data */
    bool mStarted;                      /**< Whether the stream is started or not */
    mutable Mutex mLock;                /**< Synchronize stream start/stop */
};

/**
 * \class AdaptedOutStream
 * \brief PCM writer adapted stream
 *
 * The stream with PCM data for playback. The stream uses a buffer adaptor
 * so it can accept write() calls on the producer side and seamlessly
 * connect with the PcmWriter that expects getNextBuffer() and releaseBuffer().
 */
class AdaptedOutStream : public OutStream {
 public:
    /**
     * \brief Adapted PCM output stream constructor from parameters
     *
     * Constructs an adapted PCM stream for the given parameters. Slot map
     * is created based on the number of channels in those parameters,
     * assuming that the N channels are located in the first N slots.
     *
     * \param params PCM params for the stream
     */
    AdaptedOutStream(const PcmParams &params);

    /**
     * \brief Adapted PCM output stream constructor with specific slot map
     *
     * Constructs an adapted PCM stream with requested parameters and using
     * a specific slot map.
     *
     * \param params PCM params for the stream
     * \param map Slot map to be used when registered to the writer
     */
    AdaptedOutStream(const PcmParams &params,
                     const SlotMap &map);

    /**
     * \brief Destructor for an adapted PCM output stream
     *
     * Destroys a PCM writer adapted stream object.
     */
    virtual ~AdaptedOutStream() {}

    /**
     * \brief Write data to the stream using a buffer adaptor
     *
     * Writes data to the stream using a buffer adaptor. The owner of
     * the stream can use this method while the PcmWriter or Merge can
     * use the other side of the adaptor that exposes getNextBuffer()
     * and releaseBuffer() methods.
     *
     * \param buffer Pointer to the source buffer
     * \param frames Number of frames to be written
     * \return Number of frames written, otherwise negative error code.
     */
    int write(const void *buffer, size_t frames);

 protected:
    BufferAdaptor mAdaptor;   /**< Buffer adaptor needed for write() */
};

/**
 * \class BufferedOutStream
 * \brief PCM writer buffered stream
 *
 * The stream with PCM data for playback. The stream uses a pipe so it
 * can accept write() calls on the producer side and seamlessly connect
 * with the PcmWriter that expects getNextBuffer() and releaseBuffer().
 */
class BufferedOutStream : public OutStream {
 public:
    /**
     * \brief Buffered PCM output stream constructor from parameters
     *
     * Constructs a buffered PCM stream for the given parameters. Slot map
     * is created based on the number of channels in those parameters,
     * assuming that the N channels are located in the first N slots.
     *
     * \param params PCM params for the stream
     * \param frames Size of the buffer in frames. If not specified, uses
     *               the MonoPipe default size which is 3x the frame count
     *               in the PCM params
     */
    BufferedOutStream(const PcmParams &params,
                      uint32_t frames = 0);

    /**
     * \brief Buffered PCM output stream constructor with specific slot map
     *
     * Constructs a buffered PCM stream with requested parameters and using
     * a specific slot map.
     *
     * \param params PCM params for the stream
     * \param frames Size of the buffer in frames. If not specified, uses
     *               the MonoPipe default size which is 3x the frame count
     *               in the PCM params
     * \param map Slot map to be used when registered to the writer
     */
    BufferedOutStream(const PcmParams &params,
                      const SlotMap &map,
                      uint32_t frames = 0);

    /**
     * \brief Destructor for an buffered PCM output stream
     *
     * Destroys a PCM writer buffered stream object.
     */
    virtual ~BufferedOutStream() {}

    /**
     * \brief Check the result of constructing the stream
     *
     * Result of constructing a BufferedOutStream. It must be checked before
     * using any stream methods. Result is undefined otherwise.
     *
     * \return true if stream construction is correct, false otherwise
     */
    bool initCheck() const;

    /**
     * \brief Write data to the stream using a pipe
     *
     * Writes data to the stream using a mono pipe. The owner of the
     * stream can use this method while the PcmWriter or Merge can
     * read from the pipe using a PipeReader.
     *
     * \param buffer Pointer to the source buffer
     * \param frames Number of frames to be written
     * \return Number of frames written, otherwise negative error code.
     */
    int write(const void *buffer, size_t frames);

 protected:
    MonoPipe mPipe;                /**< Mono pipe between producer and consumer */
    PipeReader mPipeReader;        /**< BufferProvider used to read from the pipe */
};

/**
 * \class PcmReader
 * \brief PCM reader
 *
 * PCM reader with unmerge capability. This reader captures data from the
 * PCM in port, unmerges and produces data for the multiple streams that
 * are registered.
 */
class PcmReader : protected ThreadBase {
 public:

    /**
     * \brief PCM reader constructor
     *
     * Constructs a PCM reader for a given capture port. The reader
     * captures data from the PCM port, unmerges and produces data for the
     * registered streams.
     *
     * \param port Pointer to the PCM port to read from
     * \param params PCM parameters to be used on the port
     */
    PcmReader(PcmInPort *port, const PcmParams &params);

    /**
     * \brief PCM reader destructor
     *
     * Destroys a PCM reader object.
     */
    virtual ~PcmReader();

    /**
     * \brief Check the result of constructing the PCM reader
     *
     * Result of constructing a PCM reader. It must be checked before
     * using any methods. Result is undefined if the PCM reader is
     * used when in an unitialized state.
     *
     * \return true if PCM construction is correct, false otherwise
     */
    virtual bool initCheck() const;

    /**
     * \brief Get the PcmParams of the PCM reader
     *
     * Get the PcmParams used in this PCM reader, that is, the parameters
     * used to open the underlying PCM capture port.
     *
     * \return Reference to the PCM params of the reader
     */
    const PcmParams& getParams() const { return mParams; }

    /**
     * \brief Register a stream
     *
     * Registers a stream to the PCM reader. Multiple streams can be
     * registered to the reader, each one is associated with one or more
     * slots of the input stream that is read from the PCM in port.
     *
     * \param stream PCM stream to be registered
     * \return 0 on success, otherwise negative error code
     */
    virtual int registerStream(sp<InStream>& stream);

    /**
     * \brief Unregister a PCM stream
     *
     * Unregisters a stream from the PCM reader. The stream must have
     * been previously registered using registerStream().
     *
     * \param stream PCM stream to be unregistered
     */
    virtual void unregisterStream(sp<InStream>& stream);

    /**
     * \brief Check if an input stream is registered
     *
     * Tests if an input stream is currently registered with the PCM reader.
     *
     * \return true if the stream is registered, false otherwise
     */
    virtual bool isStreamRegistered(sp<InStream>& stream);

    friend class InStream;

 private:
    PcmReader(const PcmReader &reader);
    PcmReader& operator=(const PcmReader &reader);

 protected:
    /**
     * \brief Open the PCM reader
     *
     * Opens the underlying PCM capture port. This method is expected
     * to be called by registered streams in their start() call.
     * Calls to this method are counted so that the PCM port is open
     * only when the first stream is started.
     *
     * \return 0 on success, otherwise negative error code
     */
    virtual int open();

    /**
     * \brief Close the PCM reader
     *
     * Closes the underlying PCM capture port. This method is expected
     * to be called by registered streams in their stop() call.
     * Calls to this method are counted so that the PCM port is closed
     * automatically when the last stream stops.
     */
    virtual void close();

    /**
     * \brief Reader's thread funcion
     *
     * Internal buffer is filled with data read from the PCM device.
     * The unmerge instance processes this buffer and produces data
     * for the registered streams.
     *
     * \return 0 on success, otherwise negative error code
     */
    int threadFunc();

    /**
     * \brief Create and attach a resampler
     *
     * Creates and attaches a resampler to an input stream. An intermediate
     * buffer to store samples coming from the PCM port is also allocated.
     *
     * \param stream The stream that requires the resampler
     * \return 0 on success, otherwise negative error code
     */
    int attachResampler(InStream *stream);

    /**
     * \brief Detach and remove the resampler
     *
     * Detaches and deletes the resampler of an output stream, if any. The
     * intermediate buffer allocated in attachResampler() is also freed.
     *
     * \param stream The stream that requires the resampler
     */
    void detachResampler(InStream *stream);

    /** Set of weak pointers to the registered input streams */
    typedef set< wp<InStream> > StreamSet;

    PcmInPort *mPort;                /**< PCM capture port */
    PcmParams mParams;               /**< PCM parameters to open the port */
    UnMerge mUnMerge;                /**< Unmerge instance if needed */
    StreamSet mStreams;              /**< Registered stream */
    BufferProvider::Buffer mBuffer;  /**< Buffer populated with own allocated memory */
    int mUsers;                      /**< Reference count of active users/streams */
    Mutex mLock;                     /**< Synchronize stream registration/unregistration */
};

/**
 * \class PcmWriter
 * \brief PCM writer
 *
 * PCM writer with merge capability. This writer can take multiple output
 * streams, merge them into a single multi-channel stream and write it
 * to a PCM output port.
 */
class PcmWriter : protected ThreadBase {
 public:
    /**
     * \brief PCM writer constructor
     *
     * Constructs a PCM writer for a given playback port. The writer
     * will accept multiple streams, merge them and write to the PCM port.
     *
     * \param port Pointer to the PCM port to write to
     * \param params PCM parameters to be used on the port
     */
    PcmWriter(PcmOutPort *port, const PcmParams &params);

    /**
     * \brief PCM merge destructor
     *
     * Destroys a PCM writer object.
     */
    virtual ~PcmWriter();

    /**
     * \brief Check the result of constructing the PCM writer
     *
     * Result of constructing a PCM writer. It must be checked before
     * using any methods. Result is undefined if the PCM writer is
     * used when in an unitialized state.
     *
     * \return true if PCM construction is correct, false otherwise
     */
    virtual bool initCheck() const;

    /**
     * \brief Get the PcmParams of the PCM writer
     *
     * Get the PcmParams used in this PCM writer, that is, the parameters
     * used to open the underlying PCM playback port.
     *
     * \return Reference to the PCM params of the reader
     */
    const PcmParams& getParams() const { return mParams; }

    /**
     * \brief Register a stream
     *
     * Registers a stream to the PCM writer. Multiple streams can be
     * registered to the writer, each one is associated with one or more
     * slots of the output stream that is written to the PCM out port.
     *
     * \param stream PCM stream to be registered
     * \return 0 on success, otherwise negative error code
     */
    virtual int registerStream(sp<OutStream>& stream);

    /**
     * \brief Unregister a PCM stream
     *
     * Unregisters a stream from the PCM writer. The stream must have
     * been previously registered using registerStream().
     *
     * \param stream PCM stream to be unregistered
     */
    virtual void unregisterStream(sp<OutStream>& stream);

    /**
     * \brief Check if an output stream is registered
     *
     * Tests if an output stream is currently registered with the PCM writer.
     *
     * \return true if the stream is registered, false otherwise
     */
    virtual bool isStreamRegistered(sp<OutStream>& stream);

    friend class OutStream;

 private:
    PcmWriter(const PcmWriter &writer);
    PcmWriter& operator=(const PcmWriter &writer);

 protected:
    /**
     * \brief Open the PCM writer
     *
     * Opens the underlying PCM playback port. This method is expected
     * to be called by registered streams in their start() call.
     * Calls to this method are counted so that the PCM port is open
     * only when the first stream is started.
     *
     * \return 0 on success, otherwise negative error code
     */
    virtual int open();

    /**
     * \brief Close the PCM writer
     *
     * Closes the underlying PCM playback port. This method is expected
     * to be called by registered streams in their stop() call.
     * Calls to this method are counted so that the PCM port is closed
     * automatically when the last stream stops.
     */
    virtual void close();

    /**
     * \brief Writer's thread funcion
     *
     * The merge instance processes its inputs and the produced buffer is
     * written to the PCM device.
     *
     * \return 0 on success, otherwise negative error code
     */
    int threadFunc();

    /**
     * \brief Create and attach a resampler
     *
     * Creates and attaches a resampler to an output stream. An intermediate
     * buffer to store resampled frames is also allocated.
     *
     * \param stream The stream that requires the resampler
     * \return 0 on success, otherwise negative error code
     */
    int attachResampler(OutStream *stream);

    /**
     * \brief Detach and remove the resampler
     *
     * Detaches and deletes the resampler of an output stream, if any. The
     * intermediate buffer allocated in attachResampler() is also freed.
     *
     * \param stream The stream that requires the resampler
     */
    void detachResampler(OutStream *stream);

    /** Set of weak pointers to the registered output streams */
    typedef set< wp<OutStream> > StreamSet;

    PcmOutPort *mPort;               /**< PCM playback port */
    PcmParams mParams;               /**< PCM parameters to open the port */
    Merge mMerge;                    /**< Merge instance if needed */
    StreamSet mStreams;              /**< Registered streams */
    BufferProvider::Buffer mBuffer;  /**< Buffer populated with own allocated memory */
    int mUsers;                      /**< Reference count of active users/streams */
    Mutex mLock;                     /**< Synchronize stream registration/unregistration */
};

} /* namespace tiaudioutils */

#endif /* _TIAUDIOUTILS_STREAM_H_ */
