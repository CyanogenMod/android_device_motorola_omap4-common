#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <linux/fs.h>
#include <linux/stat.h>
#include "swap.h"

int mkswap_main(int argc, char *argv[])
{
  int i, dev_fd, random_fd, result, exit_code = 0;
  struct stat stat;
  size_t dev_size;
  union swap_header hdr;

  if(argc < 2) {
    fprintf(stderr, "Usage: mkswap <blockdev>\n");
    return 1;
  }

  for(i = 1; i < argc; i++) {
    dev_fd = open(argv[i], O_RDWR, 0600);
    if (0 > dev_fd) {
      fprintf(stderr, "Error: cannot open %s: %s\n", argv[i], strerror(errno));
      exit_code = 1;
      continue;
    }

    result = fstat(dev_fd, &stat);
    if (0 > result) {
      fprintf(stderr, "Error: cannot stat %s: %s\n", argv[i], strerror(errno));
      exit_code = 1;
      goto close_block;
    }

    if (!S_ISBLK(stat.st_mode)) {
      fprintf(stderr, "Error: %s is not a block device\n", argv[i]);
      exit_code = 1;
      goto close_block;
    }

    result = ioctl(dev_fd, BLKGETSIZE64, &dev_size);
    if (0 > result) {
      fprintf(stderr, "Error: ioctl on %s failed: %s\n", argv[i],
          strerror(errno));
      exit_code = 1;
      goto close_block;
    }

    memset(&hdr, 0, sizeof(hdr));
    memcpy(hdr.magic.magic, "SWAPSPACE2", 10);
    hdr.info.version = 1;
    hdr.info.last_page = (dev_size / PAGE_SIZE) - 1;

    random_fd = open("/dev/urandom", O_RDONLY);
    if (0 > random_fd) {
      fprintf(stderr, "Warning: cannot open random source: %s\n",
          strerror(errno));
    } else {
      read(random_fd, hdr.info.sws_uuid, sizeof(hdr.info.sws_uuid));
      close(random_fd);
    }

    /* see definition of version 4 UUID for more information */
    hdr.info.sws_uuid[6] &= 0x0F;
    hdr.info.sws_uuid[6] |= 0x40;
    hdr.info.sws_uuid[8] &= 0x3F;
    hdr.info.sws_uuid[8] |= 0x80;

    result = write(dev_fd, &hdr, sizeof(hdr));
    if ((int)sizeof(hdr) > result) {
      fprintf(stderr, "Error: write to %s failed\n", argv[i]);
      exit_code = 1;
    }

close_block:
    close(dev_fd);
  }

  return exit_code;
}
