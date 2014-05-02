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
 * \file ALSAPcm.h
 * \brief Classes for accessing the PCM interface of an ALSA card
 *
 * Contains classes required to play and capture data from an ALSA sound card,
 * incluiding a class for the ALSA card itself.
 */

#ifndef _TIAUDIOUTILS_ALSAPCM_H_
#define _TIAUDIOUTILS_ALSAPCM_H_

#include <tiaudioutils/Base.h>
#include <tiaudioutils/Pcm.h>

namespace tiaudioutils {

class ALSAInPort;
class ALSAOutPort;

/**
 * \class ALSAInPort
 * \brief ALSA capture port
 *
 * The ALSA PCM port used for capture.
 */
class ALSAInPort : public PcmInPort {
 public:
    /**
     * \brief ALSA input port constructor
     *
     * Constructs an ALSA input/capture port with period count.
     *
     * \param card The card id
     * \param port The port id
     * \param period_count The period count
     */
    ALSAInPort(uint32_t card,
               uint32_t port,
               uint32_t period_count = kDefaultNumPeriods);

    /**
     * \brief ALSA input port destructor
     *
     * Destroys an ALSA input/capture port object.
     */
    virtual ~ALSAInPort();

    /**
     * \brief Get the id of the ALSA sound card
     *
     * Gets the id of the ALSA sound card that this port belongs to.
     *
     * \return The sound card id
     */
    uint32_t getCardId() const { return mCardId; }

    /**
     * \brief Get the port id
     *
     * Gets the ALSA input/capture port id which is the PCM device number.
     *
     * \return The PCM port id
     */
    uint32_t getPortId() const { return mPortId; }

    /**
     * \brief Get the port name
     *
     * Gets the name of the ALSA capture port: hw:<CARD>,<DEVICE>.
     *
     * \return The name of the PCM port
     */
    const char* getName() const { return mName.c_str(); }

    /**
     * \brief Open the PCM port for capture
     *
     * Opens the ALSA PCM port for capture with the given parameters.
     *
     * \param params PcmParams used to open the port
     * \return 0 on success, otherwise negative error code
     */
    int open(const PcmParams &params);

    /**
     * \brief Close the PCM port
     *
     * Closes the ALSA PCM port opened by calling open().
     */
    void close();

    /**
     * \brief Test if the PCM port is open and ready
     *
     * Tests if the PCM port is open and ready for capture.
     *
     * \return true if the port is open, false otherwise
     */
    bool isOpen() const;

    /**
     * \brief Read audio data from the PCM input port
     *
     * Reads audio data from the PCM capture port to the passed buffer.
     *
     * \param buffer Pointer to the destination buffer
     * \param frames Number of frames to be read
     * \return 0 on success, otherwise negative error code
     */
    int read(void *buffer, size_t frames);

    /**
     * \brief Start the ALSA PCM port
     *
     * Starts the ALSA PCM capture port. It's mostly needed to start hostless
     * ports that don't use read() but instead run without CPU intervention.
     * Regular PCM read through read() method doesn't need to be explicitly
     * preceded by start(), since it's done internally.
     *
     * \return 0 on success, otherwise negative error code
     */
    int start();

    /**
     * \brief Stop the ALSA PCM port
     *
     * Stops the ALSA PCM capture port. It's mostly needed to stop hostless
     * ports that don't use read() but instead run without CPU intervention.
     * Regular PCM read through read() method doesn't need to be explicitly
     * stopped, but if done, it will cause read() to immediately return.
     * This behavior can be desired to unblock read() calls.
     *
     * \return 0 on success, otherwise negative error code
     */
    int stop();

    /** Default number of periods for the tinyalsa config of the capture port */
    static const uint32_t kDefaultNumPeriods = 3;

 private:
    ALSAInPort(const ALSAInPort &port);
    ALSAInPort& operator=(const ALSAInPort &port);

 protected:
    uint32_t mCardId;      /**< Id of the ALSA card */
    uint32_t mPortId;      /**< Id of the ALSA PCM device */
    string mName;          /**< Name of the ALSA PCM port */
    uint32_t mPeriodCount; /**< Period count of this port */
    struct pcm *mPcm;      /**< tinyalsa pcm handle */
    mutable Mutex mLock;   /**< Synchronize PCM port use */
};

/**
 * \class ALSAOutPort
 * \brief ALSA playback port
 *
 * The ALSA PCM port used for playback.
 */
class ALSAOutPort : public PcmOutPort {
 public:
    /**
     * \brief ALSA output port constructor
     *
     * Constructs an ALSA output/playback port with period count.
     *
     * \param card The card id
     * \param port The port id
     * \param period_count The period count
     */
    ALSAOutPort(uint32_t card,
                uint32_t port,
                uint32_t period_count = kDefaultNumPeriods);

    /**
     * \brief ALSA output port destructor
     *
     * Destroys an ALSA output/playback port object.
     */
    virtual ~ALSAOutPort();

    /**
     * \brief Get the id of the ALSA sound card
     *
     * Gets the id of the ALSA sound card that this port belongs to.
     *
     * \return The sound card id
     */
    uint32_t getCardId() const { return mCardId; }

    /**
     * \brief Get the port id
     *
     * Gets the ALSA output/playback port id which is the PCM device number.
     *
     * \return The PCM port id
     */
    uint32_t getPortId() const { return mPortId; }

    /**
     * \brief Get the port name
     *
     * Gets the name of the ALSA playback port: hw:<CARD>,<DEVICE>.
     *
     * \return The name of the PCM port
     */
    const char* getName() const { return mName.c_str(); }

    /**
     * \brief Open the PCM port for playback
     *
     * Opens the ALSA PCM port for playback with the given parameters.
     * The port is open in non-mmap mode.
     *
     * \param params PcmParams used to open the port
     * \return 0 on success, otherwise negative error code
     */
    int open(const PcmParams &params);

    /**
     * \brief Close the PCM port
     *
     * Closes the ALSA PCM port opened by calling open().
     */
    virtual void close();

    /**
     * \brief Test if the PCM port is open and ready
     *
     * Tests if the PCM port is open and ready for playback.
     *
     * \return true if the port is open, false otherwise
     */
    bool isOpen() const;

    /**
     * \brief Write audio data to the PCM output port
     *
     * Writes audio data to the PCM playback port from the passed buffer.
     *
     * \param buffer Pointer to the source buffer
     * \param frames Number of frames to be written
     * \return Number of frames written, otherwise negative error code
     */
    int write(const void *buffer, size_t frames);

    /**
     * \brief Start the ALSA PCM port
     *
     * Starts the ALSA PCM playback port. It's mostly needed to start hostless
     * ports that don't use write() but instead run without CPU intervention.
     * Regular PCM write through write() method doesn't need to be explicitly
     * preceded by start(), since it's done internally.
     *
     * \return 0 on success, otherwise negative error code
     */
    int start();

    /**
     * \brief Stop the ALSA PCM port
     *
     * Stops the ALSA PCM playback port. It's mostly needed to stop hostless
     * ports that don't use write() but instead run without CPU intervention.
     * Regular PCM write through write() method doesn't need to be explicitly
     * stopped, but if done, it will cause write() to immediately return.
     * This behavior can be desired to unblock write() calls.
     *
     * \return 0 on success, otherwise negative error code
     */
    int stop();

    /** Default number of periods for the tinyalsa config of the playback port */
    static const uint32_t kDefaultNumPeriods = 4;

 private:
    ALSAOutPort(const ALSAOutPort &port);
    ALSAOutPort& operator=(const ALSAOutPort &port);

 protected:
    uint32_t mCardId;      /**< Id of the ALSA card */
    uint32_t mPortId;      /**< Id of the ALSA PCM device */
    string mName;          /**< Name of the ALSA PCM port */
    uint32_t mPeriodCount; /**< Period count of this port */
    struct pcm *mPcm;      /**< tinyalsa pcm handle */
    mutable Mutex mLock;   /**< Synchronize PCM port use */
};

} /* namespace tiaudioutils */

#endif /* _TIAUDIOUTILS_ALSAPCM_H_ */
