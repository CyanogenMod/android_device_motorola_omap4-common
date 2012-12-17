/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
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

#ifndef __HWC_DEV__
#define __HWC_DEV__

#include <stdint.h>
#include <stdbool.h>

#include <hardware/hwcomposer.h>

#include <linux/bltsville.h>
#include <video/dsscomp.h>
#include <video/omap_hwc.h>

#include "hal_public.h"
#include "rgz_2d.h"

struct ext_transform_t {
    __u8 rotation : 3;          /* 90-degree clockwise rotations */
    __u8 hflip    : 1;          /* flip l-r (after rotation) */
    __u8 enabled  : 1;          /* cloning enabled */
    __u8 docking  : 1;          /* docking vs. mirroring - used for state */
};
typedef struct ext_transform ext_transform_t;

/* cloning support and state */
struct omap4_hwc_ext {
    /* support */
    struct ext_transform_t mirror;      /* mirroring settings */
    struct ext_transform_t dock;        /* docking settings */
    float lcd_xpy;                      /* pixel ratio for UI */
    __u8 avoid_mode_change;             /* use HDMI mode used for mirroring if possible */
    __u8 force_dock;                     /* must dock */
    __u8 hdmi_state;                     /* whether HDMI is connected */

    /* state */
    __u8 on_tv;                         /* using a tv */
    struct ext_transform_t current;     /* current settings */
    struct ext_transform_t last;        /* last-used settings */

    /* configuration */
    __u32 last_xres_used;               /* resolution and pixel ratio used for mode selection */
    __u32 last_yres_used;
    __u32 last_mode;                    /* 2-s complement of last HDMI mode set, 0 if none */
    __u32 mirror_mode;                  /* 2-s complement of mode used when mirroring */
    float last_xpy;
    __u16 width;                        /* external screen dimensions */
    __u16 height;
    __u32 xres;                         /* external screen resolution */
    __u32 yres;
    float m[2][3];                      /* external transformation matrix */
    hwc_rect_t mirror_region;           /* region of screen to mirror */
};
typedef struct omap4_hwc_ext omap4_hwc_ext_t;

enum bltpolicy {
    BLTPOLICY_DISABLED = 0,
    BLTPOLICY_DEFAULT = 1,    /* Default blit policy */
    BLTPOLICY_ALL,            /* Test mode to attempt to blit all */
};

enum bltmode {
    BLTMODE_PAINT = 0,    /* Attempt to blit layer by layer */
    BLTMODE_REGION = 1,   /* Attempt to blit layers via regions */
};

/* ARGB image */
struct omap4_hwc_img {
    int width;
    int height;
    int rowbytes;
    int size;
    unsigned char *ptr;
};

struct omap4_hwc_module {
    hwc_module_t base;

    IMG_framebuffer_device_public_t *fb_dev;
};
typedef struct omap4_hwc_module omap4_hwc_module_t;

struct counts {
    unsigned int possible_overlay_layers;
    unsigned int composited_layers;
    unsigned int scaled_layers;
    unsigned int RGB;
    unsigned int BGR;
    unsigned int NV12;
    unsigned int dockable;
    unsigned int protected;

    unsigned int max_hw_overlays;
    unsigned int max_scaling_overlays;
    unsigned int mem;
};
typedef struct counts counts_t;

struct omap4_hwc_device {
    /* static data */
    hwc_composer_device_1_t base;
    hwc_procs_t *procs;
    pthread_t hdmi_thread;
    pthread_mutex_t lock;

    IMG_framebuffer_device_public_t *fb_dev;
    struct dsscomp_display_info fb_dis;
    int fb_fd;                  /* file descriptor for /dev/fb0 */
    int dsscomp_fd;             /* file descriptor for /dev/dsscomp */
    int hdmi_fb_fd;             /* file descriptor for /dev/fb1 */
    int pipe_fds[2];            /* pipe to event thread */

    int img_mem_size;           /* size of fb for hdmi */
    void *img_mem_ptr;          /* start of fb for hdmi */

    int flags_rgb_order;
    int flags_nv12_only;
    float upscaled_nv12_limit;

    int on_tv;                  /* using a tv */
    int force_sgx;
    omap4_hwc_ext_t ext;        /* external mirroring data */
    int idle;
    int ovls_blending;

    buffer_handle_t *buffers;
    int use_sgx;
    int swap_rb;
    unsigned int post2_layers; /* Buffers used with DSS pipes*/
    unsigned int post2_blit_buffers; /* Buffers used with blit */
    int ext_ovls;               /* # of overlays on external display for current composition */
    int ext_ovls_wanted;        /* # of overlays that should be on external display for current composition */
    int last_ext_ovls;          /* # of overlays on external/internal display for last composition */
    int last_int_ovls;

    enum bltmode blt_mode;
    enum bltpolicy blt_policy;

    int blit_flags;
    int blit_num;
    struct omap_hwc_data comp_data; /* This is a kernel data structure */
    struct rgz_blt_entry blit_ops[RGZ_MAX_BLITS];
    struct counts stats;
    int ion_fd;
    struct ion_handle *ion_handles[2];
    bool use_sw_vsync;
};
typedef struct omap4_hwc_device omap4_hwc_device_t;

#endif
