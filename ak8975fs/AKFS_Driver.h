/******************************************************************************
 *
 * Copyright (C) 2012 Asahi Kasei Microdevices Corporation, Japan
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
 *
 ******************************************************************************/
#ifndef AKMD_INC_AKMD_DRIVER_H
#define AKMD_INC_AKMD_DRIVER_H

#include <stdint.h>			/* int8_t, int16_t etc. */

#if defined(AKM_DEVICE_AK8963)
#include <linux/akm8963.h>		/* Device driver */
#elif defined(AKM_DEVICE_AK8975)
#include <linux/akm8975.h>		/* Device driver */
#elif defined(AKM_DEVICE_AK09911)
#include <linux/akm09911.h>	/* Device driver */
#endif

/*** Constant definition ******************************************************/
#define AKD_TRUE	1		/*!< Represents true */
#define AKD_FALSE	0		/*!< Represents false */
#define AKD_SUCCESS	0		/*!< Represents success.*/
#define AKD_ERROR	-1		/*!< Represents error. */

/*! 0:Don't Output data, 1:Output data */
#define AKD_DBG_DATA	0
/*! Typical interval in ns */
#define AKM_MEASUREMENT_TIME_NS	((AKM_MEASURE_TIME_US) * 1000)


/*** Type declaration *********************************************************/
typedef unsigned char BYTE;


/*** Global variables *********************************************************/

/*** Prototype of Function  ***************************************************/

int16_t AKD_InitDevice(void);

void AKD_DeinitDevice(void);

int16_t AKD_TxData(
		const BYTE address,
		const BYTE* data,
		const uint16_t numberOfBytesToWrite);

int16_t AKD_RxData(
		const BYTE address,
		BYTE* data,
		const uint16_t numberOfBytesToRead);

int16_t AKD_Reset(void);

int16_t AKD_GetSensorInfo(BYTE data[AKM_SENSOR_INFO_SIZE]);

int16_t AKD_GetSensorConf(BYTE data[AKM_SENSOR_CONF_SIZE]);

int16_t AKD_GetMagneticData(BYTE data[AKM_SENSOR_DATA_SIZE]);

void AKD_SetYPR(const short buf[AKM_YPR_DATA_SIZE]);

int16_t AKD_GetOpenStatus(int* status);

int16_t AKD_GetCloseStatus(int* status);

int16_t AKD_SetMode(const BYTE mode);

int16_t AKD_GetDelay(int64_t delay[AKM_NUM_SENSORS]);

int16_t AKD_GetLayout(int16_t* layout);

int16_t AKD_GetAccelerationData(int16_t data[3]);

#endif /* AKMD_INC_AKMD_DRIVER_H */
