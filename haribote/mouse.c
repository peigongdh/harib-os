/* ����ʼ������ */ 
/* ����������FIFO32����������ֵ+512����512~767 */ 

#include "bootpack.h"

struct FIFO32* mousefifo;
int mousedata0;

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

/* ������� */
void enable_mouse(struct FIFO32* fifo, int data0, struct MOUSE_DEC *mdec)
{
	/* ��FIFO������������Ϣ���浽ȫ�ֱ����� */
	mousefifo = fifo;
	mousedata0 = data0;
	/* �����Ч */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	/* �ɹ����̿������᷵��ACK(0xfa) */
	/* �ȴ�oxfa�׶� */
	mdec->phase = 0;
	return;	
}

/* ����phase��ֵ���λ�ȡ����3���ֽ� */
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0)
	{
		/* �ȴ�����0xfa��״̬ */
		if (dat == 0xfa)
		{
			mdec->phase = 1;
		}
		return 0;
	}
	else if (mdec->phase == 1)
	{
		/* �ȴ����ĵ�һ�ֽ� */
		if ((dat & 0xc8) == 0x08)
		{
			/* ����һ���ֽڵ���ȷ�� */
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}	
		return 0;
	}
	else if (mdec->phase == 2)
	{
		/* �ȴ����ĵڶ��ֽ� */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	else if (mdec->phase == 3)
	{
		/* �ȴ����ĵ����ֽ� */
		mdec->buf[2] = dat;
		mdec->phase = 1;
		
		/* �����Ľ�������ת��Ϊ���� */
		mdec->btn = mdec->buf[0] & 0x07;
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		if ((mdec->buf[0] & 0x10) != 0)
		{
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0)
		{
			mdec->y |= 0xffffff00;
		}
		/* ����y�����뻭������෴ */
		mdec->y = -mdec->y;
		return 1;
	}
	/* ����һ�������־��һ�㲻�ᷢ�� */
	return -1;
}

/* ����PS/2����2c���ж� */
void inthandler2c(int *esp)
{
	int data;
	io_out8(PIC1_OCW2, 0x64);	/* ֪ͨPIC1 IRQ-12�������Ѿ���� */
	io_out8(PIC0_OCW2, 0x62);	/* ֪ͨPIC1 IRQ-02�������Ѿ���� */
	data = io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo, data + mousedata0);
	return;
}
