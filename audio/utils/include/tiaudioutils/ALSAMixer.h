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
 * \file ALSAMixer.h
 * \brief Classes for controlling the mixer interface of an ALSA card
 *
 * Contains classes involved in the control of the mixer interface of an ALSA
 * sound card.
 */

#ifndef _TIAUDIOUTILS_ALSAMIXER_H_
#define _TIAUDIOUTILS_ALSAMIXER_H_

#include <string>
#include <list>
#include <map>

#include <tiaudioutils/Base.h>
#include <tiaudioutils/Pcm.h>

/* forward declaration */
struct audio_route;

namespace tiaudioutils {

using std::string;
using std::list;
using std::map;

/**
 * \class ALSAControl
 * \brief ALSA kcontrol
 *
 * An ALSA control that can be string or integer. Other control types like
 * boolean are treatead as integer type. This class is only meant to carry
 * control information like its name and value. The control takes effect
 * when is passed to an ALSAMixer.
 */
class ALSAControl {
 public:
    /**
     * \brief Integer ALSA control constructor
     *
     * Constructs an integer ALSA control that can be used for boolean,
     * integer, etc. kcontrols. It's caller responsibility to pass a
     * valid name (e.g. non-empty string).
     *
     * \param name Control name
     * \param val Control value
     */
    ALSAControl(string &name, int val);

    /**
     * \brief String ALSA control constructor
     *
     * Constructs a string ALSA control that can be used for enumerated
     * type kcontrols. It's caller responsibility to pass a valid name
     * and value (e.g. non-empty strings).
     *
     * \param name Control name
     * \param val Control value
     */
    ALSAControl(string &name, string &val);

    /**
     * \brief Integer ALSA control constructor
     *
     * Constructs an integer ALSA control that can be used for boolean,
     * integer, etc. kcontrols. It's caller responsibility to pass a
     * valid name (e.g. non-empty string).
     *
     * \param name Control name as a C-style char pointer
     * \param val Control value
     */
    ALSAControl(const char *name, int val);

    /**
     * \brief String ALSA control constructor
     *
     * Constructs a string ALSA control that can be used for enumerated
     * type kcontrols. It's caller responsibility to pass a valid name
     * and value (e.g. non-empty strings).
     *
     * \param name Control name as a C-style char pointer
     * \param val Control value as a C-style char pointer
     */
    ALSAControl(const char *name, const char *val);

    /**
     * \brief ALSA control destructor
     *
     * Destroy an ALSA control object.
     */
    virtual ~ALSAControl() {}

    /**
     * \brief Get the control name
     *
     * Gets the ALSA control name.
     *
     * \return Reference to the control name
     */
    const string& name() const { return mName; }

    /**
     * \brief Get the control string value
     *
     * Gets the string value of an ALSA control.
     *
     * \return Reference to the control value.
     *         Empty string if the control is not of string type
     */
    const string& strVal() const { return mStrVal; }

    /**
     * \brief Get the control integer value
     *
     * Gets the integer value of an ALSA control.
     *
     * \return Value of the control
     * \retval -1 if the control is not of integer type
     */
    int intVal() const { return mIntVal; }

    /**
     * \brief Test if the control type is integer
     *
     * Test if the ALSA control type is integer.
     *
     * \return true if the control is integer type, false otherwise
     */
    bool isIntegerType() const;

    /**
     * \brief Test if the control type is string
     *
     * Test if the ALSA control type is string.
     *
     * \return true if the control is string type, false otherwise
     */
    bool isStringType() const;

 protected:
    string mName;    /**< Control name */
    string mStrVal;  /**< String value of the control */
    int mIntVal;     /**< Integer value of the control */
    enum {
        TYPE_INT,    /**< Integer type: bool, int, byte, etc */
        TYPE_STR     /**< String type: enum */
    } mType;         /**< Control type category */
};

/**
 * \typedef ALSAControlList
 * \brief List of ALSA Controls
 *
 * STL-based list of ALSA controls. It's used to pass controls to be set or
 * cleared to the ALSAMixer.
 */
typedef list<ALSAControl> ALSAControlList;

/**
 * \class ALSAMixer
 * \brief ALSA Mixer
 *
 * The ALSA mixer is used to set or clear kcontrols in a sound card.
 */
class ALSAMixer {
 public:
    /**
     * \brief ALSA mixer constructor
     *
     * Constructs an ALSA mixer of a sound card.
     *
     * \param card ALSA card id
     */
    ALSAMixer(uint32_t card);

    /**
     * \brief ALSA mixer destructor
     *
     * Destroys an ALSA mixer object.
     */
    virtual ~ALSAMixer();

    /**
     * \brief Check the result of constructing an ALSA mixer
     *
     * Result of constructing a ALSA mixer. It must be checked before
     * using any methods. Result is undefined if ALSAMixer is used when
     * in an unitialized state.
     *
     * \return true is mixer construction is correct, false otherwise
     */
    bool initCheck() const;

    /**
     * \brief Set or clear the value of an ALSAControl
     *
     * Sets or clears the value of an individual ALSAControl in the card's
     * mixer.
     *
     * \param control The ALSAControl to be set or cleared
     * \param on true to set the control, false to clear it
     * \return 0 on success, otherwise negative error code
     */
    int set(const ALSAControl &control, bool on);

    /**
     * \brief Set or clear the value of a list of controls
     *
     * Sets or clears the value of an ALSAControlList to the card's mixer.
     * There is no unwind mechanism to set the controls to their initial value
     * if this method fails while setting an intermediate control of the list.
     * Controls with multiple values will all be set to the same ALSAControl
     * value.
     *
     * \param controls The list of ALSA controls to be set or cleared
     * \param on true to set the control list, false to clear it
     * \return 0 on success, otherwise negative error code
     */
    int set(const ALSAControlList &controls, bool on);

    /**
     * \brief Initialize routes to the default value
     *
     * Initializes card controls based on the default values defined in the
     * card's XML file, if the file is available.
     *
     * \return 0 on success, otherwise negative error code
     */
    int initRoutes();

    /**
     * \brief Apply or reset the controls of an audio path
     *
     * Applies or resets the controls of a named path as defined in the
     * card's XML path, if the file is available. The name of the path
     * must match any of the declared "path" elements in the XML file.
     * All the controls in the corresponding XML "path" element will be
     * applied, but only those controls that have a reset value in the
     * card defaults section will be reset.
     *
     * \param path The path name
     * \param on true to apply the path, false to reset it
     * \return 0 on success, otherwise negative error code
     */
    int setPath(const char *path, bool on);

    /**
     * \brief Apply or reset the controls for an audio device
     *
     * Applies or resets the controls needed for audio devices (one or more)
     * based on XML "path" elements named after the Android audio device
     * definitions (e.g. AUDIO_DEVICE_OUT_SPEAKER, AUDIO_DEVICE_IN_BUILTIN_MIC).
     * All the controls in the corresponding XML "path" element for a given
     * device will be applied, but only those controls that have a reset value
     * in the card defaults section will be reset.
     *
     * \param devices The audio devices to apply or reset the path
     * \param on true to apply the path, false to reset it
     * \return 0 on success, otherwise negative error code
     */
    int setPath(audio_devices_t devices, bool on);

    /**
     * \brief Update the controls for an audio device transition
     *
     * Update the audio routes for a device transition in two steps:
     * - Reset the paths for inactive devices (devices that after the
     *   transition are no longer used)
     * - Apply the paths for new active devices (devices that after the
     *   transition are newly used)
     *
     * \param oldDevices The devices before the transition
     * \param newDevices The devices required after the transition
     * \return 0 on success, otherwise negative error code
     */
    int updatePath(audio_devices_t oldDevices, audio_devices_t newDevices);

 private:
    ALSAMixer(const ALSAMixer &mixer);
    ALSAMixer& operator=(const ALSAMixer &mixer);

 protected:
    /** The prefix of the path where to search for the card's route XML file */
    static const string kAudioRoutePrefix;
    /** The suffix of the card's route XML file */
    static const string kAudioRouteSuffix;

    /**
     * \brief Initialize map of audio device to definition name
     *
     * Initialize the internal map of audio devices (audio_device_t) to device
     * name (string). It allows to search paths in the card's XML file by their
     * Android device name, e.g. AUDIO_DEVICE_OUT_SPEAKER, AUDIO_DEVICE_IN_BUILTIN_MIC.
     */
    void initDeviceMap();

    /**
     * Map between Android audio devices and their names.
     * Used to find paths in the XML routes file for a corresponding audio device.
     */
    typedef map<audio_devices_t, string> DeviceMap;

    uint32_t mCard;               /**< Card id */
    struct mixer *mMixer;         /**< tinyalsa mixer */
    struct audio_route *mRouter;  /**< XML-based audio router */
    DeviceMap mDevMap;            /**< Android device to device name */
    Mutex mLock;                  /**< Lock the access to tinyalsa mixer calls */
};

} /* namespace tiaudioutils */

#endif /* _TIAUDIOUTILS_ALSAMIXER_H_ */
