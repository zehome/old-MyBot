#include <stdio.h>
#include <stdlib.h>
#include "config.h"

int main (int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s configFile.conf\n", argv[0]);
    exit(0);
  }
  parseConfig(argv[1]);
  return 0;
}
