/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef _BDROID_BUILDCFG_H
#define _BDROID_BUILDCFG_H

#define BTM_DEF_LOCAL_NAME			"Motorola Mapphone"
#define VENDOR_LIB_RUNTIME_TUNING_ENABLED	FALSE
#define VENDOR_LIB_CONF_FILE			"/etc/bluetooth/bt_vendor.conf"
#define BLUETOOTH_UART_DEVICE_PORT		"/dev/ttyO1"
#define FW_PATCHFILE_LOCATION			"/vendor/firmware/"
#define UART_TARGET_BAUD_RATE			3000000
#define FW_PATCH_SETTLEMENT_DELAY_MS		0
#define USE_CONTROLLER_BDADDR			FALSE
#define LPM_SLEEP_MODE				1
#define LPM_IDLE_THRESHOLD			1
#define LPM_HC_IDLE_THRESHOLD			1
#define LPM_BT_WAKE_POLARITY			1
#define LPM_HOST_WAKE_POLARITY			1
#define LPM_ALLOW_HOST_SLEEP_DURING_SCO		1
#define LPM_COMBINE_SLEEP_MODE_AND_LPM		1
#define LPM_ENABLE_UART_TXD_TRI_STATE		0
#define LPM_PULSED_HOST_WAKE			0
#define LPM_IDLE_TIMEOUT_MULTIPLE		10
#define BT_WAKE_VIA_USERIAL_IOCTL		FALSE
#define BT_WAKE_VIA_PROC			FALSE
#define SCO_CFG_INCLUDED			TRUE
#define SCO_USE_I2S_INTERFACE			FALSE

#define SCO_I2SPCM_IF_MODE              1
#define SCO_I2SPCM_IF_ROLE              1
#define SCO_I2SPCM_IF_SAMPLE_RATE       0
#define SCO_I2SPCM_IF_CLOCK_RATE        1

#define SCO_PCM_ROUTING                 0
#define SCO_PCM_IF_CLOCK_RATE           4
#define SCO_PCM_IF_FRAME_TYPE           0
#define SCO_PCM_IF_SYNC_MODE            0
#define SCO_PCM_IF_CLOCK_MODE           0
#define PCM_DATA_FMT_SHIFT_MODE         0
#define PCM_DATA_FMT_FILL_BITS          0
#define PCM_DATA_FMT_FILL_METHOD        3
#define PCM_DATA_FMT_FILL_NUM           3
#define PCM_DATA_FMT_JUSTIFY_MODE       0

#endif
