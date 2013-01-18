/*
 * Copyright (C) 2010 Motorola
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

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "bcbmsg.h"
#include "mounts.h"
#include <cutils/properties.h>
#include "getsetconfig.h"
#include <cutils/log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SETCONFIG"

static const char *NO_RECOVERY_PARAM = "-norecovery";
static const char *MULTICONFIG_COMMAND = "--update_package=CACHE:update_config.zip";
static const char *RECOVERY_COMMAND = "recovery\n--update_package=CACHE:update_config.zip";
static const char *COPY_BOTA_PACKAGE = "/system/bin/cp /system/multiconfig/bota/update_config.zip /cache/";
static const char *BOTA_PACKAGE = "/system/multiconfig/bota/update_config.zip";
static const char *CACHE_BOTA_PACKAGE = "/cache/update_config.zip";
static const char *REMOVE_BOTA_PACKAGE_FROM_CACHE = "/system/bin/rm /cache/update_config.zip";
static const char *MULTICONFIG_FOLDER = "/system/multiconfig";

/*
#define LOGW(...) fprintf(stderr, "W:setconfig: " __VA_ARGS__)
#define LOGE(...) fprintf(stderr, "E:setconfig: " __VA_ARGS__)
#define LOGI(...) fprintf(stderr, "I:setconfig: " __VA_ARGS__)
*/


#ifdef LOG_VERBOSE
static void dump_data(const char *data, int len) {
    int pos;
    for (pos = 0; pos < len; ) {
        printf("%05x: %02x", pos, data[pos]);
        for (++pos; pos < len && (pos % 24) != 0; ++pos) {
            printf(" %02x", data[pos]);
        }
        printf("\n");
    }
}
#endif

extern MOTOBOX_CONFIG_ERR_T getconfig(char *out, int size);


MOTOBOX_CONFIG_ERR_T create_recovery_folder_if_neccessary()
{
    struct stat this_stat;
    char *recovery_folder = "/cache/recovery";
    MOTOBOX_CONFIG_ERR_T ret = MOTOBOX_CONFIG_ERR_NONE;
    mode_t mode= 0775;

    /*check the /cache/recovery directory */

    if (lstat(recovery_folder, &this_stat) < 0) {
        if(errno == ENOENT) {
            /* No such directory, we need create one */
            if (mkdir(recovery_folder, mode) < 0 || chmod(recovery_folder, mode) < 0) {
                LOGE("cann't mkdir /cache/recovery");
                ret = MOTOBOX_CONFIG_ERR_FILE_OPERATION;
            }

        }else {
            LOGE("cann't stat /cache/recovery");
            ret = MOTOBOX_CONFIG_ERR_FILE_OPERATION;
        }
        }
    return ret;

    }



MOTOBOX_CONFIG_ERR_T set_cache_command()
{
    int fd;
    struct stat this_stat;
    char *command_file = "/cache/recovery/command";
    char command_buf[255];

    mode_t mode= 0664;

    if (MOTOBOX_CONFIG_ERR_NONE != create_recovery_folder_if_neccessary()) {
       LOGE("error creating /cache/recovery folder");
       return MOTOBOX_CONFIG_ERR_FILE_OPERATION;
    }

    /* Check the /cache/recovery/command file */
    if(lstat(command_file, &this_stat) < 0 ) {
        if (errno == ENOENT ) {
            fd = open(command_file, O_WRONLY|O_CREAT, mode);
            if (fd < 0) {
                LOGE("cann't create %s",command_file);
                return MOTOBOX_CONFIG_ERR_FILE_OPERATION;
            }
            close(fd);
            chmod(command_file,mode);

        }else {
            LOGE("cann't stat %s",command_file);
            return MOTOBOX_CONFIG_ERR_FILE_OPERATION;
        }

    }

    /* step 3 write the wipe-data to the end of command file */

    fd = open(command_file, O_WRONLY |O_APPEND);
    if(fd < 0 ) {
        LOGE("cann't open %s with O_WRONLY |O_APPEND",command_file);
        return MOTOBOX_CONFIG_ERR_FILE_OPERATION;
    }

    memset(command_buf, 0, sizeof(command_buf));
    strncpy(command_buf, MULTICONFIG_COMMAND,sizeof(command_buf)-1) ;

    if (write(fd, command_buf,strlen(command_buf)) != strlen(command_buf)){
        LOGE("write %s error ",command_buf);
        return MOTOBOX_CONFIG_ERR_FILE_OPERATION;
    }

    return MOTOBOX_CONFIG_ERR_NONE;
}

MOTOBOX_CONFIG_ERR_T set_recovery()
{
    struct bootloader_message boot;
    memset(&boot, 0, sizeof(boot));

    if(get_bootloader_message(&boot))  {
        return MOTOBOX_CONFIG_ERR_SET_RECOVERY;
    }

    if (boot.command[0] != 0 && boot.command[0] != 255) {
        LOGI("Boot command: %.*s\n", sizeof(boot.command), boot.command);

        if( !strcmp(boot.command,"boot-recovery")){
            /* already been set to boot-recovery to the command file */
            if(set_cache_command() != MOTOBOX_CONFIG_ERR_NONE) {
                return MOTOBOX_CONFIG_ERR_SET_RECOVERY;
            }
        } else {
            /* not boot-recovery command, we quit it */
            LOGE("Busy, try setconfig later");
            return MOTOBOX_CONFIG_ERR_SET_RECOVERY;
        }

    } else {
        /* misc is clean , set the boot-recovery command */
        char command_buf[255];
        memset(command_buf, 0, sizeof(command_buf));
        strncpy(command_buf, RECOVERY_COMMAND, sizeof(command_buf)-1);

        memset(&boot, 0, sizeof(boot));
        strlcpy(boot.command, "boot-recovery", sizeof(boot.command));
        strlcpy(boot.recovery, command_buf, sizeof(boot.recovery));
        if (set_bootloader_message(&boot)) {
            return MOTOBOX_CONFIG_ERR_SET_RECOVERY;
        }

    }

    return MOTOBOX_CONFIG_ERR_NONE;
}

static MOTOBOX_CONFIG_ERR_T set_config(char* confid)
{
    int fd;
    struct stat this_stat;
    char *config_file = "/cache/recovery/new-config-id";
    MOTOBOX_CONFIG_ERR_T ret = MOTOBOX_CONFIG_ERR_NONE;

    mode_t mode= 0775;

    /* step1  check the /pds/multiconfig directory */

    if (MOTOBOX_CONFIG_ERR_NONE != create_recovery_folder_if_neccessary() ) {
        LOGE("error accessing /cache/recovery partition");
             return MOTOBOX_CONFIG_ERR_FILE_OPERATION;
    }

    /* step2 remove the /pds/multiconfig/config-id file if exists */

    if(lstat(config_file, &this_stat) == 0) {
        if (unlink(config_file) != 0) {
            LOGE("cann't remove %s",config_file);
            return MOTOBOX_CONFIG_ERR_FILE_OPERATION;
        }
    }

    /* step 3 write the config value into the new config file */
    mode = 0664;
    fd = open(config_file, O_WRONLY|O_CREAT, mode );
    if(fd < 0 ) {
        LOGE("cann't create,open %s with O_WRONLY ",config_file);
        return MOTOBOX_CONFIG_ERR_FILE_OPERATION;
    }

    if (write(fd, confid,strlen(confid)) != (int) strlen(confid)){
        LOGE("write %s error ",confid);
        ret =  MOTOBOX_CONFIG_ERR_FILE_OPERATION;
    }
    close(fd);
    chmod(config_file,mode);

    return ret;
}

MOTOBOX_CONFIG_ERR_T copy_bota_package_to_cache() {

   int ret=-1;
   struct stat s;
   if (lstat(BOTA_PACKAGE,&s) != 0) {
       LOGE("BOTA package at location [%s] doesn't exists\n",BOTA_PACKAGE);
       return MOTOBOX_CONFIG_ERR_FILE_OPERATION;
   } else {
       ret = system(COPY_BOTA_PACKAGE);
       if (WEXITSTATUS(ret) == 0){
           LOGE("Successfully Copied PACKAGE[%s] and status is [%s]\n",BOTA_PACKAGE,strerror(errno));
       } else {
           LOGE("Failed to copy bota package [%s] error [%s] ret [%d]\n",BOTA_PACKAGE,strerror(errno),ret);
           return  MOTOBOX_CONFIG_ERR_FILE_OPERATION;
       }
   }
   return MOTOBOX_CONFIG_ERR_NONE;
}

void remove_bota_package_from_cache() {

   struct stat s;
   if (lstat(CACHE_BOTA_PACKAGE,&s) != 0) {
       return ;
   }
   system(REMOVE_BOTA_PACKAGE_FROM_CACHE);
   return ;
}

void set_no_erase_data_flag() {
    mode_t mode = 0664;
    const char * noerase = "noerasedata";
    const char * noerasefile = "/cache/recovery/noerasedata";
    int fd = open(noerasefile, O_WRONLY|O_CREAT, mode);
    if( fd<0 ) {
        LOGE("Cannot write erase flag in /cache/recovery/noerasedata");
        return;
    }
    if(write(fd,noerase,strlen(noerase)) != (int) strlen(noerase)) {
        LOGE("Cannot write into file /cache/recovery/noerasedata");
        close(fd);
        return;
    }
    close(fd);
    chmod(noerasefile,mode);
}


MOTOBOX_CONFIG_ERR_T setconfig_main(int argc, char *argv[])
{

    char *confid = NULL;
    int value = MOTOBOX_CONFIG_ID_INVALID;
    MOTOBOX_CONFIG_ERR_T status = MOTOBOX_CONFIG_ERR_NONE;
    char buf[255];
    char *old_confid = NULL;
    int leadZeros = 0;

    /** Set to recovery mode or not flag */
    short isSetRecovery = 1; // TRUE by default
    short dont_erase = 0;

    struct stat s;

    /* validate argument format*/
    if(argc == 3 || argc == 2){
        /* check if multiconfig build */
        if(lstat( MULTICONFIG_FOLDER, &s ) < 0 ) {
            LOGE("This is not a multiconfig build, exiting %s\n", argv[0]);
            printf("%d\n", MOTOBOX_CONFIG_ERR_NOT_MULTICONFIG);
            return MOTOBOX_CONFIG_ERR_NOT_MULTICONFIG;
        }

        if(argc == 3) {
            if (strcmp(NO_RECOVERY_PARAM, argv[1]) == 0) {
                isSetRecovery = 0; //FALSE
                confid = argv[2];
            } else if (strcmp("-no_erase_data", argv[1]) == 0) {
                dont_erase = 1; //TRUE
                confid = argv[2];
            } else {
                LOGE("use: %s [-norecovery | -no_erase_data] config-id\n", argv[0]);

                printf("%d\n", MOTOBOX_CONFIG_ERR_INVALID_PARAM);
                return MOTOBOX_CONFIG_ERR_INVALID_PARAM;
            }
        } else if(argc == 2) {
            confid = argv[1];
        }
    } else {
        LOGE("use: %s [-norecovery | -no_erase_data] config-id\n", argv[0]);

        printf("%d\n", MOTOBOX_CONFIG_ERR_INVALID_PARAM);
        return MOTOBOX_CONFIG_ERR_INVALID_PARAM;
    }

    /* validate conf-id */
    value = atoi(confid);
    memset(buf, 0 , sizeof(buf));
    sprintf(buf, "%d", value);

    /* config-id can have leading zeros ex: 0001, ignore them for comparing */
    if(strlen(confid) > 1) { // Ignore confid="0"
        while(confid[leadZeros] == '0') leadZeros++;
    }

    /* atoi does not detect trailing chars ex: 12abc, so compare */
    if (strcmp(buf, (confid + leadZeros)) != 0){
        LOGE("Invalid config-id :%s \n",confid);

        printf("%d\n", MOTOBOX_CONFIG_ERR_INVALID_FORMAT);
        return MOTOBOX_CONFIG_ERR_INVALID_FORMAT;
    }

    if(value < MOTOBOX_CONFIG_ID_UNCONFIGURED || value > MOTOBOX_CONFIG_ID_MAX) {
        LOGE("Invalid config-id limit :%d \n", value);

        printf("%d\n", MOTOBOX_CONFIG_ERR_INVALID_FORMAT);
        return MOTOBOX_CONFIG_ERR_INVALID_FORMAT;
    }


    /** Compare with previous config-id if present */
    if (getconfig(buf,sizeof(buf)) == MOTOBOX_CONFIG_ERR_NONE) {
        old_confid = buf;
    }

    //Copying the BOTA Package to Cache partition
    status = copy_bota_package_to_cache();

    if ( status != MOTOBOX_CONFIG_ERR_NONE) {
         printf("%d\n",status);
         return status;
    }

    /* Write to config file */
    status = set_config(confid);
    if (status == MOTOBOX_CONFIG_ERR_NONE) {

        /* set into recovery mode */
        if (isSetRecovery) {
            status = set_recovery();
        } else {
            /* Caller will set into recovery mode, so just update cache command file */
            status = set_cache_command();
        }
        if(status == MOTOBOX_CONFIG_ERR_NONE) {
            if (dont_erase) {
               set_no_erase_data_flag();
            }
            sync();
        } else {
            /* Set to recovery mode failed, so revert back to older config-id */
            if(old_confid != NULL) {
              if(set_config(old_confid) != MOTOBOX_CONFIG_ERR_NONE) {
                LOGE("setconfig altered to %s but not complete.Please reconfigure.\n",confid);
                status = MOTOBOX_CONFIG_ERR_NEEDS_RECONFIG;
              }
            }
        }

    }

    if ( status != MOTOBOX_CONFIG_ERR_NONE) {
         LOGE("Remove bota package from cache partition since setconfig failed ..%d\n",status);
         remove_bota_package_from_cache();
    }

    printf("%d\n",status);
    return status;
}
