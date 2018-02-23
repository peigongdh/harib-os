/* 鼠标初始化操作 */ 
/* 鼠标输入进入FIFO32缓冲区，其值+512，在512~767 */ 

#include "bootpack.h"

struct FIFO32* mousefifo;
int mousedata0;

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

/* 激活鼠标 */
void enable_mouse(struct FIFO32* fifo, int data0, struct MOUSE_DEC *mdec)
{
	/* 将FIFO缓冲区出的信息保存到全局变量里 */
	mousefifo = fifo;
	mousedata0 = data0;
	/* 鼠标有效 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	/* 成功键盘控制器会返回ACK(0xfa) */
	/* 等待oxfa阶段 */
	mdec->phase = 0;
	return;	
}

/* 根据phase的值依次获取鼠标的3个字节 */
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0)
	{
		/* 等待鼠标的0xfa的状态 */
		if (dat == 0xfa)
		{
			mdec->phase = 1;
		}
		return 0;
	}
	else if (mdec->phase == 1)
	{
		/* 等待鼠标的第一字节 */
		if ((dat & 0xc8) == 0x08)
		{
			/* 检查第一个字节的正确性 */
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}	
		return 0;
	}
	else if (mdec->phase == 2)
	{
		/* 等待鼠标的第二字节 */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	else if (mdec->phase == 3)
	{
		/* 等待鼠标的第三字节 */
		mdec->buf[2] = dat;
		mdec->phase = 1;
		
		/* 将鼠标的接受数据转换为坐标 */
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
		/* 鼠标的y方向与画面符号相反 */
		mdec->y = -mdec->y;
		return 1;
	}
	/* 返回一个错误标志，一般不会发生 */
	return -1;
}

/* 来自PS/2鼠标的2c号中断 */
void inthandler2c(int *esp)
{
	int data;
	io_out8(PIC1_OCW2, 0x64);	/* 通知PIC1 IRQ-12的受理已经完成 */
	io_out8(PIC0_OCW2, 0x62);	/* 通知PIC1 IRQ-02的受理已经完成 */
	data = io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo, data + mousedata0);
	return;
}
