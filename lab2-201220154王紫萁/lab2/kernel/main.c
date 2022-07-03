#include "common.h"
#include "x86.h"
#include "device.h"

void kEntry(void) {
	// Interruption is disabled in bootloader
	initSerial();// initialize serial port
	// putStr("serial initiateed successfully\n");
	// TODO: 做一系列初始化
	// initialize idt
	initIdt();
	// iniialize 8259a
	initIntr();
	// initialize gdt, tss
	initSeg();
	// initialize vga device
	initVga();
	// putStr("vga initiateed successfully\n");
	// initialize keyboard device
	initKeyTable();
	putStr("keytable initiateed successfully\n");
	loadUMain(); // load user program, enter user space

	while(1);
	assert(0);
}
