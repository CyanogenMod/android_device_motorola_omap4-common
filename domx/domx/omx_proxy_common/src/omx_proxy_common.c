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
 *  @file  omx_proxy_common.c
 *         This file contains methods that provides the functionality for
 *         the OpenMAX1.1 DOMX Framework OMX Common Proxy .
 *
 *  @path \WTSD_DucatiMMSW\framework\domx\omx_proxy_common\src
 *
 *  @rev 1.0
 */

/*==============================================================
 *! Revision History
 *! ============================
 *! 29-Mar-2010 Abhishek Ranka : Revamped DOMX implementation
 *!
 *! 19-August-2009 B Ravi Kiran ravi.kiran@ti.com: Initial Version
 *================================================================*/

/* ------compilation control switches ----------------------------------------*/
#define TILER_BUFF
#define ALLOCATE_TILER_BUFFER_IN_PROXY
// This has been enabled enbled only in Android.mk
// #define ENABLE_GRALLOC_BUFFER
/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
/* ----- system and platform files ----------------------------*/
#include <string.h>

#include "timm_osal_memory.h"
#include "timm_osal_mutex.h"
#include "OMX_TI_Common.h"
#include "OMX_TI_Index.h"
#include "OMX_TI_Core.h"
/*-------program files ----------------------------------------*/
#include "omx_proxy_common.h"
#include "omx_rpc.h"
#include "omx_rpc_stub.h"
#include "omx_rpc_utils.h"
#include "OMX_TI_IVCommon.h"
#include "profile.h"

#ifdef ALLOCATE_TILER_BUFFER_IN_PROXY
#ifdef USE_ION
#include <unistd.h>
#include <ion_ti/ion.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <linux/rpmsg_omx.h>
#include <errno.h>
#endif
#endif

#ifdef  ENABLE_GRALLOC_BUFFERS
#include "native_handle.h"
#include "hal_public.h"
#endif

#ifdef TILER_BUFF
#define PortFormatIsNotYUV 0

static OMX_ERRORTYPE _RPC_IsProxyComponent(OMX_HANDLETYPE hComponent,
    OMX_BOOL * bIsProxy);

#endif

#ifdef ALLOCATE_TILER_BUFFER_IN_PROXY

static OMX_ERRORTYPE PROXY_UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE ** ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes, OMX_IN OMX_U8 * pBuffer);
#endif

#define LINUX_PAGE_SIZE 4096
#define MAXNAMESIZE 128

#define CORE_MAX 4
#define CORE_CHIRON 3
#define CORE_SYSM3 2
#define CORE_APPM3 1
#define CORE_TESLA 0

#define MAX_CORENAME_LENGTH 32
char Core_Array[][MAX_CORENAME_LENGTH] =
    { "TESLA", "DUCATI1", "DUCATI0", "CHIRON" };

/******************************************************************
 *   MACROS - LOCAL
 ******************************************************************/

#ifdef USE_ION


RPC_OMX_ERRORTYPE RPC_RegisterBuffer(OMX_HANDLETYPE hRPCCtx, int fd1, int fd2,
				     OMX_PTR *handle1, OMX_PTR *handle2,
				     PROXY_BUFFER_TYPE proxyBufferType)
{
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	int status;
	RPC_OMX_CONTEXT *pRPCCtx = (RPC_OMX_CONTEXT *) hRPCCtx;

	if ((fd1 < 0) || (handle1 ==  NULL) ||
	    (((proxyBufferType == GrallocPointers) || (proxyBufferType == BufferDescriptorVirtual2D)) && (handle2 ==  NULL))) {
		DOMX_ERROR("Either an invalid fd or a NULL handle was supplied");
		eRPCError = RPC_OMX_ErrorBadParameter;
		goto EXIT;
	}

    if(proxyBufferType == BufferDescriptorVirtual2D)
    {
        struct ion_fd_data ion_data;
        if(fd2 < 0)
        {
            DOMX_ERROR("Invalid fd supplied for second buffer component in BufferDescriptorVirtual2D");
	    eRPCError = RPC_OMX_ErrorBadParameter;
            goto EXIT;
        }
        ion_data.fd = fd1;
	ion_data.handle = NULL;
	status = ioctl(pRPCCtx->fd_omx, OMX_IOCIONREGISTER, &ion_data);
	if (status < 0) {
		DOMX_ERROR("RegisterBuffer ioctl call failed");
		eRPCError = RPC_OMX_ErrorInsufficientResources;
		goto EXIT;
	}
	if (ion_data.handle)
		*handle1 = ion_data.handle;
	else
	{
	    DOMX_ERROR("Registration failed - Invalid fd passed for Y buffer");
		eRPCError = RPC_OMX_ErrorBadParameter;
		goto EXIT;
	}
        ion_data.fd = fd2;
	ion_data.handle = NULL;
	status = ioctl(pRPCCtx->fd_omx, OMX_IOCIONREGISTER, &ion_data);
	if (status < 0) {
	   DOMX_ERROR("RegisterBuffer ioctl call failed");
	   eRPCError = RPC_OMX_ErrorInsufficientResources;
	   goto EXIT;
	   }
	if (ion_data.handle)
	    *handle2 = ion_data.handle;
	else	 {
	     DOMX_ERROR("Registration failed - Invalid fd passed for UV buffer");
	     eRPCError = RPC_OMX_ErrorBadParameter;
	     goto EXIT;
	}
    }
    else if(proxyBufferType == GrallocPointers)
    {
#ifdef ENABLE_GRALLOC_BUFFERS
		struct omx_pvr_data pvr_data;

		pvr_data.fd = fd1;
		memset(pvr_data.handles, 0x0, sizeof(pvr_data.handles));
		status = ioctl(pRPCCtx->fd_omx, OMX_IOCPVRREGISTER, &pvr_data);
		if (status < 0) {
			if (errno == ENOTTY) {
				DOMX_ERROR("OMX_IOCPVRREGISTER not supported with current kernel version");
			} else {
				DOMX_ERROR("RegisterBuffer ioctl call failed");
				eRPCError = RPC_OMX_ErrorInsufficientResources;
			}
			goto EXIT;
		}

		if (pvr_data.handles[0])
			*handle1 = pvr_data.handles[0];
		else
		{
				DOMX_ERROR("Registration failed - Invalid fd passed for gralloc - reg handle (Y) is NULL");
			    eRPCError = RPC_OMX_ErrorBadParameter;
			    goto EXIT;
		}
		if(pvr_data.num_handles > 1)
		{
			if (pvr_data.handles[1])
				*handle2 = pvr_data.handles[1];
			else
			{
				DOMX_ERROR("Registration failed - Invalid fd passed for gralloc - reg handle (UV) is NULL num_handles: %d",pvr_data.num_handles);
				eRPCError = RPC_OMX_ErrorBadParameter;
				goto EXIT;
			}
		}
		else
		{
			DOMX_DEBUG("Gralloc buffer has only one component");
			*handle2 = NULL;
		}
#else
 DOMX_ERROR("No Registerbuffer implementation for gralloc - macro mess up!");
#endif
    }
    else if(proxyBufferType == VirtualPointers || proxyBufferType == IONPointers || proxyBufferType == EncoderMetadataPointers)
    {
        struct ion_fd_data ion_data;
        ion_data.fd = fd1;
		ion_data.handle = NULL;
		status = ioctl(pRPCCtx->fd_omx, OMX_IOCIONREGISTER, &ion_data);
		if (status < 0) {
			DOMX_ERROR("RegisterBuffer ioctl call failed");
			eRPCError = RPC_OMX_ErrorInsufficientResources;
			goto EXIT;
		}
		if (ion_data.handle)
			*handle1 = ion_data.handle;
		else
		{
		    DOMX_ERROR("Registration failed - Invalid fd passed");
			eRPCError = RPC_OMX_ErrorBadParameter;
			goto EXIT;
		}
    }
    else
    {
		//invalid type
		 DOMX_ERROR("Invalid buffer type passed");
	     eRPCError = RPC_OMX_ErrorBadParameter;
		 goto EXIT;
     }
EXIT:
	return eRPCError;
}



RPC_OMX_ERRORTYPE RPC_UnRegisterBuffer(OMX_HANDLETYPE hRPCCtx, OMX_PTR handle1, OMX_PTR handle2, PROXY_BUFFER_TYPE proxyBufferType)
{
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	int status;
	struct ion_fd_data data;
	RPC_OMX_CONTEXT *pRPCCtx = (RPC_OMX_CONTEXT *) hRPCCtx;

	if ((handle1 ==  NULL) || ((proxyBufferType == BufferDescriptorVirtual2D) && (handle2 ==  NULL))) {
		eRPCError = RPC_OMX_ErrorBadParameter;
		goto EXIT;
	}
    if(proxyBufferType == BufferDescriptorVirtual2D || proxyBufferType == GrallocPointers)
    {
		data.handle = handle1;
		status = ioctl(pRPCCtx->fd_omx, OMX_IOCIONUNREGISTER, &data);
		if (status < 0) {
			DOMX_ERROR("UnregisterBuffer ioctl call failed for handle1: 0x%x",handle1);
			eRPCError = RPC_OMX_ErrorInsufficientResources;
                        //unregisterbuffer will proceed to unregister handle2 even if handle1 unregister ioctl call failed"
		}
		if(handle2 != NULL)
		{
			data.handle = handle2;
			status = ioctl(pRPCCtx->fd_omx, OMX_IOCIONUNREGISTER, &data);
			if (status < 0) {
				DOMX_ERROR("UnregisterBuffer ioctl call failed for handle2: 0x%x",handle2);
				eRPCError = RPC_OMX_ErrorInsufficientResources;
			}
		}
                if(eRPCError != RPC_OMX_ErrorNone)
                    goto EXIT;
    }
    else if(proxyBufferType == VirtualPointers || proxyBufferType == IONPointers || proxyBufferType == EncoderMetadataPointers)
    {
        data.handle = handle1;
		status = ioctl(pRPCCtx->fd_omx, OMX_IOCIONUNREGISTER, &data);
		if (status < 0) {
			DOMX_ERROR("UnregisterBuffer ioctl call failed");
			eRPCError = RPC_OMX_ErrorInsufficientResources;
			goto EXIT;
		}
    }
    else
    {
		 //invalid type
		 DOMX_ERROR("Invalid buffer type passed");
	     eRPCError = RPC_OMX_ErrorBadParameter;
		 goto EXIT;
    }


 EXIT:
	return eRPCError;
}
#endif

/* ===========================================================================*/
/**
 * @name PROXY_EventHandler()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE PROXY_EventHandler(OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2,
    OMX_PTR pEventData)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	OMX_PTR pTmpData = NULL;

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	PROXY_require((hComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter,
	    "This is fatal error, processing cant proceed - please debug");

	DOMX_ENTER
	    ("hComponent=%p, pCompPrv=%p, eEvent=%p, nData1=%p, nData2=%p, pEventData=%p",
	    hComponent, pCompPrv, eEvent, nData1, nData2, pEventData);

	switch (eEvent)
	{
#if 0	// This feature is currently not supported, so kept in if(0) to be supported in the future
	case OMX_TI_EventBufferRefCount:
		DOMX_DEBUG("Received Ref Count Event");
		/*nData1 will be pBufferHeader, nData2 will be present count. Need to find local
		   buffer header for nData1 which is remote buffer header */

		PROXY_assert((nData1 != 0), OMX_ErrorBadParameter,
		    "Received NULL buffer header from OMX component");

		/*find local buffer header equivalent */
		for (count = 0; count < pCompPrv->nTotalBuffers; ++count)
		{
			if (pCompPrv->tBufList[count].pBufHeaderRemote ==
			    nData1)
			{
				pLocalBufHdr =
				    pCompPrv->tBufList[count].pBufHeader;
				pLocalBufHdr->pBuffer =
				    (OMX_U8 *) pCompPrv->tBufList[count].
				    pBufferActual;
				break;
			}
		}
		PROXY_assert((count != pCompPrv->nTotalBuffers),
		    OMX_ErrorBadParameter,
		    "Received invalid-buffer header from OMX component");

		/*update local buffer header */
		nData1 = (OMX_U32) pLocalBufHdr;
		break;
#endif
	case OMX_EventMark:
		DOMX_DEBUG("Received Mark Event");
		PROXY_assert((pEventData != NULL), OMX_ErrorUndefined,
		    "MarkData corrupted");
		pTmpData = pEventData;
		pEventData =
		    ((PROXY_MARK_DATA *) pEventData)->pMarkDataActual;
		TIMM_OSAL_Free(pTmpData);
		break;

	default:
		break;
	}

      EXIT:
	if (eError == OMX_ErrorNone)
	{
		pCompPrv->tCBFunc.EventHandler(hComponent,
		    pCompPrv->pILAppData, eEvent, nData1, nData2, pEventData);
	} else if (pCompPrv)
	{
		pCompPrv->tCBFunc.EventHandler(hComponent,
		    pCompPrv->pILAppData, OMX_EventError, eError, 0, NULL);
	}

	DOMX_EXIT("eError: %d", eError);
	return OMX_ErrorNone;
}

/* ===========================================================================*/
/**
 * @name PROXY_EmptyBufferDone()
 * @brief
 * @param
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_EmptyBufferDone(OMX_HANDLETYPE hComponent,
    OMX_U32 remoteBufHdr, OMX_U32 nfilledLen, OMX_U32 nOffset, OMX_U32 nFlags)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	OMX_U16 count;
	OMX_BUFFERHEADERTYPE *pBufHdr = NULL;

	PROXY_require((hComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter,
	    "This is fatal error, processing cant proceed - please debug");

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER
	    ("hComponent=%p, pCompPrv=%p, remoteBufHdr=%p, nFilledLen=%d, nOffset=%d, nFlags=%08x",
	    hComponent, pCompPrv, remoteBufHdr, nfilledLen, nOffset, nFlags);

	for (count = 0; count < pCompPrv->nTotalBuffers; ++count)
	{
		if (pCompPrv->tBufList[count].pBufHeaderRemote ==
		    remoteBufHdr)
		{
			pBufHdr = pCompPrv->tBufList[count].pBufHeader;
			pBufHdr->nFilledLen = nfilledLen;
			pBufHdr->nOffset = nOffset;
			pBufHdr->nFlags = nFlags;
			/* Setting mark info to NULL. This would always be
			   NULL in EBD, whether component has propagated the
			   mark or has generated mark event */
			pBufHdr->hMarkTargetComponent = NULL;
			pBufHdr->pMarkData = NULL;
			break;
		}
	}
	PROXY_assert((count != pCompPrv->nTotalBuffers),
	    OMX_ErrorBadParameter,
	    "Received invalid-buffer header from OMX component");

	KPI_OmxCompBufferEvent(KPI_BUFFER_EBD, hComponent, &(pCompPrv->tBufList[count]));

      EXIT:
	if (eError == OMX_ErrorNone)
	{
		pCompPrv->tCBFunc.EmptyBufferDone(hComponent,
		    pCompPrv->pILAppData, pBufHdr);
	} else if (pCompPrv)
	{
		pCompPrv->tCBFunc.EventHandler(hComponent,
		    pCompPrv->pILAppData, OMX_EventError, eError, 0, NULL);
	}

	DOMX_EXIT("eError: %d", eError);
	return OMX_ErrorNone;
}

/* ===========================================================================*/
/**
 * @name PROXY_FillBufferDone()
 * @brief
 * @param
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE PROXY_FillBufferDone(OMX_HANDLETYPE hComponent,
    OMX_U32 remoteBufHdr, OMX_U32 nfilledLen, OMX_U32 nOffset, OMX_U32 nFlags,
    OMX_TICKS nTimeStamp, OMX_HANDLETYPE hMarkTargetComponent,
    OMX_PTR pMarkData)
{

	OMX_ERRORTYPE eError = OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	OMX_U16 count;
	OMX_BUFFERHEADERTYPE *pBufHdr = NULL;

	PROXY_require((hComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter,
	    "This is fatal error, processing cant proceed - please debug");

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER
	    ("hComponent=%p, pCompPrv=%p, remoteBufHdr=%p, nFilledLen=%d, nOffset=%d, nFlags=%08x",
	    hComponent, pCompPrv, remoteBufHdr, nfilledLen, nOffset, nFlags);

	for (count = 0; count < pCompPrv->nTotalBuffers; ++count)
	{
		if (pCompPrv->tBufList[count].pBufHeaderRemote ==
		    remoteBufHdr)
		{
			pBufHdr = pCompPrv->tBufList[count].pBufHeader;
			pBufHdr->nFilledLen = nfilledLen;
			pBufHdr->nOffset = nOffset;
			pBufHdr->nFlags = nFlags;
			pBufHdr->nTimeStamp = nTimeStamp;
			if (pMarkData != NULL)
			{
				/*Update mark info in the buffer header */
				pBufHdr->pMarkData =
				    ((PROXY_MARK_DATA *)
				    pMarkData)->pMarkDataActual;
				pBufHdr->hMarkTargetComponent =
				    ((PROXY_MARK_DATA *)
				    pMarkData)->hComponentActual;
				TIMM_OSAL_Free(pMarkData);
			}
			break;
		}
	}
	PROXY_assert((count != pCompPrv->nTotalBuffers),
	    OMX_ErrorBadParameter,
	    "Received invalid-buffer header from OMX component");

	KPI_OmxCompBufferEvent(KPI_BUFFER_FBD, hComponent, &(pCompPrv->tBufList[count]));

      EXIT:
	if (eError == OMX_ErrorNone)
	{
		pCompPrv->tCBFunc.FillBufferDone(hComponent,
		    pCompPrv->pILAppData, pBufHdr);
	} else if (pCompPrv)
	{
		pCompPrv->tCBFunc.EventHandler(hComponent,
		    pCompPrv->pILAppData, OMX_EventError, eError, 0, NULL);
	}

	DOMX_EXIT("eError: %d", eError);
	return OMX_ErrorNone;
}

/* ===========================================================================*/
/**
 * @name PROXY_EmptyThisBuffer()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE PROXY_EmptyThisBuffer(OMX_HANDLETYPE hComponent,
    OMX_BUFFERHEADERTYPE * pBufferHdr)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	OMX_U32 count = 0;
	OMX_COMPONENTTYPE *pMarkComp = NULL;
	PROXY_COMPONENT_PRIVATE *pMarkCompPrv = NULL;
	OMX_PTR pMarkData = NULL;
	OMX_BOOL bFreeMarkIfError = OMX_FALSE;
	OMX_BOOL bIsProxy = OMX_FALSE , bMapBuffer;

	PROXY_require(pBufferHdr != NULL, OMX_ErrorBadParameter, NULL);
	PROXY_require(hComp->pComponentPrivate != NULL, OMX_ErrorBadParameter,
	    NULL);
	PROXY_CHK_VERSION(pBufferHdr, OMX_BUFFERHEADERTYPE);

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER
	    ("hComponent=%p, pCompPrv=%p, nFilledLen=%d, nOffset=%d, nFlags=%08x",
	    hComponent, pCompPrv, pBufferHdr->nFilledLen,
	    pBufferHdr->nOffset, pBufferHdr->nFlags);

	/*First find the index of this buffer header to retrieve remote buffer header */
	for (count = 0; count < pCompPrv->nTotalBuffers; count++)
	{
		if (pCompPrv->tBufList[count].pBufHeader == pBufferHdr)
		{
			DOMX_DEBUG("Buffer Index of Match %d ", count);
			break;
		}
	}
	PROXY_assert((count != pCompPrv->nTotalBuffers),
	    OMX_ErrorBadParameter,
	    "Could not find the remote header in buffer list");

	if (pBufferHdr->hMarkTargetComponent != NULL)
	{
		pMarkComp =
		    (OMX_COMPONENTTYPE *) (pBufferHdr->hMarkTargetComponent);
		/* Check is mark comp is proxy */
		eError = _RPC_IsProxyComponent(pMarkComp, &bIsProxy);
		PROXY_assert(eError == OMX_ErrorNone, eError, "");

		/*Replacing original mark data with proxy specific structure */
		pMarkData = pBufferHdr->pMarkData;
		pBufferHdr->pMarkData =
		    TIMM_OSAL_Malloc(sizeof(PROXY_MARK_DATA), TIMM_OSAL_TRUE,
		    0, TIMMOSAL_MEM_SEGMENT_INT);
		PROXY_assert(pBufferHdr->pMarkData != NULL,
		    OMX_ErrorInsufficientResources, "Malloc failed");
		bFreeMarkIfError = OMX_TRUE;
		((PROXY_MARK_DATA *) (pBufferHdr->
			pMarkData))->hComponentActual = pMarkComp;
		((PROXY_MARK_DATA *) (pBufferHdr->
			pMarkData))->pMarkDataActual = pMarkData;

		/* If proxy comp then replace hMarkTargetComponent with remote
		   handle */
		if (bIsProxy)
		{
			pMarkCompPrv = pMarkComp->pComponentPrivate;
			PROXY_assert(pMarkCompPrv != NULL,
			    OMX_ErrorBadParameter, NULL);

			/* Replacing with remote component handle */
			pBufferHdr->hMarkTargetComponent =
			    ((RPC_OMX_CONTEXT *) pMarkCompPrv->hRemoteComp)->
			    hActualRemoteCompHandle;
		}
	}

	bMapBuffer =
		pCompPrv->proxyPortBuffers[pBufferHdr->nInputPortIndex].proxyBufferType ==
			EncoderMetadataPointers;

	KPI_OmxCompBufferEvent(KPI_BUFFER_ETB, hComponent, &(pCompPrv->tBufList[count]));

	eRPCError =
	    RPC_EmptyThisBuffer(pCompPrv->hRemoteComp, pBufferHdr,
	    pCompPrv->tBufList[count].pBufHeaderRemote, &eCompReturn,bMapBuffer);

	PROXY_checkRpcError();

      EXIT:
	/*If ETB is about to return an error then this means that buffer has not
	   been accepted by the component. Thus the allocated mark data will be
	   lost so free it here. Also replace original mark data in the header */
	if ((eError != OMX_ErrorNone) && bFreeMarkIfError)
	{
		pBufferHdr->hMarkTargetComponent =
		    ((PROXY_MARK_DATA *) (pBufferHdr->
			pMarkData))->hComponentActual;
		pMarkData =
		    ((PROXY_MARK_DATA *) (pBufferHdr->
			pMarkData))->pMarkDataActual;
		TIMM_OSAL_Free(pBufferHdr->pMarkData);
		pBufferHdr->pMarkData = pMarkData;
	}

	DOMX_EXIT("eError: %d", eError);
	return eError;
}


/* ===========================================================================*/
/**
 * @name PROXY_FillThisBuffer()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE PROXY_FillThisBuffer(OMX_HANDLETYPE hComponent,
    OMX_BUFFERHEADERTYPE * pBufferHdr)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	OMX_U32 count = 0;

	PROXY_require(pBufferHdr != NULL, OMX_ErrorBadParameter, NULL);
	PROXY_require(hComp->pComponentPrivate != NULL, OMX_ErrorBadParameter,
	    NULL);
	PROXY_CHK_VERSION(pBufferHdr, OMX_BUFFERHEADERTYPE);

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER
	    ("hComponent = %p, pCompPrv = %p, nFilledLen = %d, nOffset = %d, nFlags = %08x",
	    hComponent, pCompPrv, pBufferHdr->nFilledLen,
	    pBufferHdr->nOffset, pBufferHdr->nFlags);

	/*First find the index of this buffer header to retrieve remote buffer header */
	for (count = 0; count < pCompPrv->nTotalBuffers; count++)
	{
		if (pCompPrv->tBufList[count].pBufHeader == pBufferHdr)
		{
			DOMX_DEBUG("Buffer Index of Match %d ", count);
			break;
		}
	}
	PROXY_assert((count != pCompPrv->nTotalBuffers),
	    OMX_ErrorBadParameter,
	    "Could not find the remote header in buffer list");

	KPI_OmxCompBufferEvent(KPI_BUFFER_FTB, hComponent, &(pCompPrv->tBufList[count]));

	eRPCError = RPC_FillThisBuffer(pCompPrv->hRemoteComp, pBufferHdr,
	    pCompPrv->tBufList[count].pBufHeaderRemote, &eCompReturn);

	PROXY_checkRpcError();

      EXIT:
	DOMX_EXIT("eError: %d", eError);
	return eError;
}


/* ===========================================================================*/
/**
 * @name PROXY_AllocateBuffer()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE PROXY_AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE ** ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate, OMX_IN OMX_U32 nSizeBytes)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	OMX_U32 currentBuffer = 0, i = 0;
	OMX_BOOL bSlotFound = OMX_FALSE;
	MEMPLUGIN_BUFFER_PARAMS newbuffer_params;
	MEMPLUGIN_BUFFER_PROPERTIES newbuffer_prop;
	MEMPLUGIN_ERRORTYPE eMemError = MEMPLUGIN_ERROR_NONE;
#ifdef ALLOCATE_TILER_BUFFER_IN_PROXY
	// Do the tiler allocations in Proxy and call use buffers on Ducati.

	//Round Off the size to allocate and map to next page boundary.
	OMX_U32 nSize = (nSizeBytes + LINUX_PAGE_SIZE - 1) & ~(LINUX_PAGE_SIZE - 1);
	OMX_U8* pMemptr = NULL;
	OMX_CONFIG_RECTTYPE tParamRect;
	OMX_PARAM_PORTDEFINITIONTYPE tParamPortDef;

	PROXY_require((hComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter, NULL);
	PROXY_require(ppBufferHdr != NULL, OMX_ErrorBadParameter,
	    "Pointer to buffer header is NULL");

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER
	    ("hComponent = %p, pCompPrv = %p, nPortIndex = %p, pAppPrivate = %p, nSizeBytes = %d",
	    hComponent, pCompPrv, nPortIndex, pAppPrivate, nSizeBytes);

	/*Pick up 1st empty slot */
	/*The same empty spot will be picked up by the subsequent
	Use buffer call to fill in the corresponding buffer
	Buffer header in the list */

	bSlotFound = OMX_FALSE;
	for (i = 0; i < pCompPrv->nTotalBuffers; i++)
	{
		if (pCompPrv->tBufList[i].pBufHeader == NULL)
		{
			currentBuffer = i;
			bSlotFound = OMX_TRUE;
			break;
		}
	}

	if (bSlotFound == OMX_FALSE)
	{
		currentBuffer = pCompPrv->nTotalBuffers;
	}

		MEMPLUGIN_BUFFER_PARAMS_INIT(newbuffer_params);
		newbuffer_params.nWidth = nSize;
		newbuffer_params.bMap = pCompPrv->bMapBuffers;
		eMemError = MemPlugin_Alloc(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&newbuffer_params,&newbuffer_prop);
		if(eMemError != MEMPLUGIN_ERROR_NONE)
		{
			DOMX_ERROR("Allocation failed %d",eMemError);
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}
		pCompPrv->tBufList[currentBuffer].bufferAccessors[0].pBufferHandle = newbuffer_prop.sBuffer_accessor.pBufferHandle;
		pCompPrv->tBufList[currentBuffer].bufferAccessors[0].pBufferMappedAddress = newbuffer_prop.sBuffer_accessor.pBufferMappedAddress;
		pCompPrv->tBufList[currentBuffer].bufferAccessors[0].bufferFd = newbuffer_prop.sBuffer_accessor.bufferFd;
		pMemptr = pCompPrv->tBufList[currentBuffer].bufferAccessors[0].bufferFd;
		DOMX_DEBUG ("Ion handle recieved = %x",pCompPrv->tBufList[currentBuffer].bufferAccessors[0].pBufferHandle);

	/*No need to increment Allocated buffers here.
	It will be done in the subsequent use buffer call below*/
	eError = PROXY_UseBuffer(hComponent, ppBufferHdr, nPortIndex, pAppPrivate, nSize, pMemptr);

	if(eError != OMX_ErrorNone) {
		DOMX_ERROR("PROXY_UseBuffer in PROXY_AllocateBuffer failed with error %d (0x%08x)", eError, eError);
		MemPlugin_Free(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&newbuffer_params,&newbuffer_prop);
		goto EXIT;
	}
	if (pCompPrv->bMapBuffers == OMX_TRUE)
	{
		DOMX_DEBUG("before mapping, handle = %x, nSize = %d",pCompPrv->tBufList[currentBuffer].bufferAccessors[0].pBufferHandle,nSize);
		(*ppBufferHdr)->pBuffer =  pCompPrv->tBufList[currentBuffer].bufferAccessors[0].pBufferMappedAddress;
	} else {
		(*ppBufferHdr)->pBuffer = pCompPrv->tBufList[currentBuffer].bufferAccessors[0].pBufferHandle;
	}


#else
	//This code is the un-changed version of original implementation.
	OMX_BUFFERHEADERTYPE *pBufferHeader = NULL;
	OMX_U32 pBufHeaderRemote = 0;
	OMX_U32 currentBuffer = 0, i = 0;
	OMX_U8 *pBuffer = NULL;
	OMX_TI_PLATFORMPRIVATE *pPlatformPrivate = NULL;
	OMX_BOOL bSlotFound = OMX_FALSE;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	OMX_ERRORTYPE eCompReturn = OMX_ErrorNone;

	PROXY_require((hComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter, NULL);
	PROXY_require(ppBufferHdr != NULL, OMX_ErrorBadParameter,
	    "Pointer to buffer header is NULL");

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER
	    ("hComponent = %p, pCompPrv = %p, nPortIndex = %p, pAppPrivate = %p, nSizeBytes = %d",
	    hComponent, pCompPrv, nPortIndex, pAppPrivate, nSizeBytes);

	/*Pick up 1st empty slot */
	for (i = 0; i < pCompPrv->nTotalBuffers; i++)
	{
		if (pCompPrv->tBufList[i].pBufHeader == 0)
		{
			currentBuffer = i;
			bSlotFound = OMX_TRUE;
			break;
		}
	}
	if (!bSlotFound)
	{
		currentBuffer = pCompPrv->nTotalBuffers;
	}

	DOMX_DEBUG("In AB, no. of buffers = %d", pCompPrv->nTotalBuffers);
	PROXY_assert((pCompPrv->nTotalBuffers < MAX_NUM_PROXY_BUFFERS),
	    OMX_ErrorInsufficientResources,
	    "Proxy cannot handle more than MAX buffers");

	//Allocating Local bufferheader to be maintained locally within proxy
	pBufferHeader =
	    (OMX_BUFFERHEADERTYPE *)
	    TIMM_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE), TIMM_OSAL_TRUE, 0,
	    TIMMOSAL_MEM_SEGMENT_INT);
	PROXY_assert((pBufferHeader != NULL), OMX_ErrorInsufficientResources,
	    "Allocation of Buffer Header structure failed");

	pPlatformPrivate =
	    (OMX_TI_PLATFORMPRIVATE *)
	    TIMM_OSAL_Malloc(sizeof(OMX_TI_PLATFORMPRIVATE), TIMM_OSAL_TRUE,
	    0, TIMMOSAL_MEM_SEGMENT_INT);
	PROXY_assert(pPlatformPrivate != NULL, OMX_ErrorInsufficientResources,
	    "Allocation of platform private structure failed");
	pBufferHeader->pPlatformPrivate = pPlatformPrivate;

	DOMX_DEBUG(" Calling RPC ");

	eRPCError =
	    RPC_AllocateBuffer(pCompPrv->hRemoteComp, &pBufferHeader,
	    nPortIndex, &pBufHeaderRemote, pAppPrivate, nSizeBytes,
	    &eCompReturn);

	PROXY_checkRpcError();

	DOMX_DEBUG("Allocate Buffer Successful");
	DOMX_DEBUG("Value of pBufHeaderRemote: %p   LocalBufferHdr :%p",
	    pBufHeaderRemote, pBufferHeader);

	pCompPrv->tBufList[currentBuffer].pBufHeader = pBufferHeader;
	pCompPrv->tBufList[currentBuffer].pBufHeaderRemote = pBufHeaderRemote;


	//keeping track of number of Buffers
	pCompPrv->nAllocatedBuffers++;

	if (pCompPrv->nTotalBuffers < pCompPrv->nAllocatedBuffers)
	{
		pCompPrv->nTotalBuffers = pCompPrv->nAllocatedBuffers;
	}

	*ppBufferHdr = pBufferHeader;

      EXIT:
	if (eError != OMX_ErrorNone)
	{
		if (pPlatformPrivate)
			TIMM_OSAL_Free(pPlatformPrivate);
		if (pBufferHeader)
			TIMM_OSAL_Free(pBufferHeader);
	}
	DOMX_EXIT("eError: %d", eError);
	return eError;
#endif //ALLOCATE_TILER_BUFFER_IN_PROXY

      EXIT:
	DOMX_EXIT("eError: %d eMemError: %d", eError, eMemError);
	return eError;
}


/* ===========================================================================*/
/**
 * @name PROXY_UseBuffer()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE ** ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes, OMX_IN OMX_U8 * pBuffer)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	OMX_BUFFERHEADERTYPE *pBufferHeader = NULL;
	OMX_U32 pBufHeaderRemote = 0;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	OMX_U32 currentBuffer = 0, i = 0;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_TI_PLATFORMPRIVATE *pPlatformPrivate = NULL;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	OMX_BOOL bSlotFound = OMX_FALSE;
	OMX_PTR pAuxBuf0 = pBuffer;
	OMX_PTR pMappedMetaDataBuffer = NULL;
	OMX_TI_PARAM_METADATABUFFERINFO tMetaDataBuffer;
	OMX_U32 nBufferHeight = 0;
	OMX_CONFIG_RECTTYPE tParamRect;
	OMX_PARAM_PORTDEFINITIONTYPE tParamPortDef;
	MEMPLUGIN_BUFFER_PROPERTIES metadataBuffer_prop;
	MEMPLUGIN_BUFFER_PARAMS metadataBuffer_params;
	MEMPLUGIN_ERRORTYPE eMemError = MEMPLUGIN_ERROR_NONE;
	PROXY_require((hComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter, NULL);
	PROXY_require(pBuffer != NULL, OMX_ErrorBadParameter, "Pointer to buffer is NULL");
	PROXY_require(ppBufferHdr != NULL, OMX_ErrorBadParameter,
	    "Pointer to buffer header is NULL");

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER
	    ("hComponent = %p, pCompPrv = %p, nPortIndex = %p, pAppPrivate = %p, nSizeBytes = %d, pBuffer = %p",
	    hComponent, pCompPrv, nPortIndex, pAppPrivate, nSizeBytes,
	    pBuffer);

	/*Pick up 1st empty slot */
	for (i = 0; i < pCompPrv->nTotalBuffers; i++)
	{
		if (pCompPrv->tBufList[i].pBufHeader == 0)
		{
			currentBuffer = i;
			bSlotFound = OMX_TRUE;
			break;
		}
	}
	if (!bSlotFound)
	{
		currentBuffer = pCompPrv->nTotalBuffers;
	}
	DOMX_DEBUG("In UB, no. of buffers = %d", pCompPrv->nTotalBuffers);

	PROXY_assert((pCompPrv->nTotalBuffers < MAX_NUM_PROXY_BUFFERS),
	    OMX_ErrorInsufficientResources,
	    "Proxy cannot handle more than MAX buffers");

	//Allocating Local bufferheader to be maintained locally within proxy
	pBufferHeader =
	    (OMX_BUFFERHEADERTYPE *)
	    TIMM_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE), TIMM_OSAL_TRUE, 0,
	    TIMMOSAL_MEM_SEGMENT_INT);
	PROXY_assert((pBufferHeader != NULL), OMX_ErrorInsufficientResources,
	    "Allocation of Buffer Header structure failed");

	pPlatformPrivate =
	    (OMX_TI_PLATFORMPRIVATE *)
	    TIMM_OSAL_Malloc(sizeof(OMX_TI_PLATFORMPRIVATE), TIMM_OSAL_TRUE,
	    0, TIMMOSAL_MEM_SEGMENT_INT);
	PROXY_assert(pPlatformPrivate != NULL, OMX_ErrorInsufficientResources,
	    "Allocation of platform private structure failed");
	TIMM_OSAL_Memset(pPlatformPrivate, 0, sizeof(OMX_TI_PLATFORMPRIVATE));

	pBufferHeader->pPlatformPrivate = pPlatformPrivate;
	((OMX_TI_PLATFORMPRIVATE *)pBufferHeader->pPlatformPrivate)->nSize = sizeof(OMX_TI_PLATFORMPRIVATE);

#ifdef ENABLE_GRALLOC_BUFFERS
	if(pCompPrv->proxyPortBuffers[nPortIndex].proxyBufferType == GrallocPointers)
	{
		//Extracting buffer pointer from the gralloc buffer
		pAuxBuf0 = (OMX_U8 *)(((IMG_native_handle_t*)pBuffer)->fd[0]);
		if(((native_handle_t*)pBuffer)->numFds > 1) {
			((OMX_TI_PLATFORMPRIVATE *) pBufferHeader->pPlatformPrivate)->
				pAuxBuf1 = (OMX_U8 *)(((IMG_native_handle_t*)pBuffer)->fd[1]);
		}
		else {
			DOMX_DEBUG("Gralloc buffer has only one component");
			((OMX_TI_PLATFORMPRIVATE *) pBufferHeader->pPlatformPrivate)->
				pAuxBuf1 = NULL;
		}
	}
#endif

	DOMX_DEBUG("Preparing buffer to Remote Core...");
	pBufferHeader->pBuffer = pBuffer;

	if(pCompPrv->proxyPortBuffers[nPortIndex].proxyBufferType == EncoderMetadataPointers)
	{
		((OMX_TI_PLATFORMPRIVATE *) pBufferHeader->pPlatformPrivate)->
			pAuxBuf1 = NULL;
	}
	if(pCompPrv->proxyPortBuffers[nPortIndex].proxyBufferType == BufferDescriptorVirtual2D)
	{
		pAuxBuf0 = (OMX_U8 *)(((OMX_TI_BUFFERDESCRIPTOR_TYPE*)pBuffer)->pBuf[0]);

		((OMX_TI_PLATFORMPRIVATE *) pBufferHeader->pPlatformPrivate)->
			pAuxBuf1 = (OMX_U8 *)(((OMX_TI_BUFFERDESCRIPTOR_TYPE*)pBuffer)->pBuf[1]);
	}

	/*Initializing Structure */
	tMetaDataBuffer.nSize = sizeof(OMX_TI_PARAM_METADATABUFFERINFO);
	tMetaDataBuffer.nVersion.s.nVersionMajor = OMX_VER_MAJOR;
	tMetaDataBuffer.nVersion.s.nVersionMinor = OMX_VER_MINOR;
	tMetaDataBuffer.nVersion.s.nRevision = 0x0;
	tMetaDataBuffer.nVersion.s.nStep = 0x0;
	tMetaDataBuffer.nPortIndex = nPortIndex;
	eError = PROXY_GetParameter(hComponent, (OMX_INDEXTYPE)OMX_TI_IndexParamMetaDataBufferInfo, (OMX_PTR)&tMetaDataBuffer);
	PROXY_assert(eError == OMX_ErrorNone, eError,
	    "Get Parameter for Metadata infor failed");

	DOMX_DEBUG("Metadata size = %d",tMetaDataBuffer.nMetaDataSize);

	if(tMetaDataBuffer.bIsMetaDataEnabledOnPort)
	{
		((OMX_TI_PLATFORMPRIVATE *)pBufferHeader->pPlatformPrivate)->nMetaDataSize =
			(tMetaDataBuffer.nMetaDataSize + LINUX_PAGE_SIZE - 1) & ~(LINUX_PAGE_SIZE -1);
		MEMPLUGIN_BUFFER_PARAMS_INIT(metadataBuffer_params);
		metadataBuffer_params.nWidth = ((OMX_TI_PLATFORMPRIVATE *)pBufferHeader->pPlatformPrivate)->nMetaDataSize;
		metadataBuffer_params.bMap = OMX_TRUE;
		eMemError = MemPlugin_Alloc(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&metadataBuffer_params,&metadataBuffer_prop);
		if(eMemError != MEMPLUGIN_ERROR_NONE)
		{
			DOMX_ERROR("Allocation failed %d",eMemError);
			eError = OMX_ErrorInsufficientResources;
			goto EXIT;
		}
		((OMX_TI_PLATFORMPRIVATE *)pBufferHeader->
			pPlatformPrivate)->pMetaDataBuffer = metadataBuffer_prop.sBuffer_accessor.pBufferHandle;
		pCompPrv->tBufList[currentBuffer].bufferAccessors[2].pBufferHandle = metadataBuffer_prop.sBuffer_accessor.pBufferHandle;
		DOMX_DEBUG("Metadata buffer ion handle = %d",((OMX_TI_PLATFORMPRIVATE *)pBufferHeader->pPlatformPrivate)->pMetaDataBuffer);
		pCompPrv->tBufList[currentBuffer].bufferAccessors[2].pBufferMappedAddress = metadataBuffer_prop.sBuffer_accessor.pBufferMappedAddress;
		pCompPrv->tBufList[currentBuffer].bufferAccessors[2].bufferFd = metadataBuffer_prop.sBuffer_accessor.bufferFd;
	}

#ifdef USE_ION
	{
		// Need to register buffers when using ion and rpmsg
		eRPCError = RPC_RegisterBuffer(pCompPrv->hRemoteComp, pAuxBuf0,(int)((OMX_TI_PLATFORMPRIVATE *) pBufferHeader->pPlatformPrivate)->pAuxBuf1,
							&pCompPrv->tBufList[currentBuffer].bufferAccessors[0].pRegBufferHandle,&pCompPrv->tBufList[currentBuffer].bufferAccessors[1].pRegBufferHandle,
		                    pCompPrv->proxyPortBuffers[nPortIndex].proxyBufferType);
		 PROXY_checkRpcError();
		if (pCompPrv->tBufList[currentBuffer].bufferAccessors[0].pRegBufferHandle)
			pAuxBuf0 = pCompPrv->tBufList[currentBuffer].bufferAccessors[0].pRegBufferHandle;
		if (pCompPrv->tBufList[currentBuffer].bufferAccessors[1].pRegBufferHandle)
			pPlatformPrivate->pAuxBuf1 = pCompPrv->tBufList[currentBuffer].bufferAccessors[1].pRegBufferHandle;

		if (pPlatformPrivate->pMetaDataBuffer != NULL)
		{
			eRPCError = RPC_RegisterBuffer(pCompPrv->hRemoteComp, pCompPrv->tBufList[currentBuffer].bufferAccessors[2].bufferFd, -1,
					   &(pCompPrv->tBufList[currentBuffer].bufferAccessors[2].pRegBufferHandle), NULL, IONPointers);
			PROXY_checkRpcError();
			if (pCompPrv->tBufList[currentBuffer].bufferAccessors[2].pRegBufferHandle)
				pPlatformPrivate->pMetaDataBuffer = pCompPrv->tBufList[currentBuffer].bufferAccessors[2].pRegBufferHandle;
		}
	}
#endif

	eRPCError = RPC_UseBuffer(pCompPrv->hRemoteComp, &pBufferHeader, nPortIndex,
		pAppPrivate, nSizeBytes, pAuxBuf0, &pBufHeaderRemote, &eCompReturn);

	PROXY_checkRpcError();

	DOMX_DEBUG("Use Buffer Successful");
	DOMX_DEBUG
	    ("Value of pBufHeaderRemote: %p LocalBufferHdr :%p, LocalBuffer :%p",
	    pBufHeaderRemote, pBufferHeader, pBufferHeader->pBuffer);

	if (pCompPrv->bMapBuffers == OMX_TRUE && tMetaDataBuffer.bIsMetaDataEnabledOnPort)
	{
		((OMX_TI_PLATFORMPRIVATE *)pBufferHeader->pPlatformPrivate)->pMetaDataBuffer = pCompPrv->tBufList[currentBuffer].bufferAccessors[2].pBufferMappedAddress;
		//ion_free(pCompPrv->nMemmgrClientDesc, handleToMap);
		memset(((OMX_TI_PLATFORMPRIVATE *)pBufferHeader->pPlatformPrivate)->pMetaDataBuffer,
			0x0, tMetaDataBuffer.nMetaDataSize);
	}

	//Storing details of pBufferHeader/Mapped/Actual buffer address locally.
	pCompPrv->tBufList[currentBuffer].pBufHeader = pBufferHeader;
	pCompPrv->tBufList[currentBuffer].pBufHeaderRemote = pBufHeaderRemote;

	//keeping track of number of Buffers
	pCompPrv->nAllocatedBuffers++;
	if (pCompPrv->nTotalBuffers < pCompPrv->nAllocatedBuffers)
		pCompPrv->nTotalBuffers = pCompPrv->nAllocatedBuffers;

	DOMX_DEBUG("Updating no. of buffer to %d", pCompPrv->nTotalBuffers);

	*ppBufferHdr = pBufferHeader;

      EXIT:
	if (eError != OMX_ErrorNone)
	{
		if (pPlatformPrivate)
			TIMM_OSAL_Free(pPlatformPrivate);
		if (pBufferHeader)
			TIMM_OSAL_Free(pBufferHeader);
	}
	DOMX_EXIT("eError: %d", eError);
	return eError;
}

/* ===========================================================================*/
/**
 * @name PROXY_FreeBuffer()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE PROXY_FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_BUFFERHEADERTYPE * pBufferHdr)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone, eTmpRPCError =
	    RPC_OMX_ErrorNone;
	OMX_U32 count = 0;
	OMX_U32 pBuffer = 0;
	OMX_PTR pMetaDataBuffer = NULL;
	OMX_PTR pAuxBuf0 = NULL;
	OMX_TI_PLATFORMPRIVATE * pPlatformPrivate = NULL;
	MEMPLUGIN_BUFFER_PROPERTIES delBuffer_prop;
	MEMPLUGIN_BUFFER_PARAMS delBuffer_params;

	PROXY_require(pBufferHdr != NULL, OMX_ErrorBadParameter, NULL);
	PROXY_require(hComp->pComponentPrivate != NULL, OMX_ErrorBadParameter,
	    NULL);
	PROXY_CHK_VERSION(pBufferHdr, OMX_BUFFERHEADERTYPE);

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER
	    ("hComponent = %p, pCompPrv = %p, nPortIndex = %p, pBufferHdr = %p, pBuffer = %p",
	    hComponent, pCompPrv, nPortIndex, pBufferHdr,
	    pBufferHdr->pBuffer);

	for (count = 0; count < pCompPrv->nTotalBuffers; count++)
	{
		if (pCompPrv->tBufList[count].pBufHeader == pBufferHdr)
		{
			DOMX_DEBUG("Buffer Index of Match %d", count);
			break;
		}
	}
	PROXY_assert((count != pCompPrv->nTotalBuffers),
	    OMX_ErrorBadParameter,
	    "Could not find the mapped address in component private buffer list");

	pBuffer = (OMX_U32)pBufferHdr->pBuffer;
    pAuxBuf0 = (OMX_PTR) pBuffer;

#ifdef ENABLE_GRALLOC_BUFFERS
	if (pCompPrv->proxyPortBuffers[nPortIndex].proxyBufferType == GrallocPointers)
	{
		//Extracting buffer pointer from the gralloc buffer
		pAuxBuf0 = (OMX_U8 *)(((IMG_native_handle_t*)pBuffer)->fd[0]);
	}
#endif

	if (pCompPrv->proxyPortBuffers[nPortIndex].proxyBufferType == BufferDescriptorVirtual2D)
	{
		pAuxBuf0 = (OMX_U8 *)(((OMX_TI_BUFFERDESCRIPTOR_TYPE*)pBuffer)->pBuf[0]);
	}

	/*Not having asserts from this point since even if error occurs during
	   unmapping/freeing, still trying to clean up as much as possible */

	if (pCompPrv->tBufList[count].bufferAccessors[0].pRegBufferHandle != NULL)
		pAuxBuf0 = pCompPrv->tBufList[count].bufferAccessors[0].pRegBufferHandle;

	eRPCError =
	    RPC_FreeBuffer(pCompPrv->hRemoteComp, nPortIndex,
	    pCompPrv->tBufList[count].pBufHeaderRemote, (OMX_U32) pAuxBuf0,
	    &eCompReturn);

	if (eRPCError != RPC_OMX_ErrorNone)
		eTmpRPCError = eRPCError;

	pPlatformPrivate = (OMX_TI_PLATFORMPRIVATE *)(pCompPrv->tBufList[count].pBufHeader)->
			pPlatformPrivate;

	if (pCompPrv->tBufList[count].pBufHeader)
	{
#ifdef ALLOCATE_TILER_BUFFER_IN_PROXY
		if(pCompPrv->tBufList[count].bufferAccessors[0].pBufferHandle)
		{
				if(pBufferHdr->pBuffer)
				{
					MEMPLUGIN_BUFFER_PARAMS_INIT(delBuffer_params);
					delBuffer_prop.sBuffer_accessor.pBufferHandle = pCompPrv->tBufList[count].bufferAccessors[0].pBufferHandle;
					delBuffer_prop.sBuffer_accessor.pBufferMappedAddress = pCompPrv->tBufList[count].bufferAccessors[0].pBufferMappedAddress;
					delBuffer_prop.sBuffer_accessor.bufferFd = pCompPrv->tBufList[count].bufferAccessors[0].bufferFd;
					delBuffer_params.bMap = pCompPrv->bMapBuffers;
					delBuffer_params.nWidth = pBufferHdr->nAllocLen;
					MemPlugin_Free(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&delBuffer_params,&delBuffer_prop);
				}
				pCompPrv->tBufList[count].bufferAccessors[0].pBufferHandle = NULL;
				pCompPrv->tBufList[count].bufferAccessors[0].pBufferMappedAddress = NULL;
				pCompPrv->tBufList[count].bufferAccessors[0].bufferFd = -1;
		}
#endif
	}
		pMetaDataBuffer = pPlatformPrivate->pMetaDataBuffer;
		if (pMetaDataBuffer)
		{
				MEMPLUGIN_BUFFER_PARAMS_INIT(delBuffer_params);
				delBuffer_prop.sBuffer_accessor.pBufferHandle = pCompPrv->tBufList[count].bufferAccessors[2].pBufferHandle;
				delBuffer_prop.sBuffer_accessor.pBufferMappedAddress = pCompPrv->tBufList[count].bufferAccessors[2].pBufferMappedAddress;
				delBuffer_prop.sBuffer_accessor.bufferFd = pCompPrv->tBufList[count].bufferAccessors[2].bufferFd;
				delBuffer_params.bMap = pCompPrv->bMapBuffers;
				delBuffer_params.nWidth = pPlatformPrivate->nMetaDataSize;
				MemPlugin_Free(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&delBuffer_params,&delBuffer_prop);
				pPlatformPrivate->pMetaDataBuffer = NULL;
				pCompPrv->tBufList[count].bufferAccessors[2].pBufferHandle=NULL;
				pCompPrv->tBufList[count].bufferAccessors[2].pBufferMappedAddress = NULL;
				pCompPrv->tBufList[count].bufferAccessors[2].bufferFd = -1;
		}
#ifdef USE_ION
	{
		// Need to unregister buffers when using ion and rpmsg
		if (pCompPrv->tBufList[count].bufferAccessors[0].pRegBufferHandle != NULL)
		{
			eTmpRPCError = RPC_UnRegisterBuffer(pCompPrv->hRemoteComp,
								pCompPrv->tBufList[count].bufferAccessors[0].pRegBufferHandle,pCompPrv->tBufList[count].bufferAccessors[1].pRegBufferHandle,
								pCompPrv->proxyPortBuffers[nPortIndex].proxyBufferType);
			if (eTmpRPCError != RPC_OMX_ErrorNone) {
				eRPCError = eTmpRPCError;
			}
		}

		if (pCompPrv->tBufList[count].bufferAccessors[2].pRegBufferHandle != NULL)
		{
			eTmpRPCError = RPC_UnRegisterBuffer(pCompPrv->hRemoteComp,
								pCompPrv->tBufList[count].bufferAccessors[2].pRegBufferHandle, NULL, IONPointers);
			if (eTmpRPCError != RPC_OMX_ErrorNone) {
				eRPCError = eTmpRPCError;
			}
		}
	}
#endif
		if (pCompPrv->tBufList[count].pBufHeader->pPlatformPrivate)
		{
			TIMM_OSAL_Free(pCompPrv->tBufList[count].pBufHeader->
			    pPlatformPrivate);
		}
		TIMM_OSAL_Free(pCompPrv->tBufList[count].pBufHeader);
		TIMM_OSAL_Memset(&(pCompPrv->tBufList[count]), 0,
		    sizeof(PROXY_BUFFER_INFO));
	pCompPrv->nAllocatedBuffers--;

	PROXY_checkRpcError();

      EXIT:
	DOMX_EXIT("eError: %d", eError);
	return eError;
}


/* ===========================================================================*/
/**
 * @name PROXY_SetParameter()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE __PROXY_SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
	OMX_IN OMX_INDEXTYPE nParamIndex, OMX_IN OMX_PTR pParamStruct,
	OMX_PTR pLocBufNeedMap, OMX_U32 nNumOfLocalBuf)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	OMX_TI_PARAM_USEBUFFERDESCRIPTOR *ptBufDescParam = NULL;
#ifdef ENABLE_GRALLOC_BUFFERS
	OMX_TI_PARAMUSENATIVEBUFFER *pParamNativeBuffer = NULL;
#endif
#ifdef USE_ION
	OMX_PTR *pAuxBuf = pLocBufNeedMap;
	OMX_PTR pRegistered = NULL;
#endif

	PROXY_require((pParamStruct != NULL), OMX_ErrorBadParameter, NULL);
	PROXY_require((hComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter, NULL);

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER
		("hComponent = %p, pCompPrv = %p, nParamIndex = %d, pParamStruct = %p",
		hComponent, pCompPrv, nParamIndex, pParamStruct);

	switch(nParamIndex)
	{
#ifdef ENABLE_GRALLOC_BUFFERS
		case OMX_TI_IndexUseNativeBuffers:
		{
			//Add check version.
			pParamNativeBuffer = (OMX_TI_PARAMUSENATIVEBUFFER* )pParamStruct;
			if(pParamNativeBuffer->bEnable == OMX_TRUE)
			{
				pCompPrv->proxyPortBuffers[pParamNativeBuffer->nPortIndex].proxyBufferType = GrallocPointers;
				pCompPrv->proxyPortBuffers[pParamNativeBuffer->nPortIndex].IsBuffer2D = OMX_TRUE;
			} else
			{
				/* Reset to defaults */
				pCompPrv->proxyPortBuffers[pParamNativeBuffer->nPortIndex].proxyBufferType = VirtualPointers;
				pCompPrv->proxyPortBuffers[pParamNativeBuffer->nPortIndex].IsBuffer2D = OMX_FALSE;
			}

			break;
		}
#endif
		case OMX_TI_IndexUseBufferDescriptor:
		     ptBufDescParam = (OMX_TI_PARAM_USEBUFFERDESCRIPTOR *) pParamStruct;
		     if(ptBufDescParam->bEnabled == OMX_TRUE)
		     {
			     if(ptBufDescParam->eBufferType == OMX_TI_BufferTypeVirtual2D)
			     {
			         pCompPrv->proxyPortBuffers[ptBufDescParam->nPortIndex].proxyBufferType = BufferDescriptorVirtual2D;
			         pCompPrv->proxyPortBuffers[ptBufDescParam->nPortIndex].IsBuffer2D = OMX_TRUE;
		             }
		     }
		     else if(ptBufDescParam->bEnabled == OMX_FALSE)
		     {
			     /* Reset to defaults*/
			     pCompPrv->proxyPortBuffers[ptBufDescParam->nPortIndex].proxyBufferType = VirtualPointers;
			     pCompPrv->proxyPortBuffers[ptBufDescParam->nPortIndex].IsBuffer2D = OMX_FALSE;
		     }
			eRPCError =
				RPC_SetParameter(pCompPrv->hRemoteComp, nParamIndex, pParamStruct,
					pLocBufNeedMap, nNumOfLocalBuf, &eCompReturn);
		     break;
		default:
		{
#ifdef USE_ION
			if (pAuxBuf != NULL) {
				int fd = *((int*)pAuxBuf);
				if (fd > -1) {
					eRPCError = RPC_RegisterBuffer(pCompPrv->hRemoteComp, *((int*)pAuxBuf), -1,
							   &pRegistered, NULL, IONPointers);
					PROXY_checkRpcError();
					if (pRegistered)
						*pAuxBuf = pRegistered;
				}
			}
#endif
			eRPCError =
				RPC_SetParameter(pCompPrv->hRemoteComp, nParamIndex, pParamStruct,
					pLocBufNeedMap, nNumOfLocalBuf, &eCompReturn);
#ifdef USE_ION
			PROXY_checkRpcError();
			if (pRegistered != NULL) {
				eRPCError = RPC_UnRegisterBuffer(pCompPrv->hRemoteComp, pRegistered, NULL, IONPointers);
				PROXY_checkRpcError();
			}
#endif
		}
	}

	PROXY_checkRpcError();

 EXIT:
	DOMX_EXIT("eError: %d", eError);
	return eError;
}

/* ===========================================================================*/
/**
 * @name PROXY_SetParameter()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE PROXY_SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex, OMX_IN OMX_PTR pParamStruct)
{
	return __PROXY_SetParameter(hComponent, nParamIndex, pParamStruct, NULL, 0);
}


/* ===========================================================================*/
/**
 * @name __PROXY_GetParameter()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE __PROXY_GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
		OMX_IN OMX_INDEXTYPE nParamIndex, OMX_INOUT OMX_PTR pParamStruct,
		OMX_PTR pLocBufNeedMap)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	OMX_TI_PARAM_USEBUFFERDESCRIPTOR *ptBufDescParam = NULL;
#ifdef USE_ION
	OMX_PTR *pAuxBuf = pLocBufNeedMap;
	OMX_PTR pRegistered = NULL;
#endif

	PROXY_require((pParamStruct != NULL), OMX_ErrorBadParameter, NULL);
	PROXY_assert((hComp->pComponentPrivate != NULL),
			OMX_ErrorBadParameter, NULL);

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER
		("hComponent = %p, pCompPrv = %p, nParamIndex = %d, pParamStruct = %p",
		 hComponent, pCompPrv, nParamIndex, pParamStruct);

	switch(nParamIndex)
	{
		case OMX_TI_IndexUseBufferDescriptor:
			eRPCError = RPC_GetParameter(pCompPrv->hRemoteComp, nParamIndex, pParamStruct,
				pLocBufNeedMap, &eCompReturn);
			PROXY_checkRpcError();
			ptBufDescParam = (OMX_TI_PARAM_USEBUFFERDESCRIPTOR *) pParamStruct;
		     if(pCompPrv->proxyPortBuffers[ptBufDescParam->nPortIndex].proxyBufferType == BufferDescriptorVirtual2D)
		     {
			     ptBufDescParam->bEnabled = OMX_TRUE;
			     ptBufDescParam->eBufferType = OMX_TI_BufferTypeVirtual2D;
		     }
		     break;

		default:
		{
#ifdef USE_ION
			if (pAuxBuf != NULL) {
				int fd = *((int*)pAuxBuf);
				if (fd > -1) {
					eRPCError = RPC_RegisterBuffer(pCompPrv->hRemoteComp, *((int*)pAuxBuf), -1,
							   &pRegistered, NULL, IONPointers);
					PROXY_checkRpcError();
					if (pRegistered)
						*pAuxBuf = pRegistered;
				}
			}
#endif
			eRPCError = RPC_GetParameter(pCompPrv->hRemoteComp, nParamIndex, pParamStruct,
				pLocBufNeedMap, &eCompReturn);
			PROXY_checkRpcError();
#ifdef USE_ION
			if (pRegistered != NULL) {
				eRPCError = RPC_UnRegisterBuffer(pCompPrv->hRemoteComp, pRegistered, NULL, IONPointers);
				PROXY_checkRpcError();
			}
#endif
		}
	}

	PROXY_checkRpcError();

EXIT:
	DOMX_EXIT("eError: %d index: 0x%x", eError, nParamIndex);
	return eError;
}

/* ===========================================================================*/
/**
 * @name PROXY_GetParameter()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE PROXY_GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex, OMX_INOUT OMX_PTR pParamStruct)
{
	return __PROXY_GetParameter(hComponent, nParamIndex, pParamStruct, NULL);
}

/* ===========================================================================*/
/**
 * @name __PROXY_GetConfig()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE __PROXY_GetConfig(OMX_HANDLETYPE hComponent,
	OMX_INDEXTYPE nConfigIndex, OMX_PTR pConfigStruct, OMX_PTR pLocBufNeedMap)
{
#ifdef USE_ION
	OMX_PTR *pAuxBuf = pLocBufNeedMap;
	OMX_PTR pRegistered = NULL;
#endif

	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;

	PROXY_require((pConfigStruct != NULL), OMX_ErrorBadParameter, NULL);
	PROXY_require((hComp->pComponentPrivate != NULL),
		OMX_ErrorBadParameter, NULL);

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER("hComponent = %p, pCompPrv = %p, nConfigIndex = %d, "
				"pConfigStruct = %p",
				hComponent, pCompPrv, nConfigIndex,
				pConfigStruct);

#ifdef USE_ION
	if (pAuxBuf != NULL) {
		int fd = *((int*)pAuxBuf);
		if (fd > -1) {
			eRPCError = RPC_RegisterBuffer(pCompPrv->hRemoteComp, *((int*)pAuxBuf), -1,
					   &pRegistered, NULL, IONPointers);
			PROXY_checkRpcError();
			if (pRegistered)
				*pAuxBuf = pRegistered;
		}
	}
#endif

	eRPCError =
		RPC_GetConfig(pCompPrv->hRemoteComp, nConfigIndex, pConfigStruct,
			pLocBufNeedMap, &eCompReturn);
#ifdef USE_ION
	PROXY_checkRpcError();
	if (pRegistered != NULL) {
		eRPCError = RPC_UnRegisterBuffer(pCompPrv->hRemoteComp, pRegistered, NULL, IONPointers);
		PROXY_checkRpcError();
	}
#endif

	PROXY_checkRpcError();

 EXIT:
	DOMX_EXIT("eError: %d", eError);
	return eError;
}

/* ===========================================================================*/
/**
 * @name PROXY_GetConfig()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE PROXY_GetConfig(OMX_HANDLETYPE hComponent,
	OMX_INDEXTYPE nConfigIndex, OMX_PTR pConfigStruct)
{
	return __PROXY_GetConfig(hComponent, nConfigIndex, pConfigStruct, NULL);
}

/* ===========================================================================*/
/**
 * @name __PROXY_SetConfig()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE __PROXY_SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
	OMX_IN OMX_INDEXTYPE nConfigIndex, OMX_IN OMX_PTR pConfigStruct,
	OMX_PTR pLocBufNeedMap)
{
#ifdef USE_ION
	OMX_PTR *pAuxBuf = pLocBufNeedMap;
	OMX_PTR pRegistered = NULL;
#endif

	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;

	PROXY_require((pConfigStruct != NULL), OMX_ErrorBadParameter, NULL);

	PROXY_assert((hComp->pComponentPrivate != NULL),
		OMX_ErrorBadParameter, NULL);

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER("hComponent = %p, pCompPrv = %p, nConfigIndex = %d, "
				"pConfigStruct = %p",
				hComponent, pCompPrv, nConfigIndex,
				pConfigStruct);

#ifdef USE_ION
	if (pAuxBuf != NULL) {
		int fd = *((int*)pAuxBuf);
		if (fd > -1) {
			eRPCError = RPC_RegisterBuffer(pCompPrv->hRemoteComp, *((int*)pAuxBuf), -1,
					   &pRegistered, NULL, IONPointers);
			PROXY_checkRpcError();
			if (pRegistered)
				*pAuxBuf = pRegistered;
		}
	}
#endif

	eRPCError =
		RPC_SetConfig(pCompPrv->hRemoteComp, nConfigIndex, pConfigStruct,
			pLocBufNeedMap, &eCompReturn);

#ifdef USE_ION
	PROXY_checkRpcError();
	if (pRegistered != NULL) {
		eRPCError = RPC_UnRegisterBuffer(pCompPrv->hRemoteComp, pRegistered, NULL, IONPointers);
		PROXY_checkRpcError();
	}
#endif

	PROXY_checkRpcError();

      EXIT:
	DOMX_EXIT("eError: %d", eError);
	return eError;
}

/* ===========================================================================*/
/**
 * @name PROXY_SetConfig()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE PROXY_SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nConfigIndex, OMX_IN OMX_PTR pConfigStruct)
{
	return __PROXY_SetConfig(hComponent, nConfigIndex, pConfigStruct, NULL);
}


/* ===========================================================================*/
/**
 * @name PROXY_GetState()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_GetState(OMX_IN OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_STATETYPE * pState)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	OMX_COMPONENTTYPE *hComp = hComponent;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;

	PROXY_require((pState != NULL), OMX_ErrorBadParameter, NULL);
	PROXY_require((hComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter, NULL);

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER("hComponent = %p, pCompPrv = %p", hComponent, pCompPrv);

	eRPCError = RPC_GetState(pCompPrv->hRemoteComp, pState, &eCompReturn);

	DOMX_DEBUG("Returned from RPC_GetState, state: = %x", *pState);

	PROXY_checkRpcError();

      EXIT:
	if (eError == OMX_ErrorHardware)
	{
		*pState = OMX_StateInvalid;
		eError = OMX_ErrorNone;
		DOMX_DEBUG("Invalid state returned from RPC_GetState, state due to ducati in faulty state");
	}
	DOMX_EXIT("eError: %d", eError);
	return eError;
}



/* ===========================================================================*/
/**
 * @name PROXY_SendCommand()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE PROXY_SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_COMMANDTYPE eCmd,
    OMX_IN OMX_U32 nParam, OMX_IN OMX_PTR pCmdData)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	OMX_COMPONENTTYPE *pMarkComp = NULL;
	PROXY_COMPONENT_PRIVATE *pMarkCompPrv = NULL;
	OMX_PTR pMarkData = NULL, pMarkToBeFreedIfError = NULL;
	OMX_BOOL bIsProxy = OMX_FALSE;

	PROXY_require((hComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter, NULL);

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER
	    ("hComponent = %p, pCompPrv = %p, eCmd = %d, nParam = %d, pCmdData = %p",
	    hComponent, pCompPrv, eCmd, nParam, pCmdData);

	if (eCmd == OMX_CommandMarkBuffer)
	{
		PROXY_require(pCmdData != NULL, OMX_ErrorBadParameter, NULL);
		pMarkComp = (OMX_COMPONENTTYPE *)
		    (((OMX_MARKTYPE *) pCmdData)->hMarkTargetComponent);
		PROXY_require(pMarkComp != NULL, OMX_ErrorBadParameter, NULL);

		/* To check if mark comp is a proxy or a real component */
		eError = _RPC_IsProxyComponent(pMarkComp, &bIsProxy);
		PROXY_assert(eError == OMX_ErrorNone, eError, "");

		/*Replacing original mark data with proxy specific structure */
		pMarkData = ((OMX_MARKTYPE *) pCmdData)->pMarkData;
		((OMX_MARKTYPE *) pCmdData)->pMarkData =
		    TIMM_OSAL_Malloc(sizeof(PROXY_MARK_DATA), TIMM_OSAL_TRUE,
		    0, TIMMOSAL_MEM_SEGMENT_INT);
		PROXY_assert(((OMX_MARKTYPE *) pCmdData)->pMarkData != NULL,
		    OMX_ErrorInsufficientResources, "Malloc failed");
		pMarkToBeFreedIfError =
		    ((OMX_MARKTYPE *) pCmdData)->pMarkData;
		((PROXY_MARK_DATA *) (((OMX_MARKTYPE *)
			    pCmdData)->pMarkData))->hComponentActual =
		    pMarkComp;
		((PROXY_MARK_DATA *) (((OMX_MARKTYPE *)
			    pCmdData)->pMarkData))->pMarkDataActual =
		    pMarkData;

		/* If it is proxy component then replace hMarkTargetComponent
		   with remote handle */
		if (bIsProxy)
		{
			pMarkCompPrv = pMarkComp->pComponentPrivate;
			PROXY_assert(pMarkCompPrv != NULL,
			    OMX_ErrorBadParameter, NULL);

			/* Replacing with remote component handle */
			((OMX_MARKTYPE *) pCmdData)->hMarkTargetComponent =
			    ((RPC_OMX_CONTEXT *) pMarkCompPrv->hRemoteComp)->
			    hActualRemoteCompHandle;
		}
	}

	eRPCError =
	    RPC_SendCommand(pCompPrv->hRemoteComp, eCmd, nParam, pCmdData,
	    &eCompReturn);

	if (eCmd == OMX_CommandMarkBuffer && bIsProxy)
	{
		/*Resetting to original values */
		((OMX_MARKTYPE *) pCmdData)->hMarkTargetComponent = pMarkComp;
		((OMX_MARKTYPE *) pCmdData)->pMarkData = pMarkData;
	}

	PROXY_checkRpcError();

      EXIT:
	/*If SendCommand is about to return an error then this means that the
	   command has not been accepted by the component. Thus the allocated mark data
	   will be lost so free it here */
	if ((eError != OMX_ErrorNone) && pMarkToBeFreedIfError)
	{
		TIMM_OSAL_Free(pMarkToBeFreedIfError);
	}
	DOMX_EXIT("eError: %d", eError);
	return eError;
}



/* ===========================================================================*/
/**
 * @name PROXY_GetComponentVersion()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_GetComponentVersion(OMX_IN OMX_HANDLETYPE
    hComponent, OMX_OUT OMX_STRING pComponentName,
    OMX_OUT OMX_VERSIONTYPE * pComponentVersion,
    OMX_OUT OMX_VERSIONTYPE * pSpecVersion,
    OMX_OUT OMX_UUIDTYPE * pComponentUUID)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_COMPONENTTYPE *hComp = hComponent;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;

	DOMX_ENTER("hComponent = %p, pCompPrv = %p", hComponent, pCompPrv);

	PROXY_require((hComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter, NULL);
	PROXY_require(pComponentName != NULL, OMX_ErrorBadParameter, NULL);
	PROXY_require(pComponentVersion != NULL, OMX_ErrorBadParameter, NULL);
	PROXY_require(pSpecVersion != NULL, OMX_ErrorBadParameter, NULL);
	PROXY_require(pComponentUUID != NULL, OMX_ErrorBadParameter, NULL);

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	eRPCError = RPC_GetComponentVersion(pCompPrv->hRemoteComp,
	    pComponentName,
	    pComponentVersion, pSpecVersion, pComponentUUID, &eCompReturn);

	PROXY_checkRpcError();

      EXIT:
	DOMX_EXIT("eError: %d", eError);
	return eError;
}



/* ===========================================================================*/
/**
 * @name PROXY_GetExtensionIndex()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE PROXY_GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_STRING cParameterName, OMX_OUT OMX_INDEXTYPE * pIndexType)
{

	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv = NULL;
	OMX_COMPONENTTYPE *hComp = hComponent;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;

	PROXY_require((hComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter, NULL);
	PROXY_require(cParameterName != NULL, OMX_ErrorBadParameter, NULL);
	PROXY_require(pIndexType != NULL, OMX_ErrorBadParameter, NULL);

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER("hComponent = %p, pCompPrv = %p, cParameterName = %s",
	    hComponent, pCompPrv, cParameterName);


#ifdef ENABLE_GRALLOC_BUFFERS
	// Check for NULL Parameters
	PROXY_require((cParameterName != NULL && pIndexType != NULL),
	    OMX_ErrorBadParameter, NULL);

	// Ensure that String length is not greater than Max allowed length
	PROXY_require(strlen(cParameterName) <= 127, OMX_ErrorBadParameter, NULL);

	if(strcmp(cParameterName, "OMX.google.android.index.enableAndroidNativeBuffers") == 0)
	{
		// If Index type is 2D Buffer Allocated Dimension
		*pIndexType = (OMX_INDEXTYPE) OMX_TI_IndexUseNativeBuffers;
		goto EXIT;
	}
	else if (strcmp(cParameterName, "OMX.google.android.index.useAndroidNativeBuffer2") == 0)
	{
		//This is call just a dummy for android to support backward compatibility
		*pIndexType = (OMX_INDEXTYPE) NULL;
		goto EXIT;
	}
	else
	{
		eRPCError = RPC_GetExtensionIndex(pCompPrv->hRemoteComp,
                   cParameterName, pIndexType, &eCompReturn);

		PROXY_checkRpcError();
	}
#else
	eRPCError = RPC_GetExtensionIndex(pCompPrv->hRemoteComp,
	    cParameterName, pIndexType, &eCompReturn);

	PROXY_checkRpcError();
#endif

      EXIT:
	DOMX_EXIT("eError: %d", eError);
	return eError;
}




/* ===========================================================================*/
/**
 * @name PROXY_ComponentRoleEnum()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_U8 * cRole, OMX_IN OMX_U32 nIndex)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;

	DOMX_ENTER("hComponent = %p", hComponent);
	DOMX_DEBUG(" EMPTY IMPLEMENTATION ");

	DOMX_EXIT("eError: %d", eError);
	return eError;
}


/* ===========================================================================*/
/**
 * @name PROXY_ComponentTunnelRequest()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_ComponentTunnelRequest(OMX_IN OMX_HANDLETYPE
    hComponent, OMX_IN OMX_U32 nPort, OMX_IN OMX_HANDLETYPE hTunneledComp,
    OMX_IN OMX_U32 nTunneledPort,
    OMX_INOUT OMX_TUNNELSETUPTYPE * pTunnelSetup)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;

	DOMX_ENTER("hComponent = %p", hComponent);
	DOMX_DEBUG(" EMPTY IMPLEMENTATION ");
	PROXY_COMPONENT_PRIVATE *pOutCompPrv = NULL;
	PROXY_COMPONENT_PRIVATE *pInCompPrv  = NULL;
	OMX_COMPONENTTYPE       *hOutComp    = hComponent;
	OMX_COMPONENTTYPE       *hInComp     = hTunneledComp;
	OMX_ERRORTYPE           eCompReturn = OMX_ErrorNone;
	RPC_OMX_ERRORTYPE       eRPCError    = RPC_OMX_ErrorNone;
	PROXY_assert((hOutComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter, NULL);
	PROXY_assert((hInComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter, NULL);

        //TBD
        //PROXY_assert(nPort != 1, OMX_ErrorBadParameter, NULL);
        //PROXY_assert(nTunnelPort != 0, OMX_ErrorBadParameter, NULL);
	pOutCompPrv = (PROXY_COMPONENT_PRIVATE *) hOutComp->pComponentPrivate;
	pInCompPrv  = (PROXY_COMPONENT_PRIVATE *) hInComp->pComponentPrivate;
	DOMX_ENTER("hOutComp=%p, pOutCompPrv=%p, hInComp=%p, pInCompPrv=%p, nOutPort=%d, nInPort=%d \n",
	        hOutComp, pOutCompPrv, hInComp, pInCompPrv, nPort, nTunneledPort);

	DOMX_INFO("PROXY_ComponentTunnelRequest:: hOutComp=%p, pOutCompPrv=%p, hInComp=%p, pInCompPrv=%p, nOutPort=%d, nInPort=%d \n ",
	        hOutComp, pOutCompPrv, hInComp, pInCompPrv, nPort, nTunneledPort);
       eRPCError = RPC_ComponentTunnelRequest(pOutCompPrv->hRemoteComp, nPort,
	        pInCompPrv->hRemoteComp, nTunneledPort, pTunnelSetup, &eCompReturn);
        DOMX_INFO("\nafter: RPC_ComponentTunnelRequest = 0x%x\n ", eRPCError);
        PROXY_checkRpcError();

EXIT:
	DOMX_EXIT("eError: %d", eError);
	return eError;
}


/* ===========================================================================*/
/**
 * @name PROXY_SetCallbacks()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_SetCallbacks(OMX_HANDLETYPE hComponent,
    OMX_CALLBACKTYPE * pCallBacks, OMX_PTR pAppData)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;

	PROXY_require((pCallBacks != NULL), OMX_ErrorBadParameter, NULL);

	PROXY_assert((hComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter, NULL);

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	DOMX_ENTER("hComponent = %p, pCompPrv = %p", hComponent, pCompPrv);

	/*Store App callback and data to proxy- managed by proxy */
	pCompPrv->tCBFunc = *pCallBacks;
	pCompPrv->pILAppData = pAppData;

      EXIT:
	DOMX_EXIT("eError: %d", eError);
	return eError;
}


/* ===========================================================================*/
/**
 * @name PROXY_UseEGLImage()
 * @brief : This returns error not implemented by default as no component
 *          implements this. In case there is a requiremet, support for this
 *          can be added later.
 *
 */
/* ===========================================================================*/
static OMX_ERRORTYPE PROXY_UseEGLImage(OMX_HANDLETYPE hComponent,
    OMX_BUFFERHEADERTYPE ** ppBufferHdr,
    OMX_U32 nPortIndex, OMX_PTR pAppPrivate, void *pBuffer)
{
	return OMX_ErrorNotImplemented;
}


/* ===========================================================================*/
/**
 * @name PROXY_ComponentDeInit()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE PROXY_ComponentDeInit(OMX_HANDLETYPE hComponent)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone, eTmpRPCError =
	    RPC_OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	OMX_U32 count = 0, nStride = 0, nPortIndex = 0;
	OMX_PTR pMetaDataBuffer = NULL;
	MEMPLUGIN_BUFFER_PROPERTIES delBuffer_prop;
	MEMPLUGIN_BUFFER_PARAMS delBuffer_params;
	MEMPLUGIN_ERRORTYPE eMemError = MEMPLUGIN_ERROR_NONE;

	DOMX_ENTER("hComponent = %p", hComponent);
	PROXY_assert((hComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter, NULL);

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	MemPlugin_Close(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc);
	for (count = 0; count < pCompPrv->nTotalBuffers; count++)
	{
		if (pCompPrv->tBufList[count].pBufHeader)
		{
			//find the input or output port index
			if(pCompPrv->tBufList[count].pBufHeader->nInputPortIndex >= 0)
				nPortIndex = pCompPrv->tBufList[count].pBufHeader->nInputPortIndex;
			else if(pCompPrv->tBufList[count].pBufHeader->nOutputPortIndex >= 0)
				nPortIndex = pCompPrv->tBufList[count].pBufHeader->nOutputPortIndex;
#ifdef ALLOCATE_TILER_BUFFER_IN_PROXY
				if(pCompPrv->tBufList[count].bufferAccessors[0].pBufferHandle)
				{
						MEMPLUGIN_BUFFER_PARAMS_INIT(delBuffer_params);
						delBuffer_prop.sBuffer_accessor.pBufferHandle = pCompPrv->tBufList[count].bufferAccessors[0].pBufferHandle;
						delBuffer_prop.sBuffer_accessor.pBufferMappedAddress = pCompPrv->tBufList[count].bufferAccessors[0].pBufferMappedAddress;
						delBuffer_prop.sBuffer_accessor.bufferFd = pCompPrv->tBufList[count].bufferAccessors[0].bufferFd;
						delBuffer_params.bMap = pCompPrv->bMapBuffers;
						delBuffer_params.nWidth = pCompPrv->tBufList[count].pBufHeader->nAllocLen;
						MemPlugin_Free(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&delBuffer_params,&delBuffer_prop);

						pCompPrv->tBufList[count].bufferAccessors[0].pBufferHandle = NULL;
						pCompPrv->tBufList[count].bufferAccessors[0].pBufferMappedAddress = NULL;
						pCompPrv->tBufList[count].bufferAccessors[0].bufferFd = -1;
				}
#endif
			pMetaDataBuffer = ((OMX_TI_PLATFORMPRIVATE *)(pCompPrv->tBufList[count].pBufHeader)->
				pPlatformPrivate)->pMetaDataBuffer;
			if (pMetaDataBuffer)
			{
					MEMPLUGIN_BUFFER_PARAMS_INIT(delBuffer_params);
					delBuffer_prop.sBuffer_accessor.pBufferHandle = pCompPrv->tBufList[count].bufferAccessors[2].pBufferHandle;
					delBuffer_prop.sBuffer_accessor.pBufferMappedAddress = pCompPrv->tBufList[count].bufferAccessors[2].pBufferMappedAddress;
					delBuffer_prop.sBuffer_accessor.bufferFd = pCompPrv->tBufList[count].bufferAccessors[2].bufferFd;
					delBuffer_params.bMap = pCompPrv->bMapBuffers;
					delBuffer_params.nWidth = ((OMX_TI_PLATFORMPRIVATE *)(pCompPrv->tBufList[count].pBufHeader)->pPlatformPrivate)->nMetaDataSize;
					MemPlugin_Free(pCompPrv->pMemPluginHandle,pCompPrv->nMemmgrClientDesc,&delBuffer_params,&delBuffer_prop);
					((OMX_TI_PLATFORMPRIVATE *)(pCompPrv->tBufList[count].pBufHeader)->
					pPlatformPrivate)->pMetaDataBuffer = NULL;
					pCompPrv->tBufList[count].bufferAccessors[2].pBufferHandle = NULL;
					pCompPrv->tBufList[count].bufferAccessors[2].pBufferMappedAddress = NULL;
					pCompPrv->tBufList[count].bufferAccessors[2].bufferFd = -1;
			}
#ifdef USE_ION
	{
		// Need to unregister buffers when using ion and rpmsg
		if (pCompPrv->tBufList[count].bufferAccessors[0].pRegBufferHandle != NULL)
		{
			eTmpRPCError = RPC_UnRegisterBuffer(pCompPrv->hRemoteComp,
								pCompPrv->tBufList[count].bufferAccessors[0].pRegBufferHandle,
								pCompPrv->tBufList[count].bufferAccessors[1].pRegBufferHandle,
								pCompPrv->proxyPortBuffers[nPortIndex].proxyBufferType);
			if (eTmpRPCError != RPC_OMX_ErrorNone) {
				eRPCError = eTmpRPCError;
			}
		}

		if (pCompPrv->tBufList[count].bufferAccessors[2].pRegBufferHandle != NULL)
		{
			eTmpRPCError |= RPC_UnRegisterBuffer(pCompPrv->hRemoteComp,
								pCompPrv->tBufList[count].bufferAccessors[2].pRegBufferHandle, NULL, IONPointers);
			if (eTmpRPCError != RPC_OMX_ErrorNone) {
				eRPCError |= eTmpRPCError;
			}
		}
	}
#endif

			if (pCompPrv->tBufList[count].pBufHeader->pPlatformPrivate)
			{
				TIMM_OSAL_Free(pCompPrv->tBufList[count].pBufHeader->
				    pPlatformPrivate);
			}
			TIMM_OSAL_Free(pCompPrv->tBufList[count].pBufHeader);
			TIMM_OSAL_Memset(&(pCompPrv->tBufList[count]), 0,
			    sizeof(PROXY_BUFFER_INFO));
		}
	}

	KPI_OmxCompDeinit(hComponent);

	eRPCError = RPC_FreeHandle(pCompPrv->hRemoteComp, &eCompReturn);
	if (eRPCError != RPC_OMX_ErrorNone)
		eTmpRPCError = eRPCError;

	eRPCError = RPC_InstanceDeInit(pCompPrv->hRemoteComp);
	if (eRPCError != RPC_OMX_ErrorNone)
		eTmpRPCError = eRPCError;

	eMemError = MemPlugin_DeInit(pCompPrv->pMemPluginHandle);
	if (pCompPrv->cCompName)
	{
		TIMM_OSAL_Free(pCompPrv->cCompName);
	}

	if (pCompPrv)
	{
		TIMM_OSAL_Free(pCompPrv);
	}

	eRPCError = eTmpRPCError;
	PROXY_checkRpcError();

EXIT:
	DOMX_EXIT("eError: %d eMemError %d", eError,eMemError);
	return eError;
}
/* ===========================================================================*/
/**
 * @name OMX_ProxyCommonInit()
 * @brief
 * @param void
 * @return OMX_ErrorNone = Successful
 * @sa TBD
 *
 */
/* ===========================================================================*/
OMX_ERRORTYPE OMX_ProxyCommonInit(OMX_HANDLETYPE hComponent)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone, eCompReturn = OMX_ErrorNone;
	RPC_OMX_ERRORTYPE eRPCError = RPC_OMX_ErrorNone;
	PROXY_COMPONENT_PRIVATE *pCompPrv;
	OMX_COMPONENTTYPE *hComp = (OMX_COMPONENTTYPE *) hComponent;
	OMX_HANDLETYPE hRemoteComp = NULL;
    OMX_U32 i = 0;
    MEMPLUGIN_ERRORTYPE eMemError = MEMPLUGIN_ERROR_NONE;
	DOMX_ENTER("hComponent = %p", hComponent);

	TIMM_OSAL_UpdateTraceLevel();

	PROXY_require((hComp->pComponentPrivate != NULL),
	    OMX_ErrorBadParameter, NULL);

	pCompPrv = (PROXY_COMPONENT_PRIVATE *) hComp->pComponentPrivate;

	pCompPrv->nTotalBuffers = 0;
	pCompPrv->nAllocatedBuffers = 0;
	pCompPrv->proxyEmptyBufferDone = PROXY_EmptyBufferDone;
	pCompPrv->proxyFillBufferDone = PROXY_FillBufferDone;
	pCompPrv->proxyEventHandler = PROXY_EventHandler;

        for (i=0; i<PROXY_MAXNUMOFPORTS ; i++)
        {
              pCompPrv->proxyPortBuffers[i].proxyBufferType = VirtualPointers;
        }

	eRPCError = RPC_InstanceInit(pCompPrv->cCompName, &hRemoteComp);
	PROXY_assert(eRPCError == RPC_OMX_ErrorNone,
	    OMX_ErrorUndefined, "Error initializing RPC");
	PROXY_assert(hRemoteComp != NULL,
	    OMX_ErrorUndefined, "Error initializing RPC");

	//Send the proxy component handle for pAppData
	eRPCError =
	    RPC_GetHandle(hRemoteComp, pCompPrv->cCompName,
	    (OMX_PTR) hComponent, NULL, &eCompReturn);

	PROXY_checkRpcError();

	hComp->SetCallbacks = PROXY_SetCallbacks;
	hComp->ComponentDeInit = PROXY_ComponentDeInit;
	hComp->UseBuffer = PROXY_UseBuffer;
	hComp->GetParameter = PROXY_GetParameter;
	hComp->SetParameter = PROXY_SetParameter;
	hComp->EmptyThisBuffer = PROXY_EmptyThisBuffer;
	hComp->FillThisBuffer = PROXY_FillThisBuffer;
	hComp->GetComponentVersion = PROXY_GetComponentVersion;
	hComp->SendCommand = PROXY_SendCommand;
	hComp->GetConfig = PROXY_GetConfig;
	hComp->SetConfig = PROXY_SetConfig;
	hComp->GetState = PROXY_GetState;
	hComp->GetExtensionIndex = PROXY_GetExtensionIndex;
	hComp->FreeBuffer = PROXY_FreeBuffer;
	hComp->ComponentRoleEnum = PROXY_ComponentRoleEnum;
	hComp->AllocateBuffer = PROXY_AllocateBuffer;
	hComp->ComponentTunnelRequest = PROXY_ComponentTunnelRequest;
	hComp->UseEGLImage = PROXY_UseEGLImage;

	pCompPrv->hRemoteComp = hRemoteComp;

	eMemError = MemPlugin_Init("MEMPLUGIN_ION",&(pCompPrv->pMemPluginHandle));
	if(eMemError != MEMPLUGIN_ERROR_NONE)
	{
		DOMX_ERROR("MEMPLUGIN configure step failed");
		return OMX_ErrorUndefined;
	}
	pCompPrv->bMapBuffers = OMX_TRUE;

	eMemError = MemPlugin_Open(pCompPrv->pMemPluginHandle,&(pCompPrv->nMemmgrClientDesc));
	if(eMemError != MEMPLUGIN_ERROR_NONE)
	{
		DOMX_ERROR("Mem manager client creation failed!!!");
		return OMX_ErrorInsufficientResources;
	}
	KPI_OmxCompInit(hComponent);

      EXIT:
	if (eError != OMX_ErrorNone)
		RPC_InstanceDeInit(hRemoteComp);
	DOMX_EXIT("eError: %d", eError);

	return eError;
}

/* ===========================================================================*/
/**
 * @name _RPC_IsProxyComponent()
 * @brief This function calls GetComponentVersion API on the component and
 *        based on the component name decidec whether the component is a proxy
 *        or real component. The naming component convention assumed is
 *        <OMX>.<Company Name>.<Core Name>.<Domain>.<Component Details> with
 *        all characters in upper case for e.g. OMX.TI.DUCATI1.VIDEO.H264E
 * @param hComponent [IN] : The component handle
 *        bIsProxy [OUT]  : Set to true is handle is for a proxy component
 * @return OMX_ErrorNone = Successful
 *
 **/
/* ===========================================================================*/
OMX_ERRORTYPE _RPC_IsProxyComponent(OMX_HANDLETYPE hComponent,
    OMX_BOOL * bIsProxy)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_S8 cComponentName[MAXNAMESIZE] = { 0 }
	, cCoreName[32] =
	{
	0};
	OMX_VERSIONTYPE sCompVer, sSpecVer;
	OMX_UUIDTYPE sCompUUID;
	OMX_U32 i = 0, ret = 0;

	eError =
	    OMX_GetComponentVersion(hComponent, (OMX_STRING) cComponentName,
	    &sCompVer, &sSpecVer, &sCompUUID);
	PROXY_assert(eError == OMX_ErrorNone, eError, "");
	ret =
	    sscanf((char *)cComponentName, "%*[^'.'].%*[^'.'].%[^'.'].%*s",
	    cCoreName);
	PROXY_assert(ret == 1, OMX_ErrorBadParameter,
	    "Incorrect component name");
	for (i = 0; i < CORE_MAX; i++)
	{
		if (strcmp((char *)cCoreName, Core_Array[i]) == 0)
			break;
	}
	PROXY_assert(i < CORE_MAX, OMX_ErrorBadParameter,
	    "Unknown core name");

	/* If component name indicates remote core, it means proxy
	   component */
	if ((i == CORE_SYSM3) || (i == CORE_APPM3) || (i == CORE_TESLA))
		*bIsProxy = OMX_TRUE;
	else
		*bIsProxy = OMX_FALSE;

      EXIT:
	return eError;
}
