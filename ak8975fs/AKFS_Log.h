/******************************************************************************
 *
 * Copyright (C) 2012 Asahi Kasei Microdevices Corporation, Japan
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
 *
 ******************************************************************************/
#ifndef AKFS_INC_LOG_H
#define AKFS_INC_LOG_H

#ifndef WIN32
#include <cutils/log.h>
#endif

#undef LOG_TAG
#define LOG_TAG "AKMD_FS"

#ifndef ALOGE
#ifdef LOGE
#define ALOGE	LOGE
#endif
#define ALOGE	printf
#endif
#ifndef ALOGE_IF
#ifdef LOGE_IF
#define ALOGE_IF	LOGE_IF
#endif
#define ALOGE_IF	printf
#endif

#ifndef ALOGW
#ifdef LOGW
#define ALOGW	LOGW
#endif
#define ALOGW	printf
#endif
#ifndef ALOGW_IF
#ifdef LOGW_IF
#define ALOGW_IF	LOGW_IF
#endif
#define ALOGW_IF	printf
#endif

#ifndef ALOGI
#ifdef LOGI
#define ALOGI	LOGI
#endif
#define ALOGI	printf
#endif
#ifndef ALOGI_IF
#ifdef LOGI_IF
#define ALOGI_IF	LOGI_IF
#endif
#define ALOGI_IF	printf
#endif

#ifndef ALOGD
#ifdef LOGD
#define ALOGD	LOGD
#endif
#define ALOGD	printf
#endif
#ifndef ALOGD_IF
#ifdef LOGD_IF
#define ALOGD_IF	LOGD_IF
#endif
#define ALOGD_IF	printf
#endif

#ifndef ALOGV
#ifdef LOGV
#define ALOGV	LOGV
#endif
#define ALOGV	printf
#endif
#ifndef ALOGV_IF
#ifdef LOGV_IF
#define ALOGV_IF	LOGV_IF
#endif
#define ALOGV_IF	printf
#endif


#define DATA_AREA01	0x0001
#define DATA_AREA02	0x0002
#define DATA_AREA03	0x0004
#define DATA_AREA04	0x0008
#define DATA_AREA05	0x0010
#define DATA_AREA06	0x0020
#define DATA_AREA07	0x0040
#define DATA_AREA08	0x0080
#define DATA_AREA09	0x0100
#define DATA_AREA10	0x0200
#define DATA_AREA11	0x0400
#define DATA_AREA12	0x0800
#define DATA_AREA13	0x1000
#define DATA_AREA14	0x2000
#define DATA_AREA15	0x4000
#define DATA_AREA16	0x8000


/* Debug area definition */
#define AKMDATA_DUMP		DATA_AREA01	/*<! Dump data */
#define AKMDATA_DEBUG		DATA_AREA02 /*<! Debug */
#define AKMDATA_MAG			DATA_AREA03 /*<! Magnetic Field */
#define AKMDATA_ACC			DATA_AREA04 /*<! Accelerometer */
#define AKMDATA_ORI			DATA_AREA05 /*<! Orientation */
#define AKMDATA_LOOP		DATA_AREA06
#define AKMDATA_DRV			DATA_AREA07
#define AKMDATA_CONSOLE		DATA_AREA08
#define AKMDATA_CHECK		DATA_AREA09

#ifndef ENABLE_AKMDEBUG
#define ENABLE_AKMDEBUG		(0)	/* Enable debug output when it is 1. */
#endif

/***** Debug output ******************************************/
#if ENABLE_AKMDEBUG
#define AKMDEBUG(flag, format, ...) \
	((((int)flag) & g_dbgzone) \
	  ? (fprintf(stdout, (format), ##__VA_ARGS__)) \
	  : ((void)0))
#else
#define AKMDEBUG(flag, format, ...)
#endif

/***** Error output *******************************************/
#define AKMERROR \
	  (ALOGE("%s:%d Error.", __FUNCTION__, __LINE__))

#define AKMERROR_STR(api) \
	  (ALOGE("%s:%d %s Error (%s).", \
	  		  __FUNCTION__, __LINE__, (api), strerror(errno)))

#endif /* AKFS_INC_LOG_H */
