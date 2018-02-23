/* 初始化队列 */
/* FIFO8存储数据类型为char */
/* 改进后的FIFO32存储数据类型为int */

#include "bootpack.h"

#define FLAGS_OVERRUN	0x0001

/* 初始化FIFO32缓冲区 */
void fifo32_init(struct FIFO32 *fifo, int size, int* buf, struct TASK* task)
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size;
	fifo->flags = 0;
	fifo->p = 0;	/* 下一个数据写入位置 */
	fifo->q = 0;	/* 下一个数据读出位置 */
	fifo->task = task;
	return;
}

/* 向FIFO32传送数据并保存，返回-1表示溢出，0表示没有溢出 */
int fifo32_put(struct FIFO32 *fifo, int data)
{
	if (fifo->free == 0)
	{
		/* 溢出处理 */
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p ++;
	if (fifo->p == fifo->size)
	{
		fifo->p = 0;
	}
	fifo->free --;
	if (fifo->task != 0)
	{
		/* 若任务处于休眠状态，则将任务唤醒 */
		/* #define TASK_FLAGS_RUNNING	2 */
		if (fifo->task->flags != 2)
		{
			/* 唤醒任务 */
			task_run(fifo->task, -1, 0);
		}
	}
	return 0;
}

/* 从FIFO32取得一个数据，若缓存区为空则返回-1 */
int fifo32_get(struct FIFO32 *fifo)
{
	int data;
	if (fifo->free == fifo->size)
	{
		/* 若缓存区为空则返回-1 */
		return -1;
	}
	data = fifo->buf[fifo->q] ;
	fifo->q ++;
	if (fifo->q == fifo->size)
	{
		fifo->q = 0;
	}
	fifo->free ++;
	return data;
}

/* 报告FIFO32缓存区积攒数据量 */
int fifo32_status(struct FIFO32* fifo)
{
	return fifo->size - fifo->free;
}

