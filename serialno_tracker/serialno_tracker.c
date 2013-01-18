/*
 *   serialno_tracker - will be executed in init.rc to write serialno to property
 *
 *   Copyright Motorola 2011
 *
 *   Date         Author      Comment
 *   04/15/2011   Motorola    Creat initial version
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <cutils/properties.h>
#include <cutils/log.h>

#define SN_LEN                        10
#define SN_OFFSET                     21
#define FAC_TRACK_INFO_LOC            "/pds/factory/legacy-fti"


/********************************************************************
 * 1. Check whether device already has Track ID Serial Number
 * 2. Check whether /pds/factory/legacy-fti exists
 * 3. Update ro.serialno or ro.gsm.barcode
 ********************************************************************/
int main(int argc, char **argv)
{
    char serialno[SN_LEN+1] = {0};
    int fd;

    property_get("ro.serialno", serialno, "unknown");
    if (strncmp(serialno, "unknown", 7))  {
        ALOGD("MOTO_SN_TRACK: device has serialno=%s\n",serialno);
        return 1;
    }
    else {
        if ((fd = open(FAC_TRACK_INFO_LOC, O_RDONLY)) == -1) {
            ALOGD("MOTO_SN_TRACK: No Factory Track Info exists\n");
            return 1;
        }

        if (lseek(fd, SN_OFFSET, SEEK_SET) == -1 ) {
            ALOGD("MOTO_SN_TRACK: Fail to seek to serialno\n");
            return 1;
        }

        if (read(fd, serialno, SN_LEN) != SN_LEN ) {
            ALOGD("MOTO_SN_TRACK: Fail to read serialno\n");
        return 1;
        }

        ALOGD("MOTO_SN_TRACK: serialno=%s\n",serialno);
        property_set("ro.serialno", serialno);
        property_set("ro.gsm.barcode", serialno);
    }

    return 0;
}
