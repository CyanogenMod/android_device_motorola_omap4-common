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
 * \file Pcm.h
 * \brief Abstract classes of PCM ports, input and output
 *
 * Defines the PCM ports involved in audio playback and capture.
 */

#ifndef _TIAUDIOUTILS_PCM_H_
#define _TIAUDIOUTILS_PCM_H_

#include <hardware/audio.h>
#include <system/audio.h>

#include <tiaudioutils/Base.h>

/* forward declaration */
struct pcm_config;

namespace tiaudioutils {

/**
 * \enum StreamDirection
 * \brief Audio stream direction
 *
 * Enum that represents the direction of the audio stream: playback or capture
 */
enum StreamDirection {
    PLAYBACK, /**< Playback direction */
    CAPTURE   /**< Capture direction */
};

class PcmPort;
class PcmInPort;
class PcmOutPort;

/**
 * \class BufferProvider
 * \brief %Buffer provider
 *
 * It represents components than can provider buffers for capture or playback.
 */
class BufferProvider {
 public:
    /**
     * \class Buffer
     * \brief Audio buffer
     *
     * The audio buffer defined as a pointer to the audio data and its size
     * in frames.
     */
    struct Buffer {
    /**
     * \brief Default constructor
     *
     * Constructs a default (but invalid) audio buffer.
     */
    Buffer() : raw(NULL), frameCount(0) { }
        union {
            void *raw;
            int32_t *i32;
            int16_t *i16;
            int8_t *i8;
        };                 /**< Pointer to the audio data */
        size_t frameCount; /**< %Buffer size in frames */
    };

 protected:
    BufferProvider() {}
    virtual ~BufferProvider() {}

 public:
    /**
     * \brief Get the next buffer
     *
     * Get the next audio buffer with at most the requested number of frames.
     * The buffer provider can update it with the actual number of frames it
     * was able to provide.
     * The buffer provider is in charge of populating the buffer data pointer
     * with a buffer that is valid until the call to releaseBuffer().
     *
     * \param buffer Pointer to the buffer to fill with next buffer's info
     * \return 0 on success, otherwise negative error code
     */
    virtual int getNextBuffer(Buffer* buffer) = 0;

    /**
     * \brief Release the buffer
     *
     * Release the audio buffer previously obtained through getNextBuffer().
     * It's a logical error to release a buffer not previously obtained, result
     * is undefined.
     *
     * \param buffer Pointer to the buffer to be released
     */
    virtual void releaseBuffer(Buffer* buffer) = 0;
};

/**
 * \class PcmParams
 * \brief PCM parameters
 *
 * The PCM parameters that define an audio stream.
 */
class PcmParams {
 public:
    /**
     * \brief Default constructor
     *
     * Constructs default (but invalid) PCM parameters. PCM parameters must be
     * explicitly set for them to be valid for further use.
     */
    PcmParams();

    /**
     * \brief Constructor from individiual parameter values
     *g
     * Constructs the PCM params from explicit values for each field
     *
     * \param chan Number of channels/slots in a frame
     * \param bits Number of bits in a sample
     * \param rate Sample rate of the stream
     * \param frames Number of frames in the buffer
     */
    PcmParams(uint32_t chan, uint32_t bits, uint32_t rate, uint32_t frames = 0);

    /**
     * \brief Constructor from a tinyalsa config
     *
     * Constructs the PCM params from the config parameters of tinyalsa.
     * Most parameters are built from their corresponding counterpart, except
     * for the buffer size which is set from the tinyalsa's period size.
     *
     * \param config tinyalsa PCM config
     */
    PcmParams(const struct pcm_config &config);

    /**
     * \brief Constructor from Android's audio config
     *
     * Constructs the PCM params from the Android's audio config and an optional
     * buffer size. Parameters like channels, bit/sample and rate are built from
     * their Android's audio config counterpart. Buffer size is not defined in
     * Android's audio config hence it's taken from the passed argument.
     *
     * \param config Android audio config structuure
     * \param frames Number of frames in the audio buffer
     */
    PcmParams(const struct audio_config &config, uint32_t frames = 0);

    /**
     * \brief Test if current state of params is valid
     *
     * Test if the current state of the PCM parameters is valid.
     *
     * \return true is params are valid, false otherwise
     */
    bool isValid() const;

    /**
     * \brief Get the number of bytes in a sample
     *
     * Get the number of bytes in an audio sample.
     */
    uint32_t sampleSize() const { return sampleBits / 8; };

    /**
     * \brief Get the number of bytes in a frame
     *
     * Get the number of bytes in an audio frame.
     */
    uint32_t frameSize() const { return sampleSize() * channels; };

    /**
     * \brief Get the number of bytes in the buffer
     *
     * Get the number of bytes in the audio buffer.
     */
    uint32_t bufferSize() const { return sampleSize() * channels * frameCount; }

    /**
     * \brief Get the number of samples in the buffer
     *
     * Gets the number of audio samples in the buffer. The term sample
     * refers to the container of a single audio channel.
     */
    uint32_t sampleCount() const { return channels * frameCount; }

    /**
     * \brief Convert frames to bytes
     *
     * Convert the number of audio frames to bytes using the PCM parameters in
     * this object.
     *
     * \param frames Number of audio frames to be converted
     * \return Number of bytes in the passed frames
     */
    uint32_t framesToBytes(uint32_t frames) const;

    /**
     * \brief Convert bytes to frames
     *
     * Convert the number of bytes to frames using the PCM parameters in
     * this object.
     *
     * \param bytes Number of bytes to be converted
     * \return Number of frames in the passed frames
     */
    uint32_t bytesToFrames(uint32_t bytes) const;

    /**
     * \brief Fill pcm_config struct
     *
     * Fill a tinyalsa's pcm_config from the parameters of this object. Few fields
     * of pcm_config are populated: channels, rate, format and period_size. Other
     * fields must be populated outside this method as PcmParams contains less
     * information than pcm_config.
     *
     * \param config tinyalsa pcm_config to be filled
     */
    void toPcmConfig(struct pcm_config &config) const;

    /**
     * \brief Fill audio_config struct
     *
     * Fill an Android's pcm_config from the parameters of this object for the
     * given stream direction as channel mask is direction dependent in
     * audio_config.
     *
     * \param config audio_config struct to be filled
     * \param dir Stream direction
     */
    void toAudioConfig(struct audio_config &config, StreamDirection dir) const;

    uint32_t channels;   /**< The number of channels in a frame */
    uint32_t sampleBits; /**< The number of bits in a sample */
    uint32_t sampleRate; /**< The sample rate of the stream */
    uint32_t frameCount; /**< The number of frames in the buffer */
};

/**
 * \class PcmPort
 * \brief PCM port
 *
 * PcmPort abstracts the parts of a PCM port that are common for playback
 * and capture.
 */
class PcmPort {
 public:
    /**
     * \brief Destructor of a PCM port
     *
     * Destroys a PCM port object.
     */
    virtual ~PcmPort() {}

    /**
     * \brief Get the card id
     *
     * Get the id of the sound card that this PCM port belongs to.
     *
     * \return The sound card id
     */
    virtual uint32_t getCardId() const = 0;

    /**
     * \brief Get port id
     *
     * Get the id of the PCM port.
     *
     * \return The PCM port id
     */
    virtual uint32_t getPortId() const = 0;

    /**
     * \brief Get the port name
     *
     * Get the name of the PCM port.
     *
     * \return The name of the PCM port
     */
    virtual const char* getName() const = 0;

    /**
     * \brief Open the PCM port
     *
     * Opens a PCM port with the requested PCM parameters.
     *
     * \param params Reference to the PCM params
     * \return 0 on success, otherwise negative error code
     */
    virtual int open(const PcmParams &params) = 0;

    /**
     * \brief Close the PCM port
     *
     * Closes the PCM port if open.
     */
    virtual void close() = 0;

    /**
     * \brief Query if PCM port is open
     *
     * Query is PCM is open and ready to use.
     *
     * \return true if port is open, false otherwise
     */
    virtual bool isOpen() const = 0;

    /**
     * \brief Start the PCM port
     *
     * Starts the PCM port. It's mostly needed to start hostless ports that
     * run without CPU intervention.
     *
     * \return 0 on success, otherwise negative error code
     */
    virtual int start() = 0;

    /**
     * \brief Stop the PCM port
     *
     * Stops the PCM capture port. It's mostly needed to stop hostless that
     * run without CPU intervention.
     *
     * \return 0 on success, otherwise negative error code
     */
    virtual int stop() = 0;
};

/**
 * \class PcmInPort
 * \brief PCM input port
 *
 * PCM input port represents a port with capture capability.
 */
class PcmInPort : public PcmPort {
 public:
    /**
     * \brief Destructor of a PCM input port
     *
     * Destroys a PCM input port object.
     */
    virtual ~PcmInPort() {}

    /**
     * \brief Read audio frames
     *
     * Read audio frames from the PCM input port. The PCM input port must be
     * open first by calling open() method. Number of frames read can be less
     * than requested.
     *
     * \param buffer Pointer to the buffer where data will be copied to
     * \param frames Number of frames to be read
     * \return Number of frames read, otherwise error code
     */
    virtual int read(void *buffer, size_t frames) = 0;
};

/**
 * \class PcmOutPort
 * \brief PCM output port
 *
 * PCM output port represents a port with playback capability.
 */
class PcmOutPort : public PcmPort {
 public:
    /**
     * \brief Destructor of a PCM output port
     *
     * Destroys a PCM output port object.
     */
    virtual ~PcmOutPort() {}

    /**
     * \brief Write audio frames
     *
     * Write audio frames from the PCM input port. The PCM output port must be
     * already open by calling open() method. Number of frames written can be less
     * than requested.
     *
     * \param buffer Pointer to the buffer where playback data resides
     * \param frames Number of frames to be written
     * \return Number of frames written, otherwise error code
     */
    virtual int write(const void *buffer, size_t frames) = 0;
};

} /* namespace tiaudioutils */

#endif /* _TIAUDIOUTILS_PCM_H_ */
