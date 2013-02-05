#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <inttypes.h>

char SOURCE[] = {
#include "source.h"
}; 

#include "./compiler-mem.c"


int main () {
  if (!compiler()) {
    printf("Successful compilation.\n");

    printf("(%d bytes)\n", vm_mem_ptr);
    int i;
    for (i = 0; i < vm_mem_ptr; i++) {
      printf("0x%02x ", vm_mem[i]);
    }

    return 0;
  } else {
    printf("Error compiling.\n");
    return 1;
  }
}