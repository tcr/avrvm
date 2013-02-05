#include <Arduino.h>
#include <stdint.h>
#include "embedvm.h"



#define UNUSED __attribute__((unused))
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))

#include "compiler-mem.c"




/**
 * VM Start
 */

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

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
	switch (funcid) {
		case 0:
			Serial.println("");
			Serial.print(F("[[eof, free ram: "));
			Serial.print(freeRam());
			Serial.println(F("]]"));
			while (true) { }
			return 0;

		case 1:
			Serial.print(F("Called user function "));
			Serial.print(funcid);
			Serial.print(F(" with args: "));
			Serial.println(argc);

			for (int i = 0; i < argc; i++) {
				Serial.println(argv[i], DEC);
			}

			return 0;

		case 2:
			for (int i = 0; i < argc; i++) {
				Serial.print((char) argv[i]);
			}
			Serial.flush();
			return 0;

		case 3:
			Serial.print(F("digitalWrite pin: "));
			Serial.print(argv[0]);
			Serial.print(F(" val: "));
			Serial.println(argv[1]);
			digitalWrite(argv[0], argv[1] > 0 ? HIGH : LOW);   // sets the LED on
			return 0;

		case 4:
			Serial.print(F("delay: "));
			Serial.println(argv[0]);
  		delay(argv[0]);
  		return 0;
	}
	return 0;
}

void setup()
{
	pinMode(13, OUTPUT);      // sets the digital pin as output

	Serial.begin(9600);
	Serial.println(F("Initializing..."));

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

	Serial.print(F("Downloaded "));
	Serial.print(srci, DEC);
	Serial.println(F(" bytes."));
	Serial.println(SOURCE);
	Serial.print(F("Free RAM:"));
	Serial.println(freeRam());
	Serial.println("");

	delay(1000);
	Serial.flush();

	if (compiler()) {
		Serial.println(F("Error compiling."));
		while (true) { }
	} else {
		Serial.print(F("Success compiling ("));
		Serial.print(vm_mem_ptr, DEC);
		Serial.println(F(" bytes)"));
		for (int i = 0; i < vm_mem_ptr; i++) {
			Serial.print(F("0x"));
			Serial.print(vm_mem[i], HEX);
			Serial.print(F(" "));
		}
		Serial.println("");
		Serial.println("");

		vm.ip = 0;
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

