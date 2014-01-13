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

/* author: Vidhoon Viswanathan
 * date:    28 sept 2012
 * This file contains declarations and function prototypes
 * that are expected to be reused or common across various
 * memory PLUGIN implementations
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <timm_osal_types.h>
#include <timm_osal_memory.h>
#include <timm_osal_trace.h>

#include "memplugin.h"
#include "omx_rpc_utils.h"

extern MEMPLUGIN_TABLETYPE    MemPlugins_Map[];

MEMPLUGIN_ERRORTYPE MemPlugin_Init(char *cMemPluginName, void **pMemPluginHandle)
{
    MEMPLUGIN_OBJECT *pMemPluginHdl;
    OMX_BOOL bFound;
    OMX_U16 i = 0;
    MEMPLUGIN_ERRORTYPE eError = MEMPLUGIN_ERROR_NONE;

    if(cMemPluginName == NULL || pMemPluginHandle == NULL)
    {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        DOMX_ERROR("%s: Invalid parameter to function",__FUNCTION__);
        goto EXIT;
    }

    while(MemPlugins_Map[i].cMemPluginName != NULL)
    {
        if(strcmp(MemPlugins_Map[i].cMemPluginName,cMemPluginName) == 0)
        {
            bFound = OMX_TRUE;
            break;
        }
        else
        {
            i++;

        }
    }
    if(bFound)
    {
        eError = MemPlugins_Map[i].pMemPluginConfig(&pMemPluginHdl);
        if(eError != MEMPLUGIN_ERROR_NONE)
        {
            goto EXIT;
        }
        else
        {
            *pMemPluginHandle = pMemPluginHdl;
        }

    }
    else
    {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        DOMX_ERROR("%s: Invalid parameter to function",__FUNCTION__);
        goto EXIT;
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

MEMPLUGIN_ERRORTYPE MemPlugin_Open(void *pMemPluginHandle, OMX_U32 *pClient)
{
    MEMPLUGIN_ERRORTYPE eError = MEMPLUGIN_ERROR_NONE;
    OMX_U32 pClientDesc;

    if(pMemPluginHandle == NULL || pClient == NULL)
    {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        DOMX_ERROR("%s: Invalid parameter to function",__FUNCTION__);
        goto EXIT;
    }

    eError = ((MEMPLUGIN_OBJECT *)pMemPluginHandle)->fpOpen(pMemPluginHandle,&pClientDesc);
    if(eError != MEMPLUGIN_ERROR_NONE)
    {
        DOMX_ERROR("%s: Client Open() failed",__FUNCTION__);
        goto EXIT;
    }
    else
    {
        *pClient = pClientDesc;
    }

EXIT:
    if(eError != MEMPLUGIN_ERROR_NONE)
    {
        DOMX_EXIT("%s: failed with error %d",__FUNCTION__,eError);
    }
    else
    {
        DOMX_EXIT("%s: executed successfully with client desc %d",__FUNCTION__,*pClient);
    }
    return eError;
}

MEMPLUGIN_ERRORTYPE MemPlugin_Close(void *pMemPluginHandle, OMX_U32 nClient)
{
    MEMPLUGIN_ERRORTYPE eError = MEMPLUGIN_ERROR_NONE;

    if(pMemPluginHandle == NULL)
    {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        DOMX_ERROR("%s: Invalid parameter to function",__FUNCTION__);
        goto EXIT;
    }
    eError = ((MEMPLUGIN_OBJECT *)pMemPluginHandle)->fpClose(pMemPluginHandle,nClient);
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
MEMPLUGIN_ERRORTYPE MemPlugin_Configure(void *pMemPluginHandle, void *pConfigData)
{
    //implementation to be added later
EXIT:
    return(MEMPLUGIN_ERROR_NONE);
}

MEMPLUGIN_ERRORTYPE MemPlugin_Alloc(void *pMemPluginHandle, OMX_U32 nClient,
                                    MEMPLUGIN_BUFFER_PARAMS *pBufferParams,
                                    MEMPLUGIN_BUFFER_PROPERTIES *pBufferProp)
{
    MEMPLUGIN_ERRORTYPE eError = MEMPLUGIN_ERROR_NONE;

    if(pMemPluginHandle == NULL)
    {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        DOMX_ERROR("%s: Invalid parameter to function",__FUNCTION__);
        goto EXIT;
    }
    eError = ((MEMPLUGIN_OBJECT *)pMemPluginHandle)->fpAlloc(pMemPluginHandle,nClient,pBufferParams,pBufferProp);
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

MEMPLUGIN_ERRORTYPE MemPlugin_Free(void *pMemPluginHandle, OMX_U32 nClient,
                                    MEMPLUGIN_BUFFER_PARAMS *pBufferParams,
                                    MEMPLUGIN_BUFFER_PROPERTIES *pBufferProp)
{
    MEMPLUGIN_ERRORTYPE eError = MEMPLUGIN_ERROR_NONE;

    if(pMemPluginHandle == NULL)
    {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        DOMX_ERROR("%s: Invalid parameter to function",__FUNCTION__);
        goto EXIT;
    }
    eError = ((MEMPLUGIN_OBJECT *)pMemPluginHandle)->fpFree(pMemPluginHandle,nClient,pBufferParams,pBufferProp);
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

MEMPLUGIN_ERRORTYPE MemPlugin_DeInit(void *pMemPluginHandle)
{
      MEMPLUGIN_ERRORTYPE    eError = MEMPLUGIN_ERROR_NONE;

      if(pMemPluginHandle == NULL)
      {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        DOMX_ERROR("%s: Invalid parameter to function",__FUNCTION__);
        goto EXIT;
      }

     eError = ((MEMPLUGIN_OBJECT *)pMemPluginHandle)->fpDeInit(pMemPluginHandle);


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
