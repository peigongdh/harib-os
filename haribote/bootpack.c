#include <string.h>
#include <stdio.h>
#include "bootpack.h"

#define KEYCMD_LED	0xed

/* 控制窗口标题栏的颜色接受输入中 */
void keywin_off(struct SHEET* key_win); 
/* 控制窗口标题栏的颜色为接受输入中 */
void keywin_on(struct SHEET* key_win);
/* 新开打一个命令行 */
struct SHEET* open_console(struct SHTCTL* shtctl, unsigned int memtotal);
/* 结束命令行任务 */
void close_constask(struct TASK* task);
/* 关闭命令行图层，图层中包含其任务，同时结束命令行任务 */
void close_console(struct SHEET* sht);

void HariMain(void)
{	
	int i, mx, my;
	int new_mx = -1, new_my = 0, new_wx = 0x7fffffff, new_wy = 0;
	int j, x, y;
	int mmx = -1, mmy = -1, mmx2 = 0;
	char s[40];
	struct SHEET* sht = 0;
	unsigned int memtotal;
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse;
	unsigned char *buf_back, buf_mouse[256];
	struct FIFO32 fifo, keycmd;
	int fifobuf[128], keycmd_buf[32];
	struct TASK* task_a;
	struct TASK* task;
	
	static char keytable0[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0x08,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0x0a,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '\\',   0,   0, 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
	static char keytable1[0x80] = {
		0,   0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0x08,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0x0a,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '|', 0,   0, 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
	};
	struct SHEET* key_win;
	struct SHEET* sht2;
	int key_shift = 0;
	/* ScrollLock, NumLock, CapsLock */
	int key_leds = (binfo->leds >> 4) & 7;
	/* 向键盘控制器发送数据的状态-1表示通常状态，可以发送指令，
	不为-1时表示键盘控制器正在等待发送的数据，此时发送的数据保存在keycmd_wait中 */
	int keycmd_wait = -1;
	
	/* 字体相关 */
	int* fat;
	unsigned char* hzk;
	struct FILEINFO* finfo;
	extern char hankaku[4096];
	 
	init_gdtidt();
	init_pic();
	/* CPU接受来自外部设备的中断 */ 
	io_sti();
	/* 初始化键盘、鼠标、定时器共享的FIFO32队列 */
	fifo32_init(&fifo, 128, fifobuf, 0);
	/* 初始化键盘 */
	init_keyboard(&fifo, 256);
	/* 初始化键盘LED队列 */
	fifo32_init(&keycmd, 32, keycmd_buf, 0);
	/* 初始化鼠标 */
	enable_mouse(&fifo, 512, &mdec);
	/* 初始化定时器 */
	init_pit();
	
	io_out8(PIC0_IMR, 0xf8);
	io_out8(PIC1_IMR, 0xef);
	
	/* 内存管理 */	
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	
	/* 设定调色板 */
	init_palette();
	/* 设定图层 */
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	/* 任务a相关 */
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 0);
	task_a->langmode = 0;
	
	/* sht_back */
	sht_back = sheet_alloc(shtctl);
	buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	/* 初始化操作系统界面 */ 
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	/* sht_cons */
	key_win = open_console(shtctl, memtotal);
		
	/* sht_mouse */
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	/* 初始化鼠标指针 */ 
	init_mouse_cursor8(buf_mouse, 99);
	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
	
	sheet_slide(sht_back, 0, 0);
	sheet_slide(key_win, 32, 4);
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back, 0);
	sheet_updown(key_win, 1);
	sheet_updown(sht_mouse, 2);
	keywin_on(key_win);
	
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);
	
	*((int* )0x0fe4) = (int)shtctl;
	*((int* )0x0fec) = (int)&fifo;
	
	/* 载入字体 */ 
	/* 01 ~ 09: 非汉字 */
	/* 10 ~ 15: 空白 */
	/* 16 ~ 55: 一级汉字 */
	/* 共55 * 94 = 5170 */
	hzk = (unsigned char* )memman_alloc_4k(memman, 16 * 256 + 32 * 5170);
	fat = (int* )memman_alloc_4k(memman, 4 * 2880);
	file_readfat(fat, (unsigned char* )(ADR_DISKIMG + 0x000200));
	finfo = file_search("HZK.fnt", (struct FILEINFO* )(ADR_DISKIMG + 0x002600), 224);
	if (finfo != 0)
	{
		file_loadfile(finfo->clustno, finfo->size, hzk, fat, (char* )(ADR_DISKIMG + 0x003e00));
	}
	/* 没有字库，用英文字库填充，其余部分用oxff填充 */
	else
	{
		for (i = 0; i < 16 * 256; i ++)
		{
			hzk[i] = hankaku[i];
		}
		for (i = 16 * 256; i < 16 * 256 + 32 * 5170; i ++)
		{
			hzk[i] = 0xff;
		}
	}
	*((int* )0x0fe8) = (int)hzk;
	memman_free_4k(memman, (int)fat, 4 * 2880);
	
	for (; ;)
	{
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0)
		{
			/* 若存在向键盘发送的数据，则发送它 */
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();
		if (fifo32_status(&fifo) == 0)
		{
			/* FIFO为空，当存在搁置的绘图操作时立即执行 */
			if (new_mx >= 0)
			{
				io_sti();
				sheet_slide(sht_mouse, new_mx, new_my);
				new_mx = -1;
			}
			else if (new_wx != 0x7fffffff)
			{
				io_sti();
				sheet_slide(sht, new_wx, new_wy);
				new_wx = 0x7fffffff;
			}
			else
			{
				task_sleep(task_a);
				/* 接受中断 */
				io_sti();
			}
		}
		else
		{
			i = fifo32_get(&fifo);
			io_sti();
			if (key_win != 0 && key_win->flags == 0)
			{
				/* 当画面上只剩鼠标和背景时 */
				if (shtctl->top == 1)
				{
					key_win = 0;
				}
				else
				{
					key_win = shtctl->sheets[shtctl->top - 1];
					keywin_on(key_win);
				}
			}
			/* 处理键盘中断 */ 
			if (256 <= i && i <= 511)
			{
				if (i < 256 + 0x54)
				{
					if (key_shift == 0)
					{
						s[0] = keytable0[i - 256];
					}
					else
					{
						s[0] = keytable1[i - 256];
					}
				}
				else
				{
					s[0] = 0;
				}
				if ( s[0] >= 'A' && s[0] <= 'Z')
				{
					if (((key_leds & 4) == 0 && key_shift == 0) ||
						((key_leds & 4) != 0 && key_shift != 0))
						{
							s[0] += 0x20;
						}
				}
				/* 处理一般字符 */
				if (s[0] != 0 && key_win != 0)
				{
					fifo32_put(&key_win->task->fifo, s[0] + 256);
				}
				/* 处理tab键 */
				if (i == 256 + 0x0f && key_win != 0)
				{
					keywin_off(key_win);
					j = key_win->height - 1;
					if (j == 0)
					{
						j = shtctl->top - 1;
					}
					key_win = shtctl->sheets[j];
					keywin_on(key_win);
				}
				if (i == 256 + 0x2a)
				{
					key_shift |= 1;
				}
				/* 右Shift ON */
				if (i == 256 + 0x36)
				{
					key_shift |= 2;
				}
				/* 左Shift OFF */
				if (i == 256 + 0xaa)
				{
					key_shift &= ~1;
				}
				/* 右Shift OFF */
				if (i == 256 + 0xb6)
				{
					key_shift &= ~2;
				}
				/* CapLocks */
				if (i == 256 + 0x3a)
				{
					/* 00000100 */
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				/* NumLocks */
				if (i == 256 + 0x45)
				{
					/* 00000010 */
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				/* ScrollLocks */
				if (i == 256 + 0x46)
				{
					/* 00000001 */
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				/* F11，将窗口置于最上层 */
				if (i == 256 + 0x57 && shtctl->top > 2) 
				{
					sheet_updown(shtctl->sheets[1], shtctl->top - 1);
				}
				/* 强制结束任务按键 */ 
				/* shift + f1 */
				if (i == 256 + 0x3b && key_shift != 0 && key_win != 0)
				{
					task = key_win->task;
					if (task != 0 && task->tss.ss0 != 0)
					{
						cons_putstr0(task->cons, "\nBreak(key) :\n");
						/* 禁止中断，不能在改变寄存器值时切换到其他任务 */
						io_cli();
						task->tss.eax = (int)&(task->tss.esp0);
						task->tss.eip = (int) asm_end_app;
						io_sti();
						/* 为了确实执行结束处理，如果出于休眠状态则唤醒 */
						task_run(task, -1, 0);
					}
				}
				/* 打开一个控制台 */
				/* shift + f2 */
				if (i == 256 + 0x3c && key_shift != 0)
				{
					if (key_win != 0)
					{
						keywin_off(key_win);
					}
					key_win = open_console(shtctl, memtotal);
					sheet_slide(key_win, 32, 4);
					sheet_updown(key_win, shtctl->top);
					/* 自动将输入焦点切换到新打开的命令行窗口 */
					keywin_on(key_win);
				}
				/* 键盘成功接受到数据 */
				if (i == 256 + 0xfa)
				{
					keycmd_wait = -1;
				}
				/* 键盘未成功接受到数据 */
				if (i == 256 + 0xfe)
				{
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
			}
			/* 鼠标缓冲区 */
			else if (512 <= i && i <= 767)
			{
				if (mouse_decode(&mdec, i - 512) != 0)
				{
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0)
					{
						mx = 0;
					}
					if (my < 0)
					{
						my = 0;
					}
					if (mx > binfo->scrnx - 1)
					{
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1)
					{
						my = binfo->scrny - 1;
					}
					new_mx = mx;
					new_my = my;
					/* 按下左键 */ 
					if ((mdec.btn & 0x01) != 0) 
					{
						if (mmx < 0) 
						{
							/* 按照从上到下的顺序寻找鼠标所指向的图层 */
							for (j = shtctl->top - 1; j > 0; j --)
							{
								sht = shtctl->sheets[j];
								x = mx - sht->vx0;
								y = my - sht->vy0;
								if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize)
								{
									if (sht->buf[y * sht->bxsize + x] != sht->col_inv)
									{
										sheet_updown(sht, shtctl->top - 1);
										/* 当鼠标点击标题栏时，窗口就会被切换到画面的最上方 */
										/* 并且键盘输入也会自动切换到该窗口 */
										if (sht!= key_win)
										{
											keywin_off(key_win);
											key_win = sht;
											keywin_on(key_win);
										}
										/* 如果鼠标在标题栏位置则进入窗口移动模式 */
										if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21)
										{
											mmx = mx;
											mmy = my;
											mmx2 = sht->vx0;
											new_wy = sht->vy0;
										}
										/* 当鼠标在"X"位置时关闭窗口 */
										if (sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 5 <= y && y < 19)
										{
											/* 由应用程序生成的窗口则关闭 */
											if ((sht->flags & 0x10) != 0)
											{
												task = sht->task;
												cons_putstr0(task->cons, "\nBreak(mouse) :\n");
												io_cli();
												task->tss.eax = (int)&(task->tss.esp0);
												task->tss.eip = (int)asm_end_app;
												io_sti();
												task_run(task, -1, 0);
											}
											/* 否则关闭命令行窗口 */
											else
											{
												task = sht->task;
												/* 暂且隐藏该图层 */
												sheet_updown(sht, -1);
												keywin_off(key_win);
												key_win = shtctl->sheets[shtctl->top - 1];
												keywin_on(key_win);
												io_cli();
												fifo32_put(&task->fifo, 4);
												io_sti();
											}
										}
										break;
									}
								}
							}
						}
						/* 窗口移动模式 */
						else
						{	
							/* 计算鼠标的移动距离，更新new_wx, new_wy */
							x = mx - mmx;
							y = my - mmy;
							new_wx = (mmx2 + x + 2) & ~3;
							new_wy = new_wy + y;
							/* 更新为移动后的坐标 */
							mmy = my;
						}
					}
					/* 没有按下左键，返回通常模式 */
					else
					{
						mmx = -1;
						if (new_wx != 0x7fffffff)
						{
							sheet_slide(sht, new_wx, new_wy);
							new_wx = 0x7fffffff;
						}
					}
				}
			}
			else if (i >= 768 && i <= 1023)
			{
				close_console(shtctl->sheets0 + (i - 768));
			}
			else if (i >= 1024 && i <= 2023)
			{
				close_constask(taskctl->tasks0 + (i - 1024));
			}
			/* 只关闭命令行窗口 */
			else if (i >= 2024 && i <= 2279)
			{
				sht2 = shtctl->sheets0 + (i - 2024);
				memman_free_4k(memman, (int)sht2->buf, 256 * 165);
				sheet_free(sht2);
			}
		}
	}
}

/* 控制窗口标题栏的颜色接受输入中 */
void keywin_off(struct SHEET* key_win) 
{
	change_wtitle8(key_win, 0);
	if ((key_win->flags & 0x20) != 0)
	{
		fifo32_put(&key_win->task->fifo, 3);
	}
	return; 
}

/* 控制窗口标题栏的颜色为接受输入中 */
void keywin_on(struct SHEET* key_win)
{
	change_wtitle8(key_win, 1);
	if ((key_win->flags & 0x20) != 0)
	{
		fifo32_put(&key_win->task->fifo, 2);
	}
	return;
}

/* 打开一个任务 */
struct TASK* open_constask(struct SHEET* sht, unsigned int memtotal)
{
	struct MEMMAN* memman = (struct MEMMAN* )MEMMAN_ADDR;
	struct TASK* task = task_alloc();
	int* cons_fifo = (int* )memman_alloc_4k(memman, 128 * 4);
	task->cons_stack = memman_alloc_4k(memman, 64 * 1024);
	task->tss.esp = task->cons_stack + 64 * 1024 - 12;
	task->tss.eip = (int)&console_task;
	task->tss.es = 1 * 8;
	task->tss.cs = 2 * 8;
	task->tss.ss = 1 * 8;
	task->tss.ds = 1 * 8;
	task->tss.fs = 1 * 8;
	task->tss.gs = 1 * 8;
	*((int* )(task->tss.esp + 4)) = (int)sht;
	*((int* )(task->tss.esp + 8)) = memtotal;
	/* level = 2, priority = 2 */
	task_run(task, 2, 2);
	fifo32_init(&task->fifo, 128, cons_fifo, task);
	return task;
}

/* 新开打一个命令行 */
struct SHEET* open_console(struct SHTCTL* shtctl, unsigned int memtotal)
{
	struct MEMMAN* memman = (struct MEMMAN* )MEMMAN_ADDR;
	struct SHEET* sht = sheet_alloc(shtctl);
	unsigned char* buf = (unsigned char* )memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht, buf, 256, 165, -1);
	make_window8(buf, 256, 165, "console", 0);
	make_textbox8(sht, 8, 28, 240, 128, COL8_000000);
	sht->task = open_constask(sht, memtotal);
	/* 有光标 */
	sht->flags |= 0x20;
	return sht;
}

/* 结束命令行任务 */
void close_constask(struct TASK* task)
{
	struct MEMMAN* memman = (struct MEMMAN* )MEMMAN_ADDR;
	task_sleep(task);
	memman_free_4k(memman, task->cons_stack, 64 * 1024);
	memman_free_4k(memman, (int)task->fifo.buf, 128 * 4);
	task->flags = 0;
	return;
}

/* 关闭命令行图层，图层中包含其任务，同时结束命令行任务 */
void close_console(struct SHEET* sht)
{
	struct MEMMAN* memman = (struct MEMMAN* )MEMMAN_ADDR;
	struct TASK* task = sht->task;
	memman_free_4k(memman, (int)sht->buf, 256 * 165);
	sheet_free(sht);
	close_constask(task);
	return;
}
