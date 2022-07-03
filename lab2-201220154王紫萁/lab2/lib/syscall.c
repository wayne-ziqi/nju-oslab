#include "lib.h"
#include "types.h"
#include "const.h"


/*
 * io lib here
 * 库函数写在这
 */


//static inline int32_t syscall(int num, uint32_t a1,uint32_t a2,
int32_t syscall(int num, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret = 0;
	//Generic system call: pass system call number in AX
	//up to five parameters in DX,CX,BX,DI,SI
	//Interrupt kernel with T_SYSCALL
	//
	//The "volatile" tells the assembler not to optimize
	//this instruction away just because we don't use the
	//return value
	//
	//The last clause tells the assembler that this can potentially
	//change the condition and arbitrary memory locations.

	/*
		 Note: ebp shouldn't be flushed
	    May not be necessary to store the value of eax, ebx, ecx, edx, esi, edi
	*/
	uint32_t eax, ecx, edx, ebx, esi, edi;
	uint16_t selector;
	
	asm volatile("movl %%eax, %0":"=m"(eax));
	asm volatile("movl %%ecx, %0":"=m"(ecx));
	asm volatile("movl %%edx, %0":"=m"(edx));
	asm volatile("movl %%ebx, %0":"=m"(ebx));
	asm volatile("movl %%esi,  %0":"=m"(esi));
	asm volatile("movl %%edi,  %0":"=m"(edi));
	asm volatile("movl %0, %%eax": :"m"(num));
	asm volatile("movl %0, %%ecx"::"m"(a1));
	asm volatile("movl %0, %%edx"::"m"(a2));
	asm volatile("movl %0, %%ebx"::"m"(a3));
	asm volatile("movl %0, %%esi" ::"m"(a4));
	asm volatile("movl %0, %%edi" ::"m"(a5));
	asm volatile("int $0x80");
	asm volatile("movl %%eax, %0":"=m"(ret));
	asm volatile("movl %0, %%eax"::"m"(eax));
	asm volatile("movl %0, %%ecx"::"m"(ecx));
	asm volatile("movl %0, %%edx"::"m"(edx));
	asm volatile("movl %0, %%ebx"::"m"(ebx));
	asm volatile("movl %0, %%esi"::"m"(esi));
	asm volatile("movl %0, %%edi"::"m"(edi));
	
	asm volatile("movw %%ss, %0":"=m"(selector)); //%ds is reset after iret
	//selector = 16;
	asm volatile("movw %%ax, %%ds"::"a"(selector));
	
	return ret;
}

char getChar(){ // 对应SYS_READ STD_IN
	// TODO: 实现getChar函数，方式不限
	uint32_t tmp = 0;
	return (char)(syscall(SYS_READ, STD_IN, tmp, 1, 0, 0) & 0xff);
	
}

void getStr(char *str, int size){ // 对应SYS_READ STD_STR
	// TODO: 实现getStr函数，方式不限
	syscall(SYS_READ, STD_STR, (uint32_t)str, (uint32_t)size, 0, 0);
}

typedef char *m_va_list;
typedef unsigned int acpi_native_uint;
#define m_AUPBND (sizeof (acpi_native_uint) - 1)
#define m_ADNBND (sizeof (acpi_native_uint) - 1)
#define m_bnd(X, bnd) (((sizeof (X)) + (bnd)) & (~(bnd)))  //其作用是--倘若sizeof(X)不是4的整数倍，去余加4。
#define m_va_arg(ap, T) (*(T *)(((ap) += (m_bnd (T, m_AUPBND))) - (m_bnd (T,m_ADNBND))))
#define m_va_start(ap, A) (m_va_list)((ap) = (((char *) &(A)) + (m_bnd (A,m_AUPBND))))
#define m_va_end(ap) ( (ap) = (m_va_list)0 )


int dec2Str(int decimal, char *buffer, int size, int count);

int hex2Str(uint32_t hexadecimal, char *buffer, int size, int count);

int str2Str(char *string, char *buffer, int size, int count);

void printf(const char *format, ...) {
    int i = 0; // format index
    char buffer[MAX_BUFFER_SIZE];
    int count = 0; // buffer index
//    int index=0; // parameter index
    m_va_list ap;// address of format in stack
    int state = 0; // 0: legal character; 1: '%'; 2: illegal format
    int decimal = 0;
    uint32_t hexadecimal = 0;
    char *string = 0;
    char character = 0;
//    void* para=0;
    m_va_start(ap, format);
    while (format[i] != 0) {
        //buffer[count] = format[i];
        //count++;
        //i++;
        //TODO: 可以借助状态机（回忆数电），辅助的函数已经实现好了，注意阅读手册
        switch (state) {
            case 0:
                if (format[i] != '%') {
                    buffer[count] = format[i];
                    count++;
                    i++;
                } else {
                    state = 1;
                    i++;
                }
                break;
            case 1:
                switch (format[i]) {
                    case 'd':
                        decimal = m_va_arg(ap, int);
                        count = dec2Str(decimal, buffer, MAX_BUFFER_SIZE, count);
                        state = 0;
                        i++;
                        break;
                    case 'x':
                        buffer[count++] = '0';
                        buffer[count++] = 'x';
                        hexadecimal = m_va_arg(ap, uint32_t);
                        count = hex2Str(hexadecimal, buffer, MAX_BUFFER_SIZE, count);
                        state = 0;
                        i++;
                        break;
                    case 's':
                        string = m_va_arg(ap, char*);
                        count = str2Str(string, buffer, MAX_BUFFER_SIZE, count);
                        state = 0;
                        i++;
                        break;
                    case 'c':
                        character = m_va_arg(ap, char);
                        buffer[count++] = character;
                        state = 0;
                        i++;
                        break;
                    default:
                        state = 2;
                        break;
                }
                break;
            case 2:
                state = 0;
                i++;
                break;
        }
    }
    buffer[count++] = '\0';
    if (count != 0)
        syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)count, 0, 0);

    m_va_end(ap);
}

boolean tokenize(char *in, int *left, int *right, char split);

void str2dec(char *in, int start, int right, int *target);

void str2hex(char *in, int start, int right, uint32_t *target);

void str2str(char *in, int start, int right, char *target);

void delSpace(const char *string, int *start);

void scanf(const char* format, ...){
	int i = 0; // format index
    int count = 0; // buffer index
	char buffer[MAX_BUFFER_SIZE];
//    int index=0; // parameter index
    m_va_list ap;// address of format in stack
    int state = 0; // 0: legal character; 1: '%'; 2: illegal format
    int *addr_decimal = 0;
    uint32_t *addr_hexadecimal = 0;
    char *addr_string = 0;
    char *addr_character = 0;
	getStr(buffer, MAX_BUFFER_SIZE);
    // printf("%s\n", buffer);
//    void* para=0;
    m_va_start(ap, format);
	while(format[i]!=0){
		switch (state) {
            case 0:
                if (format[i] != '%') {
                    i++;
                } else {
                    state = 1;
                    i++;
                }
                break;
            case 1:
                switch (format[i]) {
					
                    case 'd': {//find split
                        int last = i + 1;
                        delSpace(format, &last);
                        char split = format[last];
                        delSpace(buffer, &count);
                        //find right boundary
                        int right = count;
                        boolean tokenyes = tokenize(buffer, &count, &right, split);
                        if (!tokenyes) {
                            state = 2;
                            break;
                        }
                        addr_decimal = m_va_arg(ap, int*);
                        str2dec(buffer, count, right, addr_decimal);
                        count = right + 1;
                        state = 0;
                        i = last + 1;
                        break;
                    }
                    case 'x':{
                        //find split
                        int last = i + 1;
                        delSpace(format, &last);
                        char split = format[last];
                        delSpace(buffer, &count);
                        //find right boundary
                        int right = count;
                        boolean tokenyes = tokenize(buffer, &count, &right, split);
                        if (!tokenyes) {
                            state = 2;
                            break;
                        }
                       addr_hexadecimal = m_va_arg(ap, uint32_t*);
                        str2hex(buffer, count, right, addr_hexadecimal);
                        count = right + 1;
                        state = 0;
                        i = last + 1;
                        break;
                    }
                    case 's':{
                        delSpace(buffer, &count);
                        int right = count;

                        boolean tokenyes = tokenize(buffer, &count, &right, ' ');
						if (!tokenyes) {
                            state = 2;
                            break;
                        }
                        addr_string = m_va_arg(ap, char*);
                        str2str(buffer, count, right, addr_string);
                        count = right + 1;
                        state = 0;
                        i++;
                        break;
                    }
                    case 'c':
                        delSpace(buffer, &count);
                        addr_character = m_va_arg(ap, char*);
                        *addr_character = buffer[count];
                        state = 0;
                        i++;
                        break;
                    default:
                        state = 2;
                        break;
                }
                break;
            case 2:
                state = 0;
                i++;
                break;
        }
	}
	m_va_end(ap);
}

void delSpace(const char *string, int *start) {
    while (string[*start] != 0 && string[*start] == ' ') {
        (*start)++;
    }
//    (*start)--;
}
boolean tokenize(char *in, int *left, int *right, char split) {
    while (in[*right] != 0 && in[*right] != split)(*right)++;
//    (*right)--;
    if (in[*right] == split)return TRUE;
    else return FALSE;
}
void str2dec(char *in, int start, int right, int *target) {
    int result = 0;
    for (int i = start; i < right; i++) {
        result = result * 10 + in[i] - '0';
    }
    *target = result;
}
void str2hex(char *in, int start, int right, uint32_t *target) {
    int result = 0;
    for (int i = start; i < right; i++) {
        int now = 0;
        if (in[i] >= '0' && in[i] <= '9') now = in[i] - '0';
        else if (in[i] >= 'a' && in[i] <= 'f') now = in[i] - 'a' + 10;
        result = result * 16 + now;
    }
    *target = result;
}

void str2str(char *in, int start, int right, char *target) {
    int cnt = 0;
    for (int i = start; i < right; i++) {
        target[cnt++] = in[i];
    }
    target[cnt++] = 0;
}


int dec2Str(int decimal, char *buffer, int size, int count) {
	int i=0;
	int temp;
	int number[16];

	if(decimal<0){
		buffer[count]='-';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		temp=decimal/10;
		number[i]=temp*10-decimal;
		decimal=temp;
		i++;
		while(decimal!=0){
			temp=decimal/10;
			number[i]=temp*10-decimal;
			decimal=temp;
			i++;
		}
	}
	else{
		temp=decimal/10;
		number[i]=decimal-temp*10;
		decimal=temp;
		i++;
		while(decimal!=0){
			temp=decimal/10;
			number[i]=decimal-temp*10;
			decimal=temp;
			i++;
		}
	}

	while(i!=0){
		buffer[count]=number[i-1]+'0';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i--;
	}
	return count;
}

int hex2Str(uint32_t hexadecimal, char *buffer, int size, int count) {
	int i=0;
	uint32_t temp=0;
	int number[16];

	temp=hexadecimal>>4;
	number[i]=hexadecimal-(temp<<4);
	hexadecimal=temp;
	i++;
	while(hexadecimal!=0){
		temp=hexadecimal>>4;
		number[i]=hexadecimal-(temp<<4);
		hexadecimal=temp;
		i++;
	}

	while(i!=0){
		if(number[i-1]<10)
			buffer[count]=number[i-1]+'0';
		else
			buffer[count]=number[i-1]-10+'a';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i--;
	}
	return count;
}

int str2Str(char *string, char *buffer, int size, int count) {
	int i=0;
	while(string[i]!=0){
		buffer[count]=string[i];
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i++;
	}
	return count;
}
