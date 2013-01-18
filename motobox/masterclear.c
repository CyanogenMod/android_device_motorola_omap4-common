/*
 * Copyright (C) 2008 The Android Open Source Project
 * 
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
#include <time.h>
#include <unistd.h>
#include "bcbmsg.h"
#include "mounts.h"

#include <cutils/properties.h>


static const char *COMMAND_FILE = "/cache/recovery/command";

#define LOGW(...) fprintf(stderr, "W:" __VA_ARGS__)
#define LOGE(...) fprintf(stderr, "E/" __VA_ARGS__)
#define LOGI(...) fprintf(stderr, "I:" __VA_ARGS__)



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

// BEGIN Motorola, r42707, 25/11/2009, IKMAP-1478 
// Added to allow conditional SD card data wiping

/*
 * Check flex bits that control SD card shredding
 */
static int shredding_enabled() {

    char isEnabled[PROPERTY_VALUE_MAX];
    char isDevEnabled[PROPERTY_VALUE_MAX];
    
    property_get("ro.mot.master_clear.shredsd",     isEnabled,    "false");
    property_get("ro.mot.master_clear.shredsd.dev", isDevEnabled, "false");

    return strcmp(isEnabled, "true") == 0 && strcmp(isDevEnabled, "true") == 0;
}

int set_wipe_command()
{
	int fd;
	struct stat this_stat;
	char *recovery_folder = "/cache/recovery";
	char *command_file = "/cache/recovery/command";
	char *command_buf;
	
        // BEGIN Motorola, r42707, 25/11/2009, IKMAP-1478 
        // Added to allow conditional SD card data wiping

	if (shredding_enabled()) {
	    command_buf = "--wipe_data\n--wipe_sdcard\n--silent\n";
	} else {
	    command_buf = "--wipe_data\n";
	}

        // END IKMAP-1478

	mode_t mode= 0777;
	
	/* step1  check the /cache/recoery directory */

	if (lstat(recovery_folder, &this_stat) < 0) {
		if(errno == ENOENT) {
			/* No such directory, we need create one */	
			if (mkdir(recovery_folder, mode) < 0) {
				printf("cann't mkdir /cache/recovery");
				return -1;
			}	

		}else {
			printf("cann't stat /cache/recovery");
			return -1;
		}

	}

	/* step2 check the /cache/recovery/command file */

	if(lstat(command_file, &this_stat) < 0 ) {
		if (errno == ENOENT ) {
			fd = open(command_file, O_WRONLY|O_CREAT, mode);
			if (fd < 0) {
				printf("cann't create %s",command_file);
				return -1;
			}
			close(fd);
			
		}else {
			printf("cann't stat %s",command_file);
			return -1;
		}

	}
		
	/* step 3 write the wipe-data to the end of command file */
	
	fd = open(command_file, O_WRONLY |O_APPEND);
	if(fd < 0 ) {
		printf("cann't open %s with O_WRONLY |O_APPEND",command_file);
		return -1;
	}
	
	if (write(fd, command_buf,strlen(command_buf)) != strlen(command_buf)){
		printf("write %s error ",command_buf);
		return -1;
	}	
	
	return 0;


}


int
masterclear_main(int argc, char *argv[])
{

    char *command_buf;
	struct bootloader_message boot;
	memset(&boot, 0, sizeof(boot));
    
    // BEGIN Motorola, r42707, 25/11/2009, IKMAP-1478 
    // Added to allow conditional SD card data wiping

    if (shredding_enabled()) {
        command_buf = "recovery\n--wipe_data\n--wipe_sdcard\n--silent\n";
    } else {
        command_buf = "recovery\n--wipe_data\n";
    }
 
   // END IKMAP-1478

	get_bootloader_message(&boot);

	if (boot.command[0] != 0 && boot.command[0] != 255) {
        	LOGI("Boot command: %.*s\n", sizeof(boot.command), boot.command);

		if( !strcmp(boot.command,"boot-recovery")){
			/* already been set to boot-recovery to the command file */
			if(set_wipe_command()) return -1;		
	

		}else {
			/* not boot-recovery command, we quit it */
			LOGE("Busy, try masterclear later");
			return -1;		
		}

    	} else {

		/* misc is clean , set the boot-recovery command */
		memset(&boot, 0, sizeof(boot));
   		strlcpy(boot.command, "boot-recovery", sizeof(boot.command));
		strlcpy(boot.recovery, command_buf, sizeof(boot.recovery));
		if (set_bootloader_message(&boot)) return -1;	 		

   	}
	sync(); 
	return 0;
}



