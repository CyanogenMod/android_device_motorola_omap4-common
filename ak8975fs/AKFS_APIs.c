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
#include "AKFS_Common.h"
#include "AKFS_Compass.h"
#include "AKFS_FileIO.h"
#include "AKFS_Measure.h"
#include "AKFS_APIs.h"

/******************************************************************************/
/*! Initialize #AKMPRMS structure and make APIs ready to use. This function
  must be called before application uses any APIs in this file.  If #AKMPRMS
  are already initialized, this function discards all existing data. When APIs
  are not used anymore, #AKM_Release function must be called at the end.
  When this function succeeds, form factor number is set to 0.
  @return #AKM_SUCCESS on success. #AKM_ERROR if an error occurred.
  @param[in/out] mem A pointer to a handler.
  @param[in] hpat Specify a layout pattern number.  The number is determined
  according to the mount orientation of the magnetometer.
  @param[in] regs[3] Specify the ASA values which are read out from
  fuse ROM.  regs[0] is ASAX, regs[1] is ASAY, regs[2] is ASAZ.
 */
int16 AKFS_Init(
			void		*mem,
	const	AKFS_PATNO	hpat,
	const	uint8		regs[]
)
{
	AKMPRMS *prms;
#ifdef AKM_VALUE_CHECK
	if (mem == NULL) {
		AKMDEBUG(AKMDATA_CHECK, "%s: Invalid mem pointer.", __FUNCTION__);
		return AKM_ERROR;
	}
#endif
	AKMDEBUG(AKMDATA_DUMP, "%s: hpat=%d, r[0]=0x%02X, r[1]=0x%02X, r[2]=0x%02X\n",
		__FUNCTION__, hpat, regs[0], regs[1], regs[2]);

	/* Copy pointer */
	prms = (AKMPRMS *)mem;

	/* Clear all data. */
	memset(prms, 0, sizeof(AKMPRMS));

	/* Sensitivity */
	prms->fv_hs.u.x = AKM_MAG_SENSE;
	prms->fv_hs.u.y = AKM_MAG_SENSE;
	prms->fv_hs.u.z = AKM_MAG_SENSE;
	prms->fv_as.u.x = AKM_ACC_SENSE;
	prms->fv_as.u.y = AKM_ACC_SENSE;
	prms->fv_as.u.z = AKM_ACC_SENSE;

	/* Copy ASA values */
	prms->i8v_asa.u.x = regs[0];
	prms->i8v_asa.u.y = regs[1];
	prms->i8v_asa.u.z = regs[2];

	/* Copy layout pattern */
	prms->e_hpat = hpat;

	return AKM_SUCCESS;
}

/******************************************************************************/
/*! Release allocated memory. This function must be called at the end of using
  APIs. Currently this function is empty.
  @return None
  @param[in/out] mem A pointer to a handler.
 */
void AKFS_Release(void *mem)
{
#ifdef AKM_VALUE_CHECK
	if (mem == NULL) {
		AKMDEBUG(AKMDATA_CHECK, "%s: Invalid mem pointer.", __FUNCTION__);
		return;
	}
#endif
	/* Do nothing */
}

/******************************************************************************/
/* This function is called just before a measurement sequence starts.
  Load parameters from a file and initialize library. This function must be
  called when a sequential measurement thread boots up.
  @return The return value is #AKM_SUCCESS.
  @param[in/out] mem A pointer to a handler.
  @param[in] path The path to a setting file to be read out. The path name
  should be terminated with NULL.
 */
int16 AKFS_Start(void *mem, const char *path)
{
	AKMPRMS *prms;
#ifdef AKM_VALUE_CHECK
	if (mem == NULL || path == NULL) {
		AKMDEBUG(AKMDATA_CHECK, "%s: Invalid mem pointer.", __FUNCTION__);
		return AKM_ERROR;
	}
#endif
	AKMDEBUG(AKMDATA_DUMP, "%s: path=%s\n", __FUNCTION__, path);

	/* Copy pointer */
	prms = (AKMPRMS *)mem;

	/* Read setting files from a file */
	if (AKFS_LoadParameters(prms, path) != AKM_SUCCESS) {
		AKMERROR_STR("AKFS_LoadParameters");
	}

	/* Initialize buffer */
	AKFS_InitBuffer(AKFS_HDATA_SIZE, prms->fva_hdata);
	AKFS_InitBuffer(AKFS_HDATA_SIZE, prms->fva_hvbuf);
	AKFS_InitBuffer(AKFS_ADATA_SIZE, prms->fva_avbuf);

	/* Initialize for AOC */
	AKFS_InitAOC(&prms->s_aocv);
	/* Initialize magnetic status */
	prms->i16_hstatus = 0;

	return AKM_SUCCESS;
}

/******************************************************************************/
/*! This function is called when a measurement sequence is done.
  Save parameters to a file. This function must be called when a sequential
  measurement thread ends.
  @return The return value is #AKM_SUCCESS.
  @param[in/out] mem A pointer to a handler.
  @param[in] path The path to a setting file to be written. The path name
  should be terminated with NULL.
 */
int16 AKFS_Stop(void *mem, const char *path)
{
	AKMPRMS *prms;
#ifdef AKM_VALUE_CHECK
	if (mem == NULL || path == NULL) {
		AKMDEBUG(AKMDATA_CHECK, "%s: Invalid mem pointer.", __FUNCTION__);
		return AKM_ERROR;
	}
#endif
	AKMDEBUG(AKMDATA_DUMP, "%s: path=%s\n", __FUNCTION__, path);

	/* Copy pointer */
	prms = (AKMPRMS *)mem;

	/* Write setting files to a file */
	if (AKFS_SaveParameters(prms, path) != AKM_SUCCESS) {
		AKMERROR_STR("AKFS_SaveParameters");
	}

	return AKM_SUCCESS;
}

/*!
  This function is called when new magnetometer data is available.  The output
  vector format and coordination system follow the Android definition.
  @return The return value is #AKM_SUCCESS.
   Otherwise the return value is #AKM_ERROR.
  @param[in/out] mem A pointer to a handler.
  @param[in] mag A set of measurement data from magnetometer.  X axis value
   should be in mag[0], Y axis value should be in mag[1], Z axis value should be 
   in mag[2].
  @param[in] status A status of magnetometer.  This status indicates the result
   of measurement data, i.e. overflow, success or fail, etc.
 */
int16 AKFS_Get_MAGNETIC_FIELD(
			void		*mem,
	const   int16		mag[3],
	const	int16		status,
			AKFLOAT		*hx,
			AKFLOAT		*hy,
			AKFLOAT		*hz,
			int16		*accuracy
)
{
	AKMPRMS *prms;
#ifdef AKM_VALUE_CHECK
	if (mem == NULL) {
		AKMDEBUG(AKMDATA_CHECK, "%s: Invalid mem pointer.", __FUNCTION__);
		return AKM_ERROR;
	}
	if (hx == NULL || hy == NULL || hz == NULL || accuracy == NULL) {
		AKMDEBUG(AKMDATA_CHECK, "%s: Invalid data pointer.", __FUNCTION__);
		return AKM_ERROR;
	}
#endif

	/* Copy pointer */
	prms = (AKMPRMS *)mem;

	if (AKFS_Set_MAGNETIC_FIELD(prms, mag, status) != AKM_SUCCESS) {
		return AKM_ERROR;
	}

	/* Success */
	*hx = prms->fv_hvec.u.x;
	*hy = prms->fv_hvec.u.y;
	*hz = prms->fv_hvec.u.z;
	*accuracy = prms->i16_hstatus;

	return AKM_SUCCESS;
}

/*!
  This function is called when new accelerometer data is available.  The output
  vector format and coordination system follow the Android definition.
  @return The return value is #AKM_SUCCESS when function succeeds. Otherwise
   the return value is #AKM_ERROR.
  @param[in] acc A set of measurement data from accelerometer.  X axis value
   should be in acc[0], Y axis value should be in acc[1], Z axis value should be 
   in acc[2].
  @param[in] status A status of accelerometer.  This status indicates the result
   of acceleration data, i.e. overflow, success or fail, etc.
 */
int16 AKFS_Get_ACCELEROMETER(
			void		*mem,
	const   int16		acc[3],
	const	int16		status,
			AKFLOAT		*ax,
			AKFLOAT		*ay,
			AKFLOAT		*az,
			int16		*accuracy
)
{
	AKMPRMS *prms;
#ifdef AKM_VALUE_CHECK
	if (mem == NULL) {
		AKMDEBUG(AKMDATA_CHECK, "%s: Invalid mem pointer.", __FUNCTION__);
		return AKM_ERROR;
	}
	if (ax == NULL || ay == NULL || az == NULL || accuracy == NULL) {
		AKMDEBUG(AKMDATA_CHECK, "%s: Invalid data pointer.", __FUNCTION__);
		return AKM_ERROR;
	}
#endif

	/* Copy pointer */
	prms = (AKMPRMS *)mem;

	if (AKFS_Set_ACCELEROMETER(prms, acc, status) != AKM_SUCCESS) {
		return AKM_ERROR;
	}

	/* Success */
	*ax = prms->fv_avec.u.x;
	*ay = prms->fv_avec.u.y;
	*az = prms->fv_avec.u.z;
	*accuracy = 3;

	return AKM_SUCCESS;
}

/*!
  Get orientation sensor's elements. The vector format and coordination system
   follow the Android definition.  Before this function is called, magnetic
   field vector and acceleration vector should be stored in the buffer by 
   calling #AKFS_Get_MAGNETIC_FIELD and #AKFS_Get_ACCELEROMETER.
  @return The return value is #AKM_SUCCESS when function succeeds. Otherwise
   the return value is #AKM_ERROR.
  @param[out] azimuth Azimuthal angle in degree.
  @param[out] pitch Pitch angle in degree.
  @param[out] roll Roll angle in degree.
  @param[out] accuracy Accuracy of orientation sensor.
 */
int16 AKFS_Get_ORIENTATION(
			void		*mem,
			AKFLOAT		*azimuth,
			AKFLOAT		*pitch,
			AKFLOAT		*roll,
			int16		*accuracy
)
{
	int16 akret;
	AKMPRMS *prms;
#ifdef AKM_VALUE_CHECK
	if (mem == NULL) {
		AKMDEBUG(AKMDATA_CHECK, "%s: Invalid mem pointer.", __FUNCTION__);
		return AKM_ERROR;
	}
	if (pitch== NULL || pitch == NULL || roll == NULL || accuracy == NULL) {
		AKMDEBUG(AKMDATA_CHECK, "%s: Invalid data pointer.", __FUNCTION__);
		return AKM_ERROR;
	}
#endif

	/* Copy pointer */
	prms = (AKMPRMS *)mem;

	/* Azimuth calculation */
	/* hvbuf[in] : Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted. */
	/* avbuf[in] : Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted. */
	/* azimuth[out]: Android coordinate and unit (degree). */
	/* pitch  [out]: Android coordinate and unit (degree). */
	/* roll   [out]: Android coordinate and unit (degree). */
	akret = AKFS_Direction(
		AKFS_HDATA_SIZE,
		prms->fva_hvbuf,
		CSPEC_HNAVE_D,
		AKFS_ADATA_SIZE,
		prms->fva_avbuf,
		CSPEC_ANAVE_D,
		&prms->f_azimuth,
		&prms->f_pitch,
		&prms->f_roll
	);

	if (akret == AKFS_ERROR) {
		AKMERROR;
		return AKM_ERROR;
	}

	/* Success */
	*azimuth = prms->f_azimuth;
	*pitch   = prms->f_pitch;
	*roll    = prms->f_roll;
	*accuracy = 3;

	/* Debug output */
	AKMDEBUG(AKMDATA_ORI, "Ori(?):%8.2f, %8.2f, %8.2f\n",
			*azimuth, *pitch, *roll);

	return AKM_SUCCESS;
}

