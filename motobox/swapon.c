#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h> /* __NR_swapon */

#include "swap.h"

void print_usage(void)
{
  fprintf(stderr, "Usage: swapon [-p priority] swapdevice\n");
}

int swapon_main(int argc, char *argv[])
{
  char *swapdevice = argv[1];
  int swapflags = 0, result;

  if ((argc != 2) && (argc != 4)) {
    print_usage();
    return 1;
  }

  if (4 == argc) {
    unsigned long int prio;
    char *endptr;

    if (!strncmp(argv[1], "-p", 4)) {
      print_usage();
      return 1;
    }

    prio = strtoul(argv[2], &endptr, 10);
    if ((endptr == argv[2]) || !*endptr) {
      print_usage();
      return 1;
    }

    swapflags = SWAP_FLAG_PREFER |
      ((prio << SWAP_FLAG_PRIO_SHIFT) & SWAP_FLAG_PRIO_MASK);
    swapdevice = argv[3];
  }

  result = syscall(__NR_swapon, swapdevice, swapflags);
  if (0 > result) {
    fprintf(stderr, "Error: failed to enable swap on %s: %s\n", swapdevice,
        strerror(errno));
    return 1;
  }

  return 0;
}
