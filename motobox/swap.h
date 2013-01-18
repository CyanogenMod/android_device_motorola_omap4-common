#ifndef _LINUX_SWAP_H
#define _LINUX_SWAP_H

#include <asm/page.h>

/* Obtained from ICS bionic/libc/kernel/common/linux/swap.h. (Version from
 * Bionic does not compile.)
 */

#define SWAP_FLAG_PREFER 0x8000  
#define SWAP_FLAG_PRIO_MASK 0x7fff
#define SWAP_FLAG_PRIO_SHIFT 0
#define SWAP_FLAG_DISCARD 0x10000  

union swap_header {
  struct {
    char reserved[PAGE_SIZE - 10];
    char magic[10];
  } magic;
  struct {
    char bootbits[1024];
    __u32 version;
    __u32 last_page;
    __u32 nr_badpages;
    unsigned char sws_uuid[16];
    unsigned char sws_volume[16];
    __u32 padding[117];
    __u32 badpages[1];
  } info;
};

#endif
