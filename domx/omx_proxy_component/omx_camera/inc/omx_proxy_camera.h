/*
 * Copyright (c) 2010, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>

#include <timm_osal_interfaces.h>
#include <OMX_TI_IVCommon.h>
#include <OMX_TI_Index.h>
#include "omx_proxy_common.h"
#include "timm_osal_mutex.h"
#include "omx_rpc.h"

#define COMPONENT_NAME "OMX.TI.DUCATI1.VIDEO.CAMERA"
/*Needs to be specific for every configuration wrapper*/

#undef LOG_TAG
#define LOG_TAG "CameraHAL"

#define DEFAULT_DCC 1

#define LINUX_PAGE_SIZE (4 * 1024)

#define _PROXY_OMX_INIT_PARAM(param,type) do {		\
	TIMM_OSAL_Memset((param), 0, sizeof (type));	\
	(param)->nSize = sizeof (type);			\
	(param)->nVersion.s.nVersionMajor = 1;		\
	(param)->nVersion.s.nVersionMinor = 1;		\
	} while(0)

/* VTC specific changes */
#define MAX_NUM_INTERNAL_BUFFERS 4
#define MAX_VTC_WIDTH 1920
#define MAX_VTC_HEIGHT 1080
#define BORDER_WIDTH 32
#define BORDER_HEIGHT 32
#define MAX_VTC_WIDTH_WITH_VNF (MAX_VTC_WIDTH + BORDER_WIDTH)
#define MAX_VTC_HEIGHT_WITH_VNF (MAX_VTC_HEIGHT + BORDER_HEIGHT)
#define PREVIEW_PORT 2

typedef struct OMX_PROXY_CAM_PRIVATE
{
	MEMPLUGIN_BUFFER_ACCESSOR sInternalBuffers[MAX_NUM_INTERNAL_BUFFERS][2];
	OMX_PTR  gComponentBufferAllocation[PROXY_MAXNUMOFPORTS][MAX_NUM_INTERNAL_BUFFERS];
}OMX_PROXY_CAM_PRIVATE;


OMX_ERRORTYPE PROXY_ComponentDeInit(OMX_HANDLETYPE);
OMX_ERRORTYPE __PROXY_SetConfig(OMX_HANDLETYPE, OMX_INDEXTYPE,
								OMX_PTR, OMX_PTR);
OMX_ERRORTYPE __PROXY_GetConfig(OMX_HANDLETYPE, OMX_INDEXTYPE,
								OMX_PTR, OMX_PTR);
OMX_ERRORTYPE __PROXY_SetParameter(OMX_IN OMX_HANDLETYPE, OMX_INDEXTYPE,
									OMX_PTR, OMX_PTR, OMX_U32);
OMX_ERRORTYPE __PROXY_GetParameter(OMX_IN OMX_HANDLETYPE, OMX_INDEXTYPE,
									OMX_PTR, OMX_PTR);
OMX_ERRORTYPE PROXY_SendCommand(OMX_HANDLETYPE, OMX_COMMANDTYPE,
								        OMX_U32,OMX_PTR);
OMX_ERRORTYPE CameraMaptoTilerDuc(OMX_TI_CONFIG_SHAREDBUFFER *, OMX_PTR *);
OMX_ERRORTYPE OMX_CameraVtcFreeMemory(OMX_IN OMX_HANDLETYPE hComponent);
//COREID TARGET_CORE_ID = CORE_APPM3;

extern RPC_OMX_ERRORTYPE RPC_RegisterBuffer(OMX_HANDLETYPE hRPCCtx, int fd1,
									 int fd2,
                                     OMX_PTR *handle1, OMX_PTR *handle2,
                                     PROXY_BUFFER_TYPE proxyBufferType);
extern RPC_OMX_ERRORTYPE RPC_UnRegisterBuffer(OMX_HANDLETYPE, OMX_PTR, OMX_PTR,
										PROXY_BUFFER_TYPE proxyBufferType);
