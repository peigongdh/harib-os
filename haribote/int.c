#include "bootpack.h"
#include <stdio.h>

/* PIC-(programmable interrupt controller)�ɱ���жϿ������ĳ�ʼ�� */ 
/* PIC�ļĴ�����Ϊ8λ�Ĵ��� */ 

/* IMR: "interrupt mask register" �ж����μĴ�����8λ�ֱ��Ӧ8·IRQ�ź� */
/* ���ĳһλ��ֵ��1�����λ��������Ӧ��IRQ�źű����Σ������жϲ������� */

/* ICW: "initial control word" ��ʼ���������� */
/* ICW1��ICW4��PIC�������߷�ʽ���ж��źŵĵ��������й� */
/* ICW3���й���-�����ӵ��趨������PIC���ԣ��ڼ���IRQ���PIC��������λ����Ϊ1 */ 
/* ĿǰӲ����׼�ǵ�3�����PIC���������趨Ϊ00000100 */ 

/* ICW2����IRQ֪ͨCPU���жϺ� */

void init_pic(void)
{
	/* IMR-(interrupt mask register)�ж����μĴ��� */
	io_out8(PIC0_IMR, 0xff);	/* ��ֹ�����ж� */
	io_out8(PIC1_IMR, 0xff);	/* ��ֹ�����ж� */
	
	/* ICW-(initial control word)��ʼ���������� */
	io_out8(PIC0_ICW1, 0x11);	/* ���ش���ģʽ��edge trigger mdoe�� */
	io_out8(PIC0_ICW2, 0x20);	/* IRQ0-7��INT20-27���� */
	io_out8(PIC0_ICW3, 1 << 2);	/* PIC1��PIC2���� */
	io_out8(PIC0_ICW4, 0x01);	/* �޻�����ģʽ */
	
	io_out8(PIC1_ICW1, 0x11);	/* ���ش���ģʽ��edge trigger mdoe�� */
	io_out8(PIC1_ICW2, 0x28);	/* IRQ8-15��INT28-2f���� */
	io_out8(PIC1_ICW3, 2);		/* PIC1��PIC2���� */
	io_out8(PIC1_ICW4, 0x01);	/* �޻�����ģʽ */

	io_out8(PIC0_IMR, 0xfb);	/* 11111011 PIC1����ȫ����ֹ */
	io_out8(PIC1_IMR, 0xff);	/* 11111111 ��ֹ�����ж� */
	
	return;
}

/* �ض���27���ж� */
void inthandler27(int *esp)
{
	io_out8(PIC0_OCW2, 0x67);
	return;
}
