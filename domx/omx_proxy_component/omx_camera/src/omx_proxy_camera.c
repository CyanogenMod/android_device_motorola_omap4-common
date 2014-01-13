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
 *  @file  omx_proxy_camera.c
 *         This file contains methods that provides the functionality for
 *         the OpenMAX1.1 DOMX Framework Tunnel Proxy component.
 ******************************************************************************
 This is the proxy specific wrapper that passes the component name to the
 generic proxy init() The proxy wrapper also does some runtime/static time
 config on per proxy basis This is a thin wrapper that is called when
 componentiit() of the proxy is called  static OMX_ERRORTYPE PROXY_Wrapper_init
 (OMX_HANDLETYPE hComponent, OMX_PTR pAppData);
 this layer gets called first whenever a proxy s get handle is called
 ******************************************************************************
 *  @path WTSD_DucatiMMSW\omx\omx_il_1_x\omx_proxy_component\src
 *
 *  @rev 1.0
 */

/*==============================================================
 *! Revision History
 *! ============================
 *! 19-August-2009 B Ravi Kiran ravi.kiran@ti.com: Initial Version
 *! 20-April-2012 Phanish phanish.hs@ti.com: updated wrt Slice processing
 *================================================================*/

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
#include "omx_proxy_camera.h"

#ifdef USE_ION
#include <unistd.h>
#include <ion_ti/ion.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <errno.h>
#endif

/* Tiler heap resservation specific */
#define OMAP_ION_HEAP_TILER_ALLOCATION_MASK (1<<4)

/* Incase of multiple instance, making sure DCC is initialized only for
   first instance */
static OMX_S16 numofInstance = 0;
TIMM_OSAL_PTR cam_mutex = NULL;

/* ===========================================================================*/
/**
 * @name _OMX_CameraVtcFreeMemory
 * @brief Allocated buffers are freed in this api
 *
 * @param
 * @return OMX_ErrorNone = Successful
 */
/* ===========================================================================*/
OMX_ERRORTYPE OMX_CameraVtcFreeMemory(OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    PROXY_COMPONENT_PRIVATE *pCompPrv;
    OMX_PROXY_CAM_PRIVATE* pCamPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
    OMX_U32 i = 0;
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
    MEMPLUGIN_BUFFER_PARAMS delBuffer_params;
    MEMPLUGIN_BUFFER_PROPERTIES delBuffer_prop;

    pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;
    pCamPrv = (OMX_PROXY_CAM_PRIVATE*)pCompPrv->pCompProxyPrv;

    MEMPLUGIN_BUFFER_PARAMS_INIT(delBuffer_params);

    for(i=0; i < MAX_NUM_INTERNAL_BUFFERS; i++) {
        if (pCamPrv->sInternalBuffers[i][0].pBufferHandle != NULL) {
            eRPCError = RPC_UnRegisterBuffer(pCompPrv->hRemoteComp, pCamPrv->sInternalBuffers[i][0].pRegBufferHandle, NULL , IONPointers);
            if (eRPCError != RPC_OMX_ErrorNone) {
                DOMX_ERROR("%s: DOMX: Unexpected error occurred while Unregistering Y Buffer#%d: eRPCError = 0x%x", __func__, i, eRPCError);
            }
            delBuffer_prop.sBuffer_accessor.pBufferHandle =  pCamPrv->sInternalBuffers[i][0].pBufferHandle;
            MemPlugin_Free(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&delBuffer_params,&delBuffer_prop);
            pCamPrv->sInternalBuffers[i][0].pRegBufferHandle = NULL;
            pCamPrv->sInternalBuffers[i][0].pBufferHandle = NULL;
            DOMX_DEBUG("%s: DOMX: #%d Y Memory freed; eRPCError = 0x%x", __func__, i, eRPCError);
        }
        if (pCamPrv->sInternalBuffers[i][1].pBufferHandle != NULL) {
            eRPCError = RPC_UnRegisterBuffer(pCompPrv->hRemoteComp, pCamPrv->sInternalBuffers[i][1].pRegBufferHandle, NULL , IONPointers);
            if (eRPCError != RPC_OMX_ErrorNone) {
                DOMX_ERROR("%s: DOMX: Unexpected error occurred while Unregistering UV Buffer#%d: eRPCError = 0x%x", __func__, i, eRPCError);
            }
            delBuffer_prop.sBuffer_accessor.pBufferHandle =  pCamPrv->sInternalBuffers[i][1].pBufferHandle;
            MemPlugin_Free(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&delBuffer_params,&delBuffer_prop);
            pCamPrv->sInternalBuffers[i][1].pRegBufferHandle = NULL;
            pCamPrv->sInternalBuffers[i][1].pBufferHandle = NULL;
            DOMX_DEBUG("%s: DOMX: #%d UV Memory freed; eRPCError = 0x%x", __func__, i, eRPCError);
        }
    }

EXIT:
   DOMX_EXIT("eError: %d", eError);
   return eError;
}

/* ===========================================================================*/
/**
 * @name _OMX_CameraVtcAllocateMemory
 * @brief Allocate 2D buffers for intermediate output at the output of isp
 *
 * @param
 * @return OMX_ErrorNone = Successful
 */
/* ===========================================================================*/
static OMX_ERRORTYPE _OMX_CameraVtcAllocateMemory(OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
    OMX_STATETYPE tState= OMX_StateInvalid;
    PROXY_COMPONENT_PRIVATE *pCompPrv;
    OMX_PROXY_CAM_PRIVATE* pCamPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
    OMX_U32 i = 0;
    OMX_CONFIG_RECTTYPE tFrameDim;
    OMX_U32 nFrmWidth, nFrmHeight;
    OMX_TI_PARAM_VTCSLICE tVtcConfig;
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;

    pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;
    pCamPrv = (OMX_PROXY_CAM_PRIVATE*)pCompPrv->pCompProxyPrv;

    _PROXY_OMX_INIT_PARAM(&tFrameDim, OMX_CONFIG_RECTTYPE);
    _PROXY_OMX_INIT_PARAM(&tVtcConfig, OMX_TI_PARAM_VTCSLICE);

    /* Get the current state of the component */
    eError = OMX_GetState(hComponent, &tState);
    if(OMX_ErrorNone != eError)
    {
        DOMX_ERROR("%s: Error in fetching current state - %d", __func__, tState);
    }
    else
    {
        if (tState == OMX_StateLoaded) {
            DOMX_DEBUG("%s: Current state returned is %d", __func__, tState);

            if(OMX_GetParameter(hComponent, OMX_TI_IndexParamVtcSlice, &tVtcConfig) == OMX_ErrorNone) {
                if (tVtcConfig.nSliceHeight != 0 ) {
                    OMX_CONFIG_BOOLEANTYPE tVstabParam;
                    OMX_PARAM_VIDEONOISEFILTERTYPE tVnfParam;
                    OMX_TI_PARAM_VTCSLICE *pVtcConfig = &tVtcConfig;

                    tFrameDim.nPortIndex = PREVIEW_PORT; //Preview Port
                    if(OMX_GetParameter(hComponent, OMX_TI_IndexParam2DBufferAllocDimension, &tFrameDim) == OMX_ErrorNone){
                        DOMX_DEBUG("Acquired OMX_TI_IndexParam2DBufferAllocDimension data. nWidth = %d, nHeight = %d.\n\n", tFrameDim.nWidth, tFrameDim.nHeight);
                        nFrmWidth = tFrameDim.nWidth;
                        nFrmHeight = tFrameDim.nHeight;
                    }else {
                        DOMX_DEBUG("%s: No OMX_TI_IndexParam2DBufferAllocDimension data.\n\n", __func__);
                        nFrmWidth = MAX_VTC_WIDTH_WITH_VNF;
                        nFrmHeight = MAX_VTC_HEIGHT_WITH_VNF;
                    }

                    _PROXY_OMX_INIT_PARAM(&tVnfParam, OMX_PARAM_VIDEONOISEFILTERTYPE);
                    _PROXY_OMX_INIT_PARAM(&tVstabParam, OMX_CONFIG_BOOLEANTYPE);
                    eError = OMX_GetParameter(hComponent, OMX_IndexParamFrameStabilisation, &tVstabParam);
                    if(eError != OMX_ErrorNone) {
                        DOMX_ERROR("OMX_GetParameter for OMX_IndexParamFrameStabilisation returned error %x", eError);
                        goto EXIT;
                    }
                    tVnfParam.nPortIndex = PREVIEW_PORT;
                    eError = OMX_GetParameter(hComponent, OMX_IndexParamVideoNoiseFilter, &tVnfParam);
                    if(eError != OMX_ErrorNone) {
                        DOMX_ERROR("OMX_GetParameter for OMX_IndexParamVideoNoiseFilter returned error %x", eError);
                        goto EXIT;
                    }
                    DOMX_DEBUG(" Acquired OMX_TI_IndexParamVtcSlice data. nSliceHeight = %d, bVstabOn = %d, Vnfmode = %d, nWidth = %d, nHeight = %d.\n\n", tVtcConfig.nSliceHeight, tVstabParam.bEnabled, tVnfParam.eMode, nFrmWidth, nFrmHeight);
                    if (tVstabParam.bEnabled == OMX_FALSE && tVnfParam.eMode != OMX_VideoNoiseFilterModeOff) {
                        eError = GLUE_CameraVtcAllocateMemory(hComponent,
                                                              pVtcConfig,
                                                              nFrmWidth,
                                                              nFrmHeight);
                        if(eError != OMX_ErrorNone) {
                           DOMX_ERROR("Allocate Memory for vtc config returned error %x", eError);
                           goto EXIT;
                        }
                   }
                }
            }
        }
    }
EXIT:

   DOMX_EXIT("eError: %d", eError);
   return eError;
}

static OMX_ERRORTYPE ComponentPrivateDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	TIMM_OSAL_ERRORTYPE eOsalError = TIMM_OSAL_ERR_NONE;
	PROXY_COMPONENT_PRIVATE *pCompPrv;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	OMX_U32 i, j;
    OMX_PROXY_CAM_PRIVATE* pCamPrv;
    MEMPLUGIN_BUFFER_PARAMS delBuffer_params;
    MEMPLUGIN_BUFFER_PROPERTIES delBuffer_prop;
    RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;

        MEMPLUGIN_BUFFER_PARAMS_INIT(delBuffer_params);
	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;


        OMX_CameraVtcFreeMemory(hComponent);


    if(pCompPrv->pCompProxyPrv != NULL) {
        pCamPrv = (OMX_PROXY_CAM_PRIVATE*)pCompPrv->pCompProxyPrv;
        for (i = 0; i < PROXY_MAXNUMOFPORTS; i++) {
            for (j = 0; j < MAX_NUM_INTERNAL_BUFFERS; j++) {
                if (pCamPrv->gComponentBufferAllocation[i][j]) {
                    delBuffer_prop.sBuffer_accessor.pBufferHandle = pCamPrv->gComponentBufferAllocation[i][j];
                    MemPlugin_Free(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&delBuffer_params,&delBuffer_prop);
                }
                pCamPrv->gComponentBufferAllocation[i][j] = NULL;
            }
        }

        TIMM_OSAL_Free(pCompPrv->pCompProxyPrv);
        pCompPrv->pCompProxyPrv = NULL;
        pCamPrv = NULL;
    }

	eError = PROXY_ComponentDeInit(hComponent);

      EXIT:
	return eError;
}

static OMX_ERRORTYPE Camera_SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_COMMANDTYPE eCmd,
    OMX_IN OMX_U32 nParam, OMX_IN OMX_PTR pCmdData)

{
    OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
    PROXY_COMPONENT_PRIVATE *pCompPrv;
    OMX_PROXY_CAM_PRIVATE   *pCamPrv;
    MEMPLUGIN_BUFFER_PARAMS delBuffer_params;
    MEMPLUGIN_BUFFER_PROPERTIES delBuffer_prop;
    pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

    pCamPrv = (OMX_PROXY_CAM_PRIVATE*)pCompPrv->pCompProxyPrv;

    MEMPLUGIN_BUFFER_PARAMS_INIT(delBuffer_params);
    if ((eCmd == OMX_CommandStateSet) &&
        (nParam == (OMX_STATETYPE) OMX_StateIdle))
    {
        /* Allocate memory for Video VTC usecase, if applicable. */
        eError = _OMX_CameraVtcAllocateMemory(hComponent);
        if (eError != OMX_ErrorNone) {
            DOMX_ERROR("DOMX: _OMX_CameraVtcAllocateMemory completed with error 0x%x\n", eError);
            goto EXIT;
        }
    } else if (eCmd == OMX_CommandPortDisable) {
        int i, j;
        for (i = 0; i < PROXY_MAXNUMOFPORTS; i++) {
            if ((i == nParam) || (nParam == OMX_ALL)) {
                for (j = 0; j < MAX_NUM_INTERNAL_BUFFERS; j++) {
                     if (pCamPrv->gComponentBufferAllocation[i][j]) {
                     delBuffer_prop.sBuffer_accessor.pBufferHandle = pCamPrv->gComponentBufferAllocation[i][j];
                        MemPlugin_Free(pCompPrv->pMemPluginHandle, pCompPrv->nMemmgrClientDesc,
                                       &delBuffer_params,&delBuffer_prop);
                        pCamPrv->gComponentBufferAllocation[i][j] = NULL;
                    }
                }
            }
        }
    }

    if ((eCmd == OMX_CommandStateSet) &&
	(nParam == (OMX_STATETYPE) OMX_StateLoaded))
    {
        /* Clean up resources for Video VTC usecase. */
        OMX_CameraVtcFreeMemory(hComponent);
    }

    eError =
	PROXY_SendCommand(hComponent,eCmd,nParam,pCmdData);

EXIT:

   DOMX_EXIT("eError: %d", eError);
   return eError;
}

/* ===========================================================================*/
/**
 * @name CameraGetConfig()
 * @brief For some specific indices, buffer allocated on A9 side
 *        needs to be mapped and sent to Ducati.
 * @param
 * @return OMX_ErrorNone = Successful
 */
/* ===========================================================================*/

static OMX_ERRORTYPE CameraGetConfig(OMX_IN OMX_HANDLETYPE
    hComponent, OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR pComponentParameterStructure)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_TI_CONFIG_SHAREDBUFFER *pConfigSharedBuffer = NULL;
	OMX_PTR pTempSharedBuff = NULL;
	OMX_U32 status = 0;

	switch (nParamIndex)
	{
	case OMX_TI_IndexConfigAAAskipBuffer:
	case OMX_TI_IndexConfigCamCapabilities:
	case OMX_TI_IndexConfigExifTags:
	case OMX_TI_IndexConfigAlgoAreas:
	case OMX_TI_IndexConfigGammaTable:
        case OMX_TI_IndexConfigDynamicCameraDescriptor:
		pConfigSharedBuffer =
			(OMX_TI_CONFIG_SHAREDBUFFER *) pComponentParameterStructure;

		pTempSharedBuff = pConfigSharedBuffer->pSharedBuff;

		// TODO(XXX): Cache API is not yet available. Client needs to
		// allocate tiler buffer directly and assign to pSharedBuff.
		// Ptr allocated by MemMgr_Alloc in uncacheable so there
		// would be no need to cache API

		eError = __PROXY_GetConfig(hComponent,
								nParamIndex,
								pConfigSharedBuffer,
								&(pConfigSharedBuffer->pSharedBuff));

		PROXY_assert((eError == OMX_ErrorNone), eError,
		    "Error in GetConfig");

		pConfigSharedBuffer->pSharedBuff = pTempSharedBuff;

		goto EXIT;
		break;
	default:
		break;
	}

	return __PROXY_GetConfig(hComponent,
							nParamIndex,
							pComponentParameterStructure,
							NULL);

 EXIT:
	return eError;
}

/* ===========================================================================*/
/**
 * @name CameraSetConfig()
 * @brief For some specific indices, buffer allocated on A9 side needs to
 *        be mapped and sent to Ducati.
 * @param
 * @return OMX_ErrorNone = Successful
 */
/* ===========================================================================*/


static OMX_ERRORTYPE CameraSetConfig(OMX_IN OMX_HANDLETYPE
    hComponent, OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR pComponentParameterStructure)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_TI_CONFIG_SHAREDBUFFER *pConfigSharedBuffer = NULL;
	OMX_PTR pTempSharedBuff = NULL;
	OMX_U32 status = 0;

	switch (nParamIndex)
	{
	case OMX_TI_IndexConfigAAAskipBuffer:
	case OMX_TI_IndexConfigCamCapabilities:
	case OMX_TI_IndexConfigExifTags:
	case OMX_TI_IndexConfigAlgoAreas:
	case OMX_TI_IndexConfigGammaTable:
        case OMX_TI_IndexConfigDynamicCameraDescriptor:
		pConfigSharedBuffer =
			(OMX_TI_CONFIG_SHAREDBUFFER *)
			pComponentParameterStructure;

		pTempSharedBuff = pConfigSharedBuffer->pSharedBuff;

		// TODO(XXX): Cache API is not yet available. Client needs to
		// allocate tiler buffer directly and assign to pSharedBuff.
		// Ptr allocated by MemMgr_Alloc in uncacheable so there
		// would be no need to cache API

		eError = __PROXY_SetConfig(hComponent,
								nParamIndex,
								pConfigSharedBuffer,
								&(pConfigSharedBuffer->pSharedBuff));

		PROXY_assert((eError == OMX_ErrorNone), eError,
		    "Error in GetConfig");

		pConfigSharedBuffer->pSharedBuff = pTempSharedBuff;

		goto EXIT;
		break;
	default:
		break;
	}

	return __PROXY_SetConfig(hComponent,
							nParamIndex,
							pComponentParameterStructure,
							NULL);

 EXIT:
	return eError;
}

static OMX_ERRORTYPE CameraSetParam(OMX_IN OMX_HANDLETYPE
    hComponent, OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR pComponentParameterStructure)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    PROXY_COMPONENT_PRIVATE *pCompPrv;
    OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *)hComponent;
    pCompPrv = (PROXY_COMPONENT_PRIVATE *)hComp->pComponentPrivate;

    switch (nParamIndex)
    {
	case OMX_TI_IndexParamComponentBufferAllocation:
             eError = GLUE_CameraSetParam(hComponent,
                                          nParamIndex,
                                          pComponentParameterStructure);
		goto EXIT;
		break;
	default:
		 break;
	}
	eError = __PROXY_SetParameter(hComponent,
								nParamIndex,
								pComponentParameterStructure,
							NULL, 0);
EXIT:
	if (eError != OMX_ErrorNone) {
		DOMX_ERROR(" CameraSetParam: Error in SetParam 0x%x", eError);
	}
    return eError;
}

OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE hComponent)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_ERRORTYPE dcc_eError = OMX_ErrorNone;
	OMX_COMPONENTTYPE *pHandle = NULL;
	PROXY_COMPONENT_PRIVATE *pComponentPrivate;
    OMX_U32 i = 0, j = 0;
    OMX_PROXY_CAM_PRIVATE* pCamPrv;
	MEMPLUGIN_ERRORTYPE eMemError = MEMPLUGIN_ERROR_NONE;
	pHandle = (OMX_COMPONENTTYPE *) hComponent;
	TIMM_OSAL_ERRORTYPE eOsalError = TIMM_OSAL_ERR_NONE;
	DOMX_ENTER("_____________________INSIDE CAMERA PROXY"
	    "WRAPPER__________________________\n");
	pHandle->pComponentPrivate = (PROXY_COMPONENT_PRIVATE *)
	    TIMM_OSAL_Malloc(sizeof(PROXY_COMPONENT_PRIVATE),
	    TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_INT);

	pComponentPrivate =
	    (PROXY_COMPONENT_PRIVATE *) pHandle->pComponentPrivate;
	if (pHandle->pComponentPrivate == NULL)
	{
		DOMX_ERROR(" ERROR IN ALLOCATING PROXY COMPONENT"
		    "PRIVATE STRUCTURE");
		eError = OMX_ErrorInsufficientResources;
		goto EXIT;
	}
	TIMM_OSAL_Memset(pComponentPrivate, 0,
		sizeof(PROXY_COMPONENT_PRIVATE));

	pComponentPrivate->cCompName =
	    TIMM_OSAL_Malloc(MAX_COMPONENT_NAME_LENGTH * sizeof(OMX_U8),
	    TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_INT);
	/*Copying component Name - this will be picked up in the proxy common */
	assert(strlen(COMPONENT_NAME) + 1 < MAX_COMPONENT_NAME_LENGTH);
	TIMM_OSAL_Memcpy(pComponentPrivate->cCompName, COMPONENT_NAME,
	    strlen(COMPONENT_NAME) + 1);

        pComponentPrivate->pCompProxyPrv =
            (OMX_PROXY_CAM_PRIVATE *)
            TIMM_OSAL_Malloc(sizeof(OMX_PROXY_CAM_PRIVATE), TIMM_OSAL_TRUE,
            0, TIMMOSAL_MEM_SEGMENT_INT);

        PROXY_assert(pComponentPrivate->pCompProxyPrv != NULL,
            OMX_ErrorInsufficientResources,
            "Could not allocate memory for proxy component private data structure");
        pCamPrv = (OMX_PROXY_CAM_PRIVATE*)pComponentPrivate->pCompProxyPrv;
        TIMM_OSAL_Memset(pComponentPrivate->pCompProxyPrv, 0,
                sizeof(OMX_PROXY_CAM_PRIVATE));

	pComponentPrivate->bMapBuffers = OMX_TRUE;
	/*Calling Proxy Common Init() */
	eError = OMX_ProxyCommonInit(hComponent);
	if (eError != OMX_ErrorNone)
	{
		DOMX_ERROR("\Error in Initializing Proxy");
		TIMM_OSAL_Free(pComponentPrivate->cCompName);
		TIMM_OSAL_Free(pComponentPrivate);
		TIMM_OSAL_Free(pComponentPrivate->pCompProxyPrv);
		goto EXIT;
	}

        for (i = 0; i < PROXY_MAXNUMOFPORTS; i++) {
            for (j = 0; j < MAX_NUM_INTERNAL_BUFFERS; j++) {
                pCamPrv->gComponentBufferAllocation[i][j] = NULL;
            }
        }

	pHandle->ComponentDeInit = ComponentPrivateDeInit;
	pHandle->GetConfig = CameraGetConfig;
	pHandle->SetConfig = CameraSetConfig;
	pHandle->SendCommand = Camera_SendCommand;
	pHandle->SetParameter = CameraSetParam;

      EXIT:
	return eError;
}

/*===============================================================*/
/** @fn Cam_Setup : This function is called when the the OMX Camera library is
 *                  loaded. It creates a mutex, which is used during DCC_Init()
 */
/*===============================================================*/
void __attribute__ ((constructor)) Cam_Setup(void)
{
	TIMM_OSAL_ERRORTYPE eError = TIMM_OSAL_ERR_NONE;

	eError = TIMM_OSAL_MutexCreate(&cam_mutex);
	if (eError != TIMM_OSAL_ERR_NONE)
	{
		TIMM_OSAL_Error("Creation of default mutex failed");
	}
}


/*===============================================================*/
/** @fn Cam_Destroy : This function is called when the the OMX Camera library is
 *                    unloaded. It destroys the mutex which was created by
 *                    Core_Setup().
 *
 */
/*===============================================================*/
void __attribute__ ((destructor)) Cam_Destroy(void)
{
	TIMM_OSAL_ERRORTYPE eError = TIMM_OSAL_ERR_NONE;

	eError = TIMM_OSAL_MutexDelete(cam_mutex);
	if (eError != TIMM_OSAL_ERR_NONE)
	{
		TIMM_OSAL_Error("Destruction of default mutex failed");
	}
}
