#include "boot.h"
typedef unsigned char uint8_t;
#define NULL (void*)0
void *mmemcpy(void *dst, const void *src, int size)
{
    if (dst == NULL || src == NULL || size <= 0)
        return NULL;

    uint8_t *pdst = (uint8_t *)dst;
    uint8_t *psrc = (uint8_t *)src;

    if (pdst > psrc && pdst < psrc + size) // self copy
    {
        pdst = pdst + size - 1;
        psrc = psrc + size - 1;
        while (size--)
            *pdst-- = *psrc--;
    }
    else
    {
        while ((int)size--)
            *pdst++ = *psrc++;
    }
    return dst;
}

void *mmemset(void *dst, uint8_t set, int size)
{
    if (dst == NULL)
    {
        return NULL;
    }
    uint8_t *pdst = (uint8_t *)dst;
    while ((int)size--)
    {
        *pdst++ = set;
    }
    return dst;
}


#define SECTSIZE 512

void bootMain(void)
{
	int i = 0;
	int phoff = 0x34;
	unsigned int elf = 0xf00000; //load to a farther memory
	void (*kMainEntry)(void);
	kMainEntry = (void (*)(void))0x100000;

	for (i = 0; i < 200; i++)
	{
		readSect((void *)(elf + i * 512), 1 + i);
	}
	// while(1);
	// TODO: 填写kMainEntry、phoff、offset...... 然后加载Kernel（可以参考NEMU的某次lab）
	ELFHeader *eh = (ELFHeader *)elf;
	kMainEntry = (void (*)(void))eh->entry;
	phoff = eh->phoff;
	// unsigned int offset = 0x1000;
	
	// unsigned int phentsize = eh->phentsize;
	ProgramHeader *phEntry = (ProgramHeader *)(elf + phoff);
	ProgramHeader *ph = phEntry;

	// if(ph->type !=PT_LOAD)return;
	
	for (int idx_ph = 0; idx_ph < eh -> phnum; idx_ph++)
	{
		if (ph->type == PT_LOAD)
		{
			// if(ph->off == 0x1000)return;
			mmemcpy((void *)(ph->paddr), (void*)(ph->off + elf), ph->filesz);
			mmemset((void *)(ph->paddr + ph->filesz), 0, ph->memsz - ph->filesz);
		}
		
		ph++;
	}
	

	kMainEntry();
}

void waitDisk(void)
{ // waiting for disk
	while ((inByte(0x1F7) & 0xC0) != 0x40)
		;
}

void readSect(void *dst, int offset)
{ // reading a sector of disk
	int i;
	waitDisk();
	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);

	waitDisk();
	for (i = 0; i < SECTSIZE / 4; i++)
	{
		((int *)dst)[i] = inLong(0x1F0);
	}
}
