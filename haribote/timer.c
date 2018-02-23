/*定时器*/

#include "bootpack.h"

#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

/* 空闲状态 */
#define TIMER_FLAGS_FREE	0
/* 已配置状态 */
#define TIMER_FLAGS_ALLOC	1
/* 定时器运行中 */
#define TIMER_FLAGS_USING	2

struct TIMERCTL timerctl;

/* 初始化定时器 */ 
void init_pit(void)
{
	int i;
	struct TIMER* t;
	/* 设置中断产生的频率为100Hz， 即每10ms发生一次中断 */
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	for (i = 0; i < MAX_TIMER; i ++)
	{
		/* 未使用 */
		timerctl.timers0[i].flags = TIMER_FLAGS_FREE;
	}
	t = timer_alloc();
	t->timeout = 0xffffffff;
	t->flags = TIMER_FLAGS_USING;
	t->next = 0;
	timerctl.t0 = t;
	timerctl.next = 0xffffffff;
	return;
}

/* 申请定时器，返回一个定时器指针类型，0表示没有找到 */
struct TIMER* timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i ++)
	{
		if (timerctl.timers0[i].flags == 0)
		{
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			timerctl.timers0[i].flags2 = 0;
			return &timerctl.timers0[i];
		}	
	}
	return 0;
}

/* 释放定时器 */
void timer_free(struct TIMER* timer)
{
	timer->flags = TIMER_FLAGS_FREE;
	return;
}

/* 定时器初始化 */
void timer_init(struct TIMER* timer, struct FIFO32* fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

/* 设定定时器时间 */
void timer_settime(struct TIMER* timer, unsigned int timeout)
{
	int e;
	struct TIMER* t;
	struct TIMER* s;
	/* 从现在开始多少秒后停止 */
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	
	t = timerctl.t0;
	if (timer->timeout <= t->timeout)
	{
		/* 插入最前面的情况下 */
		timerctl.t0 = timer;
		timer->next = t;
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	for (; ;)
	{
		s = t;
		t = t->next;
		if (timer->timeout <= t->timeout)
		{
			s->next = timer;
			timer->next = t;
			io_store_eflags(e);
			return;
		}
	}
}

/* 取消指定定时器 */
int timer_cancel(struct TIMER* timer)
{
	int e;
	struct TIMER* t;
	e = io_load_eflags();
	io_cli();
	if (timer->flags == TIMER_FLAGS_USING)
	{
		if (timer == timerctl.t0)
		{
			/* 第一个定时器取消处理 */
			t = timer->next;
			timerctl.t0 = t;
			timerctl.next = t->timeout;
		}
		else
		{
			t = timerctl.t0;
			for (; ;)
			{
				if (t->next == timer)
				{
					break;
				}
				t = t->next;
			}
			t->next = timer->next;
		}
		timer->flags = TIMER_FLAGS_ALLOC;
		io_store_eflags(e);
		/* 取消处理成功 */
		return 1;
	}
	io_store_eflags(e);
	/* 不需要取消处理 */
	return 0;
}

/* 取消flags2标识不为2的所有定时器 */
void timer_cancelall(struct FIFO32* fifo)
{
	int e;
	int i;
	struct TIMER* t;
	e = io_load_eflags();
	io_cli();
	for (i = 0; i < MAX_TIMER; i ++)
	{
		t = &timerctl.timers0[i];
		if (t->flags != 0 && t->flags2 != 0 && t->fifo == fifo)
		{
			timer_cancel(t);
			timer_free(t);
		}
	}
	io_store_eflags(e);
	return;
}

/* 定时器中断处理 */
void inthandler20(int* esp) 
{
	struct TIMER* timer;
	char ts = 0;
	
	/* 把IRP-00信号接受完了的信息通知给PIC */
	io_out8(PIC0_OCW2, 0x60);
	timerctl.count ++;
	if (timerctl.next > timerctl.count)
	{
		return;
	}
	timer = timerctl.t0;
	for (; ;)
	{
		if (timer->timeout > timerctl.count)
		{
			break;
		}
		/* 超时处理 */ 
		timer->flags = TIMER_FLAGS_ALLOC;
		if (timer != task_timer)
		{
			fifo32_put(timer->fifo, timer->data);
		}
		else
		{
			ts = 1;
		}
		timer = timer->next;
	}
	
	/* 清除到期的定时器 */
	timerctl.t0 = timer;
	timerctl.next = timerctl.t0->timeout;
	if (ts != 0)
	{
		task_switch();
	}
	return;
}

