#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <inttypes.h>

#ifndef SCNd8
#  define SCNd8 "hhd"
#endif
#ifndef SCNd16
#  define SCNd16 "hd"
#endif
#ifndef SCNd32
#  define SCNd32 "d"
#endif
#ifndef SCNd64
#  define SCNd64 PRI64_PREFIX "d"
#endif

#ifndef SCNu8
#  define SCNu8 "hhu"
#endif
#ifndef SCNu16
#  define SCNu16 "hu"
#endif
#ifndef SCNu32
#  define SCNu32 "u"
#endif
#ifndef SCNu64
#  define SCNu64 PRI64_PREFIX "u"
#endif

/**
 * Config
 */

#define SOURCE_SIZE 600

/**
 * Compiler
 */

// #define DEBUG 0

uint8_t vm_mem_ptr = 0;

#ifndef VM_EEPROM
  #define VM_MEM_SIZE 1000
  uint8_t vm_mem[VM_MEM_SIZE];
  void vm_write (int pos, uint8_t val) { vm_mem[pos] = val; }
  uint8_t vm_read (int pos) { return vm_mem[pos]; }
#endif
#ifdef VM_EEPROM
  #define VM_MEM_SIZE 1000
  void vm_write (int pos, uint8_t val) { EEPROM.write(pos, val); }
  uint8_t vm_read (int pos) { return EEPROM.read(pos); }
#endif

#ifdef DEBUG
  #define OP_POP_BUT_TOP(V) printf("OP_POP_BUT_TOP %d\n", V);
  #define OP_CALL(V) printf("OP_CALL %d\n", V);
  #define OP_PUSH_NUM(V) printf("OP_PUSH_NUM %d\n", V);
  #define OP_LABEL() (printf("OP_MARK\n"), 0)
  #define OP_CHANGE(V) printf("OP_CHANGE %d\n", V);
  #define OP_ADD() printf("OP_ADD\n");
  #define OP_SUB() printf("OP_SUB\n");
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
  #define OP_RET() printf("OP_RET\n");
  #define OP_RET_VOID() printf("OP_RET_VOID\n");
#endif

#ifndef DEBUG
  void OP (uint8_t val) {
    vm_write(vm_mem_ptr++, val);
  }
  #define OP_POP_BUT_TOP(V) OP(0xf8 + ((V - 1) & 0x07));
  #define OP_CALL(V) OP(0xa2); OP((int8_t) (V - vm_mem_ptr + 1));
  #define OP_PUSH_NUM(arg) if (arg <= 0x3 && arg >= -0x4) { \
      OP_PUSH_U3((int8_t) arg); \
    } else if (arg <= 0xff && arg >= 0) { \
      OP_PUSH_U8((uint8_t) arg); \
    } else { \
      OP_PUSH_16(arg); \
    }
  #define OP_LABEL() (vm_mem_ptr)
  #define OP_CHANGE(V) (vm_mem_ptr = V);
  #define OP_ADD() OP(0x80 + 0x00)
  #define OP_SUB() OP(0x80 + 0x01)
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
  #define OP_RET() OP(0x9b);
  #define OP_RET_VOID() OP(0x9c);
#endif

char SOURCE[SOURCE_SIZE];

#include "./compiler.c"