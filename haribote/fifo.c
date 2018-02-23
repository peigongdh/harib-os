/* ��ʼ������ */
/* FIFO8�洢��������Ϊchar */
/* �Ľ����FIFO32�洢��������Ϊint */

#include "bootpack.h"

#define FLAGS_OVERRUN	0x0001

/* ��ʼ��FIFO32������ */
void fifo32_init(struct FIFO32 *fifo, int size, int* buf, struct TASK* task)
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size;
	fifo->flags = 0;
	fifo->p = 0;	/* ��һ������д��λ�� */
	fifo->q = 0;	/* ��һ�����ݶ���λ�� */
	fifo->task = task;
	return;
}

/* ��FIFO32�������ݲ����棬����-1��ʾ�����0��ʾû����� */
int fifo32_put(struct FIFO32 *fifo, int data)
{
	if (fifo->free == 0)
	{
		/* ������� */
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
		/* ������������״̬���������� */
		/* #define TASK_FLAGS_RUNNING	2 */
		if (fifo->task->flags != 2)
		{
			/* �������� */
			task_run(fifo->task, -1, 0);
		}
	}
	return 0;
}

/* ��FIFO32ȡ��һ�����ݣ���������Ϊ���򷵻�-1 */
int fifo32_get(struct FIFO32 *fifo)
{
	int data;
	if (fifo->free == fifo->size)
	{
		/* ��������Ϊ���򷵻�-1 */
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

/* ����FIFO32���������������� */
int fifo32_status(struct FIFO32* fifo)
{
	return fifo->size - fifo->free;
}
