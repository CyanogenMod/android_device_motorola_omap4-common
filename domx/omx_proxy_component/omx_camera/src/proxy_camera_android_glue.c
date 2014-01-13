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
#include "omx_proxy_camera.h"
#include "timm_osal_mutex.h"
#include "omx_rpc.h"
//UTIL includes
#include "memplugin_ion.h"

#define MAX_NUM_INTERNAL_BUFFERS 4
OMX_ERRORTYPE GLUE_CameraSetParam(OMX_IN OMX_HANDLETYPE
    hComponent, OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR pComponentParameterStructure)
    {
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	MEMPLUGIN_ERRORTYPE eMemError = MEMPLUGIN_ERROR_NONE;
    MEMPLUGIN_BUFFER_PARAMS newBuffer_params,delBuffer_params;
    MEMPLUGIN_BUFFER_PROPERTIES newBuffer_prop,delBuffer_prop;
    OMX_S32 ret = 0;
    PROXY_COMPONENT_PRIVATE *pCompPrv;
    OMX_PROXY_CAM_PRIVATE* pCamPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_U32 stride_Y = 0;
    OMX_TI_PARAM_COMPONENTBUFALLOCTYPE *bufferalloc = NULL;
    int size = 0;
    MEMPLUGIN_ION_PARAMS *pIonParams;
    MEMPLUGIN_OBJECT	*pMemPluginHdl;

	DOMX_ENTER("%s: ENTERING",__FUNCTION__);
    pCompPrv = (PROXY_COMPONENT_PRIVATE *)hComp->pComponentPrivate;
    pCamPrv = (OMX_PROXY_CAM_PRIVATE*)pCompPrv->pCompProxyPrv;
    pMemPluginHdl = ((MEMPLUGIN_OBJECT *)pCompPrv->pMemPluginHandle);
    pIonParams = ((MEMPLUGIN_ION_PARAMS *)pMemPluginHdl->pPluginExtendedInfo);
    MEMPLUGIN_BUFFER_PARAMS_INIT(newBuffer_params);
    MEMPLUGIN_BUFFER_PARAMS_INIT(delBuffer_params);
    switch (nParamIndex)
    {
	case OMX_TI_IndexParamComponentBufferAllocation: {
                OMX_U32 port = 0, index = 0;
		int fd;
		bufferalloc = (OMX_TI_PARAM_COMPONENTBUFALLOCTYPE *)
			pComponentParameterStructure;

                port = bufferalloc->nPortIndex;
                index = bufferalloc->nIndex;

		newBuffer_params.nWidth = bufferalloc->nAllocWidth * bufferalloc->nAllocLines;
		newBuffer_params.eBuffer_type = TILER1D;
		newBuffer_params.eTiler_format = MEMPLUGIN_TILER_FORMAT_PAGE;
		if(pIonParams == NULL)
		{
			pIonParams = TIMM_OSAL_MallocExtn(sizeof(MEMPLUGIN_ION_PARAMS), TIMM_OSAL_TRUE,
                                      0, TIMMOSAL_MEM_SEGMENT_EXT, NULL);
			if(pIonParams == NULL)
			{
				DOMX_ERROR("%s:Error allocating pPluginExtendedInfo",__FUNCTION__);
				goto EXIT;
			}
                        pMemPluginHdl->pPluginExtendedInfo = pIonParams;
		}
		MEMPLUGIN_ION_PARAMS_INIT(pIonParams);
                //override alloc_flags for tiler 1d non secure
		pIonParams->alloc_flags = OMAP_ION_HEAP_TILER_MASK;

		eMemError = MemPlugin_Alloc(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&newBuffer_params,&newBuffer_prop);
		if(eMemError != MEMPLUGIN_ERROR_NONE)
		{
			DOMX_ERROR("%s:allocation failed size: %d",newBuffer_params.nWidth*newBuffer_params.nHeight);
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}
		bufferalloc->pBuf[0] = (OMX_PTR)newBuffer_prop.sBuffer_accessor.bufferFd;
		eError = __PROXY_SetParameter(hComponent,
					      OMX_TI_IndexParamComponentBufferAllocation,
					      bufferalloc, &bufferalloc->pBuf[0], 1);
                if (eError != OMX_ErrorNone) {
                   MemPlugin_Free(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc, &newBuffer_params,&newBuffer_prop);
                } else {
                   if (pCamPrv->gComponentBufferAllocation[port][index]) {
					   delBuffer_prop.sBuffer_accessor.pBufferHandle = pCamPrv->gComponentBufferAllocation[port][index];
                       MemPlugin_Free(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&delBuffer_params,&delBuffer_prop);
                   }
                   pCamPrv->gComponentBufferAllocation[port][index] = newBuffer_prop.sBuffer_accessor.pBufferHandle;
                }
		close (newBuffer_prop.sBuffer_accessor.bufferFd);
		newBuffer_prop.sBuffer_accessor.bufferFd = -1;
        }
		goto EXIT;
		break;
	default:
		 break;
	}
EXIT:
	pMemPluginHdl->pPluginExtendedInfo = NULL;
	if (eError != OMX_ErrorNone) {
		DOMX_ERROR("%s: Error  0x%x",__FUNCTION__, eError);
	}
    return eError;

	}

OMX_ERRORTYPE GLUE_CameraVtcAllocateMemory(OMX_IN OMX_HANDLETYPE hComponent, OMX_TI_PARAM_VTCSLICE *pVtcConfig,
											OMX_U32 nFrmWidth, OMX_U32 nFrmHeight)
{
	OMX_U32 i;
	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
    PROXY_COMPONENT_PRIVATE *pCompPrv;
    OMX_PROXY_CAM_PRIVATE* pCamPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	MEMPLUGIN_BUFFER_PARAMS newBuffer_params,delBuffer_params;
	MEMPLUGIN_BUFFER_PROPERTIES newBuffer_prop,delBuffer_prop;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	MEMPLUGIN_ERRORTYPE eMemError = MEMPLUGIN_ERROR_NONE;
	MEMPLUGIN_ION_PARAMS *pIonParams;
    MEMPLUGIN_OBJECT	*pMemPluginHdl;

    pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;
    pCamPrv = (OMX_PROXY_CAM_PRIVATE*)pCompPrv->pCompProxyPrv;
    pMemPluginHdl = ((MEMPLUGIN_OBJECT *)pCompPrv->pMemPluginHandle);
    pIonParams = ((MEMPLUGIN_ION_PARAMS *)pMemPluginHdl->pPluginExtendedInfo);

	MEMPLUGIN_BUFFER_PARAMS_INIT(newBuffer_params);
	MEMPLUGIN_BUFFER_PARAMS_INIT(delBuffer_params);
	for(i=0; i < MAX_NUM_INTERNAL_BUFFERS; i++) {
        pVtcConfig->nInternalBuffers = i;
        newBuffer_params.nWidth = nFrmWidth;
        newBuffer_params.nHeight = nFrmHeight;
        newBuffer_params.eBuffer_type = TILER1D;
        newBuffer_params.eTiler_format = MEMPLUGIN_TILER_FORMAT_8BIT;

		if(pIonParams == NULL)
		{
			pIonParams = TIMM_OSAL_MallocExtn(sizeof(MEMPLUGIN_ION_PARAMS), TIMM_OSAL_TRUE,
                                      0, TIMMOSAL_MEM_SEGMENT_EXT, NULL);
			if(pIonParams == NULL)
			{
				DOMX_ERROR("%s:Error allocating pPluginExtendedInfo",__FUNCTION__);
				goto EXIT;
			}
                        pMemPluginHdl->pPluginExtendedInfo = pIonParams;
		}
		MEMPLUGIN_ION_PARAMS_INIT(pIonParams);
                //override alloc_flags for tiler 1d non secure
		pIonParams->alloc_flags = OMAP_ION_HEAP_TILER_MASK;
		eMemError = MemPlugin_Alloc(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&newBuffer_params,&newBuffer_prop);
		if(eMemError != MEMPLUGIN_ERROR_NONE)
		{
			DOMX_ERROR("%s:allocation failed size: %d",newBuffer_params.nWidth*newBuffer_params.nHeight);
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}
        eRPCError = RPC_RegisterBuffer(pCompPrv->hRemoteComp, newBuffer_prop.sBuffer_accessor.bufferFd, -1,
                                       &pCamPrv->sInternalBuffers[i][0].pRegBufferHandle, NULL, IONPointers);
        PROXY_checkRpcError();
        pVtcConfig->IonBufhdl[0] = (OMX_PTR)pCamPrv->sInternalBuffers[i][0].pRegBufferHandle;
        pCamPrv->sInternalBuffers[i][0].pBufferHandle = newBuffer_prop.sBuffer_accessor.pBufferHandle;
        close (newBuffer_prop.sBuffer_accessor.bufferFd);
        newBuffer_prop.sBuffer_accessor.bufferFd = -1;
        DOMX_DEBUG(" DOMX: ION Buffer#%d: Y: 0x%x, eError = 0x%x, eRPCError = 0x%x\n", i, pVtcConfig->IonBufhdl[0], eError, eRPCError);

		MEMPLUGIN_BUFFER_PARAMS_INIT(newBuffer_params);
		newBuffer_params.nWidth = nFrmWidth/2;
        newBuffer_params.nHeight = nFrmHeight/2;
        newBuffer_params.eBuffer_type = TILER1D;
        newBuffer_params.eTiler_format = MEMPLUGIN_TILER_FORMAT_16BIT;
        pIonParams->alloc_flags = OMAP_ION_HEAP_TILER_MASK;
		eMemError = MemPlugin_Alloc(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&newBuffer_params,&newBuffer_prop);

		if(eMemError != MEMPLUGIN_ERROR_NONE)
		{
			DOMX_ERROR("%s:allocation failed size: %d",newBuffer_params.nWidth*newBuffer_params.nHeight);
			eError = OMX_ErrorInsufficientResources;
			if (pCamPrv->sInternalBuffers[i][0].pRegBufferHandle != NULL) {
               eRPCError = RPC_UnRegisterBuffer(pCompPrv->hRemoteComp, pCamPrv->sInternalBuffers[i][0].pRegBufferHandle, NULL , IONPointers);
               PROXY_checkRpcError();
            }
            MEMPLUGIN_BUFFER_PARAMS_INIT(delBuffer_params);
            delBuffer_prop.sBuffer_accessor.pBufferHandle = pCamPrv->sInternalBuffers[i][0].pBufferHandle;
			MemPlugin_Free(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&delBuffer_params,&delBuffer_prop );
			pCamPrv->sInternalBuffers[i][0].pBufferHandle = NULL;
			goto EXIT;
		}

		eRPCError = RPC_RegisterBuffer(pCompPrv->hRemoteComp, newBuffer_prop.sBuffer_accessor.bufferFd,-1,
                                                           &pCamPrv->sInternalBuffers[i][1].pRegBufferHandle, NULL, IONPointers);
        PROXY_checkRpcError();
        pVtcConfig->IonBufhdl[1] = pCamPrv->sInternalBuffers[i][1].pRegBufferHandle;
        pCamPrv->sInternalBuffers[i][1].pBufferHandle = newBuffer_prop.sBuffer_accessor.pBufferHandle;
        close (newBuffer_prop.sBuffer_accessor.bufferFd);
        DOMX_DEBUG("DOMX: ION Buffer#%d: UV: 0x%x, eError: 0x%x eRPCError: 0x%x\n", i, pVtcConfig->IonBufhdl[1],eError,eRPCError);

        eError = __PROXY_SetParameter(hComponent,
                                      OMX_TI_IndexParamVtcSlice,
                                      pVtcConfig,
                                      pVtcConfig->IonBufhdl, 2);
        if (eError != OMX_ErrorNone) {
            DOMX_ERROR("DOMX: PROXY_SetParameter for OMX_TI_IndexParamVtcSlice completed with error 0x%x\n", eError);
            OMX_CameraVtcFreeMemory(hComponent);
            goto EXIT;
        }
	}
EXIT:
	pMemPluginHdl->pPluginExtendedInfo = NULL;
	if (eError != OMX_ErrorNone) {
		DOMX_ERROR("%s: Error  0x%x",__FUNCTION__, eError);
	}
    return eError;
}
