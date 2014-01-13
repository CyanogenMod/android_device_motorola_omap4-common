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
/**
 *  @file  omx_proxy_video_encoder.h
 *         This file contains methods that provides the functionality for
 *         the OpenMAX1.1 DOMX Framework Proxy component.
 *********************************************************************************************
 This is the proxy specific wrapper that passes the component name to the generic proxy init()
 The proxy wrapper also does some runtime/static time onfig on per proxy basis
 This is a thin wrapper that is called when componentiit() of the proxy is called
 static OMX_ERRORTYPE PROXY_Wrapper_init(OMX_HANDLETYPE hComponent, OMX_PTR pAppData);
 this layer gets called first whenever a proxy's get handle is called
 ************************************************************************************************
 *  @path WTSD_DucatiMMSW\omx\omx_il_1_x\omx_proxy_component\omx_video_enc\inc
 *
 *  @rev 1.0
 */

/*==============================================================
 *! Revision History
 *! ============================
 * 5 October 2012: Vidhoon Viswanathan vidhoon@ti.com Initial version
 *================================================================*/

 /******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
//C includes
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <cutils/properties.h>
//TIMMOSAL and OMX includes
#include <timm_osal_interfaces.h>
#include "OMX_TI_IVCommon.h"
#include "OMX_TI_Video.h"
#include "OMX_TI_Index.h"
#include <MetadataBufferType.h>
//GRALLOC includes
#ifdef  ENABLE_GRALLOC_BUFFER
#include "native_handle.h"
#include <hal_public.h>
#include <VideoMetadata.h>
#endif

#define OMX_ENC_NUM_INTERNAL_BUF (8)
/**
 * struct OMX_PROXY_ENCODER_PRIVATE: this struct contains all data elements specific
 *                                   to PROXY ENCODER components.
 *
 * @param hBufPipe: handle to local buffer pipe
 * @param bAndroidOpaqueFormat: boolean that indicates if AndroidOpaqueFormat is set
 * @param hCC: context handle
 * @param gralloc_handle: handles of local gralloc buffers allocated
 * @param nCurBufIndex: current buffer index
 * @param mAllocDev: Local gralloc client
 *
 *  */
typedef struct OMX_PROXY_ENCODER_PRIVATE
{
	OMX_PTR  hBufPipe;
	OMX_BOOL bAndroidOpaqueFormat;
	OMX_PTR  hCC;
	IMG_native_handle_t* gralloc_handle[OMX_ENC_NUM_INTERNAL_BUF];
	OMX_S32  nCurBufIndex;
	alloc_device_t* mAllocDev;
}OMX_PROXY_ENCODER_PRIVATE;
