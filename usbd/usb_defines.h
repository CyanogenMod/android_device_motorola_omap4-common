#ifndef __USB_DEFINES_H
#define __USB_DEFINES_H
/*********************************************************************
 *
 * Filename:      usb_defines.h
 * Description:   USB daemon header file.
 *
 *      Motorola Confidential Proprietary
 *      Advanced Technology and Software Operations
 *      (c) Copyright Motorola 2009, All Rights Reserved
 *
 *      Modification Tracking
 *      Author          Date Number      Description of Changes
 *      ------------    -------------   --------------------------
 *      Motorola        01/16/2009       Created file.
 **********************************************************************/

#define USBD_SOCKET_NAME      "usbd"
#define USBD_SOCKET_FULLNAME  "/dev/socket/usbd"
#define USBD_MAX_APPS         5
#define RETRY_TIMEOUT         10
#define SOCKET_BUFFER_SIZE    1024
#define GET_DESCRIPTOR_SWITCH 0xDD

/*
 * USBD send the following strings to let UI know cable status
 */
#define USBD_CABLE_ATTACHED           "cable_connected"
#define USBD_FACTORY_CABLE_ATTACHED   "cable_connected_factory"
#define USBD_CABLE_DETACHED           "cable_disconnected"
#define USBD_GET_DESCRIPTOR           "get_descriptor"
#define USBD_ENUMERATED               "usb_enumerated"

/*
 * USBD send the following strings to let UI know ADB status
 */
#define USBD_EVENT_ADB_ON             "usbd_adb_status_on"
#define USBD_EVENT_ADB_OFF            "usbd_adb_status_off"

/*
 * USBD send the following strings to let UI know tethering status
 */
#define USBD_EVENT_TETHERING_ON             "usbd_tethering_status_on"
#define USBD_EVENT_TETHERING_OFF            "usbd_tethering_status_off"

/*
 * The following strings are used to let UI know the current USB mode.
 */
#define USBD_REQ_SWITCH_NGP           "usbd_req_switch_ngp"
#define USBD_REQ_SWITCH_MTP           "usbd_req_switch_mtp"
#define USBD_REQ_SWITCH_MSC           "usbd_req_switch_msc"
#define USBD_REQ_SWITCH_HID           "usbd_req_switch_hid"
#define USBD_REQ_SWITCH_MODEM         "usbd_req_switch_modem"
#define USBD_REQ_SWITCH_CHARGE_ONLY   "usbd_req_switch_none"
#define USBD_REQ_SWITCH_RNDIS         "usbd_req_switch_rndis"
#define USBD_REQ_SWITCH_BPTOOLS       "usbd_req_switch_bptools"

/*
 * The following strings are used to let UI know the service to start in easy mode.
 */
#define USBD_START_NGP                 "usbd_start_ngp"
#define USBD_START_MTP                 "usbd_start_mtp"
#define USBD_START_ACM                 "usbd_start_acm"
#define USBD_START_MSC_MOUNT           "usbd_start_msc_mount"

/*
 * USB Cable States reported by the usb switch driver
 */
#define CABLE_NONE    '0'
#define CABLE_USB     '1'
#define CABLE_FACTORY '2'

/*
 * The following strings are used to let UI know the connection status for USB devices.
 */
#define USBD_DISK_ATTACH		"usb_disk_plugin"
#define USBD_DISK_DETACH		"usb_disk_plugout"


typedef struct otg_admin_command {
        int             n;                        /*!< Function index >*/
        unsigned short  vid;
        unsigned short  pid;
        unsigned char   cdrom;
} otg_admin_command_t;

/*
 * USB States
 */
typedef enum {
    USBD_STATE_CABLE_DEFAULT,
    USBD_STATE_CABLE_DETACHED,
    USBD_STATE_CABLE_ATTACHED,
    USBD_STATE_ENUM_IN_PROGRESS,
    USBD_STATE_ENUMERATED
}USBD_STATE_T;

/*
 * Define USB modes
 */
typedef enum {
    USB_MODE_MIN = 0,
    USB_MODE_NGP,
    USB_MODE_CDROM,
    USB_MODE_MTP,
    USB_MODE_MSC,
    USB_MODE_MSC_ONLY,
    USB_MODE_HID,
    USB_MODE_NGP_ADB,
    USB_MODE_MTP_ADB,
    USB_MODE_MSC_ADB,
    USB_MODE_ADB,
    USB_MODE_NETWORK,
    USB_MODE_MODEM,
    USB_MODE_MODEM_ADB,
    USB_MODE_NETWORK_ADB,
    USB_MODE_CHARGE_ONLY,
    USB_MODE_CHARGE_ADB,
    USB_MODE_RNDIS,
    USB_MODE_RNDIS_ADB,
    USB_MODE_CDROM2,
    USB_MODE_BPTOOLS_ETH,
    USB_MODE_BPTOOLS_ETH_ADB,
    USB_MODE_RNDIS_BPTOOLS_ETH,
    USB_MODE_RNDIS_BPTOOLS_ETH_ADB,
    USB_MODE_TABLET_NETWORK_MTP,
    USB_MODE_TABLET_NETWORK_MTP_ADB,
    USB_MODE_MAX,
}USB_MODE_T;

#define MAX_VIDPID_LINE_SIZE   80
#define USB_CDROM_PARTITION    "usb_cdrom_partition"

#ifndef CDROMIO_DEVNODE
#ifdef MSD_CDROM_ENABLED
#define CDROMIO_DEVNODE        "/sys/devices/platform/usb_mass_storage/lun1/file"
#else
#define CDROMIO_DEVNODE        "/sys/devices/platform/usb_mass_storage/lun0/file"
#endif
#endif

#ifndef CDROM_BLOCK_DEVICE
#define CDROM_BLOCK_DEVICE     "/dev/block/cdrom"
#endif

#define SCSI_DEVICE_PATH       "/sys/bus/scsi/devices"

struct usb_mode_data {
    USB_MODE_T mode;
    /* USB mode name */
    char *name;
 
    /* Set USB Mode */
    void (*pre_enable)(void);         /* Function call before otg enabled */
    void (*enable_done)(void);        /* Function call after otg enabled */
    void (*pre_disable)(void);        /* Function call before otg disabled */
    void (*disable_done)(void);       /* Function call after otg disabled */

    /* USB mode switch */
    char *mode_req;
    char *mode_done;
    char *switch_req;
    char *resp_ok;
    char *resp_fail;
};


#endif /*__USB_DEFINES_H*/
