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
 * \file Resampler.h
 * \brief Speex-based resampler
 *
 * A Speex-based resampler that supports multiple in and out rates, but
 * only 16-bits/sample
 */

#ifndef _TIAUDIOUTILS_RESAMPLER_H_
#define _TIAUDIOUTILS_RESAMPLER_H_

#include <tiaudioutils/Pcm.h>

/* forward declaration */
struct SpeexResamplerState_;
/** Type definition as in speex_resampler.h header */
typedef struct SpeexResamplerState_ SpeexResamplerState;

namespace tiaudioutils {


/**
 * \class Resampler
 * \brief Speex-based resampler
 *
 * %Resampler based on Speex, it provides flexible input and output sample rates but
 * requires 16-bits/sample.
 */
class Resampler {
 public:
    /**
     * \brief %Resampler constructor from single parameters
     *
     * Constructs a resampler with same parameters for input and output sides.
     * The input or output sample rates can be set later through setInSampleRate()
     * or setOutSampleRate().
     *
     * \param params Input and output side params
     * \param quality Speex resampler quality
     */
    Resampler(const PcmParams &params,
              uint32_t quality = QualityDefault);

    /**
     * \brief %Resampler constructor from input and output parameters
     *
     * Constructs a resampler with the specified input and output parameters.
     * Both parameters must be valid, have 16-bits/sample and same number of
     * channels.
     *
     * \param inParams Input side parameters
     * \param outParams Output side parameters
     * \param quality Speex resampler quality
     */
    Resampler(const PcmParams &inParams,
              const PcmParams &outParams,
              uint32_t quality = QualityDefault);

    /**
     * \brief %Resampler destructor
     *
     * Destroy a resampler object.
     */
    virtual ~Resampler();

    /**
     * \brief Check the result of constructing the resampler
     *
     * Tests whether the resampler construction is correct or not. It must be
     * checked before using any methods. Result is undefined if the resampler
     * is used when in an unitialized state.
     *
     * \return true if resampler construction is correct, false otherwise
     */
    virtual bool initCheck() const;

    /**
     * \brief Set a new input sample rate
     *
     * Sets a new sample rate for the input side.
     *
     * \param rate New input rate
     * \return 0 on success, otherwise negative error code
     */
    virtual int setInSampleRate(uint32_t rate);

    /**
     * \brief Set a new output sample rate
     *
     * Sets a new sample rate for the output side.
     *
     * \param rate New output rate
     * \return 0 on success, otherwise negative error code
     */
    virtual int setOutSampleRate(uint32_t rate);

    /**
     * \brief Get resampling ratio
     *
     * Gets the numerator and denominator of the resampling ratio.
     * The ratio is expressed as input_rate / output_rate, reduced
     * to the least common denominator.
     *
     * \param num Reference for the numerator
     * \param den Reference for the denominator
     */
    void getRatio(uint32_t &num, uint32_t &den) const;

    /**
     * \brief Test if this is an upsampler
     *
     * Tests whether or not this resampler is configured for upsampling.
     *
     * \return true if this is an upsampler, false otherwise
     */
    bool isUpsampler() const { return mRatioNum > mRatioDen; }

    /**
     * \brief Test if this is a downsampler
     *
     * Tests whether or not this resampler is configured for downsampling.
     *
     * \return true if this is an downsampler, false otherwise
     */
    bool isDownsampler() const { return mRatioNum < mRatioDen; }

    /**
     * \brief Resample from buffer provider to supplied buffer
     *
     * Resamples from the specified provider and outputs at most specified frame
     * count into the passed buffer.
     *
     * \param provider Buffer provider to get the audio frames for resampling
     * \param outBuffer Pointer to the buffer where resampled data will be stored
     * \param outFrames Requested number of output frames
     * \return 0 on success, otherwise negative error code
     */
    virtual int resample(BufferProvider &provider,
                         void *outBuffer,
                         uint32_t outFrames);

    /**
     * \brief Resample from passed buffers
     *
     * Resamples audio from the specified buffers. Requested input and output
     * frames will be updated with the actual resampled frames.
     *
     * \param inBuffer Pointer to the buffer with data to be resampled
     * \param inFrames As input, the requested number of input frames.
     *                 As output, the actual number of resampled input frames
     * \param outBuffer Pointer to the buffer with resampled data
     * \param outFrames As input, the requested number of output frames.
     *                  As output, the actual number of resampled output frames
     */
    virtual int resample(const void *inBuffer, uint32_t &inFrames,
                         void *outBuffer, uint32_t &outFrames);

    static const uint32_t QualityVoIP = 3;     /**< VoIP resampler quality */
    static const uint32_t QualityDefault = 4;  /**< Default resampler quality */
    static const uint32_t QualityDesktop = 5;  /**< Desktop resampler quality */
    static const uint32_t QualityMax = 10;     /**< Maximum resampler quality */

 private:
    Resampler(const Resampler &resampler);
    Resampler& operator==(const Resampler &resampler);

 protected:
    /**
     * \brief Create the Speex-based resampler
     *
     * Creates the Speex-based resampler with the sample rate of the input and
     * output parameters.
     */
    void createResampler();

    /**
     * \brief Reallocate intermediate buffer to new frame count
     *
     * Reallocates intermediate buffer to store the specified number of frames.
     * The intermediate buffer is used to store the audio data obtained from the
     * buffer provider in the resampler() method.
     * It's caller's responsibility to ensure the resize occurs only when current
     * buffer is not large enough to hold new frame count.
     */
    void reallocateBuffer(uint32_t frames);

    static const uint32_t kHeadRoomFrames = 10; /**< Extra frames to allocate for */

    PcmParams mInParams;             /**< PCM params of the input stream */
    PcmParams mOutParams;            /**< PCM params of the output stream */
    BufferProvider::Buffer mBuffer;  /**< Local buffer to store frames to be converted */
    SpeexResamplerState *mSpeexRsmp; /**< Speex resampler */
    uint32_t mQuality;               /**< Quality of the Speex resampler */
    uint32_t mAvail;                 /**< Number of frames in the buffer */
    uint32_t mBufferFrames;          /**< Buffer size, excluding the overhead room frames */
    uint32_t mRatioNum;              /**< Numerator of the sample rate ratio */
    uint32_t mRatioDen;              /**< Denominator of the sample rate ratio */
    mutable Mutex mLock;             /**< Access to the internal Speex resampler */
};

} /* namespace tiaudioutils */

#endif /* _TIAUDIOUTILS_RESAMPLER_H */
