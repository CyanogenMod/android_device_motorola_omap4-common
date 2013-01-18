/*********************************************************************
 *
 * Filename:	  usb_app.c
 * Description:   USB daemon source code.
 *
 *	Motorola Confidential Proprietary
 *	Advanced Technology and Software Operations
 *	(c) Copyright Motorola 2009, All Rights Reserved
 *
 *	Modification Tracking
 *	Author		Date Number	 Description of Changes
 *	------------	-------------	--------------------------
 *	Motorola	01/16/2009	 Created file.
 *	Motorola	01/16/2009	 Removed USB Mass Storage over NAND flash
 *	Motorola	02/26/2009	 Add HID mode for charge only
 *	Motorola	06/30/2009	 Changed USB Daemon to support Garget Driver and USB mode switch
 *	Motorola	12/21/2009	 V1.1 - Cleaned up Error Handling in Usbd
 *	Motorola	01/13/2009	 V1.2 - Handle quick connect/disconnects
 *	Motorola	03/01/2011	 V2.0 - Rewritten Connection/Disconnection based on Switch Events
 *	Motorola	08/01/2011	 V2.1 - Changed to support tablet
 **********************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sched.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/un.h>
#include <linux/netlink.h>
#include <linux/loop.h>
#include <pthread.h>
#include <pwd.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <private/android_filesystem_config.h>

#include "usb_defines.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "usbd"
/* Update USBD Version for every code change */
#define USBD_VERSION "2.1"

#define LOOP_MAX 4096
#define LOOP_DEV_PATH_LEN 255

/*------------------------------*/
/*		MACROS		*/
/*------------------------------*/
#define MAX(x,y) (( x > y) ? x : y)
#define MAX_VERSION_SIZE 255

/*------------------------------*/
/*	LOCAL VARIABLES 	*/
/*------------------------------*/

typedef enum {
	MOTO_ACCY_TYPE_EMU_CABLE_USB = 0,
	MOTO_ACCY_TYPE_EMU_CABLE_FACTORY,
	MOTO_ACCY_TYPE_EMU_UNKNOWN
}MOTO_ACCY_TYPE_T;

typedef enum {
	MOTO_MODE_TYPE_NORMAL = 0,
	MOTO_MODE_TYPE_DEBUG,
	MOTO_MODE_TYPE_BPTOOLS,
	MOTO_MODE_TYPE_TABLET,
	MOTO_MODE_TYPE_UNKNOWN
} MOTO_MODE_TYPE_T;

#define USB_SWITCH_STATE_PATH "/sys/class/switch/usb_connected/state"

/* Current usb mode. */
USB_MODE_T  usbd_curr_usb_mode = USB_MODE_MIN;

/* Current USB State */
USBD_STATE_T usbd_current_state = USBD_STATE_CABLE_DEFAULT;

/* Local container of file descriptors for connected applications. */
static fd_set connected_apps_fd;


/* File descriptors for USBD Server. */
static int usbd_server_fd = -1;

/* File descriptors for ADB enable, PC request mode switch, and USB enmuerated */
static int usb_device_fd = -1;

static int uevent_sock = -1;

static int usbd_app_fd;

static int max_fd = -1;

static int usb_uid = AID_ROOT, usb_gid = AID_MOT_USB;

static int usb_get_des_count = 0;

static int current_usb_online = 0;

static MOTO_MODE_TYPE_T usbd_mode = MOTO_MODE_TYPE_NORMAL;
static int usb_alt_mode = 0;

static int tethering_enabled;

static MOTO_ACCY_TYPE_T  usbd_curr_cable_status = MOTO_ACCY_TYPE_EMU_UNKNOWN;

static char curr_loop_dev[LOOP_DEV_PATH_LEN + 1];

/* USB mode table for alternate modes
         TETHER DIS  TETHER ENA
ADB DIS      [ ]         [ ]
ADB ENA      [ ]         [ ]
*/
static USB_MODE_T usb_alt_mode_tlb[MOTO_MODE_TYPE_TABLET - MOTO_MODE_TYPE_BPTOOLS + 1][2][2] = {
	{/*BP TOOLS*/
		{USB_MODE_BPTOOLS_ETH, USB_MODE_RNDIS_BPTOOLS_ETH},
		{USB_MODE_BPTOOLS_ETH_ADB, USB_MODE_RNDIS_BPTOOLS_ETH_ADB}
	},
	{/*TABLET*/
		{USB_MODE_CDROM, USB_MODE_CDROM},
		{USB_MODE_TABLET_NETWORK_MTP_ADB, USB_MODE_RNDIS_ADB}
	},
};

#ifdef LOG_TAG
#define DEBUG(fmt, arg...) ALOGI("%s(): " fmt, __FUNCTION__, ##arg)
#else
#define DEBUG(fmt, arg...)
#endif

int usbd_get_adb_property(void);
int usbd_set_usb_mode(int new_mode);
/*
 * Function to set new usb mode for alternate modes
 */
int usbd_alt_mode_set(MOTO_MODE_TYPE_T mode, char *adb_buf, char *tether_buf)
{
	int adb_en = -1, tether_en = -1, alt_new_mode = -1;

	if (!strcmp(adb_buf, "adb_enable"))
		adb_en = 1;
	else if (!strcmp(adb_buf, "adb_disable"))
		adb_en = 0;

	if (!strcmp(tether_buf, "tethering_enable"))
		tether_en = 1;
	else if (!strcmp(tether_buf, "tethering_disable"))
		tether_en = 0;
	else if (adb_en < 0)
		return 0;

	if (tether_en >= 0)
		tethering_enabled = tether_en;

	if (mode == MOTO_MODE_TYPE_DEBUG) {
		if (adb_en == 1 || tether_en == 1)
			alt_new_mode = USB_MODE_MSC_ADB;
		else
			alt_new_mode = USB_MODE_MSC;
	} else if (mode == MOTO_MODE_TYPE_BPTOOLS || mode == MOTO_MODE_TYPE_TABLET) {
		if (adb_en < 0) {
			if ((adb_en = usbd_get_adb_property()) < 0)
				return -1;
		}
		alt_new_mode = (int)usb_alt_mode_tlb[mode - MOTO_MODE_TYPE_BPTOOLS]
							[adb_en][tethering_enabled];
	} else {
		DEBUG("Invalid usbd_mode : %d\n", mode);
		return -1;
	}

	if (alt_new_mode >= USB_MODE_MIN && alt_new_mode < USB_MODE_MAX)
		usbd_set_usb_mode(alt_new_mode);

	DEBUG("%s : ADB = %s, Tethering = %s ==> New mode is %d\n",
		(mode == MOTO_MODE_TYPE_DEBUG) ? "Debug" :
		(mode == MOTO_MODE_TYPE_BPTOOLS) ? "BP tools" : "Tablet",
		(adb_en == 1) ? "On" : (adb_en == 0) ? "Off" : "None",
		(tether_en == 1) ? "On" : (tether_en == 0) ? "Off" : "None",
		alt_new_mode);
	return 0;
}

int loop_create(const char *loopFile, char *loopDeviceBuffer, int len);

/*
 * The pre_enable/enable_done/pre_disable functions of MTP mode
 */
void mtp_enable_done(void)
{
}

/*
 * The pre_enable/enable_done/pre_disable functions of ADB mode
 */
void adb_pre_enable(void)
{
}

void adb_pre_disable(void)
{
}

/*
 * The pre_enable/enable_done/pre_disable functions of NGP mode
 */
void ngp_pre_enable(void)
{
}

void ngp_enable_done(void)
{
}

void ngp_pre_disable(void)
{
}

/*
 * The pre_enable/enable_done/pre_disable functions of ADB+NGP mode
 */
void ngp_adb_pre_enable(void)
{
}

void ngp_adb_enable_done(void)
{
}

void ngp_adb_pre_disable(void)
{
}

/*
 * The pre_enable/enable_done/pre_disable functions of ADB+MTP mode
 */
void mtp_adb_pre_enable(void)
{
}

void mtp_adb_enable_done(void)
{
}

void mtp_adb_pre_disable(void)
{
}


/*
 * The pre_enable/pre_disable functions of MODEM mode
 */
void modem_enable_done(void)
{
}

void modem_pre_disable(void)
{
}

/*
 * The pre_enable/pre_disable functions of ADB+MODEM mode
 */
void modem_adb_pre_enable(void)
{
}

void modem_adb_enable_done(void)
{
}
void modem_adb_pre_disable(void)
{
}

/*
 * This Function Reads the version key from the CDROM version file
 * and sends this information to the usb driver.
 * This function must be called only after cdrom_smart_version_mount has
 * been called to mount the cdrom locally at a mount point.
 */
int cdrom_read_version_key(void)
{
	/* File access to read binary data */
	FILE *file;
	unsigned long file_len = 0, max_version_len;
	int ret_val = 0, prefix_len;
	const char *path = "/cdrom/cdinfo";
	const char *prefix = "pc_mot_drv_vers_query:";
	/* Version file info */
	char buff[MAX_VERSION_SIZE];
	char *vers_info = buff;

	prefix_len = strlen(prefix);
	max_version_len = MAX_VERSION_SIZE - prefix_len - 1;
	strcpy(vers_info, prefix);
	vers_info += prefix_len;

	file = fopen(path, "rb");
	if (!file)
		ALOGD("Error to open %s(%s)\n", path, strerror(errno));
	else {
		/* Find the file length */
		fseek(file, 0, SEEK_END);
		file_len = ftell(file);
		fseek(file, 0, SEEK_SET);

		if(file_len > max_version_len)
			file_len = max_version_len;

		fread(vers_info, file_len, 1, file);
		vers_info[file_len] = '\0';
		/* Close the file pointer */
		fclose(file);

		/* Send the version info to the usb driver */
		/* Write data to "/dev/usb_device_fd" file descriptor */
		if (usb_device_fd < 0)
			ALOGD("Error: write via usb_device_fd descriptor\n");
		else {
			ret_val = write(usb_device_fd, buff,
						prefix_len + file_len + 1);
			if (ret_val < 0)
				ALOGD("Failed to send version info to the driver(%s)\n", strerror(errno));
		}
	}
	return ret_val;
}


/*
 * Function to mount/unmount the USB CD-ROM
 */
void cdrom_partition_mount(int mount)
{
	int fd, device_fd, rc;
	char ch = 0;
	const char *filename = "/cdrom/cdrom_vfat.bin";
	static int cdrom_mounted = 0;

	DEBUG("CDROM_PARTITION_MOUNT called to %s cdrom device\n", mount? "mount" : "unmount");
	fd = open(CDROMIO_DEVNODE, O_WRONLY);
	if(fd < 0 ) {
		DEBUG("Error to open %s for CDROM\n", CDROMIO_DEVNODE);
		return;
	}
	else
		DEBUG("Opened CDROM dev node successfully\n");

	/* If a block device exists for cdrom mount it directly */
	if( access(CDROM_BLOCK_DEVICE, F_OK) == 0 )
	{

		if(mount) {
			rc = write(fd, CDROM_BLOCK_DEVICE, strlen(CDROM_BLOCK_DEVICE));
			if( rc < 0)
				DEBUG("Error mounting %s for CDROM\n", CDROM_BLOCK_DEVICE);
			else
				DEBUG("Mounted cdrom block device on Mass Storage/CDROM driver successfully\n");
		} else {
			rc = write(fd, &ch, 1);
			if( rc < 0)
				DEBUG("Error unmounting block device for CDROM\n");
			else
				DEBUG("Unmounted cdrom block device successfully\n");
		}
		close(fd);
		return;
	}

	/* There is no CDROM Block device, let us mount/unmount it from the loop device */
	/* Unmount the cdrom and clear loop device if already mounted */
	if(cdrom_mounted){
		rc = write(fd, &ch, 1);
		if( rc < 0) {
			DEBUG("Error to unmount partition for CDROM\n");
		}
		else
			DEBUG("Unmounted loop device successfully\n");

		if ((device_fd = open(curr_loop_dev, O_RDONLY)) < 0) {
			ALOGE("Unable to open %s (%s)", curr_loop_dev, strerror(errno));
			return;
		}
		else
			DEBUG("Opened Loop Device Successfully \n");

		if (ioctl(device_fd, LOOP_CLR_FD, 0) < 0) {
			ALOGE("Error setting up loopback interface (%s)", strerror(errno));
			close(device_fd);
			return;
		}
		else
			DEBUG("Clear Loop Device Successfully\n");
		cdrom_mounted = 0;
		close(device_fd);

	}

	/* Create and Mount the loop device if this is a mount operation */
	if (mount) {
		rc = loop_create(filename, curr_loop_dev, LOOP_DEV_PATH_LEN);
		if (rc) {
			ALOGE("Unable to create loop dev bound %s-%s (%s)",
				curr_loop_dev, filename, strerror(errno));
			return;
		}
		else
			DEBUG("Created loop dev bound successfully\n");

		cdrom_mounted = 1;

		rc = write(fd, curr_loop_dev, strlen(curr_loop_dev));
		if (rc < 0) {
			DEBUG("Error to mount partition %s for CDROM\n", curr_loop_dev);
		}
		else
			DEBUG("Mounted loop device on Mass Storage/CDROM driver successfully\n");
	}

	close(fd);
}

void cdrom_pre_enable(void)
{
	system("echo 1 > /sys/module/g_mot_android/parameters/cdrom");
}

void cdrom_disable_done(void)
{
	system("echo 0 > /sys/module/g_mot_android/parameters/cdrom");
}

void cdrom_enable_done(void)
{
	cdrom_partition_mount(1);
}

void cdrom_pre_disable(void)
{
	cdrom_partition_mount(0);
}


/*
 * This Function Mounts the CDROM on a mount point /cdrom on the file system
 * or unmounts it from the file system.
 * Each Call to this function with imount=1 should be matched by a corresponding
 * call to imount=0.
 */
void cdrom_smart_version_mount(int imount)
{
	int device_fd, ret_val;
	/* Files to access */
	const char *filename = "/cdrom/cdrom_vfat.bin";
	const char *mount_dir = "/cdrom";
	const char *fs_type = "iso9660";


	ALOGD("CDROM_PARTITION_MOUNT called to %s cdrom device\n",
		imount? "mount" : "unmount");

	/* If a block device exists for cdrom mount it directly */
	if( access(CDROM_BLOCK_DEVICE, F_OK) == 0 ) {
		ALOGD("a block device exists for cdrom mount it directly\n");

		if(imount) {
			/* Mount the cdrom image in /cdrom and access the contents */
			ret_val = mount(CDROM_BLOCK_DEVICE, mount_dir, fs_type, MS_VERBOSE | MS_RDONLY, NULL);
			if (ret_val == 0)
				ALOGD("SUCCESS: SMART VER. CDROM mounted\n");
			else
				ALOGD("FAIL: mount SMART VER. CDROM %d (%s)\n", ret_val, strerror(errno));
		} else {
			ret_val = umount(mount_dir);
			if( ret_val == 0)
				ALOGD("Unmounted cdrom block device successfully\n");
			else
				ALOGD("Error unmounting block device for CDROM\n");
		}
		return;
	}

	/* There is no CDROM Block device, let us mount/unmount it from the loop device */
	if (imount) {
		/* Create and Mount the loop device if this is a mount operation */
		ret_val = loop_create(filename, curr_loop_dev, LOOP_DEV_PATH_LEN);
		if (ret_val) {
			ALOGD("Unable to create loop dev bound %s-%s (%s)",
				curr_loop_dev, filename, strerror(errno));
			return;
		}
		else
			ALOGD("Created loop dev bound successfully\n");

		/* Mount the cdrom image in /data/cdrom and access the contents */
		ret_val = mount(curr_loop_dev, mount_dir, fs_type, MS_VERBOSE | MS_RDONLY, NULL);
		if (ret_val == 0)
			ALOGD("SUCCESS: SMART VER. CDROM mounted\n");
		else
			ALOGD("FAIL: mount SMART VER. CDROM %d (%s)\n", ret_val, strerror(errno));

	} else {
		/* Unmount the cdrom and clear loop device if already mounted */
		ret_val = umount(mount_dir);
		if (ret_val == 0)
			ALOGD("SUCCESS: SMART VER. CDROM unmounted\n");
		else
			ALOGD("FAIL: unmount SMART VER. CDROM %d (%s)\n", ret_val, strerror(errno));

		if ((device_fd = open(curr_loop_dev, O_RDONLY)) < 0) {
			ALOGE("Unable to open %s (%s)", curr_loop_dev, strerror(errno));
			return;
		}
		else
			ALOGD("Opened Loop Device Successfully \n");

		if (ioctl(device_fd, LOOP_CLR_FD, 0) < 0) {
			ALOGE("Error setting up loopback interface (%s)", strerror(errno));
			return;
		}
		else
			ALOGD("Clear Loop Device Successfully\n");
		close(device_fd);
	}

}

/*
 * The pre_enable/pre_disable functions of MSC mode
 */
void msc_pre_enable(void)
{
#ifdef MSD_CDROM_ENABLED
	system("echo 1 > /sys/module/g_mot_android/parameters/cdrom");
	system("echo 0 > /sys/module/g_mot_android/parameters/allow_eject");
#endif
}

void msc_enable_done(void)
{
#ifdef MSD_CDROM_ENABLED
	cdrom_partition_mount(1);
#endif
	system("echo 1 > /sys/devices/virtual/usb_composite/usb_mass_storage/enable");
}

void msc_pre_disable(void)
{
#ifdef MSD_CDROM_ENABLED
	cdrom_partition_mount(0);
#endif
	system("echo 0 > /sys/devices/virtual/usb_composite/usb_mass_storage/enable");
}

void msc_disable_done(void)
{
#ifdef MSD_CDROM_ENABLED
	system("echo 0 > /sys/module/g_mot_android/parameters/cdrom");
	system("echo 1 > /sys/module/g_mot_android/parameters/allow_eject");
#endif
}

/*
 * The enable_done/pre_disable functions of MSC only mode
 */
void msc_only_enable_done(void)
{
	system("echo 1 > /sys/devices/virtual/usb_composite/usb_mass_storage/enable");
}

void msc_only_pre_disable(void)
{
	system("echo 0 > /sys/devices/virtual/usb_composite/usb_mass_storage/enable");
}

/*
 * The pre_enable/pre_disable functions of ADB+MSC mode
 */
void msc_adb_enable_done(void)
{
	system("echo 1 > /sys/devices/virtual/usb_composite/usb_mass_storage/enable");
}

void msc_adb_pre_disable(void)
{
	system("echo 0 > /sys/devices/virtual/usb_composite/usb_mass_storage/enable");
}

/*
 * The pre_enable/enable_done/pre_disable functions of Network mode
 */
void network_enable_done(void)
{
}

void network_pre_disable(void)
{
}

/*
 * The structure for all supported USB modes
 */
struct usb_mode_data usb_mode_list[USB_MODE_MAX + 1] = {
	{
		mode: USB_MODE_MIN,
		name: "unuse",
		mode_req:  "usb_unload_driver"
	},
/*      Uncomment for disabling CDROM mode
	{
		mode: USB_MODE_NGP,
		name: "acm_eth_mtp",
		pre_enable:  ngp_pre_enable,
		enable_done: ngp_enable_done,
		pre_disable: ngp_pre_disable,
		mode_req:  "usb_mode_ngp",
		mode_done:   USBD_START_NGP,
		switch_req:  USBD_REQ_SWITCH_NGP,
		resp_ok:   "usb_mode_ngp:ok",
		resp_fail: "usb_mode_ngp:fail"
	},
*/
	{
		mode: USB_MODE_NGP,
		name: "acm_eth_mtp",
		pre_enable:  ngp_pre_enable,
		enable_done: ngp_enable_done,
		pre_disable: ngp_pre_disable,
		mode_done:   USBD_START_NGP,
		switch_req:  USBD_REQ_SWITCH_NGP,
		resp_fail: "usb_mode_ngp:fail"
	},
	{
		mode: USB_MODE_CDROM,
		name: "cdrom",
		pre_enable: cdrom_pre_enable,
		enable_done: cdrom_enable_done,
		pre_disable: cdrom_pre_disable,
		disable_done: cdrom_disable_done,
		mode_req:  "usb_mode_ngp",
		resp_ok:   "usb_mode_ngp:ok",
		resp_fail: "usb_mode_ngp:fail"
	},

	{
		mode: USB_MODE_MTP,
		name: "mtp",
		enable_done: mtp_enable_done,
		mode_req:  "usb_mode_mtp",
		mode_done:  USBD_START_MTP,
		switch_req:  USBD_REQ_SWITCH_MTP,
		resp_ok:   "usb_mode_mtp:ok",
		resp_fail: "usb_mode_mtp:fail"
	},
	{
		mode: USB_MODE_MSC,
		name: "msc",
		pre_enable: msc_pre_enable,
		enable_done: msc_enable_done,
		pre_disable: msc_pre_disable,
		disable_done: msc_disable_done,
		mode_req:  "usb_mode_msc",
		mode_done:  USBD_START_MSC_MOUNT,
		switch_req:  USBD_REQ_SWITCH_MSC,
		resp_ok:   "usb_mode_msc:ok",
		resp_fail: "usb_mode_msc:fail"
	},
	{
		mode: USB_MODE_MSC_ONLY,
		name: "msc_only",
		enable_done: msc_only_enable_done,
		pre_disable: msc_only_pre_disable,
		mode_req:  "usb_mode_msc_only",
		mode_done:  USBD_START_MSC_MOUNT,
		resp_ok:   "usb_mode_msc:ok",
		resp_fail: "usb_mode_msc:fail"
	},
	{
		mode: USB_MODE_HID,
		name: "hid",
		mode_req:  "usb_mode_hid",
		switch_req:  USBD_REQ_SWITCH_HID,
		resp_ok:   "usb_mode_hid:ok",
		resp_fail: "usb_mode_hid:fail"
	},
	{
		mode: USB_MODE_NGP_ADB,
#ifdef AT_RELAY
		name: "acm_acm1_eth_mtp_adb",
#else
		name: "acm_eth_mtp_adb",
#endif
		pre_enable:  ngp_adb_pre_enable,
		enable_done: ngp_adb_enable_done,
		pre_disable: ngp_adb_pre_disable,
		mode_req:  "usb_mode_ngp_adb",
		mode_done:  USBD_START_NGP,
		switch_req:  USBD_REQ_SWITCH_NGP,
		resp_ok:   "usb_mode_ngp:ok",
		resp_fail: "usb_mode_ngp:fail"
	},
	{
		mode: USB_MODE_MTP_ADB,
		name: "mtp_adb",
		pre_enable:  mtp_adb_pre_enable,
		pre_disable: mtp_adb_pre_disable,
		enable_done: mtp_adb_enable_done,
		mode_req:  "usb_mode_mtp_adb",
		mode_done:  USBD_START_MTP,
		switch_req:  USBD_REQ_SWITCH_MTP,
		resp_ok:   "usb_mode_mtp:ok",
		resp_fail: "usb_mode_mtp:fail"
	},
	{
		mode: USB_MODE_MSC_ADB,
		name: "msc_adb",
		enable_done: msc_adb_enable_done,
		pre_disable: msc_adb_pre_disable,
		mode_req:  "usb_mode_msc_adb",
		mode_done:  USBD_START_MSC_MOUNT,
		switch_req:  USBD_REQ_SWITCH_MSC,
		resp_ok:   "usb_mode_msc:ok",
		resp_fail: "usb_mode_msc:fail"
	},
	{
		mode: USB_MODE_ADB,
		name: "android_adb_only",
		pre_enable: adb_pre_enable,
		pre_disable: adb_pre_disable,
		mode_req:  "usb_mode_adb",
		switch_req:  USBD_REQ_SWITCH_HID,
		resp_ok:   "usb_mode_hid:ok",
		resp_fail: "usb_mode_hid:fail"
	},
	{
		mode: USB_MODE_NETWORK,
		name: "eth",
		enable_done: network_enable_done,
		pre_disable: network_pre_disable,
	},
	{
		mode: USB_MODE_MODEM,
		name: "acm",
		pre_disable: modem_pre_disable,
		enable_done: modem_enable_done,
		mode_req:  "usb_mode_modem",
		switch_req:  USBD_REQ_SWITCH_MODEM,
		resp_ok:   "usb_mode_modem:ok",
		resp_fail: "usb_mode_modem:fail"
	},
	{
		mode: USB_MODE_MODEM_ADB,
		name: "modem_adb",
		pre_enable: modem_adb_pre_enable,
		enable_done: modem_adb_enable_done,
		pre_disable: modem_adb_pre_disable,
		mode_req:  "usb_mode_modem_adb",
		switch_req:  USBD_REQ_SWITCH_MODEM,
		resp_ok:   "usb_mode_modem:ok",
		resp_fail: "usb_mode_modem:fail"
	},
	{
		mode: USB_MODE_NETWORK_ADB,
#ifdef AT_RELAY
		name: "acm1_eth_adb",
#else
		name: "eth_adb",
#endif
		enable_done: network_enable_done,
		pre_disable: network_pre_disable
	},
	{
		mode: USB_MODE_CHARGE_ONLY,
		name: "charge_only",
		mode_req:  "usb_mode_charge_only",
		switch_req:  USBD_REQ_SWITCH_CHARGE_ONLY,
		resp_ok:   "usb_mode_msc:ok",
		resp_fail: "usb_mode_msc:fail"
	},
	{
		mode: USB_MODE_CHARGE_ADB,
		name: "charge_adb",
		mode_req:  "usb_mode_charge_adb",
		switch_req:  USBD_REQ_SWITCH_CHARGE_ONLY,
		resp_ok:   "usb_mode_msc:ok",
		resp_fail: "usb_mode_msc:fail"
	},
	{
		mode: USB_MODE_RNDIS,
		name: "rndis",
		mode_req:  "usb_mode_rndis",
		switch_req:  USBD_REQ_SWITCH_RNDIS,
		resp_ok:   "usb_mode_rndis:ok",
		resp_fail: "usb_mode_rndis:fail"
	},
	{
		mode: USB_MODE_RNDIS_ADB,
		name: "rndis_adb",
		mode_req:  "usb_mode_rndis_adb",
		switch_req:  USBD_REQ_SWITCH_RNDIS,
		resp_ok:   "usb_mode_rndis:ok",
		resp_fail: "usb_mode_rndis:fail"
	},
	{
		mode: USB_MODE_CDROM2,
		name: "cdrom2",
		pre_enable: cdrom_pre_enable,
		enable_done: cdrom_enable_done,
		pre_disable: cdrom_pre_disable,
		disable_done: cdrom_disable_done,
	},
	{
		mode: USB_MODE_BPTOOLS_ETH,
		name: "acm_acm1_acm2_acm3_eth",
		mode_req: "usb_mode_acm_acm1_acm2_acm3_eth",
		switch_req: USBD_REQ_SWITCH_BPTOOLS,
		resp_ok: "usb_mode_bptools:ok",
		resp_fail: "usb_mode_bptools:fail"
	},
	{
		mode: USB_MODE_BPTOOLS_ETH_ADB,
		name: "acm_acm1_acm2_acm3_eth_adb",
		mode_req: "usb_mode_acm_acm1_acm2_acm3_eth_adb",
		switch_req: USBD_REQ_SWITCH_BPTOOLS,
		resp_ok: "usb_mode_bptools:ok",
		resp_fail: "usb_mode_bptools:fail"
	},
	{
		mode: USB_MODE_RNDIS_BPTOOLS_ETH,
		name: "rndis_acm_acm1_acm2_acm3_eth",
		mode_req: "usb_mode_rndis_acm_acm1_acm2_acm3_eth",
		switch_req: USBD_REQ_SWITCH_BPTOOLS,
		resp_ok: "usb_mode_bptools:ok",
		resp_fail: "usb_mode_bptools:fail"
	},
	{
		mode: USB_MODE_RNDIS_BPTOOLS_ETH_ADB,
		name: "rndis_acm_acm1_acm2_acm3_eth_adb",
		mode_req: "usb_mode_rndis_acm_acm1_acm2_acm3_eth_adb",
		switch_req: USBD_REQ_SWITCH_BPTOOLS,
		resp_ok: "usb_mode_bptools:ok",
		resp_fail: "usb_mode_bptools:fail"
	},
	{
		mode: USB_MODE_TABLET_NETWORK_MTP,
		name: "eth_mtp"
	},
	{
		mode: USB_MODE_TABLET_NETWORK_MTP_ADB,
		name: "eth_mtp_adb"
	},
	{
		mode: USB_MODE_MAX,
		name: "unuse",
	},
};

/*
 * Set the usb mode.
 */
int usbd_set_usb_mode(int new_mode)
{
	/* Call pre_disable of current USB mode */
	if (usb_mode_list[usbd_curr_usb_mode].pre_disable)
	{
		usb_mode_list[usbd_curr_usb_mode].pre_disable();
	}

	/* Call disable_done of current USB mode */
	if (usb_mode_list[usbd_curr_usb_mode].disable_done)
	{
		usb_mode_list[usbd_curr_usb_mode].disable_done();
	}


	usbd_curr_usb_mode = new_mode;

	/* The unavailable USB mode USB_MODE_MIN */
	if(new_mode == USB_MODE_MIN) {
		return 0;
	}

	if (usb_mode_list[new_mode].pre_enable)
	{
		usb_mode_list[new_mode].pre_enable();
	}

	DEBUG("new_mode: %s\n",usb_mode_list[new_mode].name);
	write(usb_device_fd, usb_mode_list[new_mode].name, strlen(usb_mode_list[new_mode].name)+1);

	return 0;
}

/*
 * Create USBD socket
 */
int  usbd_socket_init(void)
{
	  usbd_server_fd = socket_local_server(USBD_SOCKET_NAME, ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
	  
	  /* If we couldn't make the socket, close the USB Daemon. */
	  if (usbd_server_fd < 0)
	  {
		DEBUG("usbd_socket_init error create socket\n");
		return -1;
	  }
	  chown(USBD_SOCKET_FULLNAME, usb_uid, usb_gid);
	  chmod(USBD_SOCKET_FULLNAME, 00660);

	  return 0;
}

/*
 * Application socket message process
 */
 /*important function*/
int usbd_socket_event(int client_fd)
{
	int rc, i;
	char buf[SOCKET_BUFFER_SIZE];

	memset(buf, 0, SOCKET_BUFFER_SIZE);

	rc = read(client_fd, buf, SOCKET_BUFFER_SIZE);
	if (rc < 0) {
		//This might be an error due to socket close
		DEBUG(" Socket Read Failure with errno %d, i.e %s\n", errno, strerror(errno));
		return -1;
	} else if( !rc ) {
		DEBUG("Socket Connection Closed\n");
		close(client_fd);
		return -ECONNRESET;
	}


	DEBUG("recieved %s\n", buf);

	/* Read and Ignore the Application message if it is a Factory Cable */
	if (usbd_curr_cable_status == MOTO_ACCY_TYPE_EMU_CABLE_FACTORY)
		return 0;

	for (i = USB_MODE_MIN; i < USB_MODE_MAX; i++)
	{
		if(usb_mode_list[i].mode_req != NULL) {
			if(strcmp(buf, usb_mode_list[i].mode_req) == 0) 
			{
				DEBUG("Matched new usb mode = %d , current mode = %d\n", i, usbd_curr_usb_mode);
				break;
			}
		}
	}

	if( i == USB_MODE_MIN ){
		usbd_set_usb_mode(i);
		return 0;
	}

	if ((i > USB_MODE_MIN) && (i < USB_MODE_MAX) )
	{
		if (usb_mode_list[i].resp_ok != NULL) {
			rc = write(client_fd,  usb_mode_list[i].resp_ok, strlen(usb_mode_list[i].resp_ok) + 1);

			if (rc < 0) {
				//This might be an error due to socket close
				DEBUG("Socket Write Failure with errno %d, i.e %s\n", errno, strerror(errno));
					return -1;
			}
		}

		// whether the current mode is the requested mode
		if(usbd_curr_usb_mode != (USB_MODE_T)i)  
		{
			if(current_usb_online)
				rc = usbd_set_usb_mode(i);
			else
				DEBUG("Not Setting the usb mode, since we went offline \n");
		}
		else {
			if (usb_mode_list[usbd_curr_usb_mode].mode_done != NULL) {
				rc = write(client_fd, usb_mode_list[usbd_curr_usb_mode].mode_done, strlen(usb_mode_list[usbd_curr_usb_mode].mode_done) + 1);
				if (rc < 0) {
					//This might be an error due to socket close
					DEBUG("Socket Write Failure with errno %d, i.e %s\n", errno, strerror(errno));
					return -1;
				}
			}
		} 
	}


	return 0;
}

/*
 * Get the max FD
 */
int usbd_get_maxfd(void)
{
	int fd;
   
	fd = uevent_sock; 
	fd = MAX(fd, usb_device_fd);
	fd = MAX(fd, usbd_server_fd);
	fd = MAX(fd, usbd_app_fd);
	return fd;
}

/*
 * Save the application connection
 */
void usbd_save_socket_fd(int newfd)
{
	if(usbd_app_fd == -1) {
		FD_SET(newfd, &connected_apps_fd);
		usbd_app_fd = newfd;
		return;
	}
	DEBUG("New socket connection is not supported");
}

/*
 * ADB event handle, and will send ADB status to application
 */
int usbd_get_adb_property(void)
{
	int rc, en_flag;
	char buf[PROPERTY_VALUE_MAX];

	/* Send ADB status */
	rc = property_get("persist.service.adb.enable", buf, "0");
	if( rc == 0 ) {  // Error to get ADB property
		return -1;
	}
	en_flag = 0;
	if(!strcmp(buf, "1")) {
		en_flag = 1;
	}
	return en_flag;
}

int usbd_get_startup_mode(void)
{
	char buf[PROPERTY_VALUE_MAX];

	/* Check the Boot mode */
	property_get("ro.usb_mode", buf, "normal");

	if(!strcmp(buf, "debug"))
		return MOTO_MODE_TYPE_DEBUG;

	/* Check the usb.android_config */
	property_get("persist.usb.android_config", buf, "0");

	if(!strcmp(buf, "1"))
		return MOTO_MODE_TYPE_DEBUG;

	property_get("ro.bootmode", buf, "normal");

	if(!strcmp(buf, "bp-tools"))
		return MOTO_MODE_TYPE_BPTOOLS;

#ifdef USBD_TABLET_MODE
	return MOTO_MODE_TYPE_TABLET;
#endif

	return MOTO_MODE_TYPE_NORMAL;
}

int usbd_enumerate_default_mode()
{
	switch (usbd_mode) {
		case MOTO_MODE_TYPE_DEBUG:
			if(usbd_get_adb_property())
				usbd_set_usb_mode(USB_MODE_MSC_ADB);
			else
				usbd_set_usb_mode(USB_MODE_MSC);
		break;
		case MOTO_MODE_TYPE_BPTOOLS:
			if(usbd_get_adb_property())
				usbd_set_usb_mode(USB_MODE_BPTOOLS_ETH_ADB);
			else
				usbd_set_usb_mode(USB_MODE_BPTOOLS_ETH);
		break;
		case MOTO_MODE_TYPE_TABLET:
			if(usbd_get_adb_property())
				usbd_set_usb_mode(USB_MODE_TABLET_NETWORK_MTP_ADB);
			else
				usbd_set_usb_mode(USB_MODE_CDROM);
		break;
		default:
		break;
	}

        return 0;
}

int usbd_factorycable_adb_enabled(void)
{
        char buf[PROPERTY_VALUE_MAX];

        /* Check if ADB is enabled for the Factory mode for debugging */
        property_get("persist.factory.allow_adb", buf, "0");

        if(!strcmp(buf, "1"))
                return 1;

        return 0;
}

int usbd_send_adb_status(int newfd, int en_flag)
{
	int rc;
	 
	if ( en_flag == 1 ) {
		rc = write(newfd, USBD_EVENT_ADB_ON, strlen(USBD_EVENT_ADB_ON)+1);
		DEBUG("Send ADB Enable message\n");
	}
	else  {
		rc = write(newfd, USBD_EVENT_ADB_OFF, strlen(USBD_EVENT_ADB_OFF)+1);
		DEBUG("Send ADB Disable message\n");
	}

	return rc;
}

int usbd_send_tethering_status(int newfd, int en_flag)
{
        int rc;

        if ( en_flag == 1 ) {
                rc = write(newfd, USBD_EVENT_TETHERING_ON, strlen(USBD_EVENT_TETHERING_ON)+1);
                DEBUG("Send tethering Enable message\n");
        }
        else  {
                rc = write(newfd, USBD_EVENT_TETHERING_OFF, strlen(USBD_EVENT_TETHERING_OFF)+1);
                DEBUG("Send tethering Disable message\n");
        }

        return rc;
}



/*
 * Notify App of the Cable State . This is done once after the App connects to usbd
 */
int  usbd_notify_current_status(int appfd)
{
	int rc = 0;
	char* notification_message = NULL;

	switch (usbd_current_state) {
		case USBD_STATE_CABLE_ATTACHED:
			notification_message = USBD_CABLE_ATTACHED;
			break;
		case USBD_STATE_CABLE_DETACHED:
			notification_message = USBD_CABLE_DETACHED;
			break;
		case USBD_STATE_ENUM_IN_PROGRESS:
			notification_message = USBD_GET_DESCRIPTOR;
			break;
		case USBD_STATE_ENUMERATED:
			notification_message = USBD_ENUMERATED;
			break;
		case USBD_STATE_CABLE_DEFAULT:
			break;
		default:
			break;

	}

	if (notification_message != NULL) {
		DEBUG("Notifying App with Current Status : %s \n", notification_message);
		rc = write(appfd, notification_message , strlen(notification_message) + 1);
		if (rc < 0 ) {
			DEBUG(" Write Error : Notifying App with Current Status \n");
			return -1;
		}

	} else
		DEBUG("Nothing to notify \n");

	return rc;
}

/*
 * Detect whether the USB enumeration is done.
 * If the enumeartion is done,  send the enumeration ok message
 */
int usbd_enum_process(int newfd)
{
	int rc;
	
	DEBUG("current usb mode = %d\n",usbd_curr_usb_mode);
	
	// Execute enable_done	  
	if (usb_mode_list[usbd_curr_usb_mode].enable_done) {
		usb_mode_list[usbd_curr_usb_mode].enable_done();
	}

	if ((newfd >= 0) && usb_mode_list[usbd_curr_usb_mode].mode_done)
	{
		DEBUG("send %s to fd[%d]\n", usb_mode_list[usbd_curr_usb_mode].mode_done, newfd);
		rc = write(newfd, usb_mode_list[usbd_curr_usb_mode].mode_done, strlen(usb_mode_list[usbd_curr_usb_mode].mode_done) + 1);
		if (rc < 0) {
			//This might be an error due to socket close
			DEBUG("Socket Write Failure with errno %d, i.e %s\n", errno, strerror(errno));
			return -1;
		}
	}

	DEBUG("enum done\n");

	return 0;

}


/*
 *  Change U
 */
void usb_req_mode_switch(char* tag)
{
	char * temp_ptr = NULL;
	int  rc, en_flag, i, mode = 0;

	en_flag = usbd_get_adb_property();
	if( en_flag < 0) {
		return;
	}

	for(i = USB_MODE_MIN; i < USB_MODE_MAX; i ++) {
		if( !strcmp(usb_mode_list[i].name, tag) && usb_mode_list[i].switch_req ) {
			mode = i;
			temp_ptr = usb_mode_list[i].switch_req;
			DEBUG("switch_req=%s\n", usb_mode_list[i].switch_req);
			break;
		}
	}

	if (!temp_ptr) {
		if (!strcmp(tag, "cdrom2")) {
			usbd_set_usb_mode(USB_MODE_CDROM2);
		}
		return;
	}

	// If current mode is CDROM, change to NGP directly and do not notify UI

	if( (usbd_curr_usb_mode == USB_MODE_CDROM) || (usbd_curr_usb_mode == USB_MODE_CDROM2) ) {
		DEBUG("USB_MODE_CDROM switch to NGP...\n");
		if (usbd_mode == MOTO_MODE_TYPE_TABLET) {
			if (en_flag == 0) {
				if (tethering_enabled)
					usbd_set_usb_mode(USB_MODE_RNDIS);
				else
					usbd_set_usb_mode(USB_MODE_TABLET_NETWORK_MTP);
			} else if (en_flag == 1) {
				if (tethering_enabled)
					usbd_set_usb_mode(USB_MODE_RNDIS_ADB);
				else
					usbd_set_usb_mode(USB_MODE_TABLET_NETWORK_MTP_ADB);
			}
		} else {
			if (en_flag == 0) {
				usbd_set_usb_mode(USB_MODE_NGP);
			} else if(en_flag == 1) {
				usbd_set_usb_mode(USB_MODE_NGP_ADB);
			}
		}
		return;
	}

	if(usbd_app_fd >= 0) {
		DEBUG("usb switch to %s...\n", usb_mode_list[mode].name);
		rc = write(usbd_app_fd, temp_ptr, strlen(temp_ptr) + 1);
		if (rc < 0) {
			//This might be an error due to socket close
			DEBUG("Socket Write Failure with errno %d, i.e %s\n", errno, strerror(errno));
			/* Clear the file descriptor. */
			FD_CLR(usbd_app_fd, &connected_apps_fd);
			usbd_app_fd = -1;
			max_fd = usbd_get_maxfd();
		}
	}

}

/*
 * This function is used to read information out of sysfs
 */
int copy_string_from_file(const char *filename, char *result, int size)
{
	int fd = -1;

	fd = open(filename, O_RDONLY, 0);
	if(fd == -1)
		return -1;

	ssize_t count = read(fd, result, size);
	if(count < 0) {
		close(fd);
		return -1;
	}
	if(count > 0)
		result[count-1] = '\0';
	else
		result[0] = '\0';

	close(fd);

	return 0;
}

int readUsbSwitchState(void)
{
	FILE *fp;
	char state[255];

	if ((fp = fopen(USB_SWITCH_STATE_PATH, "r"))) {
		if (fgets(state, sizeof(state), fp)) {
			fclose(fp);
			switch (state[0]) {
			case CABLE_USB:
				return MOTO_ACCY_TYPE_EMU_CABLE_USB;
			case CABLE_FACTORY:
				return MOTO_ACCY_TYPE_EMU_CABLE_FACTORY;
			case CABLE_NONE:
			default:
				return MOTO_ACCY_TYPE_EMU_UNKNOWN;
			}
		} else {
			ALOGE("Failed to read usb switch (%s)", strerror(errno));
			fclose(fp);
			return -1;
		}
	} else {
		ALOGE("Failed opening %s (errno = %s)", USB_SWITCH_STATE_PATH, strerror(errno));
		return -1;
	}
}

#define USBD_UEVENT_MSG_LEN 64*1024
int process_usb_uevent_message(int socket)
{
	char buffer[USBD_UEVENT_MSG_LEN];
	int count;
	int busbEvent = 0;
	int current_cable_status;

	char cred_msg[CMSG_SPACE(sizeof(struct ucred))];
	struct iovec iov = {buffer, sizeof(buffer)};
	struct sockaddr_nl snl;
	struct msghdr hdr = {&snl, sizeof(snl), &iov, 1, cred_msg, sizeof(cred_msg), 0};

	count = recvmsg(socket, &hdr, 0);

	if(count <= 0)
		return busbEvent;

	if ((snl.nl_groups != 1) || (snl.nl_pid != 0)) {
		/* ignoring non-kernel netlink multicast message */
		return busbEvent;
	}

	struct cmsghdr * cmsg = CMSG_FIRSTHDR(&hdr);
	if (cmsg == NULL || cmsg->cmsg_type != SCM_CREDENTIALS) {
		/* no sender credentials received, ignore message */
		return busbEvent;
	}

	struct ucred * cred = (struct ucred *)CMSG_DATA(cmsg);
	if (cred->uid != 0) {
		/* message from non-root user, ignore */
		return busbEvent;
	}

	if (count >= (USBD_UEVENT_MSG_LEN-1))	/* overflow -- discard */
		return busbEvent;

	buffer[count] = '\0';
	buffer[count+1] = '\0';

	if ((strcasestr(buffer, "usb_connected")) && (strcasestr(buffer, "switch"))) {
		current_cable_status = readUsbSwitchState();

		if (current_cable_status < 0) {
			ALOGE("Failed to read Cable State - returning \n");
			return busbEvent;
		} else {
			usbd_curr_cable_status = current_cable_status;
			DEBUG("Cable State Changed , state = %d\n",usbd_curr_cable_status);
		}

		busbEvent = 1;
		if (usbd_curr_cable_status != MOTO_ACCY_TYPE_EMU_UNKNOWN) {
			current_usb_online = 1;
			usbd_current_state = USBD_STATE_CABLE_ATTACHED;
		} else {
			current_usb_online = 0;
			usbd_current_state = USBD_STATE_CABLE_DETACHED;
			usb_get_des_count = 0;
			write(usb_device_fd, "usb_cable_detach", strlen("usb_cable_detach")+1);
		}
	} else if (strcasestr(buffer, "scsi_device")) {
		char* s = buffer;
		char* end = s + count;

		char* a = NULL;
		char* subsys_type = NULL;
		char* dev_path = NULL;
		int first_run = 1;
		char* p = NULL;

		while (s < end) {
			if (first_run) {
				first_run = 0;
				for (p = s; *p != '@'; p++);
				p++;
				dev_path = p;
			}

			if (!strncmp("ACTION=", s, strlen("ACTION=")))
				a = s + strlen("ACTION=");
			else if (!strncmp("SUBSYSTEM=", s, strlen("SUBSYSTEM=")))
				subsys_type = s + strlen("SUBSYSTEM=");
			s += (strlen(s) + 1);
		}

		if (subsys_type && a)
			DEBUG("subsystem = %s, Action = %s", subsys_type, a);

		if (subsys_type && !strcmp(subsys_type, "scsi_device")) {
			if (a && ((!strcmp(a, "add")) || (!strcmp(a, "remove")))) {
				char filename[255];
				char prodname[128];
				char type[128];
				char event_string[255];
				char *tmp = NULL;
				int paren_count;
				int ret_val;

				/* read product name out of sysfs */
				sprintf(filename, "%s%s%s", "/sys", dev_path, "/device/type");
				ret_val = copy_string_from_file(filename, type, 128);
				if (ret_val == 0)
					DEBUG("SCSI Device Type = %s \n", type);
				else {
					DEBUG("SCSI Device Type not determined \n");
					return busbEvent;
				}

				if (strcmp(type,"0"))
					return busbEvent;

				sprintf(filename, "%s%s%s", "/sys", dev_path, "/device/model");
				if ((ret_val = copy_string_from_file(filename, prodname, 128)) <= 0) {
					if (prodname == NULL || ret_val < 0)
						snprintf(prodname, 7, "NONAME");

					/* We only support storage devices directly connected to
					 * the Dock Hub. Devices connected via chained hubs are not
					 * supported at this point of time. The device path is of the
					 * format "/devices/platform/<controller-name>/<busno>/<port-number>/
					 * <device>/<interface>/<scsi-host-name>/<scsi-target-name>/
					 * <scsi-device-name>/scsi_disk/<scsi-device-name>"
					 */
					s = dev_path;
					end = s + strlen(dev_path);
					/* We need to find the <device> part of the device path */
					for (p = end, paren_count = 6; p > s; p--) {
						if (paren_count && (*p == '/')) {
							tmp = p;
							--paren_count;
						} else if (!paren_count && (*p == '-')) {
							snprintf(dev_path, tmp - p, "%s", p + 1);
							break;
						}
					}
					if (p == s)
						dev_path = '\0';

					DEBUG("devpath = %s\n", dev_path);
					if (!strcmp(a, "add"))
						snprintf(event_string, 255, "%s:%s:/mnt/usbdisk_%s",
							 USBD_DISK_ATTACH, prodname, dev_path);
					else /* remove */
						snprintf(event_string, 255, "%s:%s:/mnt/usbdisk_%s",
							 USBD_DISK_DETACH, prodname, dev_path);

					write(usbd_app_fd, event_string, strlen(event_string)+1);
					DEBUG("SCSI %s event for %s\n", a, prodname);
				} else
					DEBUG ("Could not open %s\n", filename);
			} else
				DEBUG("Unsupported Action \n");
		}
	}

	return busbEvent;
}

int uevent_socket_init()
{

	int uevent_sz = 64*1024;
	struct sockaddr_nl nladdr;
	int on = 1;

	// Socket to listen on for uevent changes
	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;
	nladdr.nl_pid = getpid();
	nladdr.nl_groups = 0xffffffff;


	if ((uevent_sock = socket(PF_NETLINK,
		SOCK_DGRAM,NETLINK_KOBJECT_UEVENT)) < 0) {
		ALOGE("Unable to create uevent socket: %s", strerror(errno));
		return -1;
	}

	if (setsockopt(uevent_sock, SOL_SOCKET, SO_RCVBUFFORCE, &uevent_sz,
			sizeof(uevent_sz)) < 0) {
		ALOGE("Unable to set uevent socket options: %s", strerror(errno));
		return -1;
	}

	if (setsockopt(uevent_sock, SOL_SOCKET, SO_PASSCRED, &on,
			sizeof(on)) <0) {
		ALOGE("Unable to set uevent socket options SO_PASSCRED: %s", strerror(errno));
		return -1;
	}

	if (bind(uevent_sock, (struct sockaddr *) &nladdr, sizeof(nladdr)) < 0) {
		ALOGE("Unable to bind uevent socket: %s", strerror(errno));
		return -1;
	}

	return 0;

}

int usbd_get_flashdrive_status()
{
	DIR *dir;
	struct dirent *entry;

	dir = opendir(SCSI_DEVICE_PATH);
	if(!dir)
		return -1;

	while ((entry = readdir(dir)) != NULL)  {
		if (!strncmp("host", (char *) entry->d_name, 5))  {
			char fullpath[256];
			FILE *file;
			snprintf(fullpath, 256, "%s/%s/uevent", SCSI_DEVICE_PATH, (char *)entry->d_name);
			DEBUG("SCSI device attached at boot, send uevent for %s\n", fullpath);
			file = fopen(fullpath, "w");
			if(!file)
				ALOGE("Unable to open file %s with error %s\n", fullpath, strerror(errno));
			else {
				fprintf(file, "add");
				fclose(file);
			}

			break;
		}
	}

	return 0;
}


int usbd_get_initial_cable_status()
{
	current_usb_online = 0;
	usbd_current_state = USBD_STATE_CABLE_DEFAULT;
	int current_cable_status = readUsbSwitchState();

	if (current_cable_status < 0) {
		ALOGE("Failed to read initial Cable State - defaulting \n");
		return -1;
	} else {
		usbd_curr_cable_status = current_cable_status;
		DEBUG("Initial cable_type = %d\n",usbd_curr_cable_status);
	}

	if (usbd_curr_cable_status != MOTO_ACCY_TYPE_EMU_UNKNOWN) {
		current_usb_online = 1;
		usbd_current_state = USBD_STATE_CABLE_ATTACHED;
	}

	return 0;
}

void usbd_handle_cable_status_change(void)
{
	int rc;

	if(usbd_curr_cable_status == MOTO_ACCY_TYPE_EMU_CABLE_FACTORY) {
		/*
		 *A factory cable has been connected. Just set the Factory Mode right away
		 */
		if(usbd_factorycable_adb_enabled())
			usbd_set_usb_mode(USB_MODE_NETWORK_ADB);
		else
			usbd_set_usb_mode(USB_MODE_NETWORK);
		return;
	}
	else if(!usb_alt_mode) {
		/*
		 *We are Connected to the App . Let the App respond to the connect/disconnect
		 *events and set the mode as necessary. Just notify the app.
		 */
		DEBUG("Cable Status Changed, need to notify Cable Status to App \n");
		if(usbd_app_fd >= 0) {
			rc = usbd_notify_current_status(usbd_app_fd);

			if(rc < 0) {
				 /* Clear the file descriptor. */
				DEBUG("Cable Status chnged, usbd_app_fd clr\n");
				 FD_CLR(usbd_app_fd, &connected_apps_fd);
				 usbd_app_fd = -1;
				 max_fd = usbd_get_maxfd();
			}
		}
	} else if(usbd_curr_cable_status == MOTO_ACCY_TYPE_EMU_CABLE_USB) {
		/*Initializing tethering_enabled value used in alternate mode*/
		tethering_enabled = 0;
		/*
		 *We are in an alternate mode . Behave like a Stock
		 *Android phone . Just enumerate the default alternate mode right away
		 */
		DEBUG("Alternate Mode - Enumerate the default mode \n");
		usbd_enumerate_default_mode();
	}
}

/*
 * main ...
 */
int main (void)
{
	int rc, newfd;
	fd_set rset;
	struct sockaddr addr;
	int alen = sizeof(struct sockaddr);
	/*
	 * devbuf used to store the string such as the below format:
	 * acm_eth_mtp_adb:adb_enable:tethering_enable:enumerated"
	 */
	char devbuf[90];
	/*
	 * adbEnablebuf used to store the string such as
	 * "none" or "adb_enable" or "adb_disable"
	 */
	char adbEnablebuf[20];
	/*
	 * tethingEnablebuf used to store the string such as
	 * "none" or "tethering_enable" or "tethering_disable"
	 */
	char tetheringEnablebuf[20];
	/*
	 * pcSwitchbuf used to store the string such as
	 * none" or usb mode string such as "acm_eth_mtp_adb" etc
	 */
	char pcSwitchbuf[50];
	/*
	 * enubuf used to store the string such as
	 * "none" or "enumerated" or "get_desc"
	 */
	char enubuf[90];
	char *tmpDevbuf = NULL;
	int length;
	int en_flag;


	DEBUG("Start usbd - version %s\n", USBD_VERSION);

	/* Initialize the gobal parameters */
	usbd_app_fd = -1;

#ifdef USBD_FILE_DEBUG
	pthread_mutex_init(&usbd_log_mutex, NULL);
#endif

	DEBUG("Initializing uevent_socket \n");
	if (uevent_socket_init() < 0) {
		return -1;
	};

	DEBUG("Initializing usb_device_mode \n");
	usb_device_fd = open("/dev/usb_device_mode", O_RDWR);
	if( usb_device_fd < 0)
	{
		ALOGE("Unable to open usb_device_mode '%s' (%s)","/dev/usb_device_mode", strerror(errno));
		return -errno;
	} 

	usbd_mode = usbd_get_startup_mode();
	DEBUG("Phone was started up in %s mode \n", usbd_mode ? "an alternative" : "normal" );

	if(usbd_mode)
		usb_alt_mode = 1;

	if(!usb_alt_mode) {
		DEBUG("Initializing usbd socket \n");
		if( usbd_socket_init() < 0 )
		{
			ALOGE("failed to create socket server\n");
			close(usb_device_fd);
			return -errno;
		}
	}

	if( usbd_get_initial_cable_status() < 0) {
		ALOGE("failed to get cable status\n");
		return -1;
	};

	DEBUG("Initial Cable State = %s\n", current_usb_online ? "Cable Attached": "Cable Detached");

	/* Create and initialize file descriptor set. */
	FD_ZERO(&connected_apps_fd);
	FD_SET(uevent_sock, &connected_apps_fd);
	if(!usb_alt_mode)
		FD_SET(usbd_server_fd, &connected_apps_fd);
	FD_SET(usb_device_fd, &connected_apps_fd);
	max_fd = usbd_get_maxfd();

	/* Mount CDROM to access VERSION info and pass it to the kernel */
	cdrom_smart_version_mount(1);

	rc = cdrom_read_version_key();
	if (rc >= 0)
		ALOGD("Successfully sent %d bytes to the driver\n", rc);
	else
		ALOGD("Failed to send data to the driver %d (%s)\n", rc, strerror(errno));

	/* Unmount CDROM */
	cdrom_smart_version_mount(0);

	/*Handle the case when the phone is powered up with a usb cable
	 * usbd comes up after the connection events are sent by the kernel
	 */
	if(current_usb_online) {
		if(usbd_curr_cable_status == MOTO_ACCY_TYPE_EMU_CABLE_FACTORY) {
			if(usbd_factorycable_adb_enabled())
				usbd_set_usb_mode(USB_MODE_NETWORK_ADB);
			else
				usbd_set_usb_mode(USB_MODE_NETWORK);
		} else if(usb_alt_mode && (usbd_curr_cable_status == MOTO_ACCY_TYPE_EMU_CABLE_USB)) {
			/* No App connected, enumerate in default mode MSC or MSC_ADB */
			DEBUG("No App to Notify , Enumerate in default mode \n");
			usbd_enumerate_default_mode();
		}
	}

	while(1)
	{
		rset = connected_apps_fd;

		select(max_fd + 1, &rset, NULL, NULL, NULL);


		 /* process uevent from the kernel */
		if(FD_ISSET(uevent_sock, &rset))
		{
			if(process_usb_uevent_message(uevent_sock))
				usbd_handle_cable_status_change();
		}


		/* Usb Device Events
		 */
		if(FD_ISSET(usb_device_fd, &rset)) {
			DEBUG("get event from usb_device_fd\n");
			tmpDevbuf = devbuf;
			memset(devbuf, 0 , sizeof(devbuf));
			rc = read(usb_device_fd, devbuf, sizeof(devbuf)-1);
			DEBUG("devbuf: %s\nrc: %d usbd_curr_cable_status: %d\n",devbuf,rc, usbd_curr_cable_status);
			if(rc > 0) {
				sscanf(devbuf, "%[^:]", pcSwitchbuf);
				DEBUG("pcSwitchbuf = %s\n",pcSwitchbuf);

				tmpDevbuf = devbuf + strlen(pcSwitchbuf) + 1;
				sscanf(tmpDevbuf, "%[^:]", adbEnablebuf);
				tmpDevbuf +=  strlen(adbEnablebuf) + 1;
				DEBUG("adbEnablebuf: %s\n",adbEnablebuf);

				/* In userdebug build, 'adb root' may be called to change the authority.
				 * Process 'adb_enable' & 'adb disable' event even though FTM cable conntected
				 * for adb server to recognize as newly attached adb device on PC
				 */
				if(usbd_curr_cable_status == MOTO_ACCY_TYPE_EMU_CABLE_FACTORY)
				{
					if(usbd_factorycable_adb_enabled() && !strcmp(adbEnablebuf, "adb_enable"))
						usbd_set_usb_mode(USB_MODE_NETWORK_ADB);
					else if (!strcmp(adbEnablebuf, "adb_disable"))
						usbd_set_usb_mode(USB_MODE_NETWORK);
				}
				else
				{
					/* Handle USB Device Events for all cases except Factory Cable */
					tmpDevbuf = devbuf + strlen(pcSwitchbuf) + strlen(adbEnablebuf) + 2;
					sscanf(tmpDevbuf, "%[^:]", tetheringEnablebuf);
					tmpDevbuf +=  strlen(tetheringEnablebuf) + 1;
					DEBUG("tetheringEnablebuf: %s\n",tetheringEnablebuf);


					length = strlen(devbuf)-strlen(pcSwitchbuf)-strlen(adbEnablebuf)-strlen(tetheringEnablebuf)-2;
					DEBUG("length = %d\n",length);
					memset(enubuf, 0, sizeof(enubuf));
					if (length > 0) {
						memcpy(enubuf, tmpDevbuf ,length);
					}
					DEBUG("enubuf: %s\n",enubuf);

					if (usb_alt_mode && current_usb_online)
						usbd_alt_mode_set(usbd_mode, adbEnablebuf, tetheringEnablebuf);
					 else if (usbd_app_fd >= 0) {
						if (!strcmp(adbEnablebuf, "adb_enable"))
							usbd_send_adb_status(usbd_app_fd, 1);
						else if (!strcmp(adbEnablebuf, "adb_disable"))
							usbd_send_adb_status(usbd_app_fd, 0);

						if (!strcmp(tetheringEnablebuf, "tethering_enable"))
							usbd_send_tethering_status(usbd_app_fd, 1);
						else if (!strcmp(tetheringEnablebuf, "tethering_disable"))
							usbd_send_tethering_status(usbd_app_fd, 0);
					}

					if (usbd_curr_cable_status == MOTO_ACCY_TYPE_EMU_CABLE_USB  &&
					    current_usb_online) {

						if (strcmp(pcSwitchbuf, "none")) {
							usb_req_mode_switch(pcSwitchbuf);
						}

						if (!strncmp(enubuf,"get_desc", 8)) {
							usb_get_des_count++;
							if (usb_get_des_count == 1) {
								usbd_current_state = USBD_STATE_ENUM_IN_PROGRESS;
								DEBUG("received get_descriptor, enum in progress\n");
								DEBUG("usbd_app_fd = %d\n", usbd_app_fd);
								if (usbd_app_fd >= 0) {
									DEBUG("Notifying Apps that Get_Descriptor was called...\n");
									rc = write(usbd_app_fd, USBD_GET_DESCRIPTOR, strlen(USBD_GET_DESCRIPTOR) + 1);
									if (rc < 0) {
										/* Clear the file descriptor. */
										FD_CLR(usbd_app_fd, &connected_apps_fd);
										usbd_app_fd = -1;
										max_fd = usbd_get_maxfd();
									}
								}
							}

						}
						else if(!strncmp(enubuf, "enumerated", 10)) {
							DEBUG("recieved enumerated\n");
							DEBUG("usbd_app_fd  = %d\n",usbd_app_fd);
							usbd_current_state = USBD_STATE_ENUMERATED;

							rc = usbd_enum_process(usbd_app_fd);
							if(rc < 0) {
								/* Clear the file descriptor. */
								FD_CLR(usbd_app_fd, &connected_apps_fd);
								usbd_app_fd = -1;
								max_fd = usbd_get_maxfd();
							}
						}
					}
				}
			}

		}


		/* Now handle the event from USB Daemon. */
		if(!usb_alt_mode && FD_ISSET(usbd_server_fd, &rset))
		{
			 DEBUG("get event from usbd server fd\n");
			 /* Handle the event when an app is trying to connect */
			 newfd = accept(usbd_server_fd, (struct sockaddr*)&addr, &(alen));
			 if(newfd >= 0) {
				DEBUG("usbd_save_socket_fd ...\n");
				usbd_save_socket_fd(newfd);
				max_fd = usbd_get_maxfd();
				en_flag = usbd_get_adb_property();
				usbd_send_adb_status(newfd, en_flag);
				/* Notify current status to the newly connected app if it is
				 * not a factory cable
				 */
				if (usbd_curr_cable_status != MOTO_ACCY_TYPE_EMU_CABLE_FACTORY)
					usbd_notify_current_status(newfd);
					if(usbd_get_flashdrive_status() < 0)
						ALOGE("failed to get flash drive status\n");

			 }
		}

		/* Traverse through all connected applications. */
		/* Check if this application is connected and its file descriptor is set. */
		if (!usb_alt_mode && (usbd_app_fd >= 0) && (FD_ISSET(usbd_app_fd, &rset)))
		{
			DEBUG("Read and handle a pending message from the App \n");
			/* Handle the event, and check if socket is closed. */
			rc = usbd_socket_event(usbd_app_fd);
			if(rc < 0)
			{
				 /* Clear the file descriptor. */
				 FD_CLR(usbd_app_fd, &connected_apps_fd);
				 usbd_app_fd = -1;
				 max_fd = usbd_get_maxfd();
			}
		}

	} /* while (1) */

	DEBUG("usbd exit\n");
#ifdef USBD_FILE_DEBUG
	pthread_mutex_destroy(&usbd_log_mutex);
#endif
	close(usb_device_fd);

	return 0;
}

int loop_create(const char *loopFile, char *loopDeviceBuffer, int len)
{
	int i;
	int fd;
	char filename[LOOP_DEV_PATH_LEN + 1];

	for (i = 0; i < LOOP_MAX; i++) {
		struct loop_info li;
		int rc;

		sprintf(filename, "/dev/block/loop%d", i);

		/*
		 * The kernel starts us off with 8 loop nodes, but more
		 * are created on-demand if needed.
		 */
		mode_t mode = 0660 | S_IFBLK;
		unsigned int dev = (0xff & i) | ((i << 12) & 0xfff00000) | (7 << 8);
		if (mknod(filename, mode, dev) < 0) {
			if (errno != EEXIST) {
				printf("Error creating loop device node (%s)", strerror(errno));
				return -1;
			}
		}

		if ((fd = open(filename, O_RDWR)) < 0) {
			printf("Unable to open %s (%s)", filename, strerror(errno));
			return -1;
		}

		rc = ioctl(fd, LOOP_GET_STATUS, &li);
		if (rc < 0 && errno == ENXIO)
			break;

		close(fd);

		if (rc < 0) {
			printf("Unable to get loop status for %s (%s)", filename,
				 strerror(errno));
			return -1;
		}
	}

	if (i == LOOP_MAX) {
		printf("Exhausted all loop devices");
		errno = ENOSPC;
		return -1;
	}

	strncpy(loopDeviceBuffer, filename, len - 1);

	int file_fd;

	if ((file_fd = open(loopFile, O_RDWR)) < 0) {
		printf("Unable to open %s (%s)", loopFile, strerror(errno));
		close(fd);
		return -1;
	}

	if (ioctl(fd, LOOP_SET_FD, file_fd) < 0) {
		printf("Error setting up loopback interface (%s)", strerror(errno));
		close(file_fd);
		close(fd);
		return -1;
	}

	close(fd);
	close(file_fd);
	return 0;
}

