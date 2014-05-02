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

#include <system/audio.h>

#include <tiaudioutils/Log.h>
#include <tiaudioutils/Base.h>

namespace tiaudioutils {

using std::make_pair;

SlotMap::SlotMap(uint32_t mask)
{
    uint32_t pos = 0;

    while (mask) {
        if (mask & 1) {
            insert(make_pair<uint32_t, uint32_t>(pos, pos));
        }
        pos++;
        mask >>= 1;
    }
}

SlotMap::SlotMap(uint32_t srcMask, uint32_t dstMask)
{
    uint32_t numChanSrc = popcount(srcMask);
    uint32_t numChanDst = popcount(dstMask);

    if (numChanSrc != numChanDst) {
        ALOGW("SlotMap: inconsistent channel count in source (%d) and dest (%d)",
              numChanSrc, numChanDst);
        return;
    }

    uint32_t srcPos;
    uint32_t dstPos;

    while (srcMask && dstMask) {
        srcPos = ffs(srcMask);
        dstPos = ffs(dstMask);

        if (srcPos && dstPos) {
            /* ffs returns one plus the index */
            srcPos--;
            dstPos--;
            insert(make_pair<uint32_t, uint32_t>(dstPos, srcPos));
            srcMask &= ~(1U << srcPos);
            dstMask &= ~(1U << dstPos);
        }
    }
}

bool SlotMap::isValid() const
{
    return (size() && ((int)size() == popcount(getDstMask())));
}

uint32_t SlotMap::getChannelCount() const
{
    return size();
}

uint32_t SlotMap::getSrcMask() const
{
    uint32_t mask = 0;

    for (const_iterator i = begin(); i != end(); ++i) {
        mask |= (1U << i->second);
    }

    return mask;
}

uint32_t SlotMap::getDstMask() const
{
    uint32_t mask = 0;

    for (const_iterator i = begin(); i != end(); ++i) {
        mask |= (1U << i->first);
    }

    return mask;
}

/* ---------------------------------------------------------------------------------------- */

ThreadBase::ThreadBase()
    : mName("Thread"), mRunning(false)
{
}

ThreadBase::ThreadBase(const string &name)
    : mName(name), mRunning(false)
{
}

ThreadBase::ThreadBase(const char *name)
    : mName(""), mRunning(false)
{
    if (name)
        mName += name;
}

ThreadBase::~ThreadBase()
{
    void *res;

    if (mRunning) {
        ALOGW("%s is forcefully exiting", name());
        mRunning = false;
        pthread_join(mThread, &res);
    }
}

int ThreadBase::run()
{
    AutoMutex lock(mMutex);

    ALOGI("Thread %s is starting", name());
    mRunning = true;
    int ret = pthread_create(&mThread, NULL, threadWrapper, this);
    if (ret) {
        ALOGE("Thread %s: failed to create pthread %d", name(), ret);
        return ret;
    }

    ret = pthread_setname_np(mThread, mName.substr(0, mNameMaxLength - 1).c_str());
    if (ret)
        ALOGE("Thread %s: failed to set thread name %d", name(), ret);

    return ret;
}

int ThreadBase::stop()
{
    AutoMutex lock(mMutex);
    void *res;

    ALOGI("Thread %s is exiting", name());
    mRunning = false;
    pthread_join(mThread, &res);

    return (int)res;
}

bool ThreadBase::isRunning() const
{
    AutoMutex lock(mMutex);
    return mRunning;
}

int ThreadBase::setName(string newname)
{
    AutoMutex lock(mMutex);
    int ret = 0;

    mName = newname;
    if (mRunning) {
        ret = pthread_setname_np(mThread, mName.substr(0, mNameMaxLength - 1).c_str());
        if (ret)
            ALOGE("Thread %s: failed to set new name %d", name(), ret);
    }

    return ret;
}

int ThreadBase::_threadLoop()
{
    int ret = 0;

    while (mRunning) {
        ret = threadFunc();
        if (ret)
            break;
    }

    return ret;
}

extern "C" void* threadWrapper(void *me)
{
    return (void *)static_cast<ThreadBase *>(me)->_threadLoop();
}

} /* namespace tiaudioutils */
