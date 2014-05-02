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
 * \file MumStream.h
 * \brief Classes for merge and unmerge audio streams
 *
 * Merge class allows multiple incoming inputs to be assembled in a single
 * output with flexible slot mapping. Unmerge class produces multiple outputs
 * from a single input stream.
 */

#ifndef _TIAUDIOUTILS_MUMSTREAM_H_
#define _TIAUDIOUTILS_MUMSTREAM_H_

#include <sys/types.h>
#include <system/audio.h>
#include <errno.h>
#include <set>

#include <tiaudioutils/Pcm.h>
#include <tiaudioutils/Base.h>

#include <utils/RefBase.h>
#include <utils/StrongPointer.h>

namespace tiaudioutils {

using std::set;
using android::sp;
using android::wp;

class InStream;
class OutStream;

/**
 * \class Merge
 * \brief %Merge
 *
 * Merge multiple audio streams into a single output stream.
 */
class Merge {
 public:
    /**
     * \brief Constructor of a merge
     *
     * Constructs a merge object with the given stream parameters. Initially,
     * the merge object contains no streams to be merged.
     *
     * \param params Output stream parameters (see PcmParams)
     */
    Merge(const PcmParams &params);

    /**
     * \brief Destructor of a merge
     *
     * Destroys a merge object.
     */
    virtual ~Merge();

    /**
     * \brief Check the result of constructing a merge object
     *
     * Result of constructing a Merge. It must be checked before using any Merge
     * methods.
     * Result is undefined otherwise.
     *
     * \return true if merge construction is correct, false otherwise
     */
    virtual bool initCheck() const;

    /**
     * \brief Get the PcmParams of the merge instance
     *
     * Get the PcmParams used in this merge instance.
     *
     * \return Reference to the PCM params of the merge
     */
    virtual const PcmParams& getParams() const { return mParams; }

    /**
     * \brief Register a stream for merge
     *
     * Register a output stream to be merged.
     *
     * \param stream Pointer to a Stream instance to be registered
     * \return 0 on success, otherwise negative error code
     */
    virtual int registerStream(const sp<OutStream>& stream);

    /**
     * \brief Un-register a stream
     *
     * Un-register an output stream from a merge instance.
     *
     * \param stream Pointer to a Stream instance to be un-registered
     */
    virtual void unregisterStream(sp<OutStream>& stream);

    /**
     * \brief Process the merge stream
     *
     * Process the merge stream operation. It's caller responsibility to pass a
     * valid buffer previously allocated.
     *
     * \param outBuffer Buffer where merged data will be copied to.
     * \return 0 on success, otherwise negative error code
     */
    virtual int process(BufferProvider::Buffer &outBuffer);

 private:
    Merge(const Merge &merge);
    Merge& operator=(const Merge &merge);

 protected:
    /**
     * \brief Merge an individual stream
     *
     * Merge a given stream buffer into the output.
     *
     * \param stream The stream to be merged
     * \param inBuffer The source buffer to be merged
     * \param outBuffer The destination buffer
     */
    virtual void merge(sp<OutStream> stream,
                       BufferProvider::Buffer &inBuffer,
                       BufferProvider::Buffer &outBuffer);

    /** Set of weak pointers to the registered output streams */
    typedef set< wp<OutStream> > StreamSet;

    PcmParams mParams;         /**< PCM parameters used during merge operation */
    uint32_t mDstMask;         /**< The mask of the channels currently being merged */
    uint32_t mContainerBytes;  /**< The number of bytes in the container per sample */
    StreamSet mStreams;        /**< Set of registered OutStream objects */
    Mutex mLock;               /**< Lock to synchronize registration and processing */
};

/**
 * \class UnMerge
 * \brief %UnMerge
 *
 * UnMerge a single audio stream into multiple streams.
 */
class UnMerge {
 public:
    /**
     * \brief Constructor of an unmerge
     *
     * Constructs an unmerge object with the given stream parameters. Initially,
     * the unmerge object contains no streams to be un-merged.
     *
     * \param params Input stream parameters (see PcmParams)
     */
    UnMerge(const PcmParams &params);

    /**
     * \brief Destructor of an unmerge
     *
     * Destroys an unmerge object.
     */
    virtual ~UnMerge();

    /**
     * \brief Check the result of constructing an unmerge object
     *
     * Result of constructing an UnMerge. It must be checked before using any
     * UnMerge methods.
     * Result is undefined otherwise.
     *
     * \return true if unmerge construction is correct, false otherwise
     */
    virtual bool initCheck() const;

    /**
     * \brief Get the PcmParams of the unmerge instance
     *
     * Get the PcmParams used in this unmerge instance.
     *
     * \return Reference to the PCM params of the unmerge
     */
    virtual const PcmParams& getParams() const { return mParams; }

    /**
     * \brief Register an stream for unmerge
     *
     * Register an input stream to be filled with un-merged content.
     *
     * \param stream Pointer to an InputStream instance to be registered
     * \return 0 on success, otherwise negative error code
     */
    virtual int registerStream(const sp<InStream>& stream);

    /**
     * \brief Un-register an stream
     *
     * Un-register an input stream from an UnMerge instance.
     *
     * \param stream Pointer to an InStream instance to be un-registered
     */
    virtual void unregisterStream(sp<InStream>& stream);

    /**
     * \brief Process the unmerge stream
     *
     * Process the unmerge stream operation. It's caller responsibility to pass
     * a valid buffer previously allocated.
     *
     * \param inBuffer Buffer from where unmerged data will be copied.
     * \return 0 on success, otherwise negative error code
     */
    virtual int process(BufferProvider::Buffer &inBuffer);

 private:
    UnMerge(const UnMerge &unmerge);
    UnMerge& operator=(const UnMerge &unmerge);

 protected:
    /**
     * \brief UnMerge an individual stream
     *
     * UnMerge data from the input to a given stream buffer.
     *
     * \param stream The stream to be filled with unmerged content
     * \param inBuffer The source buffer to be unmerged
     * \param outBuffer The destination buffer
     */
    virtual void unmerge(sp<InStream> stream,
                         BufferProvider::Buffer &inBuffer,
                         BufferProvider::Buffer &outBuffer);

    /** Set of weak pointers to the registered input streams */
    typedef set< wp<InStream> > StreamSet;

    PcmParams mParams;         /**< PCM parameters used during unmerge operation */
    uint32_t mSrcMask;         /**< The mask of the channels currently being unmerged */
    uint32_t mContainerBytes;  /**< The number of bytes in the container per sample */
    StreamSet mStreams;        /**< Set of registered InStream objects */
    Mutex mLock;               /**< Lock to synchronize registration and processing */
};

} /* namespace tiaudioutils */

#endif /* _TIAUDIOUTILS_MUMSTREAM_H_ */
