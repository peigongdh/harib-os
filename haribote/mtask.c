/* 多任务 */

#include "bootpack.h"

struct TASKCTL* taskctl;
struct TIMER* task_timer;

/* 空闲状态 */
#define TASK_FLAGS_FREE		0
/* 已配置状态 */
#define TASK_FLAGS_SLEEP	1
/* 定时器运行中 */
#define TASK_FLAGS_RUNNING	2

/* 向struct TASKLEVEL中添加一个任务 */
void task_add(struct TASK* task)
{
	struct TASKLEVEL *tl = &taskctl->level[task->level];
	tl->tasks[tl->running] = task;
	tl->running ++;
	task->flags = TASK_FLAGS_RUNNING;
	return;
}

/* 从struct TASKLEVEL中删除一个任务 */
void task_remove(struct TASK* task)
{
	int i;
	struct TASKLEVEL* tl = &taskctl->level[task->level];
	
	/* 寻找task所在的位置 */
	for (i = 0; i < tl->running; i ++)
	{
		if (tl->tasks[i] == task)
		{
			break;
		}
	}
	
	tl->running --;
	if (i < tl->now)
	{
		tl->now --;
	}
	if (tl->now >= tl->running)
	{
		tl->now = 0;
	}
	task->flags = 1;
	
	/* 移动 */
	for (; i < tl->running; i ++)
	{
		tl->tasks[i] = tl->tasks[i + 1];
	}
	return;
}

/* 切换任务切换到哪个LEVEL */
void task_switchsub(void)
{
	int i;
	/* 寻找最上层的LEVEL */
	for (i = 0; i < MAX_TASKLEVELS; i ++)
	{
		if (taskctl->level[i].running > 0)
		{
			break;
		}
	}
	taskctl->now_lv = i;
	taskctl->lv_change = 0;
	return;
}

/* 闲置任务 */
/* 创建哨兵任务，并把它放在最下层LEVEL中 */
void task_idle(void)
{
	for (; ;)
	{
		io_hlt();
	}
}

/* 任务管理器初始化，返回任务管理器内存地址 */
struct TASK* task_init(struct MEMMAN* memman)
{
	int i;
	struct TASK* task;
	struct TASK* idle;
	struct SEGMENT_DESCRIPTOR* gdt = (struct SEGMENT_DESCRIPTOR* )ADR_GDT;
	taskctl = (struct TASKCTL* )memman_alloc_4k(memman, sizeof (struct TASKCTL));
	for (i = 0; i < MAX_TASKS; i ++)
	{
		taskctl->tasks0[i].flags = TASK_FLAGS_FREE;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		taskctl->tasks0[i].tss.ldtr = (TASK_GDT0 + MAX_TASKS + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int)&taskctl->tasks0[i].tss, AR_TSS32);
		set_segmdesc(gdt + TASK_GDT0 + MAX_TASKS + i, 15, (int)taskctl->tasks0[i].ldt, AR_LDT);
	}
	for (i = 0; i < MAX_TASKLEVELS; i ++)
	{
		taskctl->level[i].running = 0;
		taskctl->level[i].now = 0;
	}
	/* 将任务管理器添加到任务列表中 */
	task = task_alloc();
	task->flags = TASK_FLAGS_RUNNING;
	/* 0.02秒运行时间 */
	task->priority = 2;
	/* 最高LEVEL */
	task->level = 0;
	task_add(task);
	task_switchsub() ;	
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, task->priority);
	
	/* 添加空闲的哨兵任务 */
	idle = task_alloc();
	idle->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
	idle->tss.eip = (int) &task_idle;
	idle->tss.es = 1 * 8;
	idle->tss.cs = 2 * 8;
	idle->tss.ss = 1 * 8;
	idle->tss.ds = 1 * 8;
	idle->tss.fs = 1 * 8;
	idle->tss.gs = 1 * 8;
	task_run(idle, MAX_TASKLEVELS - 1, 1);
	
	return task;
}

/* 任务申请，返回0表示任务上限已满 */
struct TASK* task_alloc(void)
{
	int i;
	struct TASK* task;
	for (i = 0; i < MAX_TASKS; i ++)
	{
		if (taskctl->tasks0[i].flags == TASK_FLAGS_FREE)
		{
			task = &taskctl->tasks0[i];
			task->flags = TASK_FLAGS_SLEEP;
			task->tss.eflags = 0x00000202;
			task->tss.eax = 0;
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.iomap = 0x40000000;
			/* 应用程序没有运行时，该值为0 */
			task->tss.ss0 = 0;
			return task;
		}
	}
	/* 任务上限已满 */
	return 0;
}

/* 任务运行 */
void task_run(struct TASK* task, int level, int priority)
{
	if (level < 0)
	{
		/* 不改变LEVEL */
		level = task->level;
	}
	/* 只有在priority>0时改变其优先级 */
	if (priority > 0)
	{
		task->priority = priority;
	}
	
	if(task->flags == TASK_FLAGS_RUNNING && task->level != level)
	{
		task_remove(task);
	}
	if (task->flags != TASK_FLAGS_RUNNING)
	{
		task->level = level;
		task_add(task);
	}
	taskctl->lv_change = 1;
	return;
}

/* 对当前任务列表中的任务进行切换 */
void task_switch(void)
{
	struct TASKLEVEL* tl = &taskctl->level[taskctl->now_lv];
	struct TASK* new_task;
	struct TASK* now_task = tl->tasks[tl->now];
	tl->now ++;
	if (tl->now == tl->running)
	{
		tl->now = 0;
	}
	if (taskctl->lv_change != 0) 
	{
		task_switchsub();
		tl = &taskctl->level[taskctl->now_lv];
	}
	new_task = tl->tasks[tl->now];
	timer_settime(task_timer, new_task->priority);
	if (new_task != now_task)
	{
		farjmp(0, new_task->sel);
	}
	return;
}

/* 任务休眠 */
void task_sleep(struct TASK* task)
{
	struct TASK* now_task;
	if (task->flags == TASK_FLAGS_RUNNING)
	{
		now_task = task_now();
		task_remove(task);
		if (task == now_task)
		{
			/* 如果是让自己休眠，则需要进行任务切换 */
			task_switchsub();
			/* 在设定后获取当前任务的值 */
			now_task = task_now();
			farjmp(0, now_task->sel);
		}
	}
	return;
}

/* 返回当前活动着的任务指针 */
struct TASK* task_now(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	return tl->tasks[tl->now];
}
