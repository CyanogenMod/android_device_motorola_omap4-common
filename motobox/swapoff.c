#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h> /* __NR_swapoff */

int swapoff_main(int argc, char *argv[])
{
  int result;

  if (argc != 2) {
    fprintf(stderr, "Usage: swapoff swapdevice\n");
    return 1;
  }

  result = syscall(__NR_swapoff, argv[1]);
  if (0 > result) {
    fprintf(stderr, "Error: failed to disable swap on %s: %s\n", argv[1],
        strerror(errno));
    return 1;
  }

  return 0;
}
