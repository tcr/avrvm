#include <Arduino.h>
#include "EEPROM.h"
#include <stdint.h>
#include "embedvm.h"



#define UNUSED __attribute__((unused))
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))

#define VM_EEPROM 1
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
	if (addr + (is16bit ? 1 : 0) >= VM_MEM_SIZE)
		return 0;
	if (is16bit)
		return (vm_read(addr) << 8) | vm_read(addr+1);
	return vm_read(addr);
}

void mem_write(uint16_t addr, int16_t value, bool is16bit, void *ctx)
{
	if (addr + (is16bit ? 1 : 0) >= VM_MEM_SIZE)
		return;
	if (is16bit) {
		vm_write(addr, value >> 8);
		vm_write(addr+1, value);
	} else
		vm_write(addr, value);
}

int16_t call_user(uint8_t funcid, uint8_t argc, int16_t *argv, void *ctx)
{
	int r = 0;

	switch (funcid) {
		case 0:
			Serial.println("");
			Serial.print(F("[[eof, free ram: "));
			Serial.print(freeRam());
			Serial.println(F("]]"));
			while (true) { }
			return 0;

		case 1:
			Serial.print(F("Logging "));
			Serial.print(argc);
			Serial.println(F(" arguments:"));

			for (int i = 0; i < argc; i++) {
				Serial.print(F("["));
				Serial.print(i);
				Serial.print(F("] "));
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
			Serial.print(F("delay: "));
			Serial.println(argv[0]);
  		delay(argv[0]);
  		return 0;

		case 4:
			Serial.print(F("pinMode: "));
			Serial.print(argv[0]);
			Serial.print(F(" val: "));
			Serial.println(argv[1]);
			pinMode(argv[0], argv[1] > 0 ? INPUT : OUTPUT);
			return 0;

		case 5:
			Serial.print(F("digitalRead pin: "));
			Serial.print(argv[0]);
			r = digitalRead(argv[0]);
			Serial.print(F(" val: "));
			Serial.println(r, DEC);
			return r;

		case 6:
			Serial.print(F("digitalWrite pin: "));
			Serial.print(argv[0]);
			Serial.print(F(" val: "));
			Serial.println(argv[1]);
			digitalWrite(argv[0], argv[1] > 0 ? HIGH : LOW);   // sets the LED on
			return 0;
	}
	return 0;
}

void downloadScript ()
{
	Serial.println(F("Downloading..."));

	int lasttime = millis();
	while (Serial.available() < 2 && millis() - lasttime < 3*1000) {
		continue;
	}
	if (Serial.available() < 2) {
		Serial.println(F("Restoring from EEPROM.\n"));
		return;
	}
	int targetlen = (Serial.read() << 8) + (Serial.read() & 0xff) + 1;
	Serial.print(F("Expecting "));
	Serial.print(targetlen, DEC);
	Serial.println(F(" bytes."));

	// Read from serial and compile.
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

	if (targetlen != srci) {
		Serial.print(F("Invalid length.\n\n"));
		while (Serial.available()) {
			Serial.read();
		}
		return;
	}
	Serial.print(F("Free RAM: "));
	Serial.println(freeRam());
	Serial.println("");

	delay(1000);

	int start = 0;
	if (compiler(&start)) {
		Serial.println(F("Error compiling."));
		while (true) { }
	} else {
		Serial.print(F("Success compiling ("));
		Serial.print(vm_mem_ptr, DEC);
		Serial.println(F(" bytes)"));
		for (int i = 0; i < vm_mem_ptr; i++) {
			Serial.print(F("0x"));
			Serial.print((uint8_t) vm_read(i), HEX);
			Serial.print(F(" "));
		}
		Serial.println("");
		Serial.println("");
	}

	// Write start to EEPROM.
	EEPROM.write(1000, (uint8_t) (start >> 8));
	EEPROM.write(1001, (uint8_t) (start & 0xFF));

	while (Serial.available()) {
		Serial.read();
	}
}

void setup()
{
	Serial.begin(9600);
	Serial.flush();
	Serial.println(F("\n\nInitializing..."));
	while (Serial.available() > 0) {
		Serial.read();
	}
	downloadScript();

	// Start.
	vm.ip = (EEPROM.read(1000) << 8) + EEPROM.read(1001);
	vm.sp = vm.sfp = VM_MEM_SIZE;
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

