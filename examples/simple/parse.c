#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <inttypes.h>

/* debug */
#define OP_PUSH_16(V) printf("OP_PUSH_16 %d\n", V);
#define OP_PUSH_U8(V) printf("OP_PUSH_U8 %d\n", V);
#define OP_PUSH_U3(V) printf("OP_PUSH_U3 %d\n", V);
#define OP_USERFUNC(V) printf("OP_USERFUNC %d\n", V);
#define OP_POP() printf("OP_POP\n");
#define OP_JUNLESS_8(V) printf("OP_JUNLESS_8 %d\n", V);
#define OP_JUMP_8(V) printf("OP_JUMP_8 %d\n", V);
#define OP_RET_VOID() printf("OP_RET_VOID\n");
/* </debug */



char SOURCE[] = {
#include "source.h"
}; 

#include "./compiler.c"


int main () {
  if (!compiler()) {
    printf("Successful compilation.\n");
    return 0;
  } else {
    printf("Error compiling.\n");
    return 1;
  }
}