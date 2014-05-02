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
 * \file NullPcm.h
 * \brief Classes for accessing the PCM interface of a null card
 *
 * Contains classes required to play and capture data from a null sound card.
 */

#ifndef _TIAUDIOUTILS_NULLPCM_H_
#define _TIAUDIOUTILS_NULLPCM_H_

#include <limits.h>

#include <tiaudioutils/Base.h>
#include <tiaudioutils/Pcm.h>

namespace tiaudioutils {

/**
 * \class NullInPort
 * \brief Null capture port
 *
 * The null PCM port used for capture. Buffers are filled with silence by the
 * null port at the same rate that an actual hardware port would.
 */
class NullInPort : public PcmInPort {
 public:
    /**
     * \brief Null input port constructor
     *
     * Constructs a null input/capture port.
     */
    NullInPort();

    /**
     * \brief Null input port destructor
     *
     * Destroys a null input/capture port object.
     */
    virtual ~NullInPort() {}

    /**
     * \brief Get the id of the sound card
     *
     * Gets the id of the sound card that this port belongs to. Neither the
     * null port or the card actually exist in the system (as opposite to
     * the ALSA dummy/null card), so the returned id is chosen not to
     * conflict with any valid card that may be registered in the system.
     *
     * \return The sound card id
     */
    uint32_t getCardId() const { return UINT_MAX; }

    /**
     * \brief Get the port id
     *
     * Gets the null input/capture port id. The id itself is valid but
     * meaningless since there is no sound card in the system for the null PCM.
     *
     * \return The PCM port id
     */
    uint32_t getPortId() const { return 0; }

    /**
     * \brief Get the port name
     *
     * Gets the name of the null capture port.
     *
     * \return The name of the PCM port
     */
    const char* getName() const { return mName.c_str(); }

    /**
     * \brief Open the null port for capture
     *
     * Opens the null PCM port for capture with the given parameters. There is
     * no hardware port actually opened by this method. The PCM parameters
     * used to open the port are used to calculate the buffer time in read().
     *
     * \param params PcmParams used to open the port
     * \return 0 on success, otherwise negative error code
     */
    int open(const PcmParams &params);

    /**
     * \brief Close the null PCM port
     *
     * Closes the null PCM port opened by calling open(). There is no hardware
     * port actually closed by this method.
     */
    void close();

    /**
     * \brief Test if the PCM port is open and ready
     *
     * Tests if the PCM port is open and ready for capture.
     *
     * \return true if the port is open, false otherwise
     */
    bool isOpen() const { return mOpen; }

    /**
     * \brief Read audio data from the null PCM input port
     *
     * Reads data from the null port which produces a buffer filled with silence.
     * No actual data is read from hardware, but the call blocks for the time
     * equivalent to the requested frames.
     *
     * \param buffer Pointer to the destination buffer
     * \param frames Number of frames to be read
     * \return 0 on success, otherwise negative error code
     */
    int read(void *buffer, size_t frames);

    /**
     * \brief Start the null PCM port
     *
     * Starts the null PCM capture port. No meaningful action is performed
     * by this method.
     *
     * \return 0 on success, otherwise negative error code
     */
    int start() { return 0; }

    /**
     * \brief Stop the null PCM port
     *
     * Stops the null PCM capture port. No meaningful action is performed
     * by this method.
     *
     * \return 0 on success, otherwise negative error code
     */
    int stop() { return 0; }

 private:
    NullInPort(const NullInPort &port);
    NullInPort& operator=(const NullInPort &port);

 protected:
    bool mOpen;         /**< State of the port: open or closed */
    string mName;       /**< Name of the null PCM port */
    PcmParams mParams;  /**< PCM params of the port */
    Mutex mLock;        /**< Synchronize PCM port use */
};

/**
 * \class NullOutPort
 * \brief Null playback port
 *
 * The null PCM port used for playback. Buffers are consumed by the null port
 * at the same rate that an actual hardware port would.
 */
class NullOutPort : public PcmOutPort {
 public:
    /**
     * \brief Null output port constructor
     *
     * Constructs a null output/playback port.
     */
    NullOutPort();

    /**
     * \brief Null output port destructor
     *
     * Destroys a null output/playback port object.
     */
    virtual ~NullOutPort() {}

    /**
     * \brief Get the id of the sound card
     *
     *
     * Gets the id of the sound card that this port belongs to. Neither the
     * null port or the card actually exist in the system (as opposite to
     * the ALSA dummy/null card), so the returned id is chosen not to
     * conflict with any valid card that may be registered in the system.
     *
     * \return The sound card id
     */
    uint32_t getCardId() const { return UINT_MAX; }

    /**
     * \brief Get the port id
     *
     * Gets the null output/playback port id. The id itself is valid but
     * meaningless since there is no sound card in the system for the null PCM.
     *
     * \return The PCM port id
     */
    uint32_t getPortId() const { return 0; }

    /**
     * \brief Get the port name
     *
     * Gets the name of the null playback port.
     *
     * \return The name of the PCM port
     */
    const char* getName() const { return mName.c_str(); }

    /**
     * \brief Open the null port for playback
     *
     * Opens the null PCM port for playback with the given parameters. There is
     * no hardware port actually opened by this method. The PCM parameters
     * used to open the port are used to calculate the buffer time in write().
     *
     * \param params PcmParams used to open the port
     * \return 0 on success, otherwise negative error code
     */
    int open(const PcmParams &params);

    /**
     * \brief Close the null PCM port
     *
     * Closes the null PCM port opened by calling open(). There is no hardware
     * port actually closed by this method.
     */
    void close();

    /**
     * \brief Test if the PCM port is open and ready
     *
     * Tests if the PCM port is open and ready for playback.
     *
     * \return true if the port is open, false otherwise
     */
    bool isOpen() const { return mOpen; }

    /**
     * \brief Write audio data to the null PCM output port
     *
     * Writes data to the null output port. No data write actually occurs
     * but the call blocks for the time equivalent to the requested frames.
     *
     * \param buffer Pointer to the source buffer
     * \param frames Number of frames to be written
     * \return Number of frames written, otherwise negative error code
     */
    int write(const void *buffer, size_t frames);

    /**
     * \brief Start the null PCM port
     *
     * Starts the null PCM playback port. No meaningful action is performed
     * by this method.
     *
     * \return 0 on success, otherwise negative error code
     */
    int start() { return 0; }

    /**
     * \brief Stop the null PCM port
     *
     * Stops the null PCM playback port. No meaningful action is performed
     * by this method.
     *
     * \return 0 on success, otherwise negative error code
     */
    int stop() { return 0; }

 private:
    NullOutPort(const NullOutPort &port);
    NullOutPort& operator=(const NullOutPort &port);

 protected:
    bool mOpen;         /**< State of the port: open or closed */
    string mName;       /**< Name of the null PCM port */
    PcmParams mParams;  /**< PCM params of the port */
    Mutex mLock;        /**< Synchronize PCM port use */
};

} /* namespace tiaudioutils */

#endif /* _TIAUDIOUTILS_NULLPCM_H_ */
