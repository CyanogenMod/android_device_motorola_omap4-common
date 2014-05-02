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
 * \file SimpleStream.h
 * \brief Classes for writing and reading data to/from PCM ports
 *
 * Contains the SimpleReader, SimpleWriter and their corresponding streams.
 * They are called "simple" because they are non-threaded and non-merge/unmerge
 * capable.
 * The underlying PCM ports are not tied to any specific card or port type,
 * as long as they implement interfaces defined in Pcm.h.
 */

#ifndef _TIAUDIOUTILS_SIMPLESTREAM_H_
#define _TIAUDIOUTILS_SIMPLESTREAM_H_

#include <tiaudioutils/Pcm.h>

namespace tiaudioutils {

class SimpleWriter;
class SimpleReader;
class Resampler;

/**
 * \class SimpleInStream
 * \brief PCM reader stream
 *
 * The stream with PCM data for capture.
 */
class SimpleInStream {
 public:
    /**
     * \brief Simple PCM input constructor
     *
     * Constructs a PCM stream for the given parameters. Channel reorder
     * is not possible with this type of stream.
     *
     * \param params PCM params for the stream
     */
    SimpleInStream(const PcmParams &params);

    /**
     * \brief Simple PCM input destructor
     *
     * Destroys the PCM stream.
     */
    virtual ~SimpleInStream();

    /**
     * \brief Check the result of constructing the simple PCM input
     *
     * Result of constructing a simple PCM input. It must be checked before
     * using any methods. Result is undefined if the PCM input is
     * used when in an unitialized state.
     *
     * \return true if PCM output is correct, false otherwise
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
     * \brief Start the stream
     *
     * Starts the stream for use. The PCM reader will open the underlying
     * PCM port.
     *
     * \return 0 on success, otherwise negative error code
     */
    virtual int start();

    /**
     * \brief Stop the stream
     *
     * Stops the stream. The PCM reader will close the underlying PCM port.
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
     * Reads data from the stream which directly translates into reading from
     * the PCM port. The stream must be started through start() before
     * the stream is able to read.
     *
     * \param buffer Pointer to the destination buffer
     * \param frames Number of frames to be read
     * \return Number of frames read, otherwise negative error code
     */
    virtual int read(void *buffer, size_t frames);

    friend class SimpleReader;

 private:
    SimpleInStream(const SimpleInStream &stream);
    SimpleInStream& operator=(const SimpleInStream &stream);

 protected:
    PcmParams mParams;        /**< PCM params of the stream */
    SimpleReader *mReader;    /**< PCM reader that the stream is registered to */
    bool mStarted;            /**< Whether the stream is started or not */
    mutable Mutex mLock;      /**< Synchronize stream start/stop */
};

/**
 * \class SimpleOutStream
 * \brief PCM writer stream
 *
 * The stream with PCM data for playback that can be used with SimpleWriter.
 */
class SimpleOutStream {
 public:
    /**
     * \brief Simple PCM output stream constructor
     *
     * Constructs a PCM stream for the given parameters. Channel reorder
     * is not possible with this type of stream.
     *
     * \param params PCM params for the stream
     */
    SimpleOutStream(const PcmParams &params);

    /**
     * \brief Simple PCM output stream destructor
     *
     * Destroys the PCM stream.
     */
    virtual ~SimpleOutStream();

    /**
     * \brief Check the result of constructing the simple PCM output
     *
     * Result of constructing a simple PCM output. It must be checked before
     * using any methods. Result is undefined if the PCM output is
     * used when in an unitialized state.
     *
     * \return true if PCM output is correct, false otherwise
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
     * \brief Start the stream
     *
     * Starts the stream for use. The PCM writer will open the underlying
     * PCM port.
     *
     * \return 0 on success, otherwise negative error code
     */
    virtual int start();

    /**
     * \brief Stop the stream
     *
     * Stops the stream. The PCM writer will close the underlying PCM port.
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
     * Writes data to the stream which directly translates into writing to
     * the PCM port. The stream must be started through start() before
     * the stream is able to write.
     *
     * \param buffer Pointer to the source buffer
     * \param frames Number of frames to be written
     * \return Number of frames written, otherwise negative error code
     */
    virtual int write(const void *buffer, size_t frames);

    friend class SimpleWriter;

 private:
    SimpleOutStream(const SimpleOutStream &stream);
    SimpleOutStream& operator=(const SimpleOutStream &stream);

 protected:
    PcmParams mParams;        /**< PCM params of the stream */
    SimpleWriter *mWriter;    /**< PCM writer that the stream is registered to */
    bool mStarted;            /**< Whether the stream is started or not */
    mutable Mutex mLock;      /**< Synchronize stream start/stop */
};

/**
 * \class SimpleReader
 * \brief Simple PCM reader
 *
 * Basic PCM reader where a single stream is recorded from a capture port.
 * There is no merge capability or buffer adaptation.
 */
class SimpleReader {
 public:
    /**
     * \brief Simple PCM reader constructor
     *
     * Constructs a simple PCM reader for a given capture port
     *
     * \param port Pointer to the PCM port to read from
     * \param params PCM parameters to be used on the port
     */
    SimpleReader(PcmInPort *port, const PcmParams &params);

    /**
     * \brief Simple PCM reader destructor
     *
     * Destroys a simple PCM reader object.
     */
    virtual ~SimpleReader();

    /**
     * \brief Check the result of constructing the simple PCM reader
     *
     * Result of constructing a simple PCM reader. It must be checked before
     * using any methods. Result is undefined if the PCM simple reader is
     * used when in an unitialized state.
     *
     * \return true if PCM reader construction is correct, false otherwise
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
     * Registers a stream to the PCM reader. A single stream can be registered
     * to this type of PCM reader.
     *
     * \param stream PCM stream to be registered
     * \return 0 on success, otherwise negative error code
     */
    virtual int registerStream(SimpleInStream *stream);

    /**
     * \brief Unregister a PCM stream
     *
     * Unregisters a stream from the PCM reader. The stream must have
     * been previously registered using registerStream().
     *
     * \param stream PCM stream to be unregistered
     */
    virtual void unregisterStream(SimpleInStream *stream);

    /**
     * \brief Check if an input stream is registered
     *
     * Tests if an input stream is currently registered with the PCM reader.
     *
     * \return true if the stream is registered, false otherwise
     */
    virtual bool isStreamRegistered(SimpleInStream *stream);

    friend class SimpleInStream;

 private:
    SimpleReader(const SimpleReader &reader);
    SimpleReader& operator=(const SimpleReader &reader);

 protected:
    /**
     * \brief Open the PCM reader
     *
     * Opens the underlying PCM capture port. This method is expected
     * to be called by the registered stream in its start() call.
     *
     * \return 0 on success, otherwise negative error code
     */
    virtual int open();

    /**
     * \brief Close the PCM reader
     *
     * Closes the underlying PCM capture port. This method is expected
     * to be called by the registered stream in its stop() call.
     */
    virtual void close();

    /**
     * \brief Read data from the PCM capture port
     *
     * Read audio data from the PCM capture port.
     *
     * \param buffer Pointer to the destination buffer
     * \param frames Number of frames to be read
     * \return Number of frames read, otherwise negative error code
     */
    virtual int read(void *buffer, size_t frames);

    /**
     * \class ReadProvider
     * \brief PCM read provider
     *
     * Buffer provider that delivers buffers filled with data read from
     * the PCM input port. It's used as the buffer provider for the
     * resampler to ensure that it delivers the requested number of frames.
     */
    class ReadProvider : public BufferProvider {
    public:
        /**
         * Constructs a buffer provider that captures data from the PCM
         * port set in the specified reader, uses its resampler and
         * intermediate buffer to return the resampled data.
         *
         * \param reader Reader that has the resampler and intermediate buffer
         */
        ReadProvider(SimpleReader &reader) : mReader(reader) {}

        /**
         * Destroys the PCM capture buffer provider
         */
        virtual ~ReadProvider() {}

        /**
         * \brief Get the next buffer with new PCM input data
         *
         * Gets the next audio buffer with new PCM data read from the
         * port used by the reader.
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
    protected:
        SimpleReader &mReader;      /**< Reader that has the resampler and buffer */
    };

    PcmInPort *mPort;        /**< PCM in port that produces stream data */
    PcmParams mParams;       /**< PCM params of the port */
    SimpleInStream *mStream; /**< The registered stream */
    Resampler *mResampler;    /**< Resampler if needed */
    BufferProvider::Buffer mBuffer; /**< Intermediate buffer for data to be resampled */
    ReadProvider mBufferProvider;   /**< Buffer provider used for resampling */
    Mutex mLock;             /**< Used to synchronize methods of the reader */
};

/**
 * \class SimpleWriter
 * \brief Simple PCM writer
 *
 * Basic PCM writer where a single stream is played to a playback port.
 * There is no merge capability or buffer adaptation.
 */
class SimpleWriter {
 public:
    /**
     * \brief Simple PCM writer constructor
     *
     * Constructs a simple PCM writer for a given playback port
     *
     * \param port Pointer to the PCM port to write to
     * \param params PCM parameters to be used on the port
     */
    SimpleWriter(PcmOutPort *port, const PcmParams &params);

    /**
     * \brief Simple PCM writer destructor
     *
     * Destroys a simple PCM writer object.
     */
    virtual ~SimpleWriter();

    /**
     * \brief Check the result of constructing the simple PCM writer
     *
     * Result of constructing a simple PCM writer. It must be checked before
     * using any methods. Result is undefined if the PCM simple writer is
     * used when in an unitialized state.
     *
     * \return true if PCM writer construction is correct, false otherwise
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
     * Registers a stream to the PCM writer. A single stream can be registered
     * to this type of PCM writer.
     *
     * \param stream PCM stream to be registered
     * \return 0 on success, otherwise negative error code
     */
    virtual int registerStream(SimpleOutStream *stream);

    /**
     * \brief Unregister a PCM stream
     *
     * Unregisters a stream from the PCM writer. The stream must have
     * been previously registered using registerStream().
     *
     * \param stream PCM stream to be unregistered
     */
    virtual void unregisterStream(SimpleOutStream *stream);

    /**
     * \brief Check if an output stream is registered
     *
     * Tests if an output stream is currently registered with the PCM writer.
     *
     * \return true if the stream is registered, false otherwise
     */
    virtual bool isStreamRegistered(SimpleOutStream *stream);

    friend class SimpleOutStream;

 private:
    SimpleWriter(const SimpleWriter &writer);
    SimpleWriter& operator=(const SimpleWriter &writer);

 protected:
    /**
     * \brief Open the PCM writer
     *
     * Opens the underlying PCM playback port. This method is expected
     * to be called by the registered stream in its start() call.
     *
     * \return 0 on success, otherwise negative error code
     */
    virtual int open();

    /**
     * \brief Close the PCM writer
     *
     * Closes the underlying PCM playback port. This method is expected
     * to be called by the registered stream in its stop() call.
     */
    virtual void close();

    /**
     * \brief Write data to the PCM playback port
     *
     * Write audio data to the PCM playback port.
     *
     * \param buffer Pointer to the source buffer
     * \param frames Number of frames to be written
     * \return Number of frames written, otherwise negative error code
     */
    virtual int write(const void *buffer, size_t frames);

    PcmOutPort *mPort;        /**< PCM out port that consumes stream data */
    PcmParams mParams;        /**< PCM params of the port */
    SimpleOutStream *mStream; /**< The registered stream */
    Resampler *mResampler;    /**< Resampler if needed */
    BufferProvider::Buffer mBuffer; /**< Intermediate buffer for resampled data */
    Mutex mLock;              /**< Used to synchronize methods of the writer */
};

} /* namespace tiaudioutils */

#endif /* _TIAUDIOUTILS_SIMPLESTREAM_H_ */
