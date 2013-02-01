#include <Arduino.h>
#include <stdint.h>
#include <inttypes.h>
#include "embedvm.h"

#include "vmcode.hdr"

#define UNUSED __attribute__((unused))

#ifndef SCNd8
  #define SCNd8 "hhd"
#endif
#ifndef SCNu8
  #define SCNu8 "hhu"
#endif

/**
 * Compiler
 */

//uint8_t vm_mem[256] = { EMBEDVM_SECT_SRAM_DATA };
//uint8_t vm_mem[1024] = {0x91, 0x91, 0xb0, 0x9d, 0x91, 0x91, 0x92, 0xb0, 0x9d, 0x92, 0x91, 0x91, 0x93, 0xb0, 0x9d, 0x93, 0x92, 0x91, 0x91, 0x98, 0x04, 0xb0, 0x9d, 0x91, 0xa6, 0x04, 0xa0, 0xfd, 0x9c};

uint8_t vm_mem[1024];
uint8_t vm_mem_ptr = 0;

void OP (uint8_t val) { vm_mem[vm_mem_ptr++] = val; }
#define OP_PUSH_U8(V) OP(0x98); OP(V);
#define OP_PUSH_U3(V) OP(0x90 + (0x07 & V));
#define OP_USERFUNC(V) OP(0xb0 + (0x0f & V));
#define OP_POP() OP(0x9d);
#define OP_JUNLESS_8(V) OP(0xa6); OP(V);
#define OP_JUMP_8(V) OP(0xa0); OP(V);
#define OP_RET_VOID() OP(0x9c);


#include "./compiler.c"




/**
 * VM Start
 */

struct embedvm_s vm = { };

int16_t mem_read(uint16_t addr, bool is16bit, void *ctx)
{
	if (addr + (is16bit ? 1 : 0) >= sizeof(vm_mem))
		return 0;
	if (is16bit)
		return (vm_mem[addr] << 8) | vm_mem[addr+1];
	return vm_mem[addr];
}

void mem_write(uint16_t addr, int16_t value, bool is16bit, void *ctx)
{
	if (addr + (is16bit ? 1 : 0) >= sizeof(vm_mem))
		return;
	if (is16bit) {
		vm_mem[addr] = value >> 8;
		vm_mem[addr+1] = value;
	} else
		vm_mem[addr] = value;
}

int16_t call_user(uint8_t funcid, uint8_t argc, int16_t *argv, void *ctx)
{
	Serial.print("Called user function ");
	Serial.print(funcid);
	Serial.print(" with args: ");
	Serial.println(argc);

	for (int i = 0; i < argc; i++) {
		Serial.println(argv[i], DEC);
	}

	return 0;
}

void setup()
{
	Serial.begin(9600);
	Serial.println("Initializing...");

	memset(vm_mem, 0, sizeof(vm_mem));
	compiler();

	/*
	for (int i = 0; i < 100; i++) {
		Serial.println(vm_mem[i], HEX);
	}
	while (true) { }
		*/

	vm.ip = EMBEDVM_SYM_main;
	vm.sp = vm.sfp = sizeof(vm_mem);
	vm.mem_read = &mem_read;
	vm.mem_write = &mem_write;
	vm.call_user = &call_user;
}

void loop()
{
	// Serial.print("<");
	// Serial.print(vm.ip, DEC);
	// Serial.print(">");
	embedvm_exec(&vm);
}

