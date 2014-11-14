/*
 * Definitions for akm8975 compass chip.
 */
#ifndef AKM8975_H
#define AKM8975_H

#include <linux/ioctl.h>

#ifdef __KERNEL__

#define AKM8975_I2C_NAME "akm8975"

struct akm8975_platform_data {
	int (*init)(void);
	void (*exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);
	char layout;
	char project_name[64];
	int gpio_DRDY;
};

#endif  /* __KERNEL__ */

#define AKM_MISCDEV_NAME "akm8975_dev"

/*! \name AK8975 operation mode
 \anchor AK8975_Mode
 Defines an operation mode of the AK8975.*/
/*! @{*/
#define AK8975_MODE_SNG_MEASURE	0x01
#define	AK8975_MODE_SELF_TEST	0x08
#define	AK8975_MODE_FUSE_ACCESS	0x0F
#define	AK8975_MODE_POWERDOWN	0x00
/*! @}*/

#define AKM_MODE_SNG_MEASURE	AK8975_MODE_SNG_MEASURE
#define AKM_MODE_SELF_TEST	AK8975_MODE_SELF_TEST
#define AKM_MODE_FUSE_ACCESS	AK8975_MODE_FUSE_ACCESS
#define AKM_MODE_POWERDOWN	AK8975_MODE_POWERDOWN

#define SENSOR_DATA_SIZE	8
#define RWBUF_SIZE			16
#define AKM_YPR_DATA_SIZE	12
#define AKM_RWBUF_SIZE		RWBUF_SIZE
#define AKM_NUM_SENSORS		3
#define YPR_DATA_SIZE		AKM_YPR_DATA_SIZE

#define AKM_SENSOR_INFO_SIZE    2
#define AKM_SENSOR_CONF_SIZE    3
#define AKM_SENSOR_DATA_SIZE    SENSOR_DATA_SIZE

#define MAG_DATA_READY		1
#define ACC_DATA_READY		2
#define ORI_DATA_READY		4
#define FUSION_DATA_READY	ORI_DATA_READY

/*! \name AK8975 register address
\anchor AK8975_REG
Defines a register address of the AK8975.*/
/*! @{*/
#define AK8975_REG_WIA		0x00
#define AK8975_REG_INFO		0x01
#define AK8975_REG_ST1		0x02
#define AK8975_REG_HXL		0x03
#define AK8975_REG_HXH		0x04
#define AK8975_REG_HYL		0x05
#define AK8975_REG_HYH		0x06
#define AK8975_REG_HZL		0x07
#define AK8975_REG_HZH		0x08
#define AK8975_REG_ST2		0x09
#define AK8975_REG_CNTL		0x0A
#define AK8975_REG_RSV		0x0B
#define AK8975_REG_ASTC		0x0C
#define AK8975_REG_TS1		0x0D
#define AK8975_REG_TS2		0x0E
#define AK8975_REG_I2CDIS	0x0F
/*! @}*/

#define AKM_MEASURE_TIME_US	10000

/*! \name AK8975 fuse-rom address
\anchor AK8975_FUSE
Defines a read-only address of the fuse ROM of the AK8975.*/
/*! @{*/
#define AK8975_FUSE_ASAX	0x10
#define AK8975_FUSE_ASAY	0x11
#define AK8975_FUSE_ASAZ	0x12
/*! @}*/

#define AKMIO                   0xA1

/* IOCTLs for AKM library */
#define ECS_IOCTL_WRITE             _IOW(AKMIO, 0x01, char*)
#define ECS_IOCTL_READ              _IOWR(AKMIO, 0x02, char*)
#define ECS_IOCTL_RESET             _IO(AKMIO, 0x03)
#define ECS_IOCTL_SET_MODE          _IOW(AKMIO, 0x04, short)
#define ECS_IOCTL_GETDATA           _IOR(AKMIO, 0x05, char[SENSOR_DATA_SIZE])
#define ECS_IOCTL_GET_DATA          ECS_IOCTL_GETDATA
#define ECS_IOCTL_SET_YPR           _IOW(AKMIO, 0x06, short[AKM_YPR_DATA_SIZE])
#define ECS_IOCTL_GET_OPEN_STATUS   _IOR(AKMIO, 0x07, int)
#define ECS_IOCTL_GET_CLOSE_STATUS  _IOR(AKMIO, 0x08, int)
#define ECS_IOCTL_GET_DELAY         _IOR(AKMIO, 0x30, long long int[AKM_NUM_SENSORS])
#define ECS_IOCTL_GET_PROJECT_NAME  _IOR(AKMIO, 0x0D, char[64])
#define ECS_IOCTL_GET_LAYOUT        _IOR(AKMIO, 0x0E, short)
#define ECS_IOCTL_GET_ACCEL			_IOR(AKMIO, 0x31, short[3])

/* IOCTLs for APPs */
#define ECS_IOCTL_APP_SET_MODE		_IOW(AKMIO, 0x10, short)/* NOT use */
#define ECS_IOCTL_APP_SET_MFLAG		_IOW(AKMIO, 0x11, short)
#define ECS_IOCTL_APP_GET_MFLAG		_IOW(AKMIO, 0x12, short)
#define ECS_IOCTL_APP_SET_AFLAG		_IOW(AKMIO, 0x13, short)
#define ECS_IOCTL_APP_GET_AFLAG		_IOR(AKMIO, 0x14, short)
#define ECS_IOCTL_APP_SET_DELAY		_IOW(AKMIO, 0x18, long long int[AKM_NUM_SENSORS])
#define ECS_IOCTL_APP_GET_DELAY		ECS_IOCTL_GET_DELAY
/* Set raw magnetic vector flag */
#define ECS_IOCTL_APP_SET_MVFLAG	_IOW(AKMIO, 0x19, short)
/* Get raw magnetic vector flag */
#define ECS_IOCTL_APP_GET_MVFLAG	_IOR(AKMIO, 0x1A, short)
#define ECS_IOCTL_APP_SET_ACCEL		_IOW(AKMIO, 0x1B, short[3])
#define ECS_IOCTL_APP_SET_TFLAG         _IOR(AKMIO, 0x15, short)
#define ECS_IOCTL_APP_SET_SOLANAFLAG    _IOW(AKMIO, 0x86, short)

#endif

