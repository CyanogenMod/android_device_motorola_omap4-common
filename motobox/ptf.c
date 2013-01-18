/*
 * Copyright (C) 2010 Motorola
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
  

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define PRODUCT_VERSION_STR "Sholes"
#include <linux/fb.h>
#include <sys/ioctl.h>

/*==================================================================================================
				LOCAL FUNCTION PROTOTYPES
 ==================================================================================================*/
static void display_properties(char* disp);

/*==================================================================================================
                                      LOCAL CONSTANTS
 ==================================================================================================*/

// Supported commands
static const char* CMD_SWVERSION = "swversion";
static const char* CMD_DISPPROPS = "dispprops";

/*==================================================================================================
                                           GLOBAL FUNCTIONS
 ==================================================================================================*/
 
/*==================================================================================================
 
 FUNCTION: ptf_main
 
 DESCRIPTION:
     ptf tool entry point
 
 ARGUMENTS PASSED:
    argc number of arguments including the tool name, ptf.
    argv list of arguments
         argv[0] is the tool name, "ptf"
         argv[1] ptf command. See the supported commands listed above
         argv[2,3,...] comand specific arguments
 
 REFERENCE ARGUMENTS PASSED:
 
 
 RETURN VALUE:
    0 for success
 
 PRE-CONDITIONS:
    None
 
 POST-CONDITIONS:
    None
 
 INVARIANTS:
    None
 
 ==================================================================================================*/
int ptf_main (int argc, char **argv)
{
    int ret = 0;
    if(argc == 1) {
        printf("argument expected\n");
    }
    else if(strcmp(argv[1], CMD_SWVERSION) == 0) {
        printf("%s\n", PRODUCT_VERSION_STR);
    }
    else if (strcmp(argv[1], CMD_DISPPROPS) == 0) {
        char* disp = ((argc == 2) ? "0" : argv[2]);
        display_properties(disp);
    }
    else {
        printf("ERROR: unexpected command: %s\n", argv[1]);
    }

    return ret;
}

/*==================================================================================================
                                           LOCAL FUNCTIONS
 ==================================================================================================*/
/*
 prints the display properties
 "/dev/graphics/fb0"
 */
/*==================================================================================================
 
 FUNCTION: display_properties
 
 DESCRIPTION:
     Prints display properties
 
 ARGUMENTS PASSED:
     disp - selects the frame buffer device.
            If disp==NULL /dev/graphics/fb0 is selected
            If disp=="X"  /dev/graphics/fbX is selected
 
 
 REFERENCE ARGUMENTS PASSED:
     None
 
 RETURN VALUE:
     None
 
 PRE-CONDITIONS:
     None
 
 POST-CONDITIONS:
     None
 
 INVARIANTS:
    None
 
 ==================================================================================================*/
static void display_properties(char* disp)
{
    char device[20];
    int fb;
    struct fb_var_screeninfo vinfo;
    int ioctlret;

    if(strlen(disp) > 2) {
        printf("ERROR: invalid %s argument: %s\n", CMD_DISPPROPS, disp);
        return;
    }

    sprintf(device, "%s%s", "/dev/graphics/fb", disp);
    fb = open(device, O_RDONLY);
    if(fb < 0){
        printf("ERROR: cannot open device: %s\n", device);
        return;
    }

    ioctlret = ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);
    close(fb);
    if(ioctlret < 0)
    {   /* ERROR in ioctl call*/
        printf("ERROR: ioctl returned: %d\n", ioctlret);
        return;
    }
    printf("xres:%u\n", vinfo.xres);
    printf("yres:%u\n", vinfo.yres);
    printf("xres_virtual:%u\n", vinfo.xres_virtual);
    printf("yres_virtual:%u\n", vinfo.yres_virtual);
    printf("xoffset:%u\n", vinfo.xoffset);
    printf("yoffset:%u\n", vinfo.yoffset);
    printf("bits_per_pixel:%u\n", vinfo.bits_per_pixel);
    printf("grayscale:%u\n", vinfo.grayscale);
    printf("nonstd:%u\n", vinfo.nonstd);
    printf("activate:%u\n", vinfo.activate);
    printf("height:%u\n", vinfo.height);
    printf("width:%u\n", vinfo.width);
    printf("rotate:%u\n", vinfo.rotate);
}
