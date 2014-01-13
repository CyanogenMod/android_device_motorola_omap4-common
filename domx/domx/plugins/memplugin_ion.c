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

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "memplugin_ion.h"
#include <timm_osal_types.h>
#include <timm_osal_memory.h>
#include <timm_osal_trace.h>
#include "omx_rpc_utils.h"

/******************************************************************
 *   MACROS DEFINITION
 ******************************************************************/
#define MEMPLUGIN_ION_PARAMS_COPY(ion_params_src, ion_params_dest) do {\
        ion_params_dest.alloc_flags = ion_params_src->alloc_flags;\
        ion_params_dest.map_flags = ion_params_src->map_flags;\
        ion_params_dest.nAlign = ion_params_src->nAlign;\
        ion_params_dest.prot = ion_params_src->prot ;\
        ion_params_dest.nOffset =  ion_params_src->nOffset ;\
}while(0)


MEMPLUGIN_ERRORTYPE MemPlugin_ION_Init(void **pMemPluginHandle)
{
    MEMPLUGIN_ERRORTYPE    eError = MEMPLUGIN_ERROR_NONE;
    MEMPLUGIN_OBJECT      *pMemPluginHdl;

    pMemPluginHdl = TIMM_OSAL_MallocExtn(sizeof(MEMPLUGIN_OBJECT), TIMM_OSAL_TRUE,
                                      0, TIMMOSAL_MEM_SEGMENT_EXT, NULL);
    if(pMemPluginHdl == NULL)
    {
        eError = MEMPLUGIN_ERROR_NORESOURCES;
        DOMX_ERROR("%s: allocation failed",__FUNCTION__);
        goto EXIT;
    }

    TIMM_OSAL_Memset(pMemPluginHdl, 0, sizeof(MEMPLUGIN_OBJECT));

    pMemPluginHdl->fpOpen = MemPlugin_ION_Open;
    pMemPluginHdl->fpClose = MemPlugin_ION_Close;
    pMemPluginHdl->fpConfig = MemPlugin_ION_Configure;
    pMemPluginHdl->fpAlloc = MemPlugin_ION_Alloc;
    pMemPluginHdl->fpFree = MemPlugin_ION_Free;
    pMemPluginHdl->fpDeInit = MemPlugin_ION_DeInit;
    *pMemPluginHandle = pMemPluginHdl;

EXIT:
    if(eError != MEMPLUGIN_ERROR_NONE)
    {
        DOMX_EXIT("%s: failed with error %d",__FUNCTION__,eError);
    }
    else
    {
        DOMX_EXIT("%s: executed successfully",__FUNCTION__);
    }
    return eError;
}

MEMPLUGIN_ERRORTYPE MemPlugin_ION_Open(void *pMemPluginHandle,OMX_U32 *pClient)
{
    MEMPLUGIN_ERRORTYPE    eError = MEMPLUGIN_ERROR_NONE;
    OMX_U32 memClient = 0;
    memClient = ion_open();
    if(memClient == 0)
    {
        DOMX_ERROR("ion open failed");
        eError = MEMPLUGIN_ERROR_UNDEFINED;
        goto EXIT;
    }
    else
    {
        *pClient = memClient;
    }
EXIT:
    if(eError != MEMPLUGIN_ERROR_NONE)
    {
        DOMX_EXIT("%s: failed with error %d",__FUNCTION__,eError);
    }
    else
    {
        DOMX_EXIT("%s: executed successfully",__FUNCTION__);
    }
    return eError;
}

MEMPLUGIN_ERRORTYPE MemPlugin_ION_Close(void *pMemPluginHandle, OMX_U32 nClient)
{
    MEMPLUGIN_ERRORTYPE    eError = MEMPLUGIN_ERROR_NONE;

    ion_close(nClient);

EXIT:
    return eError;
}
MEMPLUGIN_ERRORTYPE MemPlugin_ION_Configure(void *pMemPluginHandle, void *pConfigData)
{
    //implementation to be added later
EXIT:
    return(MEMPLUGIN_ERROR_NONE);
}

MEMPLUGIN_ERRORTYPE MemPlugin_ION_Alloc(void *pMemPluginHandle, OMX_U32 nClient,
                                    MEMPLUGIN_BUFFER_PARAMS *pIonBufferParams,
                                    MEMPLUGIN_BUFFER_PROPERTIES *pIonBufferProp)
{
    OMX_S16 ret;
    struct ion_handle *temp;
    size_t stride;
    MEMPLUGIN_ERRORTYPE eError = MEMPLUGIN_ERROR_NONE;
    MEMPLUGIN_ION_PARAMS sIonParams;
    MEMPLUGIN_OBJECT    *pMemPluginHdl = (MEMPLUGIN_OBJECT *)pMemPluginHandle;

    if(pIonBufferParams->nWidth <= 0)
    {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        DOMX_ERROR("%s: width should be positive %d", __FUNCTION__,pIonBufferParams->nWidth);
        goto EXIT;
    }

    if(pMemPluginHdl->pPluginExtendedInfo == NULL)
    {
        MEMPLUGIN_ION_PARAMS_INIT(&sIonParams);
    }
    else
    {
        MEMPLUGIN_ION_PARAMS_COPY(((MEMPLUGIN_ION_PARAMS *)pMemPluginHdl->pPluginExtendedInfo),sIonParams);
    }
    if(pIonBufferParams->eBuffer_type == DEFAULT)
    {
        ret = (OMX_S16)ion_alloc(nClient,
                                    pIonBufferParams->nWidth,
                                    sIonParams.nAlign,
                                    sIonParams.alloc_flags,
                                    &temp);
        if(ret || (int)temp == -ENOMEM)
        {
            if(sIonParams.alloc_flags != OMAP_ION_HEAP_SECURE_INPUT)
            {
               //for non default types of allocation - no retry with tiler 1d - throw error
//STARGO: ducati secure heap is too small, need to allocate from heap
#if 0
               DOMX_ERROR("FAILED to allocate secure buffer of size=%d. ret=0x%x",pIonBufferParams->nWidth, ret);
               eError = MEMPLUGIN_ERROR_NORESOURCES;
               goto EXIT;
#endif
               DOMX_ERROR("FAILED to allocate secure buffer of size=%d. ret=0x%x - trying tiler 1d space",pIonBufferParams->nWidth, ret);
               pIonBufferParams->eBuffer_type = TILER1D;
               pIonBufferParams->eTiler_format = MEMPLUGIN_TILER_FORMAT_PAGE;
               sIonParams.alloc_flags = OMAP_ION_HEAP_TILER_MASK;
               sIonParams.nAlign = -1;
            }
            else
            {
                // for default non tiler (OMAP_ION_HEAP_SECURE_INPUT) retry allocating from tiler 1D
                DOMX_DEBUG("FAILED to allocate from non tiler space - trying tiler 1d space");
                pIonBufferParams->eBuffer_type = TILER1D;
                pIonBufferParams->eTiler_format = MEMPLUGIN_TILER_FORMAT_PAGE;
                sIonParams.alloc_flags = OMAP_ION_HEAP_TILER_MASK;
                sIonParams.nAlign = -1;
            }
        }
    }
    if(pIonBufferParams->eBuffer_type == TILER1D)
    {
        ret = (OMX_S16)ion_alloc_tiler(nClient,
                                        pIonBufferParams->nWidth,
                                        pIonBufferParams->nHeight,
                                        pIonBufferParams->eTiler_format,
                                        sIonParams.alloc_flags,
                                        &temp,
                                        &(pIonBufferProp->nStride));

         if (ret || ((int)temp == -ENOMEM))
         {
               DOMX_ERROR("FAILED to allocate buffer of size=%d. ret=0x%x",pIonBufferParams->nWidth, ret);
               eError = MEMPLUGIN_ERROR_NORESOURCES;
               goto EXIT;
         }
    }
    else if(pIonBufferParams->eBuffer_type == TILER2D)
    {
        DOMX_ERROR("Tiler 2D not implemented");
        eError = MEMPLUGIN_ERROR_NOTIMPLEMENTED;
        goto EXIT;
    }
    else if(!temp)
    {
        DOMX_ERROR("Undefined option for buffer type");
        eError = MEMPLUGIN_ERROR_UNDEFINED;
        goto EXIT;
    }
    pIonBufferProp->sBuffer_accessor.pBufferHandle = (OMX_PTR)temp;
    pIonBufferProp->nStride =  stride;

    if(pIonBufferParams->bMap == OMX_TRUE)
    {
        ret = (OMX_S16) ion_map(nClient,
                                pIonBufferProp->sBuffer_accessor.pBufferHandle,
                                pIonBufferParams->nWidth*pIonBufferParams->nHeight,
                                sIonParams.prot,
                                sIonParams.map_flags,
                                sIonParams.nOffset,
                                (unsigned char **) &(pIonBufferProp->sBuffer_accessor.pBufferMappedAddress),
                                &(pIonBufferProp->sBuffer_accessor.bufferFd));

        if(ret < 0)
        {
                DOMX_ERROR("userspace mapping of ION buffers returned error");
                eError = MEMPLUGIN_ERROR_NORESOURCES;
                goto EXIT;
        }
    }
    else
    {
        ret = (OMX_S16) ion_share(nClient,
                                    pIonBufferProp->sBuffer_accessor.pBufferHandle,
                                    &(pIonBufferProp->sBuffer_accessor.bufferFd));
        if(ret < 0)
        {
                DOMX_ERROR("ION share returned error");
                eError = MEMPLUGIN_ERROR_NORESOURCES;
                goto EXIT;
        }
    }
EXIT:
      if (eError != MEMPLUGIN_ERROR_NONE) {
          DOMX_EXIT("%s exited with error 0x%x",__FUNCTION__,eError);
         return eError;
      }
      else {
          DOMX_EXIT("%s executed successfully",__FUNCTION__);
         return MEMPLUGIN_ERROR_NONE;
      }
}

MEMPLUGIN_ERRORTYPE MemPlugin_ION_Free(void *pMemPluginHandle,OMX_U32 nClient,
                                    MEMPLUGIN_BUFFER_PARAMS *pIonBufferParams,
                                    MEMPLUGIN_BUFFER_PROPERTIES *pIonBufferProp)
{
    MEMPLUGIN_ERRORTYPE eError = MEMPLUGIN_ERROR_NONE;
    DOMX_ENTER("%s: called with arguments ion fd %d buffer-params 0x%x buffer-prop 0x%x",__FUNCTION__,nClient,pIonBufferParams,pIonBufferProp);

    //unmap
    if(pIonBufferParams->bMap == OMX_TRUE)
    {
    munmap(pIonBufferProp->sBuffer_accessor.pBufferMappedAddress, pIonBufferParams->nHeight * pIonBufferParams->nWidth);
    }
    //close
    close(pIonBufferProp->sBuffer_accessor.bufferFd);
    //free
    ion_free(nClient, (struct ion_handle*)pIonBufferProp->sBuffer_accessor.pBufferHandle);

EXIT:
      if (eError != MEMPLUGIN_ERROR_NONE) {
          DOMX_EXIT("%s exited with error 0x%x",__FUNCTION__,eError);
         return eError;
      }
      else {
          DOMX_EXIT("%s executed successfully",__FUNCTION__);
         return MEMPLUGIN_ERROR_NONE;
      }
}
MEMPLUGIN_ERRORTYPE MemPlugin_ION_DeInit(void *pMemPluginHandle)
{
    MEMPLUGIN_ERRORTYPE eError = MEMPLUGIN_ERROR_NONE;
    MEMPLUGIN_OBJECT    *pMemPluginHdl = (MEMPLUGIN_OBJECT *)pMemPluginHandle;

    if(pMemPluginHdl->pPluginExtendedInfo != NULL)
    {
        TIMM_OSAL_Free(((MEMPLUGIN_ION_PARAMS *)pMemPluginHdl->pPluginExtendedInfo));
    }
    TIMM_OSAL_Free((MEMPLUGIN_OBJECT *)pMemPluginHandle);
    pMemPluginHandle = NULL;
EXIT:
    return (eError);
}
