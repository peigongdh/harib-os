#include "bootpack.h"
#include <stdio.h>

/* PIC-(programmable interrupt controller)可编程中断控制器的初始化 */ 
/* PIC的寄存器均为8位寄存器 */ 

/* IMR: "interrupt mask register" 中断屏蔽寄存器，8位分别对应8路IRQ信号 */
/* 如果某一位的值是1，则该位所产生对应的IRQ信号被屏蔽，避免中断产生混乱 */

/* ICW: "initial control word" 初始化控制数据 */
/* ICW1和ICW4与PIC主板配线方式、中断信号的电气特性有关 */
/* ICW3是有关主-从连接的设定，对主PIC而言，第几号IRQ与从PIC相连，该位即设为1 */ 
/* 目前硬件标准是第3号与从PIC相连，即设定为00000100 */ 

/* ICW2决定IRQ通知CPU的中断号 */

void init_pic(void)
{
	/* IMR-(interrupt mask register)中断屏蔽寄存器 */
	io_out8(PIC0_IMR, 0xff);	/* 禁止所有中断 */
	io_out8(PIC1_IMR, 0xff);	/* 禁止所有中断 */
	
	/* ICW-(initial control word)初始化控制数据 */
	io_out8(PIC0_ICW1, 0x11);	/* 边沿触发模式（edge trigger mdoe） */
	io_out8(PIC0_ICW2, 0x20);	/* IRQ0-7由INT20-27接受 */
	io_out8(PIC0_ICW3, 1 << 2);	/* PIC1由PIC2连接 */
	io_out8(PIC0_ICW4, 0x01);	/* 无缓冲区模式 */
	
	io_out8(PIC1_ICW1, 0x11);	/* 边沿触发模式（edge trigger mdoe） */
	io_out8(PIC1_ICW2, 0x28);	/* IRQ8-15由INT28-2f接受 */
	io_out8(PIC1_ICW3, 2);		/* PIC1由PIC2连接 */
	io_out8(PIC1_ICW4, 0x01);	/* 无缓冲区模式 */

	io_out8(PIC0_IMR, 0xfb);	/* 11111011 PIC1以外全部禁止 */
	io_out8(PIC1_IMR, 0xff);	/* 11111111 禁止所有中断 */
	
	return;
}

/* 特定的27号中断 */
void inthandler27(int *esp)
{
	io_out8(PIC0_OCW2, 0x67);
	return;
}
