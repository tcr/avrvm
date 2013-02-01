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

#define VM_MEM_SIZE 300
#define SOURCE_SIZE 1024

/**
 * Compiler
 */

//uint8_t vm_mem[256] = { EMBEDVM_SECT_SRAM_DATA };
//uint8_t vm_mem[1024] = {0x91, 0x91, 0xb0, 0x9d, 0x91, 0x91, 0x92, 0xb0, 0x9d, 0x92, 0x91, 0x91, 0x93, 0xb0, 0x9d, 0x93, 0x92, 0x91, 0x91, 0x98, 0x04, 0xb0, 0x9d, 0x91, 0xa6, 0x04, 0xa0, 0xfd, 0x9c};

uint8_t vm_mem[VM_MEM_SIZE];
uint8_t vm_mem_ptr = 0;

void OP (uint8_t val) { vm_mem[vm_mem_ptr++] = val; }
#define OP_PUSH_U8(V) OP(0x98); OP(V);
#define OP_PUSH_U3(V) OP(0x90 + (0x07 & V));
#define OP_USERFUNC(V) OP(0xb0 + (0x0f & V));
#define OP_POP() OP(0x9d);
#define OP_JUNLESS_8(V) OP(0xa6); OP(V);
#define OP_JUMP_8(V) OP(0xa0); OP(V);
#define OP_RET_VOID() OP(0x9c);

char SOURCE[SOURCE_SIZE] = "";

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

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setup()
{
	Serial.begin(9600);
	Serial.println("Initializing...");

	int srci = 0;
	while (srci < sizeof(SOURCE)) {
		if (Serial.available() > 0) {
		  // read the incoming byte:
		  uint8_t b = Serial.read();
		  SOURCE[srci++] = b;
		  if (b == '\0') {
		  	break;
		  }
		}
	}

	Serial.print("Downloaded ");
	Serial.print(srci, DEC);
	Serial.println(" bytes.");
	Serial.println(SOURCE);
	Serial.print("Free RAM:");
	Serial.println(freeRam());
	Serial.println("");

	delay(1000);

	if (compiler()) {
		Serial.println("Error compiling.");
	} else {
		Serial.println("Success compiling.");
		for (int i = 0; i < vm_mem_ptr; i++) {
			Serial.print("0x");
			Serial.print(vm_mem[i], HEX);
			Serial.print(" ");
		}
		Serial.println("");
		Serial.println("");

		vm.ip = EMBEDVM_SYM_main;
		vm.sp = vm.sfp = sizeof(vm_mem);
		vm.mem_read = &mem_read;
		vm.mem_write = &mem_write;
		vm.call_user = &call_user;
	}
}

void loop()
{
	// Serial.print("<");
	// Serial.print(vm.ip, DEC);
	// Serial.print(">");

	embedvm_exec(&vm);
}
