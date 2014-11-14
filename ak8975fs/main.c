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
#include "AKFS_Disp.h"
#include "AKFS_FileIO.h"
#include "AKFS_Measure.h"
#include "AKFS_APIs.h"

#ifndef WIN32
#include <sched.h>
#include <pthread.h>
#include <linux/input.h>
#endif

/*** Constant definition ******************************************************/
#define ERROR_INITDEVICE		(-1)
#define ERROR_OPTPARSE			(-2)
#define ERROR_SELF_TEST			(-3)
#define ERROR_READ_FUSE			(-4)
#define ERROR_INIT				(-5)
#define ERROR_GETOPEN_STAT		(-6)
#define ERROR_STARTCLONE		(-7)
#define ERROR_GETCLOSE_STAT		(-8)

#define AKM_SELFTEST_MIN_X	-100
#define AKM_SELFTEST_MAX_X	100
#define AKM_SELFTEST_MIN_Y	-100
#define AKM_SELFTEST_MAX_Y	100
#define AKM_SELFTEST_MIN_Z	-1000
#define AKM_SELFTEST_MAX_Z	-300

#define CONVERT_ACC(a)	((int)((a) * 720 / 9.8f))
#define CONVERT_MAG(m)	((int)((m) / 0.06f))
#define CONVERT_ORI(o)	((int)((o) * 64))

/*** Global variables *********************************************************/
int g_stopRequest = 0;
int g_opmode = 0;
int g_dbgzone = 0;
int g_mainQuit = AKD_FALSE;

/* Static variable. */
static pthread_t s_thread;  /*!< Thread handle */

/*** Sub Function *************************************************************/
/*!
  Read sensitivity adjustment data from fuse ROM.
  @return If data are read successfully, the return value is #AKM_SUCCESS.
   Otherwise the return value is #AKM_ERROR.
  @param[out] regs The read ASA values. When this function succeeds, ASAX value
   is saved in regs[0], ASAY is saved in regs[1], ASAZ is saved in regs[2].
 */
int16 AKFS_ReadConf(
		uint8	regs[3]
)
{
	BYTE conf[AKM_SENSOR_CONF_SIZE];

#ifdef AKM_VALUE_CHECK
	if (AKM_SENSOR_CONF_SIZE != 3) {
		AKMERROR_STR("You may refer invalid header file.");
		return AKM_ERROR;
	}
#endif

	if (AKD_GetSensorConf(conf) != AKD_SUCCESS) {
		AKMERROR;
		return AKM_ERROR;
	}
	regs[0] = conf[0];
	regs[1] = conf[1];
	regs[2] = conf[2];

	AKMDEBUG(AKMDATA_DUMP, "%s: asa(dec)=%d,%d,%d\n",
			__FUNCTION__, regs[0], regs[1], regs[2]);

	return AKM_SUCCESS;
}

/*!
  This function calculate the duration of sleep for maintaining
   the loop keep the period.
  This function calculates "minimum - (end - start)".
  @return The result of above equation in nanosecond.
  @param end The time of after execution.
  @param start The time of before execution.
  @param minimum Loop period of each execution.
 */
struct timespec AKFS_CalcSleep(
	const struct timespec* end,
	const struct timespec* start,
	const int64_t minimum
)
{
	int64_t endL;
	int64_t startL;
	int64_t diff;

	struct timespec ret;

	endL = (int64_t)((int64_t)end->tv_sec * 1000000000) + (int64_t)end->tv_nsec;
	startL = (int64_t)((int64_t)start->tv_sec * 1000000000) + (int64_t)start->tv_nsec;
	diff = minimum;

	diff -= (endL - startL);

	/* Don't allow negative value */
	if (diff < 0) {
		diff = 0;
	}

	/* Convert to timespec */
	if (diff > 1000000000) {
	ret.tv_sec = diff / 1000000000;
		ret.tv_nsec = diff % 1000000000;
	} else {
		ret.tv_sec = 0;
		ret.tv_nsec = diff;
	}
	return ret;
}

/*!
  Get interval of each sensors from device driver.
  @return If this function succeeds, the return value is #AKM_SUCCESS.
   Otherwise the return value is #AKM_ERROR.
  @param flag This variable indicates what sensor frequency is updated.
  @param minimum This value show the minimum loop period in all sensors.
 */
int16 AKFS_GetInterval(
		uint16*  flag,
		int64_t* minimum
)
{
	/* Accelerometer, Magnetometer, Fusion */
	/* Delay is in nano second unit. */
	/* Negative value means the sensor is disabled.*/
	int64_t delay[AKM_NUM_SENSORS];
	int i;

#ifdef AKM_VALUE_CHECK
	if (AKM_NUM_SENSORS != 3) {
		AKMERROR_STR("You may refer invalid header file.");
		return AKM_ERROR;
	}
#endif

	if (AKD_GetDelay(delay) != AKD_SUCCESS) {
		AKMERROR;
		return AKM_ERROR;
	}
	AKMDEBUG(AKMDATA_LOOP, "delay[A,M,O]=%lld,%lld,%lld\n",
		delay[0], delay[1], delay[2]);

	/* update */
	*minimum = 1000000000;
	*flag = 0;
	for (i=0; i<AKM_NUM_SENSORS; i++) {
		/* Set flag */
		if (delay[i] >= 0) {
			*flag |= 1 << i;
			if (*minimum > delay[i]) {
				*minimum = delay[i];
			}
		}
	}
	return AKM_SUCCESS;
}

/*!
  If this program run as console mode, measurement result will be displayed
   on console terminal.
  @return None.
 */
void AKFS_OutputResult(
	const	uint16			flag,
	const	AKSENSOR_DATA*	acc,
	const	AKSENSOR_DATA*	mag,
	const	AKSENSOR_DATA*	ori
)
{
	short buf[AKM_YPR_DATA_SIZE];

#ifdef AKM_VALUE_CHECK
	if (AKM_YPR_DATA_SIZE < 12) {
		AKMERROR_STR("You may refer invalid header file.");
		return;
	}
#endif
	/* Store to buffer */
	buf[0] = flag;					/* Data flag */
	buf[1] = CONVERT_MAG(mag->x);	/* Mx */
	buf[2] = CONVERT_MAG(mag->y);	/* My */
	buf[3] = CONVERT_MAG(mag->z);	/* Mz */
	buf[4] = mag->status;			/* Mag status */
	buf[5] = CONVERT_ACC(acc->x);	/* Ax */
	buf[6] = CONVERT_ACC(acc->y);	/* Ay */
	buf[7] = CONVERT_ACC(acc->z);	/* Az */
	buf[8] = acc->status;			/* Acc status */
	buf[9] = CONVERT_ORI(ori->x);	/* yaw */
	buf[10] = CONVERT_ORI(ori->y);	/* pitch */
	buf[11] = CONVERT_ORI(ori->z);	/* roll */

	if (g_opmode & OPMODE_CONSOLE) {
		/* Console mode */
		Disp_Result(buf);
	}

	/* Set result to driver */
	AKD_SetYPR(buf);
}


/*!
 A thread function which is raised when measurement is started.
 @param[in] args This parameter is not used currently.
 */
static void* thread_main(void* args)
{
	AKMPRMS	*prms;
	BYTE    i2cData[AKM_SENSOR_DATA_SIZE]; /* ST1 ~ ST2 */
	int16	mag[3];
	int16	mstat;
	int16	acc[3];
	struct	timespec tsstart= {0, 0};
	struct	timespec tsend = {0, 0};
	struct	timespec doze;
	int64_t	minimum;
	uint16	flag;
	AKSENSOR_DATA sv_acc;
	AKSENSOR_DATA sv_mag;
	AKSENSOR_DATA sv_ori;
	AKFLOAT tmpx, tmpy, tmpz;
	int16 tmp_accuracy;

	prms = (AKMPRMS *)args;
	minimum = -1;

	/* Initialize library functions and device */
	if (AKFS_Start(prms, CSPEC_SETTING_FILE) != AKM_SUCCESS) {
		AKMERROR;
		goto MEASURE_END;
	}

	while (g_stopRequest != AKM_TRUE) {
		/* Beginning time */
		if (clock_gettime(CLOCK_MONOTONIC, &tsstart) < 0) {
			AKMERROR;
			goto MEASURE_END;
		}

		/* Get interval */
		if (AKFS_GetInterval(&flag, &minimum) != AKM_SUCCESS) {
			AKMERROR;
			goto MEASURE_END;
		}

		if ((flag & ACC_DATA_READY) || (flag & FUSION_DATA_READY)) {
			/* Get accelerometer */
			if (AKD_GetAccelerationData(acc) != AKD_SUCCESS) {
				AKMERROR;
				goto MEASURE_END;
			}

			/* Calculate accelerometer vector */
			if (AKFS_Get_ACCELEROMETER(prms, acc, 0, &tmpx, &tmpy, &tmpz, &tmp_accuracy) == AKM_SUCCESS) {
				sv_acc.x = tmpx;
				sv_acc.y = tmpy;
				sv_acc.z = tmpz;
				sv_acc.status = tmp_accuracy;
			} else {
				flag &= ~ACC_DATA_READY;
				flag &= ~FUSION_DATA_READY;
			}
		}

		if ((flag & MAG_DATA_READY) || (flag & FUSION_DATA_READY)) {
			/* Set to measurement mode  */
			if (AKD_SetMode(AKM_MODE_SNG_MEASURE) != AKD_SUCCESS) {
				AKMERROR;
				goto MEASURE_END;
			}

			/* Wait for DRDY and get data from device */
			if (AKD_GetMagneticData(i2cData) != AKD_SUCCESS) {
				AKMERROR;
				goto MEASURE_END;
			}
			/* raw data to x,y,z value */
			mag[0] = (int)((int16_t)(i2cData[2]<<8)+((int16_t)i2cData[1]));
			mag[1] = (int)((int16_t)(i2cData[4]<<8)+((int16_t)i2cData[3]));
			mag[2] = (int)((int16_t)(i2cData[6]<<8)+((int16_t)i2cData[5]));
			mstat = i2cData[0] | i2cData[7];

			/* Calculate magnetic field vector */
			if (AKFS_Get_MAGNETIC_FIELD(prms, mag, mstat, &tmpx, &tmpy, &tmpz, &tmp_accuracy) == AKM_SUCCESS) {
				sv_mag.x = tmpx;
				sv_mag.y = tmpy;
				sv_mag.z = tmpz;
				sv_mag.status = tmp_accuracy;
			} else {
				flag &= ~MAG_DATA_READY;
				flag &= ~FUSION_DATA_READY;
			}
		}

		if (flag & FUSION_DATA_READY) {
			if (AKFS_Get_ORIENTATION(prms, &tmpx, &tmpy, &tmpz, &tmp_accuracy) == AKM_SUCCESS) {
				sv_ori.x = tmpx;
				sv_ori.y = tmpy;
				sv_ori.z = tmpz;
				sv_ori.status = tmp_accuracy;
			} else {
				flag &= ~FUSION_DATA_READY;
			}
		}

		/* Output result */
		AKFS_OutputResult(flag, &sv_acc, &sv_mag, &sv_ori);

		/* Ending time */
		if (clock_gettime(CLOCK_MONOTONIC, &tsend) < 0) {
			AKMERROR;
			goto MEASURE_END;
		}

		/* Calculate duration */
		doze = AKFS_CalcSleep(&tsend, &tsstart, minimum);
		AKMDEBUG(AKMDATA_LOOP, "Sleep: %6.2f msec\n", (doze.tv_nsec/1000000.0f));
		nanosleep(&doze, NULL);

#ifdef WIN32
		if (_kbhit()) {
			_getch();
			break;
		}
#endif
	}

MEASURE_END:
	/* Set to PowerDown mode */
	if (AKD_SetMode(AKM_MODE_POWERDOWN) != AKD_SUCCESS) {
		AKMERROR;
	}

	/* Save parameters */
	if (AKFS_Stop(prms, CSPEC_SETTING_FILE) != AKM_SUCCESS) {
		AKMERROR;
	}
	return ((void*)0);
}

/*!
  Signal handler.  This should be used only in DEBUG mode.
  @param[in] sig Event
 */
static void signal_handler(int sig)
{
	if (sig == SIGINT) {
		AKMERROR;
		g_stopRequest = 1;
		g_mainQuit = AKD_TRUE;
	}
}

/*!
 Starts new thread.
 @return If this function succeeds, the return value is 1. Otherwise,
 the return value is 0.
 */
static int startClone(void *mem)
{
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	g_stopRequest = 0;
	if (pthread_create(&s_thread, &attr, thread_main, mem) == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*!
 This function parse the option.
 @retval 1 Parse succeeds.
 @retval 0 Parse failed.
 @param[in] argc Argument count
 @param[in] argv Argument vector
 @param[out] layout_patno
 */
int OptParse(
	int		argc,
	char*	argv[],
	AKFS_PATNO*	layout_patno)
{
#ifdef WIN32
	/* Static */
#if defined(AKFS_WIN32_PAT1)
	*layout_patno = PAT1;
#elif defined(AKFS_WIN32_PAT2)
	*layout_patno = PAT2;
#elif defined(AKFS_WIN32_PAT3)
	*layout_patno = PAT3;
#elif defined(AKFS_WIN32_PAT4)
	*layout_patno = PAT4;
#elif defined(AKFS_WIN32_PAT5)
	*layout_patno = PAT5;
#else
	*layout_patno = PAT1;
#endif
	g_opmode = OPMODE_CONSOLE;
	/*g_opmode = 0;*/
	g_dbgzone = AKMDATA_DUMP | AKMDATA_DEBUG | AKMDATA_CONSOLE;
#else
	int		opt;
	char	optVal;

	*layout_patno = PAT_INVALID;

	while ((opt = getopt(argc, argv, "sm:z:")) != -1) {
		switch(opt){
			case 'm':
				optVal = (char)(optarg[0] - '0');
				if ((PAT1 <= optVal) && (optVal <= PAT8)) {
					*layout_patno = (AKFS_PATNO)optVal;
					AKMDEBUG(AKMDATA_DEBUG, "%s: Layout=%d\n", __FUNCTION__, optVal);
				}
				break;
			case 's':
				g_opmode |= OPMODE_CONSOLE;
				break;
            case 'z':
                /* If error detected, hopefully 0 is returned. */
                errno = 0;
                g_dbgzone = (int)strtol(optarg, (char**)NULL, 0); 
                AKMDEBUG(AKMDATA_DEBUG, "%s: Dbg Zone=%d\n", __FUNCTION__, g_dbgzone);
                break;
			default:
				AKMERROR_STR("Invalid argument");
				return 0;
		}
	}

	/* If layout is not specified with argument, get parameter from driver */
	if (*layout_patno == PAT_INVALID) {
		int16_t n = 0;
		if (AKD_GetLayout(&n) == AKD_SUCCESS) {
			if ((PAT1 <= n) && (n <= PAT8)) {
				*layout_patno = (AKFS_PATNO)n;
			}
		}
		AKMDEBUG(AKMDATA_DEBUG, "Layout=%d\n", n);
	}
	/* Error */
	if (*layout_patno == PAT_INVALID) {
		AKMERROR_STR("No layout is specified.");
		return 0;
	}
#endif

	return 1;
}

void ConsoleMode(void *mem)
{
	/*** Console Mode *********************************************/
	while (AKD_TRUE) {
		/* Select operation */
		switch (Menu_Main()) {
		case MODE_Measure:
			/* Reset flag */
			g_stopRequest = 0;
			/* Measurement routine */
			thread_main(mem);
			break;

		case MODE_Quit:
			return;

		default:
			AKMERROR_STR("Unknown operation mode.\n");
			break;
		}
	}
}

int main(int argc, char **argv)
{
	int			retValue = 0;
	AKMPRMS		prms;
	AKFS_PATNO	pat;
	uint8		regs[3];

	/* Show the version info of this software. */
	Disp_StartMessage();

#if ENABLE_AKMDEBUG
	/* Register signal handler */
	signal(SIGINT, signal_handler);
#endif

	/* Open device driver */
	if(AKD_InitDevice() != AKD_SUCCESS) {
		retValue = ERROR_INITDEVICE;
		goto MAIN_QUIT;
	}

	/* Parse command-line options */
	/* This function calls device driver function to get layout */
	if (OptParse(argc, argv, &pat) == 0) {
		retValue = ERROR_OPTPARSE;
		goto MAIN_QUIT;
	}

	/* Self Test */
	/*
	if (g_opmode & OPMODE_FST){
		if (AKFS_SelfTest() != AKD_SUCCESS) {
			retValue = ERROR_SELF_TEST;
			goto MAIN_QUIT;
		}
	}*/

	/* OK, then start */
	if (AKFS_ReadConf(regs) != AKM_SUCCESS) {
		retValue = ERROR_READ_FUSE;
		goto MAIN_QUIT;
	}

	/* Initialize library. */
	if (AKFS_Init(&prms, pat, regs) != AKM_SUCCESS) {
		retValue = ERROR_INIT;
		goto MAIN_QUIT;
	}

	/* Start console mode */
	if (g_opmode & OPMODE_CONSOLE) {
		ConsoleMode((void *)&prms);
		goto MAIN_QUIT;
	}

	/*** Start Daemon ********************************************/
	while (g_mainQuit == AKD_FALSE) {
		int st = 0;
		/* Wait until device driver is opened. */
		if (AKD_GetOpenStatus(&st) != AKD_SUCCESS) {
			retValue = ERROR_GETOPEN_STAT;
			goto MAIN_QUIT;
		}
		if (st == 0) {
			ALOGI("Suspended.\n");
		} else {
			ALOGI("Compass Opened.\n");
			/* Reset flag */
			g_stopRequest = 0;
			/* Start measurement thread. */
			if (startClone((void *)&prms) == 0) {
				retValue = ERROR_STARTCLONE;
				goto MAIN_QUIT;
			}

			/* Wait until device driver is closed. */
			if (AKD_GetCloseStatus(&st) != AKD_SUCCESS) {
				retValue = ERROR_GETCLOSE_STAT;
				g_mainQuit = AKD_TRUE;
			}
			/* Wait thread completion. */
			g_stopRequest = 1;
			pthread_join(s_thread, NULL);
			ALOGI("Compass Closed.\n");
		}
	}

MAIN_QUIT:

	/* Release library */
	AKFS_Release(&prms);
	/* Close device driver. */
	AKD_DeinitDevice();
	/* Show the last message. */
	Disp_EndMessage(retValue);

	return retValue;
}


