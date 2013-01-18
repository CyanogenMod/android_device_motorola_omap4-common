/*
 *
 * Copyright (C) 2010 Motorola
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
 */
#ifndef MOTOBOX_CONFIG_ERR_DEFS_H
#define MOTOBOX_CONFIG_ERR_DEFS_H

/** Motobox get,set config error codes */
typedef enum
{
    MOTOBOX_CONFIG_ERR_NONE             = 0x00, /* Success */
    MOTOBOX_CONFIG_ERR_INVALID_PARAM    = 0x51, /* Invalid param */
    MOTOBOX_CONFIG_ERR_FILE_OPERATION   = 0x52, /* File operation failure */
    MOTOBOX_CONFIG_ERR_INVALID_FORMAT   = 0x53, /* Invalid config-id format */
    MOTOBOX_CONFIG_ERR_REPEAT_ID        = 0x54, /* Rewriting with existing config-id */
    MOTOBOX_CONFIG_ERR_SET_RECOVERY     = 0x55, /* Error setting device into recovery mode */
    MOTOBOX_CONFIG_ERR_NEEDS_RECONFIG   = 0x56, /* Incomplete, needs reconfiguration */
    MOTOBOX_CONFIG_ERR_NOT_MULTICONFIG  = 0x57, /* Not a multiconfig build */
} MOTOBOX_CONFIG_ERR_T;

/** Config-id constants */
#define MOTOBOX_CONFIG_ID_MIN             0
#define MOTOBOX_CONFIG_ID_MAX    2147483647     /* 4 byte Signed int max */
#define MOTOBOX_CONFIG_ID_INVALID        -2
#define MOTOBOX_CONFIG_ID_UNCONFIGURED   -1

#endif
