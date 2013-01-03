// Overload this file in your own device-specific config if you need
// non-standard property_perms and/or control_perms structs
//
// To avoid conflicts...
// if you redefine property_perms, #define PROPERTY_PERMS there
// if you redefine control_perms, #define CONTROL_PARMS there
//
// A typical file will look like:
//

// Alternatively you can append to the existing property_perms and/or
// control_perms structs with the following:
#define PROPERTY_PERMS_APPEND \
    { "net.caif0.",       AID_RADIO,    0 }, \
    { "net.usb0.",        AID_RADIO,    0 }, \
    { "net.usb1.",        AID_RADIO,    0 }, \
    { "net.qmi0.",        AID_RADIO,    0 }, \
    { "net.qmi1.",        AID_RADIO,    0 }, \
    { "net.qmi2.",        AID_RADIO,    0 }, \
    { "net.gannet0.",     AID_RADIO,    0 }, \
    { "net.dns",          AID_DHCP,     0 }, \
    { "net.dns",          AID_VPN,      0 }, \
    { "net.vpnclient",    AID_VPN,      0 }, \
    { "net.dnschange",    AID_VPN,      0 }, \
    { "serialno",         AID_RADIO,    0 }, \
    { "radio.",           AID_RADIO,    0 }, \
    { "log.",             AID_SHELL,    AID_LOG }, \
    { "persist.log.",     AID_SHELL,    AID_LOG }, \
    { "persist.tcmd.",    AID_MOT_TCMD,   0 }, \
    { "tcmd.",            AID_MOT_TCMD, AID_MOT_WHISPER }, \
    { "persist.mot.proximity.", AID_RADIO, 0}, \
    { "mot.backup_restore.",AID_MOT_TCMD, 0}, \
    { "mot.",             AID_MOT_TCMD, 0 }, \
    { "cdma.nbpcd.supported", AID_RADIO, AID_RADIO }, \
    { "hw.",              AID_MOT_WHISPER, 0 }, \
    { "lte.default.protocol", AID_RADIO,    0 }, \
    { "lte.ignoredns",     AID_RADIO,    0 }, \
    { "vzw.inactivetimer", AID_RADIO,    0 }, \
    { "android.telephony.apn-restore", AID_RADIO,    0 }, \
    { "hw.",              AID_MEDIA,   0 }, \
    { "persist.ril.event.report", AID_RADIO, 0 },

#define CONTROL_PERMS_APPEND \
    { "hciattach", AID_MOT_TCMD, AID_MOT_TCMD }, \
    { "bluetoothd",AID_MOT_TCMD, AID_MOT_TCMD }, \
    { "bt_start", AID_MOT_TCMD, AID_MOT_TCMD }, \
    { "bt_stop", AID_MOT_TCMD, AID_MOT_TCMD }, \
    { "whisperd", AID_MOT_TCMD, AID_MOT_TCMD }, \
    { "gadget-lte-modem", AID_RADIO, AID_RADIO }, \
    { "gadget-qbp-modem", AID_RADIO, AID_RADIO }, \
    { "gadget-qbp-diag", AID_RADIO, AID_RADIO }, \
    { "ftmipcd", AID_RADIO, AID_RADIO }, \
    { "mdm_usb_suspend", AID_RADIO, AID_RADIO },
    
