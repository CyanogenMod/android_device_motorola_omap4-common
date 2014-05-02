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

#include <errno.h>

#include <tiaudioutils/Log.h>
#include <tiaudioutils/NullPcm.h>

namespace tiaudioutils {

NullInPort::NullInPort()
    : mOpen(false), mName("NullIn")
{
}

int NullInPort::open(const PcmParams &params)
{
    AutoMutex lock(mLock);
    mParams = params;
    mOpen = true;

    return 0;
}

void NullInPort::close()
{
    AutoMutex lock(mLock);
    mOpen = false;
}

int NullInPort::read(void *buffer, size_t frames)
{
    AutoMutex lock(mLock);
    if (!mOpen) {
        ALOGE("NullInPort: port is closed, cannot read");
        return -EAGAIN;
    }

    memset(buffer, 0, mParams.framesToBytes(frames));
    usleep((frames * 1000000) / mParams.sampleRate);

    return frames;
}

/* ---------------------------------------------------------------------------------------- */

NullOutPort::NullOutPort()
    : mOpen(false), mName("NullOut")
{
}

int NullOutPort::open(const PcmParams &params)
{
    AutoMutex lock(mLock);
    mParams = params;
    mOpen = true;

    return 0;
}

void NullOutPort::close()
{
    AutoMutex lock(mLock);
    mOpen = false;
}

int NullOutPort::write(const void *buffer, size_t frames)
{
    AutoMutex lock(mLock);
    if (!mOpen) {
        ALOGE("NullOutPort: port is closed, cannot write");
        return -EAGAIN;
    }

    usleep((frames * 1000000) / mParams.sampleRate);

    return frames;
}

} /* namespace tiaudioutils */
