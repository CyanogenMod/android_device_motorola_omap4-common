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
 * This file contains two portions:
 *
 * PART1: contains all PLUGIN related type defintions
 *        such as error types, buffer types, buffer params
 *        that are generic to all PLUGINS and expected to
 *        be reused.
 *
 * PART2: contains all GENERIC INTERFACE type definitions
 *        and methods that would be used to access PLUGIN
 *        implementations from common DOMX.
 */

 /* ----- system and platform files ----------------------------*/
#ifndef MEMPLUGIN_H
#define MEMPLUGIN_H
#include <string.h>

#include "OMX_TI_Core.h"
/***************************************************************
 * PART 1
 * PLUGIN RELATED TYPE DEFINITIONS
 *
 *
 * *************************************************************
 */
typedef enum MEMPLUGIN_ERRORTYPE
{
     MEMPLUGIN_ERROR_NONE = 0x00,
     MEMPLUGIN_ERROR_BADPARAMETER,
     MEMPLUGIN_ERROR_NORESOURCES,
     MEMPLUGIN_ERROR_UNDEFINED,
     MEMPLUGIN_ERROR_NOTIMPLEMENTED
}MEMPLUGIN_ERRORTYPE;

typedef enum MEMPLUGIN_BUFFERTYPE
{
   DEFAULT = 0x00, //NON TILER
   TILER1D,
   TILER2D
}MEMPLUGIN_BUFFERTYPE;

typedef enum MEMPLUGIN_TILER_FORMAT
{
    MEMPLUGIN_TILER_FORMAT_8BIT = 0,
    MEMPLUGIN_TILER_FORMAT_16BIT = 1,
    MEMPLUGIN_TILER_FORMAT_32BIT = 2,
    MEMPLUGIN_TILER_FORMAT_PAGE = 3
}MEMPLUGIN_TILER_FORMAT;
/**
 * MEMPLUGIN_BUFFER_ACCESSOR: This structure maintaines all possible accessors for an allocated buffer
 *
 * @param pBufferHandle:    This is a pointer to buffer handle
 * @param bufferFd:        This is the context free file descriptor obtained by mapping
 * @param pRegBufferHandle:    This is the handle obtained after registration of the buffer with rpmsg
 * @param pBufferMappedAddress:    This is the mapped address of the buffer obtained after mapping
 *
 */
 //TBD: if pRegBufferHandle can be part of this structure - it is kind of ION specific
typedef struct MEMPLUGIN_BUFFER_ACCESSOR
{
    OMX_PTR pBufferHandle;
    OMX_U32 bufferFd;
    OMX_PTR pRegBufferHandle;
    OMX_PTR pBufferMappedAddress;
}MEMPLUGIN_BUFFER_ACCESSOR;

// @struct MEMPLUGIN_BUFFER_INPARAMS - comprises of all buffer properties required as input for Plugin APIs
typedef struct MEMPLUGIN_BUFFER_PARAMS
{
OMX_U32 nHeight;
OMX_U32 nWidth;
OMX_BOOL bMap;
MEMPLUGIN_TILER_FORMAT eTiler_format; //expected to be passed from user for tiler1D/2D == in case of nontiler to tiler upgrade, defaulted to TILER_PIXEL_FMT_PAGE
MEMPLUGIN_BUFFERTYPE eBuffer_type;
}MEMPLUGIN_BUFFER_PARAMS;
// @struct MEMPLUGIN_BUFFER_OUTPARAMS - comprises of all buffer properties required as input for Plugin APIs
typedef struct MEMPLUGIN_BUFFER_PROPERTIES
{
MEMPLUGIN_BUFFER_ACCESSOR sBuffer_accessor;
OMX_U32 nStride;
}MEMPLUGIN_BUFFER_PROPERTIES;

/***************************************************************
 * PART 2
 * PLUGIN INTERFACE RELATED TYPE AND METHOD DEFINITIONS
 *
 *
 * *************************************************************
 */

/** @struct MEMPLUGIN_OBJECT - comprises of all plugin related data including:
 *
 * pPluginExtendedInfo - plugin specific extended data structure
 * fpOpen - function pointer interface to Plugin Open()
 * fpAlloc - function pointer interface to Plugin Allocation method
 * fpFree - function pointer interface to Plugin Freeing method
 * fpClose - function pointer interface to Plugin Close()
 *
 */
typedef struct MEMPLUGIN_OBJECT
{
    OMX_PTR pPluginExtendedInfo;
    MEMPLUGIN_ERRORTYPE (*fpConfig)(void *pMemPluginHandle,
                                    void *pConfigData);
    MEMPLUGIN_ERRORTYPE (*fpOpen)(void *pMemPluginHandle,
                                    OMX_U32 *pClient);
    MEMPLUGIN_ERRORTYPE (*fpAlloc)(void *pMemPluginHandle,
                                    OMX_U32 nClient,
                                    MEMPLUGIN_BUFFER_PARAMS *pBufferParams,
                                    MEMPLUGIN_BUFFER_PROPERTIES *pBufferProp);
    MEMPLUGIN_ERRORTYPE (*fpFree)(void *pMemPluginHandle,
                                    OMX_U32 nClient,
                                    MEMPLUGIN_BUFFER_PARAMS *pBufferParams,
                                    MEMPLUGIN_BUFFER_PROPERTIES *pBufferProp);
    MEMPLUGIN_ERRORTYPE (*fpClose)(void *pMemPluginHandle,
                                    OMX_U32 nClient);
    MEMPLUGIN_ERRORTYPE (*fpDeInit)(void *pMemPluginHandle);
}MEMPLUGIN_OBJECT;

typedef MEMPLUGIN_ERRORTYPE (*MEMPLUGIN_CONFIGTYPE)(MEMPLUGIN_OBJECT **);

typedef struct MEMPLUGIN_TABLETYPE
{
    char cMemPluginName[50];
    MEMPLUGIN_CONFIGTYPE pMemPluginConfig;
}MEMPLUGIN_TABLETYPE;

/******************************************************************
 *   FUNCTIONS DEFINITION
 ******************************************************************/
 /*MemPlugin_Init: To initialize a MemPlugin from DOMX PROXY
  *                Must be called before any other API calls.
  *                Maps the plugin handle to corresponding proxy
  *                element for further access
  *
  * @param cMemPluginName: string that identifies plugin
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  */
 MEMPLUGIN_ERRORTYPE MemPlugin_Init(char *cMemPluginName,
                                            void **pMemPluginHandle);
 /*MemPlugin_Configure: To do different types of plugin specific
  *                     config operations required.
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param pConfigData: data specific to configuration to be done
  */
 MEMPLUGIN_ERRORTYPE MemPlugin_Configure(void *pMemPluginHandle,
                                            void *pConfigData);
 /*MemPlugin_Open: To open and obtain a client for the MemPlugin
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param pClient: MemPlugin client used for operations
  */
 MEMPLUGIN_ERRORTYPE MemPlugin_Open(void *pMemPluginHandle,
                                    OMX_U32 *pClient);
 /*MemPlugin_Close: To close the client of MemPlugin
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param nClient: MemPlugin client used for operations
  */
 MEMPLUGIN_ERRORTYPE MemPlugin_Close(void  *pMemPluginHandle,
                                        OMX_U32 nClient);
 /*MemPlugin_Alloc: To allocate a buffer
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param nClient: MemPlugin client used for operations
  * @param pBufferParams: All buffer IN params required for allocation
  * @param pBufferProp: All buffer properties obtained from allocation
  */
 MEMPLUGIN_ERRORTYPE MemPlugin_Alloc(void *pMemPluginHandle,
                                        OMX_U32 nClient,
                                        MEMPLUGIN_BUFFER_PARAMS *pBufferParams,
                                        MEMPLUGIN_BUFFER_PROPERTIES *pBufferProp);
 /*MemPlugin_Free: To free a buffer
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param nClient: MemPlugin client used for operations
  * @param pBufferParams: All buffer IN params required for freeing
  * @param pBufferProp: All buffer properties required for freeing
  */
 MEMPLUGIN_ERRORTYPE MemPlugin_Free(void *pMemPluginHandle,
                                    OMX_U32 nClient,
                                    MEMPLUGIN_BUFFER_PARAMS *pBufferParams,
                                    MEMPLUGIN_BUFFER_PROPERTIES *pBufferProp);
 /*MemPlugin_DeInit: To destroy a MemPlugin from DOMX PROXY
  *
  * @param pMemPluginHandle: handle that provides identifies the
  *                          plugin to be destroyed
  */
 MEMPLUGIN_ERRORTYPE MemPlugin_DeInit(void *pMemPluginHandle);
#endif
