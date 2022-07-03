#include "lib.h"
#include "types.h"

int cnt;

void t1() {
    for (; cnt < 20; cnt++) {
        printf("thread p1, loop: %d\n", cnt);
    }
    pthread_destroy();
}

void t2() {
    for (; cnt < 10; cnt++) {
        printf("thread p2, loop: %d\n", cnt);
    }
    pthread_destroy();
}

int main(void) {
    printf("printf test begin...\n");
    printf("the answer should be:\n");
    printf("#######################################################\n");
    printf("Hello, welcome to OSlab! I'm the body of the game. ");
    printf("Bootblock loads me to the memory position of 0x100000, and Makefile also tells me that I'm at the location of 0x100000. ");
    printf("\\%%~!@#/(^&*()_+`1234567890-=...... ");
    printf("Now I will test your printf: ");
    printf("1 + 1 = 2, 123 * 456 = 56088\n0, -1, -2147483648, -1412567295, -32768, 102030\n0, ffffffff, 80000000, abcdef01, ffff8000, 18e8e\n");
    printf("#######################################################\n");
    // printf("your answer:\n");
    // printf("=======================================================\n");
    // printf("%s %s%scome %co%s", "Hello,", "", "wel", 't', " ");
    // printf("%c%c%c%c%c! ", 'O', 'S', 'l', 'a', 'b');
    // printf("I'm the %s of %s. %s 0x%x, %s 0x%x. ", "body", "the game", "Bootblock loads me to the memory position of", 0x100000, "and Makefile also tells me that I'm at the location of", 0x100000);
    // printf("\\%%~!@#/(^&*()_+`1234567890-=...... ");
    // printf("Now I will test your printf: ");
    // printf("%d + %d = %d, %d * %d = %d\n", 1, 1, 1 + 1, 123, 456, 123 * 456);
    // printf("%d, %d, %d, %d, %d, %d\n", 0, 0xffffffff, 0x80000000, 0xabcdef01, -32768, 102030);
    // printf("%x, %x, %x, %x, %x, %x\n", 0, 0xffffffff, 0x80000000, 0xabcdef01, -32768, 102030);
    // printf("=======================================================\n");
    // printf("Test end!!! Good luck!!!\n");
    cnt = 0;
    uint32_t pid1 = pthread_create((uint32_t)t1);
    uint32_t pid2 = pthread_create((uint32_t)t2);
    printf("thread1 id: %d, thread2 id: %d\n", pid1, pid2);
    pthread_destroy();
    return 0;
}
