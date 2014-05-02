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
 * \file Base.h
 * \brief Miscellaneous base clases
 *
 * Miscellaneous classes used across tiaudioutils.
 */

#ifndef _TIAUDIOUTILS_BASE_H_
#define _TIAUDIOUTILS_BASE_H_

#include <sys/types.h>
#include <utils/Mutex.h>
#include <pthread.h>
#include <string>
#include <map>

namespace tiaudioutils {

using std::string;
using std::map;
using android::Mutex;
using android::AutoMutex;

/**
 * \class SlotMap
 * \brief Slot map
 *
 * The slot map between source and destination streams. Each channel in the
 * source can be mapped to any channel in the destination. That gives enough
 * flexibility for doing buffer remix. Destination channel is stored first in
 * the map and source channel is stored second. This enables a map to be specified
 * which can duplicate a single source channel to multiple destination channels.
 */
class SlotMap : public map<uint32_t, uint32_t> {
 public:
    /**
     * \brief Default constructor
     *
     * Constructs a default (but invalid) slot map. Entries to the map can be added
     * later to make the map valid.
     */
    SlotMap() {}

    /**
     * \brief Symmetric slot map constructor
     *
     * Constructs a slot map where the source slots map to the same slots in
     * the destination.
     *
     * \param mask Mask of the slots to be mapped
     */
    SlotMap(uint32_t mask);

    /**
     * \brief Consecutive slots map constructor
     *
     * Constructs a slot map where the slots in the source are processed in
     * ascending order and mapped to their corresponding slot in the destination,
     * also processed in ascending order.
     *
     * \param srcMask Mask of the source slots to be mapped
     * \param dstMask Mask of the destination slots to be mapped
     */
    SlotMap(uint32_t srcMask, uint32_t dstMask);

    /**
     * \brief Check if the slot map is valid
     *
     * Checks if the current state of the slot map is valid, defined as:
     * - at least one slot is mapped
     * - same number of source and destination channels
     *
     * \return true if slot map is valid, false otherwise
     */
    bool isValid() const;

    /**
     * \brief Get the number of channels in the map
     *
     * Gets the number of slots/channels in the map. Calling this function when
     * the map is in invalid state is a logical error.
     *
     * \return Number of slots/channels being mapped
     */
    uint32_t getChannelCount() const;

    /**
     * \brief Get the mask of the source slots
     *
     * Gets the mask of the source slots. No assumptions about the map order must
     * be made from the mask value.
     *
     * \return The mask with the slot positions in the source being mapped
     */
    uint32_t getSrcMask() const;

    /**
     * \brief Get the mask of the destination slots
     *
     * Gets the mask of the destination slots. No assumptions about the map order
     * must be made from the mask value.
     *
     * \return The mask with the slot positions in the destination being mapped
     */
    uint32_t getDstMask() const;
};

/**
 * The pthread's start routine passed during thread creation. It runs the
 * _threadLoop() of ThreadBase.
 *
 * \param me Pointer to the ThreadBase object.
 */
extern "C" void* threadWrapper(void *me);

/**
 * \class ThreadBase
 * \brief Thread base
 *
 * Base class of a thread. Derived classes must implement threadFunc() to give
 * implementation specific functionality to the thread. The thread will continue
 * executing threadFunc() as long as: threadFunc() returns 0 or stop() is not
 * called.
 */
class ThreadBase {
 public:
    /**
     * \brief Default constructor
     *
     * Constructs an unnamed thread.
     */
    ThreadBase();

    /**
     * \brief Named thread constructor
     *
     * Constructs a named thread.
     *
     * \param name Thread name
     */
    ThreadBase(const string &name);

    /**
     * \brief Named thread constructor
     *
     * Constructs a named thread.
     *
     * \param name Thread name
     */
    ThreadBase(const char *name);

    /**
     * \brief Destructor
     *
     * Destroys a thread similar to the way stop() would.
     */
    virtual ~ThreadBase();

    /**
     * \brief Start thread execution
     *
     * Starts the execution of the thread whose main function is threadFunc()
     * implemented by derived classes. Thread execution continues as long as
     * the threadFunc() returns 0 and it hasn't been stopped by stop().
     * Execution loop continues otherwise.
     *
     * \return 0 on success, otherwise negative error code
     */
    int run();

    /**
     * \brief Stop thread execution
     *
     * Stops the execution of the thread on completion of next thread loop.
     * Status of threadFunc() upon stopping is returned by this method.
     *
     * \return The return value of threadFunc()
     */
    int stop();

    /**
     * \brief Test if thread is running
     *
     * Test if the thread is running.
     *
     * \return true if thread is running, false otherwise
     */
    bool isRunning() const;

    /**
     * \brief Get the thread name
     *
     * Get the name of the thread.
     *
     * \return Thread name as a C-style string
     */
    const char *name() const { return mName.c_str(); }

    /**
     * \brief Set the thread name
     *
     * Set the name of the thread.
     *
     * \param newname New name of the thread
     * \return 0 on success, otherwise negative error code
     */
    int setName(string newname);

    friend void* threadWrapper(void *me);

 private:
    ThreadBase(const ThreadBase &thread);
    ThreadBase& operator=(const ThreadBase &thread);

    /**
     * \brief Thread's internal loop
     *
     * The thread's internal loop calls threadFunc() until is returns a value
     * different than 0 or the thread is stopped through stop().
     *
     * \return The return value of threadFunc()
     */
    int _threadLoop();

    pthread_t mThread;      /**< pthread handle of this thread */
    string mName;           /**< Name of the thread */
    volatile bool mRunning; /**< Running status of the thread */
    mutable Mutex mMutex;

    static const int mNameMaxLength = 16; /* Max length of the thread name */

 protected:
    /**
     * \brief Implementation-specific thread function
     *
     * Derived classes must implement it. It should return 0 if the function has
     * to be executed again on the next loop. Returning an error code breaks the
     * thread loop.
     *
     * \return 0 if function has to be executed again, otherwise an error code
     */
    virtual int threadFunc() = 0;
};

} /* namespace tiaudioutils */

#endif /* _TIAUDIOUTILS_BASE_H_ */
