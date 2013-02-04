#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <inttypes.h>

/* debug */

#define DEBUG 1

#ifdef DEBUG
#define OP_DUP(D) printf("OP_DUP %d\n", D);
#define OP_POP_LOCAL(V) printf("OP_POP_LOCAL %d\n", V);
#define OP_PUSH_LOCAL(V) printf("OP_PUSH_LOCAL %d\n", V);
#define OP_PUSH_ZEROES(V) printf("OP_PUSH_ZEROES %d\n", V);
#define OP_PUSH_16(V) printf("OP_PUSH_16 %d\n", V);
#define OP_PUSH_U8(V) printf("OP_PUSH_U8 %d\n", V);
#define OP_PUSH_U3(V) printf("OP_PUSH_U3 %d\n", V);
#define OP_USERFUNC(V) printf("OP_USERFUNC %d\n", V);
#define OP_POP() printf("OP_POP\n");
#define OP_JUNLESS_8(V) printf("OP_JUNLESS_8 %d\n", V);
#define OP_JUMP_8(V) printf("OP_JUMP_8 %d\n", V);
#define OP_RET_VOID() printf("OP_RET_VOID\n");
#endif

/* </debug */

uint8_t vm_mem[1024];
uint8_t vm_mem_ptr = 0;

#ifndef DEBUG
void OP (uint8_t val) { vm_mem[vm_mem_ptr++] = val; }
#define OP_DUP(V) OP(0xc0 + (V & 0x07) + 0x05) // TODO V <= 5
#define OP_PUSH_LOCAL(V) OP(0x00 + (0x3f & V));
#define OP_POP_LOCAL(V) OP(0x40 + (0x3f & V));
#define OP_PUSH_ZEROES(V) OP(0xf0 + ((V - 1) & 0xf)); // TODO split > 7
#define OP_PUSH_16(V) OP(0x9a); OP(V >> 8); OP(V & 0xff);
#define OP_PUSH_U8(V) OP(0x98); OP(V);
#define OP_PUSH_U3(V) OP(0x90 + (0x07 & V));
#define OP_USERFUNC(V) OP(0xb0 + (0x0f & V));
#define OP_POP() OP(0x9d);
#define OP_JUNLESS_8(V) OP(0xa6); OP(V);
#define OP_JUMP_8(V) OP(0xa0); OP(V);
#define OP_RET_VOID() OP(0x9c);
#endif


char SOURCE[] = {
#include "source.h"
}; 

#include "./compiler.c"


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