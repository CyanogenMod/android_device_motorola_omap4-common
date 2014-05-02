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
 * \file Log.h
 * \brief Definitions used for logging in Android
 *
 * Contains the definition of the library's tag name and the very verbose
 * log level macro. Defining the log level (LOG_NDEBUG) here will affect all
 * modules in the library.
 */

#ifndef _TIAUDIOUTILS_LOG_H_
#define _TIAUDIOUTILS_LOG_H_

/* As usual, LOG_NDEBUG and VERY_VERBOSE_LOGGING must be defined before */

/** The tag name of the tiaudioutils library in Android */
#define LOG_TAG "tiaudioutils"

/** Definition of the very-verbose logging macro */
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(...) do { } while(0)
#endif

#include <cutils/log.h>

#endif /* _TIAUDIOUTILS_LOG_H_ */
