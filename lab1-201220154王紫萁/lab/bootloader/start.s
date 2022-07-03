# start.s

/* 在保护模式下加载app */
.code16

.global start
start:
	# 关闭中断
	cli 
	
	# 开启A20地址线
	pushw %ax
	movb $0x2,%al
	outb %al,$0x92
	popw %ax
	
	# 加载GDTR
	data32 addr32 lgdt gdtDesc 

	# TODO: 把cr0的最低位设置为1

	movl %cr0, %eax
	orl $0x1, %eax
	movl %eax, %cr0

	# 长跳转切换到保护模式
	data32 ljmp $0x08, $start32 

.code32
start32:
	movw $0x10, %ax # setting data segment selector
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %ss
	movw $0x18, %ax # setting graphics data segment selector
	movw %ax, %gs
	movl $0x8000, %eax # setting esp
	movl %eax, %esp

	# TODO：跳转到bootMain
	call bootMain

.p2align 2
gdt: 
	# GDT 在这里定义 
	# .word limit[15:0],base[15:0]
	# .byte base[23:16],(0x90|(type)),(0xc0|(limit[19:16])),base[31:24]

	# 第一个描述符是NULL
	.word 0,0
	.byte 0,0,0,0

	# TODO：代码段描述符，对应cs
	.word 0xffff, 0
	.byte 0, (0x90 | (0xA)),	(0xC0 |  (0xf)), 0

	# TODO：数据段描述符，对应ds
	.word 0xffff, 0
	.byte 0, (0x90 | (0x2)),	(0xC0 |  (0xf)), 0

	# TODO：图像段描述符，对应gs base:  1011 1(0xb8000)
	.word 0xffff, 0x8000
	.byte 0x0b, (0x90 | (0x0)),	(0xC0 |  (0xf)), 0


gdtDesc: 
	# gdtDesc definition here
	.word (gdtDesc - gdt -1)
	.long gdt

