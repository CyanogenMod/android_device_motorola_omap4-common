/*
 *   mot_boot_mode - Check phone bootup mode, which will be
 *                   executed in init.rc to decide what kind
 *                   of services should be started during phone
 *                   powerup.
 *
 *   Copyright Motorola 2009
 *
 *   Date         Author      Comment
 *   01/03/2009   Motorola    Creat initial version
 *   17/04/2009   Motorola    Support Charge Only Mode
 *   
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include <cutils/properties.h>
#include <cutils/log.h>

#define MOTO_PU_REASON_CHARGE_ONLY    "0x00000100"
#define MOTO_PU_REASON_CPCAP_RESET    "0x00040000"
#define MOTO_PU_REASON_KPANIC 	      "0x00020000"
#define MOTO_PU_REASON_WDRESET        "0x00008000"
#define MOTO_CID_RECOVER_BOOT	      "0x01"
#define MOTO_DATA_12M		      "1"

/********************************************************************
 * Check /proc/bootinfo, compare the given field with an expected value.
 * Input:
 *     feild: one of the fields in bootinfo
 *     value : expected value you want the field to be
 * Return value:
 * 1: value match
 * 0: value dose not match
 ********************************************************************/
int check_bootinfo(const char * field, const char* value)
{
    char data[1024], bootreason[32];
    int fd, n;
    char *x, *pwrup_rsn;

    memset(bootreason, 0, 32);

    fd = open("/proc/bootinfo", O_RDONLY);
    if (fd < 0) return 0;

    n = read(fd, data, 1023);
    close(fd);
    if (n < 0) return 0;

    data[n] = '\0';

    pwrup_rsn = strstr(data, field);
    if (pwrup_rsn) {
        x = strstr(pwrup_rsn, ": ");
        if (x) {
            x += 2;
            n = 0;
            while (*x && !isspace(*x)) {
                bootreason[n++] = *x;
                x++;
                if (n == 31) break;
            }
            bootreason[n] = '\0';
        }
    }
    if (!strcmp(bootreason, value))
        return 1;
    else
        return 0;
}

/********************************************************************
 * Check kpanic and wdreset and cpcapreset bootmode.
 * Return value:
 * Type: int
 * 1: kpanic or wdreset mode
 * 0: other mode
 * ******************************************************************/
int check_abnormal_reboot(void)
{
    char abnormal_boot[PROPERTY_VALUE_MAX];

    memset(abnormal_boot, 0, 32);

    property_get("ro.bootmode", abnormal_boot, "unknown");

    if((!strncmp(abnormal_boot, "kpanic", 6)) ||
	(!strncmp(abnormal_boot, "wdreset", 7)) ||
	(!strncmp(abnormal_boot, "wdcpcap", 7))) {
        ALOGD("MOTO_PUPD: bootmode=%s\n", abnormal_boot);
	return 1;
    } else {
	if((check_bootinfo("POWERUPREASON", MOTO_PU_REASON_KPANIC)) ||
	(check_bootinfo("POWERUPREASON", MOTO_PU_REASON_WDRESET)) ||
	(check_bootinfo("POWERUPREASON", MOTO_PU_REASON_CPCAP_RESET))) {
		return 1;
	} else {
		return 0;
	}
    }
}

#define COM_FS_PATH "/data/chargeonlymode"
#define COM_MAX_MODE_LENGTH 20

char mode[COM_MAX_MODE_LENGTH];

/********************************************************************
 * Check previous phone mode before reset
 * Return value:
 * Type: int
 * 1: com mode
 * 0: not com mode
 * ******************************************************************/
int check_com_reset(void)
{
        int fd = open(COM_FS_PATH, O_RDWR);
        if (fd < 0) {
                return 0;
        } else if (read(fd, mode, COM_MAX_MODE_LENGTH) <= 0) {
                return 0;
	} else {
                if (!strncmp(mode,"charging", 8)) {
                	return 1;
		} else {
                	return 0;
		}
        }

        ftruncate(fd, 0);
        close(fd);
        return 0;
}

/********************************************************************
 * Check POWERUPREASON or ro.bootmode, decide phone powerup to charge only mode or not
 * Return value:
 * Type: int
 * 1: charge_only_mode
 * 0: NOT charge_only_mode
 ********************************************************************/
int boot_reason_charge_only(void)
{
    char powerup_reason[PROPERTY_VALUE_MAX];

    memset(powerup_reason, 0, 32);

    property_get("ro.bootmode", powerup_reason, "unknown");

    if(!strncmp(powerup_reason, "charger", 7)) {
        ALOGD("MOTO_PUPD: bootmode=%s\n", powerup_reason);
        return 1;
    } else if(check_bootinfo("POWERUPREASON", MOTO_PU_REASON_CHARGE_ONLY)) {
		return 1;
    } else if(check_abnormal_reboot() && check_com_reset()) {
		return 1;
    } else {
	return 0;
    }
}

/********************************************************************
 * Check CID_RECOVER_BOOT or ro.bootmode, decide phone powerup to recovery mode or not
 * Return value:
 * Type: int
 * 1: Recovery mode
 * 0: NOT recovery mode
 * ******************************************************************/
int check_cid_recover_boot(void)
{
    char cid_recover_boot[PROPERTY_VALUE_MAX];

    memset(cid_recover_boot, 0, 32);

    property_get("ro.bootmode", cid_recover_boot, "unknown");

    if(!strncmp(cid_recover_boot, "cidtcmd", 7)) {
        ALOGD("MOTO_PUPD: bootmode=%s\n", cid_recover_boot);
	return 1;
    } else {
	return(check_bootinfo("\nCID_RECOVER_BOOT", MOTO_CID_RECOVER_BOOT));
    }
}

/********************************************************************
 * Check 12m file is set or not (tcmd suspend related)
 * return value:
 * Type: int
 * 1: 12m is set
 * 0: 12m is not set
 ********************************************************************/
int check_data_12m(void)
{
    /*TODO: This 12m feature for TCMD need to be locked down*/
    /*and implemented by TCMD team */
    return 0;
}

/********************************************************************
 * 1. Check cid_recover_boot, set property
 * 2. Check boot reason is charge only mode or not, set property
 * 3. Check 12m (tcmd suspend related), set property
 * TODO: Is the priority/order right?
 ********************************************************************/
int main(int argc, char **argv)
{
    ALOGD("MOTO_PUPD: mot_boot_mode\n");

    if (check_cid_recover_boot()){

        ALOGD("MOTO_PUPD: check_cid_recover_boot: 1\n");
        property_set("tcmd.cid.recover.boot", "1");
        property_set("tcmd.suspend", "1");

    }else if (boot_reason_charge_only()){

    	ALOGD("MOTO_PUPD: boot_reason_charge_only: 1\n");
        property_set("sys.chargeonly.mode", "1");

    }else if (check_data_12m()){

        ALOGD("MOTO_PUPD: mot_boot_mode 12m: 1\n");
        property_set("tcmd.12m.test", "1");
        property_set("tcmd.suspend", "1");

    }else{

       	ALOGD("MOTO_PUPD: mot_boot_mode : normal\n");
       	property_set("tcmd.suspend", "0");
    }

    return 0;
}
