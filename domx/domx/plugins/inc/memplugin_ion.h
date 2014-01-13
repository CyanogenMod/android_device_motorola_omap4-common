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
 * This file contains all ION plugin related types
 * and methods that will be accessed only from
 * within or corresponding GLUE layers.
 */

#include <unistd.h>
#include <ion_ti/ion.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <linux/rpmsg_omx.h>
#include <errno.h>

#include "memplugin.h"

/* struct MEMPLUGIN_ION_PARAMS : contains all params that are specific to
 *                               ION APIs for buffer allocation.
 * @param nAlign: alignment of buffers
 * @param nOffset: offset needed for buffers
 * @param alloc_flags: allocation flags depending on type of buffer needed
 * @param map_flags: mapping flags depending on type of mapping
 * @param prot: buffer protection flag
 */
typedef struct MEMPLUGIN_ION_PARAMS {
    size_t nAlign;
    off_t nOffset;
    OMX_U32 alloc_flags;
    OMX_S32 map_flags;
    OMX_S32 prot;
}MEMPLUGIN_ION_PARAMS;
/******************************************************************
 *   MACROS DEFINITION
 ******************************************************************/
#define MEMPLUGIN_ION_PARAMS_INIT(buffer_ion_params) do {\
        (buffer_ion_params)->alloc_flags = 1 << OMAP_ION_HEAP_SECURE_INPUT;\
        (buffer_ion_params)->map_flags = MAP_SHARED;\
        (buffer_ion_params)->nAlign = 0x1000;\
        (buffer_ion_params)->prot = PROT_READ | PROT_WRITE;\
        (buffer_ion_params)->nOffset = 0;\
}while(0)
/******************************************************************
 *   FUNCTIONS DEFINITION
 ******************************************************************/
 /*MemPlugin_ION_Init: To initialize a MemPlugin from DOMX PROXY
  *                Must be called before any other API calls.
  *                Maps the plugin handle to corresponding proxy
  *                element for further access
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  */
MEMPLUGIN_ERRORTYPE MemPlugin_ION_Init(void **pMemPluginHandle);
 /*MemPlugin_Configure: To do different types of plugin specific
  *                     config operations required.
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param pConfigData: data specific to configuration to be done
  */
MEMPLUGIN_ERRORTYPE MemPlugin_ION_Configure(void *pMemPluginHandle,
                                        void *pConfigData);
/*MemPlugin_Open: To open and obtain a client for the MemPlugin
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param pClient: MemPlugin client used for operations
  */
MEMPLUGIN_ERRORTYPE MemPlugin_ION_Open(void *pMemPluginHandle,
                                        OMX_U32 *pClient);
/*MemPlugin_Close: To close the client of MemPlugin
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param nClient: MemPlugin client used for operations
  */
MEMPLUGIN_ERRORTYPE MemPlugin_ION_Close(void *pMemPluginHandle,
                                        OMX_U32 nClient);
/*MemPlugin_Alloc: To allocate a buffer
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param nClient: MemPlugin client used for operations
  * @param pBufferParams: All buffer IN params required for allocation
  * @param pBufferProp: All buffer properties obtained from allocation
  */
MEMPLUGIN_ERRORTYPE MemPlugin_ION_Alloc(void *pMemPluginHandle,
                                        OMX_U32 nClient,
                                        MEMPLUGIN_BUFFER_PARAMS *pIonBufferParams,
                                        MEMPLUGIN_BUFFER_PROPERTIES *pIonBufferProp);
/*MemPlugin_Free: To free a buffer
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param nClient: MemPlugin client used for operations
  * @param pBufferParams: All buffer IN params required for freeing
  * @param pBufferProp: All buffer properties required for freeing
  */
MEMPLUGIN_ERRORTYPE MemPlugin_ION_Free(void *pMemPluginHandle,
                                        OMX_U32 nClient,
                                        MEMPLUGIN_BUFFER_PARAMS *pIonBufferParams,
                                        MEMPLUGIN_BUFFER_PROPERTIES *pIonBufferProp);
 /*MemPlugin_DeInit: To destroy a MemPlugin from DOMX PROXY
  *
  * @param pMemPluginHandle: handle that provides identifies the
  *                          plugin to be destroyed
  */
MEMPLUGIN_ERRORTYPE MemPlugin_ION_DeInit(void *pMemPluginHandle);
