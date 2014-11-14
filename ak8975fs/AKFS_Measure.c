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
 */
#include "AKFS_APIs.h"
#include "AKFS_Measure.h"


/******************************************************************************/
/*! This function is called when new magnetometer data is available.  The
  coordination system of input vector is sensor local coordination system.
  The input vector will be converted to micro tesla unit (i.e. uT), then
  rotated using layout matrix (i.e. i16_hlayout).
  A magnetic offset is estimated automatically in this function.
  As a result of it, offset subtracted vector is stored in #AKMPRMS structure.

  @return #AKM_SUCCESS on success. Otherwise the return value is #AKM_ERROR.
  @param[in] prms A pointer to #AKMPRMS structure.
  @param[in] mag A set of measurement data from magnetometer.  X axis value
  should be in mag[0], Y axis value should be in mag[1], Z axis value should be
  in mag[2].
  @param[in] status A status of magnetometer.  This status indicates the result
  of measurement data, i.e. overflow, success or fail, etc.
 */
int16 AKFS_Set_MAGNETIC_FIELD(
			AKMPRMS		*prms,
	const	int16		mag[3],
	const	int16		status
)
{
	int16 akret;
	int16 aocret;
	AKFLOAT radius;

	AKMDEBUG(AKMDATA_MAG, "%s: m[0]=%d, m[1]=%d, m[2]=%d, st=%d\n",
		__FUNCTION__, mag[0], mag[1], mag[2], status);

	/* Decomposition */
	/* mag  [in] : sensor local coordinate, sensor local unit. */
	/* hdata[out]: sensor local coordinate, sensitivity adjusted (i.e. uT). */
	akret = AKFS_Decomp(
		mag,
		status,
		&prms->i8v_asa,
		AKFS_HDATA_SIZE,
		prms->fva_hdata
	);
	if (akret == AKFS_ERROR) {
		AKMERROR;
		return AKM_ERROR;
	}

	/* Rotate coordination */
	/* hdata[in] : sensor local coordinate, sensitivity adjusted. */
	/* hdata[out]: Android coordinate, sensitivity adjusted. */
	akret = AKFS_Rotate(
		prms->e_hpat,
		&prms->fva_hdata[0]
	);
	if (akret == AKFS_ERROR) {
		AKMERROR;
		return AKM_ERROR;
	}

	/* Offset calculation is done in this function */
	/* hdata[in] : Android coordinate, sensitivity adjusted. */
	/* ho   [out]: Android coordinate, sensitivity adjusted. */
	aocret = AKFS_AOC(
		&prms->s_aocv,
		prms->fva_hdata,
		&prms->fv_ho
	);

	/* Subtract offset */
	/* hdata[in] : Android coordinate, sensitivity adjusted. */
	/* ho   [in] : Android coordinate, sensitivity adjusted. */
	/* hvbuf[out]: Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted. */
	akret = AKFS_VbNorm(
		AKFS_HDATA_SIZE,
		prms->fva_hdata,
		1,
		&prms->fv_ho,
		&prms->fv_hs,
		AKM_MAG_SENSE,
		AKFS_HDATA_SIZE,
		prms->fva_hvbuf
	);
	if (akret == AKFS_ERROR) {
		AKMERROR;
		return AKM_ERROR;
	}

	/* Averaging */
	/* hvbuf[in] : Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted. */
	/* hvec [out]: Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted, averaged. */
	akret = AKFS_VbAve(
		AKFS_HDATA_SIZE,
		prms->fva_hvbuf,
		CSPEC_HNAVE_V,
		&prms->fv_hvec
	);
	if (akret == AKFS_ERROR) {
		AKMERROR;
		return AKM_ERROR;
	}

	/* Check the size of magnetic vector */
	radius = AKFS_SQRT(
			(prms->fv_hvec.u.x * prms->fv_hvec.u.x) +
			(prms->fv_hvec.u.y * prms->fv_hvec.u.y) +
			(prms->fv_hvec.u.z * prms->fv_hvec.u.z));

	if (radius > AKFS_GEOMAG_MAX) {
		prms->i16_hstatus = 0;
	} else {
		if (aocret == AKFS_SUCCESS) {
			prms->i16_hstatus = 3;
		}
	}

	/* Debug output */
	AKMDEBUG(AKMDATA_MAG, "Mag(%d):%8.2f, %8.2f, %8.2f\n",
		prms->i16_hstatus, prms->fv_hvec.u.x, prms->fv_hvec.u.y, prms->fv_hvec.u.z);

	return AKM_SUCCESS;
}

/******************************************************************************/
/*! This function is called when new accelerometer data is available.  The
  coordination system of input vector is Android coordination system.
  The input vector will be converted to SI unit (i.e. m/s/s).

  @return #AKM_SUCCESS on success. Otherwise the return value is #AKM_ERROR.
  @param[in] prms A pointer to #AKMPRMS structure.
  @param[in] acc A set of measurement data from accelerometer.  X axis value
  should be in acc[0], Y axis value should be in acc[1], Z axis value should be
  in acc[2].
  @param[in] status A status of accelerometer.  This status indicates the result
  of accelerometer data. Currently, this parameter is not used.
 */
int16 AKFS_Set_ACCELEROMETER(
			AKMPRMS		*prms,
	const	int16		acc[3],
	const	int16		status
)
{
	int16 akret;

	AKMDEBUG(AKMDATA_ACC, "%s: a[0]=%d, a[1]=%d, a[2]=%d, st=%d\n",
		__FUNCTION__, acc[0], acc[1], acc[2], status);

	/* Make a spare area for new data */
	akret = AKFS_BufShift(
		AKFS_ADATA_SIZE,
		1,
		prms->fva_avbuf
	);
	if (akret == AKFS_ERROR) {
		AKMERROR;
		return AKM_ERROR;
	}
	/* Subtract offset, adjust sensitivity */
	/* acc  [in] : Android coordinate, sensor local unit. */
	/* avbuf[out]: Android coordinate, sensitivity adjusted (SI unit), */
	/*			   offset subtracted. */
	prms->fva_avbuf[0].u.x = AKM_ACC_TARGET * (((AKFLOAT)acc[0] - prms->fv_ao.u.x) / prms->fv_as.u.x);
	prms->fva_avbuf[0].u.y = AKM_ACC_TARGET * (((AKFLOAT)acc[1] - prms->fv_ao.u.y) / prms->fv_as.u.y);
	prms->fva_avbuf[0].u.z = AKM_ACC_TARGET * (((AKFLOAT)acc[2] - prms->fv_ao.u.z) / prms->fv_as.u.z);

#ifdef AKFS_OUTPUT_AVEC
	/* Averaging */
	/* avbuf[in] : Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted. */
	/* avec [out]: Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted, averaged. */
	akret = AKFS_VbAve(
		AKFS_ADATA_SIZE,
		prms->fva_avbuf,
		CSPEC_ANAVE_V,
		&prms->fv_avec
	);
	if (akret == AKFS_ERROR) {
		AKMERROR;
		return AKM_ERROR;
	}

	/* Debug output (accuracy is always '3' */
	AKMDEBUG(AKMDATA_ACC, "Acc(3):%8.2f, %8.2f, %8.2f\n",
			prms->fv_avec.u.x, prms->fv_avec.u.y, prms->fv_avec.u.z);
#endif

	return AKM_SUCCESS;
}

