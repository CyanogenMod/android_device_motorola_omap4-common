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
#ifndef AKFS_INC_COMPASS_H
#define AKFS_INC_COMPASS_H

#include "AKFS_Common.h"
#include "AKFS_CSpec.h"

#ifdef WIN32
#include "AKM_LinuxDriver.h"
#else
#include "AKFS_Driver.h"
#endif

/****************************************/
/* Include files for AKM OSS library.   */
/****************************************/
#include "./libAKM_OSS/AKFS_Configure.h"
#include "./libAKM_OSS/AKFS_AOC.h"
#include "./libAKM_OSS/AKFS_Decomp.h"
#include "./libAKM_OSS/AKFS_Device.h"
#include "./libAKM_OSS/AKFS_Direction.h"
#include "./libAKM_OSS/AKFS_Math.h"
#include "./libAKM_OSS/AKFS_VNorm.h"

/*** Constant definition ******************************************************/
#define AKM_MAG_SENSE			(1.0)
#define AKM_ACC_SENSE			(720)
#define AKM_ACC_TARGET			(9.80665f)

/*** Type declaration *********************************************************/
typedef struct _AKSENSOR_DATA{
	AKFLOAT	x;
	AKFLOAT	y;
	AKFLOAT	z;
    int8	status;
} AKSENSOR_DATA;

/*! A parameter structure. */
/* ix*_ : x-bit integer */
/* f**_ : floating value */
/* p**_ : pointer */
/* e**_ : enum */
/* *s*_ : struct */
/* *v*_ : vector (special type of struct) */
/* **a_ : array */
typedef struct _AKMPRMS{

	/* Variables for Decomp. */
	AKFVEC			fva_hdata[AKFS_HDATA_SIZE];
	uint8vec		i8v_asa;

	/* Variables forAOC. */
	AKFS_AOC_VAR	s_aocv;

	/* Variables for Magnetometer buffer. */
	AKFVEC			fva_hvbuf[AKFS_HDATA_SIZE];
	AKFVEC			fv_ho;
	AKFVEC			fv_hs;
	AKFS_PATNO		e_hpat;

	/* Variables for Accelerometer buffer. */
	AKFVEC			fva_avbuf[AKFS_ADATA_SIZE];
	AKFVEC			fv_ao;
	AKFVEC			fv_as;

	/* Variables for Direction. */
	AKFLOAT			f_azimuth;
	AKFLOAT			f_pitch;
	AKFLOAT			f_roll;

	/* Variables for vector output */
	AKFVEC			fv_hvec;
	AKFVEC			fv_avec;
	int16			i16_hstatus;

} AKMPRMS;

#endif

