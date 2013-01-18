/*
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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "getsetconfig.h"
#include <cutils/log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "GETCONFIG"

/*
#define LOGW(...) fprintf(stderr, "W:getconfig: " __VA_ARGS__)
#define ALOGE(...) fprintf(stderr, "E:getconfig: " __VA_ARGS__)
#define LOGI(...) fprintf(stderr, "I:getconfig: " __VA_ARGS__)
*/

static const char *CONFIG_FILE = "/system/multiconfig/config-id";
static const char *MASTER_CONFIG_FILE = "/system/multiconfig/master-config-id";
static const char *MULTICONFIG_FOLDER = "/system/multiconfig";
static const char *CONFIG_FOLDERS = "/system/multiconfig/ap";

MOTOBOX_CONFIG_ERR_T getconfig(char *out, int size)
{

    char confid[255];
    struct stat s;
    FILE *fp = NULL;
    int value = MOTOBOX_CONFIG_ID_INVALID;
    char buf[255];
    const char *file_path_buffer = CONFIG_FILE;
    int leadZeros = 0;

    memset(out, 0 , size);

    if(lstat(MULTICONFIG_FOLDER,&s) < 0 ) {
        ALOGE("This is not a multiconfig build, exiting.\n");
        snprintf (out, size, "%d", value);
        return MOTOBOX_CONFIG_ERR_NOT_MULTICONFIG;
    }

    if (lstat(CONFIG_FILE,&s) < 0 ) {
         /*If config-id file doesn't exist then read the value
           from master-config-id file*/
        if (lstat(MASTER_CONFIG_FILE,&s) < 0) {
            ALOGE("master-config-id file at '%s' doesn't exists. Error(%s) \n",MASTER_CONFIG_FILE,strerror(errno));
        snprintf(out, size, "%d", value);
        return MOTOBOX_CONFIG_ERR_FILE_OPERATION;
    }
        /*reset file path buffer and set it to master-config-id file */
        file_path_buffer = MASTER_CONFIG_FILE;
    }

    if(!(fp = fopen(file_path_buffer,"r"))) {
        ALOGE("Unable to open config-id file '%s' (%s) \n",file_path_buffer,strerror(errno));

        snprintf(out, size, "%d", value);
        return MOTOBOX_CONFIG_ERR_FILE_OPERATION;
    }

    memset(confid, 0 , sizeof(confid));
    if (!fgets(confid, sizeof(confid), fp)) {
        ALOGE("Unable to read conf-id file\n");
        fclose(fp);

        snprintf(out, size, "%d", value);
        return MOTOBOX_CONFIG_ERR_FILE_OPERATION;
    }

    fclose(fp);

    /* validate config-id */
    value = atoi(confid);
    memset(buf, 0 , sizeof(buf));
    sprintf(buf, "%d", value);

    /* config-id can have leading zeros ex: 0001, ignore those for comparing */
    if(strlen(confid) > 1) { // Ignore confid="0"
        while(confid[leadZeros] == '0') leadZeros++;
    }

    /* atoi does not detect trailing chars ex: 12abc, so compare */
    if (strcmp(buf, (confid + leadZeros)) != 0){
        ALOGE("Invalid config-id :%s \n",confid);

        value = MOTOBOX_CONFIG_ID_INVALID;
        snprintf(out, size, "%d", value);
        return MOTOBOX_CONFIG_ERR_INVALID_FORMAT;
    }

    if(value < MOTOBOX_CONFIG_ID_UNCONFIGURED || value > MOTOBOX_CONFIG_ID_MAX) {
        ALOGE("Invalid config-id value :%d \n",value);

        value = MOTOBOX_CONFIG_ID_INVALID;
        snprintf(out, size, "%d", value);
        return MOTOBOX_CONFIG_ERR_INVALID_FORMAT;
    }

    strcpy(out, confid);
    /* Success */
    return MOTOBOX_CONFIG_ERR_NONE;
}
MOTOBOX_CONFIG_ERR_T list_configids(char *buffer, int size)
{
    DIR              *dir;
    struct dirent    *ent;
    if ((dir = opendir(CONFIG_FOLDERS)) == NULL) {
        snprintf(buffer, size, "%d", MOTOBOX_CONFIG_ID_INVALID);
        return MOTOBOX_CONFIG_ERR_NOT_MULTICONFIG;
    }
    while ((ent = readdir(dir)) != NULL) {
        char * d_name = ent->d_name;
        if(strcmp(d_name, ".") == 0 || strcmp(d_name, "..") == 0 || strcmp(d_name, "backup") == 0) {
            continue;
        }
        /* MAX of 5 config-ids per device, so no buffer overrun */
        snprintf(buffer, size, "%s ", d_name);
        buffer = buffer + (strlen(d_name) + 1);
    }
    closedir(dir);
    return MOTOBOX_CONFIG_ERR_NONE;
}
MOTOBOX_CONFIG_ERR_T getconfig_main(int argc, char *argv[])
{
    char confid[255];
    MOTOBOX_CONFIG_ERR_T status = MOTOBOX_CONFIG_ERR_INVALID_PARAM;
    if ((argc == 2) && strcmp(argv[1], "-list") == 0) {
        status = list_configids(confid, sizeof(confid));
    }
    else if(argc != 1) {
        fprintf(stderr, "use: %s [-list]\n", argv[0]);
        snprintf(confid,sizeof(confid),"%d", MOTOBOX_CONFIG_ID_INVALID);
    }
    else  {
        status = getconfig(confid,sizeof(confid));
    }
    printf("%s\n", confid);
    return status;
}

