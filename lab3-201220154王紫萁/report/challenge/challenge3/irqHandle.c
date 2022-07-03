#include "common.h"
#include "device.h"
#include "x86.h"

//this is k_thread branch

#define va_to_pa(va) (va + (current + 1) * 0x100000)
#define pa_to_va(pa) (pa - (current + 1) * 0x100000)

extern TSS tss;
extern int displayRow;
extern int displayCol;

extern ProcessTable pcb[MAX_PCB_NUM];
extern int current; // current process
extern FILE file_list[NR_FILES];

#define curThread (pcb[current].currentTid)

void GProtectFaultHandle(struct StackFrame* sf);

void timerHandle(struct StackFrame* sf);

void syscallHandle(struct StackFrame* sf);

void syscallWrite(struct StackFrame* sf);
void syscallPrint(struct StackFrame* sf);
void syscallFork(struct StackFrame* sf);
void syscallExec(struct StackFrame* sf);
void syscallSleep(struct StackFrame* sf);
void syscallExit(struct StackFrame* sf);
void syscallTcreate(struct StackFrame* sf);
void syscallTdestroy(struct StackFrame* sf);

void irqHandle(struct StackFrame* sf) { // pointer sf = esp
    /* Reassign segment register */
    asm volatile("movw %%ax, %%ds" ::"a"(KSEL(SEG_KDATA)));

    /* Save esp to stackTop */
    //为了中断嵌套
    pcb[current].tcb[curThread].stackTop = (uint32_t)sf;

    switch (sf->irq) {
    case -1:
        break;
    case 0xd:
        GProtectFaultHandle(sf);
        break;
    case 0x20:
        timerHandle(sf);
        break;
    case 0x80:
        syscallHandle(sf);
        break;
    default: assert(0);
    }
}

void GProtectFaultHandle(struct StackFrame* sf) {
    assert(0);
    return;
}

void switch_proc(uint32_t firstRunnable) {

    current = firstRunnable;
    pcb[current].timeCount = 0;
    pcb[current].state = STATE_RUNNING;
    putStr("current:");
    putNum(current);
    putChar('\n');
    pcb[current].tcb[curThread].state = STATE_RUNNING;
    uint32_t tmpStackTop = pcb[current].tcb[curThread].stackTop;
    tss.esp0 = (uint32_t) & (pcb[current].tcb[curThread].stackTop);
    asm volatile("movl %0,%%esp" ::"m"(tmpStackTop));
    asm volatile("popl %gs");
    asm volatile("popl %fs");
    asm volatile("popl %es");
    asm volatile("popl %ds");
    asm volatile("popal");
    asm volatile("addl $8,%esp");
    asm volatile("sti");
    asm volatile("iret");
}

void switch_thrd(uint32_t runnableThread) {
    putStr("runnable thread: ");
    putNum(runnableThread);
    putChar('\n');
    pcb[current].currentTid = runnableThread;
    pcb[current].tcb[curThread].timeCount = 0;
    pcb[current].tcb[curThread].state = STATE_RUNNING;
    uint32_t tmpStackTop = pcb[current].tcb[curThread].stackTop;
    tss.esp0 = (uint32_t) & (pcb[current].tcb[curThread].stackTop);
    asm volatile("movl %0,%%esp" ::"m"(tmpStackTop));
    asm volatile("popl %gs");
    asm volatile("popl %fs");
    asm volatile("popl %es");
    asm volatile("popl %ds");
    asm volatile("popal");
    asm volatile("addl $8,%esp");
    asm volatile("sti");
    asm volatile("iret");
}

void timerHandle(struct StackFrame* sf) {
    //TODO 完成进程调度，建议使用时间片轮转，按顺序调度
    disableInterrupt();
    int firstRunnable = -1;
    for (int i = 0; i < MAX_PCB_NUM; i++) {
        if (pcb[i].state == STATE_BLOCKED) {
            pcb[i].sleepTime--;
            if (pcb[i].sleepTime <= 0) {
                pcb[i].sleepTime = 0;
                pcb[i].state = STATE_RUNNABLE;
            }
        }
    }
    int ndead = 0;
    for (; ndead < MAX_TCB_NUM; ndead++) {
        if (pcb[current].tcb[ndead].state == STATE_RUNNABLE || pcb[current].tcb[ndead].state == STATE_RUNNING)
            break;
    }
    if (ndead == MAX_TCB_NUM) {
        putValidPid(current);
        pcb[current].state = STATE_DEAD;
        pcb[current].timeCount = MAX_TIME_COUNT;
    }

    //ensure fairness
    for (int i = (current + 1) % MAX_PCB_NUM; i != current; i = (i + 1) % MAX_PCB_NUM) {
        if (firstRunnable == -1 && i != 0 && pcb[i].state == STATE_RUNNABLE) //IDLE needn't be worried about since it's always RUNNING
        {
            firstRunnable = i;
            break;
        }
    }
    switch (pcb[current].state) {
    case STATE_RUNNING:
        /* code */
        pcb[current].timeCount++;

        if (pcb[current].timeCount >= MAX_TIME_COUNT) {
            //switch to next user runnable
            if (firstRunnable != -1) {
                pcb[current].state = STATE_RUNNABLE;
                pcb[current].timeCount = 0;
                pcb[current].tcb[curThread].state = STATE_RUNNABLE;
                switch_proc(firstRunnable);
            } else {
                pcb[current].timeCount = 0; //keep running
            }
        } else {
            int runnableThread = -1;
            for (int j = (pcb[current].currentTid + 1) % MAX_TCB_NUM; j != pcb[current].currentTid; j = (j + 1) % MAX_TCB_NUM) {
                if (runnableThread == -1 && pcb[current].tcb[j].state == STATE_RUNNABLE) {
                    runnableThread = j;
                    break;
                }
            }
            switch (pcb[current].tcb[curThread].state) {
            case STATE_RUNNING:
                /* code */
                pcb[current].tcb[curThread].timeCount++;
                if (pcb[current].tcb[curThread].timeCount >= MAX_TIME_COUNT / 16) {
                    if (runnableThread != -1) {
                        pcb[current].tcb[curThread].state = STATE_RUNNABLE;
                        pcb[current].tcb[curThread].timeCount = 0;
                        switch_thrd(runnableThread);
                    } else {
                        pcb[current].tcb[curThread].timeCount = 0;
                    }
                }
                break;
            case STATE_DEAD:
            case STATE_RUNNABLE:
                if (runnableThread != -1) {
                    switch_thrd(runnableThread);
                }
                break;
            default:
                putStr("thread must be running or runnable\n");
                assert(0);
                break;
            }
        }
        break;

    case STATE_BLOCKED:
    case STATE_DEAD:
        if (firstRunnable == -1) {
            firstRunnable = 0;
        }
        switch_proc(firstRunnable);
        break;

    case STATE_RUNNABLE:
        putStr("current can\'t be runnable");
        assert(0);
        break;
    default:
        assert(0);
        break;
    }
    enableInterrupt();
}

void syscallHandle(struct StackFrame* sf) {
    switch (sf->eax) { // syscall number
    case 0:
        syscallWrite(sf);
        break; // for SYS_WRITE
    /* Add Fork,Sleep... */
    case 1:
        syscallFork(sf);
        break;
    case 2:
        syscallExec(sf);
        break;
    case 3:
        syscallSleep(sf);
        break;
    case 4:
        syscallExit(sf);
        break;
    case 5:
        syscallTcreate(sf);
        break;
    case 6:
        syscallTdestroy(sf);
        break;
    default: break;
    }
}

void syscallWrite(struct StackFrame* sf) {
    switch (sf->ecx) { // file descriptor
    case 0:
        syscallPrint(sf);
        break; // for STD_OUT
    default: break;
    }
}

void syscallPrint(struct StackFrame* sf) {
    int sel = sf->ds; // segment selector for user data, need further modification
    char* str = (char*)sf->edx;
    int size = sf->ebx;
    int i = 0;
    int pos = 0;
    char character = 0;
    uint16_t data = 0;
    asm volatile("movw %0, %%es" ::"m"(sel));
    for (i = 0; i < size; i++) {
        asm volatile("movb %%es:(%1), %0"
                     : "=r"(character)
                     : "r"(str + i));
        if (character == '\n') {
            displayRow++;
            displayCol = 0;
            if (displayRow == 25) {
                displayRow = 24;
                displayCol = 0;
                scrollScreen();
            }
        } else {
            data = character | (0x0c << 8);
            pos = (80 * displayRow + displayCol) * 2;
            asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
            displayCol++;
            if (displayCol == 80) {
                displayRow++;
                displayCol = 0;
                if (displayRow == 25) {
                    displayRow = 24;
                    displayCol = 0;
                    scrollScreen();
                }
            }
        }
    }
    updateCursor(displayRow, displayCol);
    sf->eax = size;
    return;
}

void memcpy(void* dst, void* src, size_t size) {
    for (uint32_t j = 0; j < size; j++) {
        *(uint8_t*)(dst + j) = *(uint8_t*)(src + j);
    }
}

void syscallFork(struct StackFrame* sf) {
    //TODO 完善它
    //TODO 查找空闲pcb，如果没有就返回-1
    disableInterrupt();
    int i = 0;
    // for (; i < MAX_PCB_NUM; i++) {
    //     if (pcb[i].state == STATE_DEAD)
    //         break;
    // }
    i = getValidPid();

    if (i == MAX_PCB_NUM) {
        pcb[current].tcb[curThread].regs.eax = -1;
        return;
    } else {
        pcb[current].tcb[curThread].regs.eax = i;
    }

    //TODO 拷贝地址空间

    memcpy((void*)(0x100000 * (i + 1)), (void*)(0x100000 * (current + 1)), 0x100000);

    // 拷贝pcb，这部分代码给出了，请注意理解
    memcpy(&pcb[i], &pcb[current], sizeof(ProcessTable));
    for (int j = 0; j < MAX_TCB_NUM; j++) {
        memcpy(&pcb[i].tcb[j], &pcb[current].tcb[j], sizeof(ThreadTable));
        memcpy(&pcb[i].tcb[j].stack, &pcb[current].tcb[j].stack, MAX_STACK_SIZE);
    }
    // current tid maintain the same
    pcb[i].pid = i;
    pcb[i].state = STATE_RUNNABLE;
    pcb[i].tcb[pcb[i].currentTid].regs.eax = 0;
    pcb[i].tcb[pcb[i].currentTid].regs.cs = USEL(1 + i * 2);
    pcb[i].tcb[pcb[i].currentTid].regs.ds = USEL(2 + i * 2);
    pcb[i].tcb[pcb[i].currentTid].regs.es = USEL(2 + i * 2);
    pcb[i].tcb[pcb[i].currentTid].regs.fs = USEL(2 + i * 2);
    pcb[i].tcb[pcb[i].currentTid].regs.gs = USEL(2 + i * 2);
    pcb[i].tcb[pcb[i].currentTid].regs.ss = USEL(2 + i * 2);
    pcb[i].tcb[pcb[i].currentTid].stackTop = (uint32_t) & (pcb[i].tcb[pcb[i].currentTid].regs);
    pcb[i].tcb[pcb[i].currentTid].state = STATE_RUNNABLE;
    pcb[i].tcb[pcb[i].currentTid].timeCount = 0;
    pcb[i].tcb[pcb[i].currentTid].sleepTime = 0;

    enableInterrupt();
}

void syscallTcreate(struct StackFrame* sf) {
    disableInterrupt();
    assert(pcb[current].state == STATE_RUNNING);
    // putStr("current tid: ");
    // putNum(pcb[current].currentTid);
    // putChar('\n');
    //the entry of the function has to be given in sf->ecx
    int i = 0;
    for (; i < MAX_TCB_NUM; i++) {
        if (pcb[current].tcb[i].state == STATE_DEAD) break;
    }
    if (i == MAX_TCB_NUM) {
        sf->eax = -1;
        return;
    }
    // putStr("valid tid: ");
    // putNum(i);
    // putChar('\n');

    memcpy(&pcb[current].tcb[i], &pcb[current].tcb[curThread], sizeof(ThreadTable));
    memcpy(&pcb[current].tcb[i].stack, &pcb[current].tcb[curThread].stack, MAX_STACK_SIZE);
    pcb[current].tcb[i].pid = i;
    pcb[current].tcb[i].state = STATE_RUNNABLE;
    pcb[current].tcb[i].timeCount = 0;
    pcb[current].tcb[i].sleepTime = 0;
    pcb[current].tcb[i].stackTop = (uint32_t) & (pcb[current].tcb[i].regs);
    pcb[current].tcb[i].regs.esp = 0x100000 - i * 0x10000;
    pcb[current].tcb[i].regs.eip = sf->ecx;
    pcb[current].tcb[i].regs.eax = i;
    sf->eax = i;
    enableInterrupt();
}

void syscallTdestroy(struct StackFrame* sf) {
    disableInterrupt();
    putStr("to destroy thread: ");
    putNum(curThread);
    putChar('\n');
    pcb[current].tcb[curThread].state = STATE_DEAD;
    pcb[current].tcb[curThread].timeCount = MAX_TIME_COUNT;
    sf->eax = 0;
    enableInterrupt();
    asm volatile("int $0x20");
}

int strcmp(const char* str1, const char* str2) {
    int ret = 0;
    while (!(ret = *(unsigned char*)str1 - *(unsigned char*)str2) && *str1) {
        str1++;
        str2++;
    }
    if (ret < 0) {
        return -1;
    } else if (ret > 0) {
        return 1;
    }
    return 0;
}

int search_file(char* name) {
    int i = 0;
    while (i < NR_FILES) {
        // putStr(file_list[i].name);
        // putChar('\n');
        if (strcmp(name, file_list[i].name) == 0) break;
        i++;
    }
    return 1;
}

void getUserStr(char* buffer, char* startInKernel) {
    char ch;
    for (int i = 0; i < 32; i++) {
        asm volatile("movb %%es:(%1), %0"
                     : "=r"(ch)
                     : "r"(startInKernel + i));
        buffer[i] = ch;
        if (ch == '\0') break;
    }
}

void syscallExec(struct StackFrame* sf) {
    // TODO 完成exec
    // hint: 用loadelf，已经封装好了
    // disableInterrupt();
    // putStr("exec\n");
    uint32_t entry = 0;
    uint32_t secstart = 0;
    uint32_t secnum = 0;
    int argc = (int)sf->ecx;
    int sel = sf->ds;
    asm volatile("movw %0, %%es" ::"m"(sel));
    char** argv = (char**)sf->edx;
    char buf_name[32];
    char* filename;
    asm volatile("movl %%es:(%1), %0"
                 : "=r"(filename)
                 : "r"(argv));
    getUserStr(buf_name, filename);
    if (argc == 0) {
        sf->eax = -1;
        return;
    }
    int file_idx = search_file(buf_name);
    if (file_idx == NR_FILES) {
        sf->eax = -1;
        return;
    }

    secstart = file_list[file_idx].secstart;
    secnum = file_list[file_idx].secnum;
    pcb[current].state = STATE_RUNNING;
    loadelf(secstart, secnum, (current + 1) * 0x100000, &entry);
    pcb[current].tcb[0].state = STATE_RUNNING;
    for (int j = 1; j < MAX_TCB_NUM; j++)
        pcb[current].tcb[j].state = STATE_DEAD;
    pcb[current].tcb[0].regs.eip = entry;
    pcb[current].tcb[0].stackTop = (uint32_t) & (pcb[current].tcb[0].regs);
    pcb[current].timeCount = pcb[current].tcb[0].timeCount = 0;
    pcb[current].sleepTime = pcb[current].tcb[0].sleepTime = 0;
    pcb[current].pid = current;
    pcb[current].tcb[0].pid = 0;
    pcb[current].tcb[0].regs.ss = USEL(2 + 2 * current);
    pcb[current].tcb[0].regs.esp = 0x100000;
    asm volatile("pushfl");
    asm volatile("popl %0"
                 : "=r"(pcb[current].tcb[0].regs.eflags));
    pcb[current].tcb[0].regs.eflags = pcb[current].tcb[0].regs.eflags | 0x200;
    pcb[current].tcb[0].regs.cs = USEL(1 + 2 * current);
    pcb[current].tcb[0].regs.eip = entry;
    pcb[current].tcb[0].regs.ds = USEL(2 + 2 * current);
    pcb[current].tcb[0].regs.es = USEL(2 + 2 * current);
    pcb[current].tcb[0].regs.fs = USEL(2 + 2 * current);
    pcb[current].tcb[0].regs.gs = USEL(2 + 2 * current);
    pcb[current].tcb[0].regs.eax = 0;
    sf->eax = 0;
}

void syscallSleep(struct StackFrame* sf) {
    //TODO:实现它
    // assert(sf->ecx >= 0);
    disableInterrupt();
    pcb[current].sleepTime = sf->ecx;
    pcb[current].state = STATE_BLOCKED;
    pcb[current].timeCount = MAX_TIME_COUNT;
    sf->eax = 0;
    enableInterrupt();
    asm volatile("int $0x20");
}

void syscallExit(struct StackFrame* sf) {
    //TODO 先设置成dead，然后用int 0x20进入调度
    // putNum(current);
    // putStr(" exit\n");
    disableInterrupt();
    // putStr("CURRENT: ");
    // putNum(current);
    // putChar('\n');
    // for (int i = 0; i < MAX_PCB_NUM; i++) {
    //     if (pcb[i].state == STATE_BLOCKED) {
    //         putStr("BLOCKED: ");
    //         putNum(i);
    //         putChar('\n');
    //     }
    //     if (pcb[i].state == STATE_RUNNABLE) {
    //         putStr("Runnable: ");
    //         putNum(i);
    //         putChar('\n');
    //     }
    //     if (pcb[i].state == STATE_DEAD) {
    //         putStr("DEAD: ");
    //         putNum(i);
    //         putChar('\n');
    //     }
    //     if (pcb[i].state == STATE_RUNNING) {
    //         putStr("RUNNING: ");
    //         putNum(i);
    //         putChar('\n');
    //     }
    // }
    putValidPid(current);
    pcb[current].state = STATE_DEAD;
    for (int i = 0; i < MAX_TCB_NUM; i++) {
        pcb[current].tcb[i].state = STATE_DEAD;
    }
    pcb[current].timeCount = MAX_TIME_COUNT;
    sf->eax = 0;
    enableInterrupt();
    asm volatile("int $0x20");
}
