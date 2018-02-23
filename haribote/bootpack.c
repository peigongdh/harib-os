#include <string.h>
#include <stdio.h>
#include "bootpack.h"

#define KEYCMD_LED	0xed

/* ���ƴ��ڱ���������ɫ���������� */
void keywin_off(struct SHEET* key_win); 
/* ���ƴ��ڱ���������ɫΪ���������� */
void keywin_on(struct SHEET* key_win);
/* �¿���һ�������� */
struct SHEET* open_console(struct SHTCTL* shtctl, unsigned int memtotal);
/* �������������� */
void close_constask(struct TASK* task);
/* �ر�������ͼ�㣬ͼ���а���������ͬʱ�������������� */
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
	/* ����̿������������ݵ�״̬-1��ʾͨ��״̬�����Է���ָ�
	��Ϊ-1ʱ��ʾ���̿��������ڵȴ����͵����ݣ���ʱ���͵����ݱ�����keycmd_wait�� */
	int keycmd_wait = -1;
	
	/* ������� */
	int* fat;
	unsigned char* hzk;
	struct FILEINFO* finfo;
	extern char hankaku[4096];
	 
	init_gdtidt();
	init_pic();
	/* CPU���������ⲿ�豸���ж� */ 
	io_sti();
	/* ��ʼ�����̡���ꡢ��ʱ�������FIFO32���� */
	fifo32_init(&fifo, 128, fifobuf, 0);
	/* ��ʼ������ */
	init_keyboard(&fifo, 256);
	/* ��ʼ������LED���� */
	fifo32_init(&keycmd, 32, keycmd_buf, 0);
	/* ��ʼ����� */
	enable_mouse(&fifo, 512, &mdec);
	/* ��ʼ����ʱ�� */
	init_pit();
	
	io_out8(PIC0_IMR, 0xf8);
	io_out8(PIC1_IMR, 0xef);
	
	/* �ڴ���� */	
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	
	/* �趨��ɫ�� */
	init_palette();
	/* �趨ͼ�� */
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	/* ����a��� */
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 0);
	task_a->langmode = 0;
	
	/* sht_back */
	sht_back = sheet_alloc(shtctl);
	buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	/* ��ʼ������ϵͳ���� */ 
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	/* sht_cons */
	key_win = open_console(shtctl, memtotal);
		
	/* sht_mouse */
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	/* ��ʼ�����ָ�� */ 
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
	
	/* �������� */ 
	/* 01 ~ 09: �Ǻ��� */
	/* 10 ~ 15: �հ� */
	/* 16 ~ 55: һ������ */
	/* ��55 * 94 = 5170 */
	hzk = (unsigned char* )memman_alloc_4k(memman, 16 * 256 + 32 * 5170);
	fat = (int* )memman_alloc_4k(memman, 4 * 2880);
	file_readfat(fat, (unsigned char* )(ADR_DISKIMG + 0x000200));
	finfo = file_search("HZK.fnt", (struct FILEINFO* )(ADR_DISKIMG + 0x002600), 224);
	if (finfo != 0)
	{
		file_loadfile(finfo->clustno, finfo->size, hzk, fat, (char* )(ADR_DISKIMG + 0x003e00));
	}
	/* û���ֿ⣬��Ӣ���ֿ���䣬���ಿ����oxff��� */
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
			/* ����������̷��͵����ݣ������� */
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();
		if (fifo32_status(&fifo) == 0)
		{
			/* FIFOΪ�գ������ڸ��õĻ�ͼ����ʱ����ִ�� */
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
				/* �����ж� */
				io_sti();
			}
		}
		else
		{
			i = fifo32_get(&fifo);
			io_sti();
			if (key_win != 0 && key_win->flags == 0)
			{
				/* ��������ֻʣ���ͱ���ʱ */
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
			/* ��������ж� */ 
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
				/* ����һ���ַ� */
				if (s[0] != 0 && key_win != 0)
				{
					fifo32_put(&key_win->task->fifo, s[0] + 256);
				}
				/* ����tab�� */
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
				/* ��Shift ON */
				if (i == 256 + 0x36)
				{
					key_shift |= 2;
				}
				/* ��Shift OFF */
				if (i == 256 + 0xaa)
				{
					key_shift &= ~1;
				}
				/* ��Shift OFF */
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
				/* F11���������������ϲ� */
				if (i == 256 + 0x57 && shtctl->top > 2) 
				{
					sheet_updown(shtctl->sheets[1], shtctl->top - 1);
				}
				/* ǿ�ƽ������񰴼� */ 
				/* shift + f1 */
				if (i == 256 + 0x3b && key_shift != 0 && key_win != 0)
				{
					task = key_win->task;
					if (task != 0 && task->tss.ss0 != 0)
					{
						cons_putstr0(task->cons, "\nBreak(key) :\n");
						/* ��ֹ�жϣ������ڸı�Ĵ���ֵʱ�л����������� */
						io_cli();
						task->tss.eax = (int)&(task->tss.esp0);
						task->tss.eip = (int) asm_end_app;
						io_sti();
						/* Ϊ��ȷʵִ�н������������������״̬���� */
						task_run(task, -1, 0);
					}
				}
				/* ��һ������̨ */
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
					/* �Զ������뽹���л����´򿪵������д��� */
					keywin_on(key_win);
				}
				/* ���̳ɹ����ܵ����� */
				if (i == 256 + 0xfa)
				{
					keycmd_wait = -1;
				}
				/* ����δ�ɹ����ܵ����� */
				if (i == 256 + 0xfe)
				{
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
			}
			/* ��껺���� */
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
					/* ������� */ 
					if ((mdec.btn & 0x01) != 0) 
					{
						if (mmx < 0) 
						{
							/* ���մ��ϵ��µ�˳��Ѱ�������ָ���ͼ�� */
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
										/* �������������ʱ�����ھͻᱻ�л�����������Ϸ� */
										/* ���Ҽ�������Ҳ���Զ��л����ô��� */
										if (sht!= key_win)
										{
											keywin_off(key_win);
											key_win = sht;
											keywin_on(key_win);
										}
										/* �������ڱ�����λ������봰���ƶ�ģʽ */
										if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21)
										{
											mmx = mx;
											mmy = my;
											mmx2 = sht->vx0;
											new_wy = sht->vy0;
										}
										/* �������"X"λ��ʱ�رմ��� */
										if (sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 5 <= y && y < 19)
										{
											/* ��Ӧ�ó������ɵĴ�����ر� */
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
											/* ����ر������д��� */
											else
											{
												task = sht->task;
												/* �������ظ�ͼ�� */
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
						/* �����ƶ�ģʽ */
						else
						{	
							/* ���������ƶ����룬����new_wx, new_wy */
							x = mx - mmx;
							y = my - mmy;
							new_wx = (mmx2 + x + 2) & ~3;
							new_wy = new_wy + y;
							/* ����Ϊ�ƶ�������� */
							mmy = my;
						}
					}
					/* û�а������������ͨ��ģʽ */
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
			/* ֻ�ر������д��� */
			else if (i >= 2024 && i <= 2279)
			{
				sht2 = shtctl->sheets0 + (i - 2024);
				memman_free_4k(memman, (int)sht2->buf, 256 * 165);
				sheet_free(sht2);
			}
		}
	}
}

/* ���ƴ��ڱ���������ɫ���������� */
void keywin_off(struct SHEET* key_win) 
{
	change_wtitle8(key_win, 0);
	if ((key_win->flags & 0x20) != 0)
	{
		fifo32_put(&key_win->task->fifo, 3);
	}
	return; 
}

/* ���ƴ��ڱ���������ɫΪ���������� */
void keywin_on(struct SHEET* key_win)
{
	change_wtitle8(key_win, 1);
	if ((key_win->flags & 0x20) != 0)
	{
		fifo32_put(&key_win->task->fifo, 2);
	}
	return;
}

/* ��һ������ */
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

/* �¿���һ�������� */
struct SHEET* open_console(struct SHTCTL* shtctl, unsigned int memtotal)
{
	struct MEMMAN* memman = (struct MEMMAN* )MEMMAN_ADDR;
	struct SHEET* sht = sheet_alloc(shtctl);
	unsigned char* buf = (unsigned char* )memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht, buf, 256, 165, -1);
	make_window8(buf, 256, 165, "console", 0);
	make_textbox8(sht, 8, 28, 240, 128, COL8_000000);
	sht->task = open_constask(sht, memtotal);
	/* �й�� */
	sht->flags |= 0x20;
	return sht;
}

/* �������������� */
void close_constask(struct TASK* task)
{
	struct MEMMAN* memman = (struct MEMMAN* )MEMMAN_ADDR;
	task_sleep(task);
	memman_free_4k(memman, task->cons_stack, 64 * 1024);
	memman_free_4k(memman, (int)task->fifo.buf, 128 * 4);
	task->flags = 0;
	return;
}

/* �ر�������ͼ�㣬ͼ���а���������ͬʱ�������������� */
void close_console(struct SHEET* sht)
{
	struct MEMMAN* memman = (struct MEMMAN* )MEMMAN_ADDR;
	struct TASK* task = sht->task;
	memman_free_4k(memman, (int)sht->buf, 256 * 165);
	sheet_free(sht);
	close_constask(task);
	return;
}
