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
#ifndef AKFS_INC_COMMON_H
#define AKFS_INC_COMMON_H

#ifdef WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif    					
  					
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <stdarg.h>
#include <crtdbg.h>
#include <tchar.h>
#include "../Android/Android.h"

#define ENABLE_AKMDEBUG	1

#else
#include <stdio.h>     /* frpintf */
#include <stdlib.h>    /* atoi */
#include <string.h>    /* memset */
#include <unistd.h>
#include <stdarg.h>    /* va_list */
#include <errno.h>     /* errno */

#endif

#include "AKFS_Log.h"

/*** Constant definition ******************************************************/
#define AKM_TRUE	1	/*!< Represents true */
#define AKM_FALSE	0	/*!< Represents false */
#define AKM_SUCCESS	0	/*!< Represents success */
#define AKM_ERROR	-1	/*!< Represents error */

#define OPMODE_CONSOLE		(0x01)
#define OPMODE_FST			(0x02)

/*** Type declaration *********************************************************/

/*** Global variables *********************************************************/
extern int g_stopRequest;	/*!< 0:Not stop,  1:Stop */
extern int g_opmode;		/*!< 0:Daemon mode, 1:Console mode. */
extern int g_dbgzone;		/*!< Debug zone. */

/*** Prototype of function ****************************************************/

#endif /* AKMD_INC_AKCOMMON_H */

