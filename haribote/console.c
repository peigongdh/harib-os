/* 命令行 */

#include "bootpack.h"
#include <stdio.h>
#include <string.h>

/* 控制台任务 */
void console_task(struct SHEET* sheet, int memtotal)
{
	int i;
	char cmdline[30];
	struct TASK* task = task_now();
	struct MEMMAN* memman = (struct MEMMAN* )MEMMAN_ADDR;
	struct CONSOLE cons;
	int* fat = (int* )memman_alloc_4k(memman, 4 * 2880);
	struct FILEHANDLE fhandle[8];
	unsigned char* hzk = (char*)*((int* )0x0fe8);

	/* 未使用标记为0 */
	for (i = 0; i < 8; i ++)
	{
		fhandle[i].buf = 0;
	}
	task->fhandle = fhandle;
	task->fat = fat;
		
	cons.sht = sheet;
	cons.cur_x = 8;
	cons.cur_y = 28;
	cons.cur_c = -1;
	task->cons = &cons;
	task->cmdline = cmdline;
	task->langbyte1 = 0;

	if (cons.sht != 0)
	{
		cons.timer = timer_alloc();
		timer_init(cons.timer, &task->fifo, 1);
		timer_settime(cons.timer, 50);
	}
	file_readfat(fat, (unsigned char* )(ADR_DISKIMG + 0x000200));
	
	/* 显示命令提示符 */
	cons_putchar(&cons, '>', 1);
	
	/* 设定默认字体 */
	if (hzk[4096] != 0xff)
	{
		task->langmode = 1;
	}
	else
	{
		task->langmode = 0;
	}
	
	for (; ;)
	{
		io_cli();
		if (fifo32_status(&task->fifo) == 0)
		{
			task_sleep(task);
			io_sti();
		}
		else
		{
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1 && cons.sht != 0)
			{
				if (i != 0)
				{
					if (cons.cur_c >= 0)
					{
						timer_init(cons.timer, &task->fifo, 0);
						cons.cur_c = COL8_FFFFFF;
					}
				}
				else
				{
					timer_init(cons.timer, &task->fifo, 1);
					if (cons.cur_c >= 0)
					{
						cons.cur_c = COL8_000000;
					}
				}
				timer_settime(cons.timer, 50);
			}
			if (i == 2)
			{
				cons.cur_c = COL8_FFFFFF;
			}
			if (i == 3)
			{
				if (cons.sht != 0)
				{
					boxfill8(cons.sht->buf, cons.sht->bxsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				}
				cons.cur_c = -1;
			}
			/* 点击命令行窗口的"X"按钮 */
			if (i == 4)
			{
				cmd_exit(&cons, fat);
			}
			if (i >= 256 && i <= 511)
			{
				/* 退格键 */
				if (i == 8 + 256)
				{	
					if (cons.cur_x > 16)
					{
						/* 用空格擦除光标后将光标前移一位 */
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
					}
				}
				/* enter键 */
				else if (i == 10 + 256)
				{
					/* 用空格将光标擦除 */
					cons_putchar(&cons, ' ', 0);
					cmdline[cons.cur_x / 8 - 2]  = 0;
					cons_newline(&cons);
					/* 运行命令 */
					cons_runcmd(cmdline, &cons, fat, memtotal);
					if (cons.sht == 0)
					{
						cmd_exit(&cons, fat);
					}
					/* 显示提示符 */ 
					cons_putchar(&cons, '>', 1);
				}
				/* 一般字符 */
				else
				{	
					if (cons.cur_x < 240)
					{
						/* 显示一个字符之后将光标后移一位 */
						cmdline[cons.cur_x / 8 - 2] = i - 256;
						cons_putchar(&cons, i - 256, 1);
					}
				}
			}
			/* 重新显示光标 */
			if (cons.sht != 0)
			{
				if (cons.cur_c >= 0)
				{
					boxfill8(cons.sht->buf, cons.sht->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				}
				sheet_refresh(cons.sht, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
			}		
		}
	}
}

/* 在cons控制台打印字符chr，move为1时光标后移一位 */
void cons_putchar(struct CONSOLE* cons, int chr, char move)
{
	char s[2];
	s[0] = chr;
	s[1] = 0;
	if (s[0] == 0x09)
	{
		for (; ;)
		{
			if (cons->sht != 0)
			{
				putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
			}
			cons->cur_x += 8;
			if (cons->cur_x == 8 + 240)
			{
				cons_newline(cons);
			}
			/* 被32整除则break */
			if (((cons->cur_x - 8) & 0x1f) == 0)
			{
				break;
			}
		}
	}
	/* 解析换行 */
	else if (s[0] == 0x0a)
	{
		cons_newline(cons);
	}
	/* 解析回车 */
	else if (s[0] == 0x0d)
	{
		/* 这里暂且不进行任何操作 */
	}
	/* 解析一般字符 */
	else
	{
		if (cons->sht != 0)
		{
			putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
		}
		/* 位移为0时不作移动 */ 
		if (move != 0)
		{
			cons->cur_x += 8;
			/* 到达最右端换行 */
			if (cons->cur_x == 8 + 240)
			{
				cons_newline(cons);
			}
		}
	}
	return;
}

/* 换行函数，当到达最后一行的时候会自动滚动 */
void cons_newline(struct CONSOLE* cons)
{
	int x, y;
	struct SHEET* sheet = cons->sht;
	struct TASK* task = task_now();
	if (cons->cur_y < 28 + 112)
	{
		/* 换行 */
		cons->cur_y += 16;
	}
	else
	{
		/* 滚动 */
		if (sheet != 0)
		{
			for (y = 28; y < 28 + 112; y ++)
			{
				for (x = 8; x < 8 + 240; x ++)
				{
					sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
				}
			}
			for (y = 28 + 112; y < 28 + 128; y ++)
			{
				for (x = 8; x < 8 + 240; x ++)
				{
					sheet->buf[x + y * sheet->bxsize] = COL8_000000;
				}
			}
			sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
		}
		if (task->langmode == 1 && task->langbyte1 != 0)
		{
			cons->cur_x += 8;
		}
	}
	cons->cur_x = 8;
	return;
}

/* 根据cmdline匹配运行特定的命令 */
void cons_runcmd(char* cmdline, struct CONSOLE* cons, int* fat, int memtotal)
{
	if (strcmp(cmdline, "mem") == 0)
	{
		cmd_mem(cons, memtotal);
	}
	else if (strcmp(cmdline, "cls") == 0)
	{
		cmd_cls(cons);
	}
	else if (strcmp(cmdline, "dir") == 0)
	{
		cmd_dir(cons);
	}
	else if (strncmp(cmdline, "type ", 5) == 0)
	{
		cmd_type(cons, fat, cmdline);
	}
	else if (strcmp(cmdline, "exit") == 0)
	{
		cmd_exit(cons, fat);
	}
	else if (strncmp(cmdline, "start ", 6) == 0)
	{
		cmd_start(cons, cmdline, memtotal);
	}
	else if (strncmp(cmdline, "ncst ", 5) == 0)
	{
		cmd_ncst(cons, cmdline, memtotal);
	}
	else if (strncmp(cmdline, "langmode ", 9) == 0)
	{
		cmd_langmode(cons, cmdline);
	}
	else if (cmdline[0] != 0)
	{
		/* 执行cmd_app，返回0表示不是命令，不是应用程序，也不是空行，打印提示 */
		if (cmd_app(cons, fat, cmdline) == 0)
		{
			cons_putstr0(cons, "Bad command.\n\n");
		}
	}
	return;
}

/* mem显示内存命令 */
void cmd_mem(struct CONSOLE* cons, int memtotal)
{
	char s[60];
	struct MEMMAN* memman = (struct MEMMAN* )MEMMAN_ADDR;

	sprintf(s, "total    %dMB\nfree     %dKB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	cons_putstr0(cons, s);
	return;
}

/* cls清屏命令 */
void cmd_cls(struct CONSOLE* cons)
{
	int x, y;
	struct SHEET* sheet = cons->sht;
	for (y = 28; y < 28 + 128; y ++)
	{
		for (x = 8; x < 8 + 240; x ++)
		{
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	cons->cur_y = 28;
	return;
}

/* dir显示目录命令 */
void cmd_dir(struct CONSOLE* cons)
{
	int i, j;
	char s[30];
	struct FILEINFO* finfo = (struct FILEINFO* )(ADR_DISKIMG + 0x002600);
	
	for (i = 0; i < 224; i ++)
	{
		if (finfo[i].name[0] == 0x00)
		{
			break;
		}
		if (finfo[i].name[0] != 0xe5)
		{
			if ((finfo[i].type & 0x18) == 0)
			{
				sprintf(s, "filename.ext    %7d\n", finfo[i].size);
				for (j = 0; j < 8; j ++)
				{
					s[j] = finfo[i].name[j];
				}
				s[9] = finfo[i].ext[0];
				s[10] = finfo[i].ext[1];
				s[11] = finfo[i].ext[2];
				cons_putstr0(cons, s);
			}
		}
	}
	cons_newline(cons);
	return;
}

/* type命令，显示文件内容 */ 
void cmd_type(struct CONSOLE* cons, int* fat, char* cmdline)
{
	char* p;
	struct MEMMAN* memman = (struct MEMMAN* )MEMMAN_ADDR;
	struct FILEINFO* finfo = file_search(cmdline + 5, (struct FILEINFO* )(ADR_DISKIMG + 0x002600), 224);
	
	/* 找到文件的情况 */
	if (finfo != 0)
	{
		p = (char* )memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char* )(ADR_DISKIMG + 0x003e00));
		cons_putstr1(cons, p, finfo->size);
		memman_free_4k(memman, (int)p, finfo->size);
	}
	else
	{
		cons_putstr0(cons, "File not found.\n");
	}
	cons_newline(cons);
	return;
}

/* exit命令 */
void cmd_exit(struct CONSOLE* cons, int* fat)
{
	struct MEMMAN* memman = (struct MEMMAN* )MEMMAN_ADDR;
	struct TASK* task = task_now();
	struct SHTCTL* shtctl = (struct SHTCTL* ) *((int* )0x0fe4);
	struct FIFO32* fifo = (struct FIFO32* ) *((int* )0x0fec);
	timer_cancel(cons->timer);
	memman_free_4k(memman, (int)fat, 4 * 2880);
	io_cli();
	fifo32_put(fifo, cons->sht - shtctl->sheets0 + 768);
	io_sti();
	/* 有命令行窗口时，我们可以通过图层的地址告诉task_a需要结束哪个任务 */
	if (cons->sht != 0)
	{
		/* 768 ~ 1023 */
		fifo32_put(fifo, cons->sht - shtctl->sheets0 + 768);
	}
	/* 无命令行窗口的情况下，将TASK结构的地址告诉task_a */
	else
	{
		/* 1024 ~ 2023 */
		fifo32_put(fifo, task - taskctl->tasks0 + 1024);
	}
	for (; ;)
	{
		task_sleep(task);
	}
}

/* 设定语言模式 */
void cmd_langmode(struct CONSOLE* cons, char* cmdline)
{
	struct TASK* task = task_now();
	unsigned char mode = cmdline[9] - '0';
	if (mode <= 1)
	{
		task->langmode = mode;
	}
	else
	{
		cons_putstr0(cons, "mode number error.\n");
	}
	cons_newline(cons);
	return;
}

/* 启动应用程序 */
void cmd_start(struct CONSOLE* cons, char* cmdline, int memtotal)
{
	struct SHTCTL* shtctl = (struct SHTCTL* )*((int* )0xfe4);
	struct SHEET* sht = open_console(shtctl, memtotal);
	struct FIFO32* fifo = &sht->task->fifo;
	int i;
	sheet_slide(sht, 32, 4);
	sheet_updown(sht, shtctl->top);
	/* 将命令行输入的字符串逐字复制到新的命令行窗口中 */
	for (i = 6; cmdline[i] != 0; i ++)
	{
		fifo32_put(fifo, cmdline[i] + 256);
	}
	fifo32_put(fifo, 10 + 256);
	cons_newline(cons);
	return;
}

/* 不用命令行启动应用程序 */
void cmd_ncst(struct CONSOLE* cons, char* cmdline, int memtotal) 
{
	struct TASK* task = open_constask(0, memtotal);
	struct FIFO32* fifo = &task->fifo;
	int i;
	for (i = 5; cmdline[i] != 0; i ++)
	{
		fifo32_put(fifo, cmdline[i] + 256);
	}
	fifo32_put(fifo, 10 + 256);
	cons_newline(cons);
	return;
}

/* 应用程序 */
int cmd_app(struct CONSOLE* cons, int* fat, char* cmdline)
{
	int i;
	int segsiz, datsiz, esp, dathrb;
	char* p;
	char* q;
	char name[18];
	struct SHTCTL* shtctl;
	struct SHEET* sht;
	struct FILEINFO* finfo;
	struct TASK* task = task_now();
	struct MEMMAN* memman = (struct MEMMAN* )MEMMAN_ADDR;
	
	/* 根据命令行生成文件名 */
	for (i = 0; i < 13; i ++)
	{
		if (cmdline[i] <= ' ')
		{
			break;
		}
		name[i] = cmdline[i];
	}
	/* 暂且将文件名的后面置为0 */
	name[i] = 0;
	
	/* 寻找文件 */
	finfo = file_search(name, (struct FILEINFO* )(ADR_DISKIMG + 0x002600), 224);
	/* 找不到文件，在文件名后面加上".hrb" 后重新寻找*/
	if (finfo == 0 && name[i - 1] != '.')
	{
		name[i] = '.';
		name[i + 1] = 'H';
		name[i + 2] = 'R';
		name[i + 3] = 'B';
		name[i + 4] = 0;
		finfo = file_search(name, (struct FILEINFO* )(ADR_DISKIMG + 0x002600), 224);
	}
	/* 找到文件的情况 */
	if (finfo != 0)
	{
		p = (char* )memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char* )(ADR_DISKIMG + 0x003e00));
		if (finfo->size >= 36 && strncmp(p + 4, "Hari", 4) == 0 && *p == 0x00)
		{
			segsiz = *((int* )(p + 0x0000));
			esp = *((int* )(p + 0x000c));
			datsiz = *((int* )(p + 0x0010));
			dathrb = *((int* )(p + 0x0014));
			/* 应用程序专用内存空间 */
			q = (char* )memman_alloc_4k(memman, segsiz);
			task->ds_base = (int)q;
			/* 在段定义的地方，如果将访问权限加上0x60，就可以将段设置为应用程序用 */
			/* 此时如果存入操作系统用的段地址就会产生异常 */
			set_segmdesc(task->ldt + 0, finfo->size - 1, (int)p, AR_CODE32_ER + 0x60);
			set_segmdesc(task->ldt + 1, segsiz - 1, (int)q, AR_DATA32_RW + 0x60);
			/* 数据段的大小根据.hrb文件中指定的值进行分配，完成复制后启动程序 */
			for (i = 0; i < datsiz; i ++)
			{
				q[esp + i] = p[dathrb + i];
			}
			start_app(0x1b, 0 * 8 + 4, esp, 1 * 8 + 4, &(task->tss.esp0));
			shtctl = (struct SHTCTL* ) * ((int* )0x0fe4);
			/* 将未关闭的文件关闭 */
			for (i = 0; i < 8; i ++)
			{
				if (task->fhandle[i].buf != 0)
				{
					memman_free_4k(memman, (int)task->fhandle[i].buf, task->fhandle[i].size);
					task->fhandle[i].buf = 0;
				}
			}
			/* 遍历所有图层，如果图层的task为将要结束的应用程序任务，则关闭该图层 */
			for (i = 0; i < MAX_SHEETS; i ++)
			{
				sht = &(shtctl->sheets0[i]);
				/* 找到应用程序残留的窗口 */
				if ((sht->flags & 0x11) == 0x11 && sht->task == task)
				{
					sheet_free(sht);
				}
			}
			timer_cancelall(&task->fifo) ;
			memman_free_4k(memman, (int)q, segsiz);
			task->langbyte1 = 0;
		}
		else
		{
			cons_putstr0(cons, ".hrb file format error.\n");
		}
		memman_free_4k(memman, (int)p, finfo->size);
		cons_newline(cons);
		return 1;
	}
	/* 没有找到文件返回0 */
	return 0; 
}

/* 打印字符串s，以0结束 */
void cons_putstr0(struct CONSOLE* cons, char* s)
{
	for (; *s != 0; s ++)
	{
		cons_putchar(cons, *s, 1);
	}
	return;
}

/* 根据字符串长度l来打印字符串 */
void cons_putstr1(struct CONSOLE* cons, char* s, int l)
{
	int i;
	for (i = 0; i < l; i ++)
	{
		cons_putchar(cons, s[i], 1);
	}
	return;
}

/* 操作系统API */
/* 开头寄存器的顺序是按照PUSHAD的顺序写的 */
int* hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax) 
{
	int i;
	struct FILEINFO* finfo;
	struct FILEHANDLE* fh; 
	struct MEMMAN* memman = (struct MEMMAN* )MEMMAN_ADDR;
	struct TASK* task = task_now();
	int ds_base = task->ds_base;
	struct FIFO32* sys_fifo = (struct FIFO32* )*((int* )0x0fec);
	struct CONSOLE* cons = task->cons;
	struct SHTCTL* shtctl = (struct SHTCTL* ) * ((int* )0x0fe4);
	struct SHEET* sht;
	/* eax后面的地址 */
	int* reg = &eax + 1;
	/* 强行改写通过PUSHAD保存的值 */
	/* reg[0]: EDI, reg[1]: ESI, reg[2]: EBP, reg[3]: ESP */
	/* reg[4]: EBX, reg[5]: EDX, reg[6]: ECX, reg[7]: EAX */
	
	if (edx == 1)
	{
		cons_putchar(cons, eax & 0xff, 1);
	}
	else if (edx == 2)
	{
		cons_putstr0(cons, (char*)ebx + ds_base);
//		sprintf(s, "%08X\n", ebx);
//		cons_putstr0(cons, s);
	}
	else if (edx == 3)
	{
		cons_putstr1(cons, (char* )ebx + ds_base, ecx);
	}
	/* 异常程序结束，返回tss.esp0的地址 */
	else if (edx == 4)
	{
		return &(task->tss.esp0);
	}
	else if (edx == 5)
	{
		sht = sheet_alloc(shtctl);
		sht->task = task;
		sht->flags |= 0x10;
		sheet_setbuf(sht, (char*)ebx + ds_base, esi, edi, eax);
		make_window8((char* )ebx + ds_base, esi, edi, (char* )ecx + ds_base, 0);
		sheet_slide(sht, ((shtctl->xsize - esi) / 2) & ~3, (shtctl->ysize - edi) / 2);
		sheet_updown(sht, shtctl->top);
		reg[7] = (int)sht;
	}
	else if (edx == 6)
	{
		sht = (struct SHEET* )(ebx & 0xfffffffe);
		putfonts8_asc(sht->buf, sht->bxsize, esi, edi, eax, (char* )ebp + ds_base);
		/* EBX为偶数时刷新 */
		if ((ebx & 1) == 0)
		{
			sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
		}
	}
	else if (edx == 7)
	{
		sht = (struct SHEET* )(ebx & 0xfffffffe);
		boxfill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi, edi);
		/* EBX为偶数时刷新 */
		if ((ebx & 1) == 0)
		{
			sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
		}
	}
	else if (edx == 8)
	{
		memman_init((struct MEMMAN* )(ebx + ds_base));
		/* 以16字节为单位 */
		ecx &= 0xfffffff0;
		memman_free((struct MEMMAN* )(ebx + ds_base), eax, ecx);
	}
	else if (edx == 9)
	{
		/* 以16字节为单位进位取整 */
		ecx = (ecx + 0x0f) & 0xfffffff0;
		reg[7] = memman_alloc((struct MEMMAN* )(ebx + ds_base), ecx);
	}
	else if (edx == 10)
	{
		/* 以16字节为单位进位取整 */
		ecx = (ecx + 0x0f) & 0xfffffff0;
		memman_free((struct MEMMAN* )(ebx + ds_base), eax, ecx);
	}
	else if (edx == 11)
	{
		sht = (struct SHEET* )(ebx & 0xfffffffe);
		sht->buf[sht->bxsize * edi + esi] = eax;
		/* EBX为偶数时刷新 */
		if ((ebx & 1) == 0)
		{
			sheet_refresh(sht, esi, edi, esi + 1, edi + 1);
		}
	}
	else if (edx == 12)
	{
		sht = (struct SHEET* )ebx;
		sheet_refresh(sht, eax, ecx, esi, edi);
	}
	else if (edx == 13)
	{
		sht = (struct SHEET* )(ebx & 0xfffffffe);
		hrb_api_linewin(sht, eax, ecx, esi, edi, ebp);
		if ((ebx & 1) == 0)
		{
			sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
		}
	}
	else if (edx == 14)
	{
		sheet_free((struct SHEET* )ebx);
	}
	else if (edx == 15)
	{
		for (; ;)
		{
			io_cli();
			if (fifo32_status(&task->fifo) == 0)
			{
				if (eax != 0)
				{
					/* FIFO为空，休眠并等待 */
					task_sleep(task);
				}
				else
				{
					io_sti();
					reg[7] = -1;
					return 0;
				}
			}
			i = fifo32_get(&task->fifo);
			io_sti();
			/* 光标用定时器 */
			if (i <= 1)
			{
				/* 应用程序运行时不需要显示光标，因此总是将下次显示用的值置为1 */
				timer_init(cons->timer, &task->fifo, 1);
				timer_settime(cons->timer, 50);
			}
			/* 光标ON */
			if (i == 2)
			{
				cons->cur_c = COL8_FFFFFF; 
			}
			/* 光标OFF */
			if (i == 3)
			{
				cons->cur_c = -1;
			}
			/* 只关闭命令行命令窗口 */
			if (i == 4)
			{
				timer_cancel(cons->timer);
				io_cli();
				fifo32_put(sys_fifo, cons->sht - shtctl->sheets0 + 2024);
				cons->sht = 0;
				io_sti();
			}
			/* 键盘数据（通过任务A） */
			if (i >= 256)
			{
				reg[7] = i - 256;
				return 0;
			}
		}
	}
	else if (edx == 16)
	{
		reg[7] = (int)timer_alloc();
		/* 允许自动取消 */
		((struct TIMER* )reg[7])->flags2 = 1;
	}
	else if (edx == 17)
	{
		/* 在向应用程序传递FIFO数据时，eax要先减去256 */
		timer_init((struct TIMER* )ebx, &task->fifo, eax + 256);
	}
	else if (edx == 18)
	{
		timer_settime((struct TIMER* )ebx, eax);
	}
	else if (edx == 19)
	{
		timer_free((struct TIMER* )ebx);
	}
	else if (edx == 20)
	{
		if (eax == 0)
		{
			i = io_in8(0x61);
			io_out8(0x61, i & 0x0d);
		}
		else
		{
			/* eax */
			i = 1193180000;
			io_out8(0x43, 0xb6);
			io_out8(0x42, i & 0xff);
			io_out8(0x42, i >> 8);
			i = io_in8(0x61);
			io_out8(0x61, (i | 0x03) & 0x0f);
		}
	}
	else if (edx == 21)
	{
		for (i = 0; i < 8; i ++)
		{
			if (task->fhandle[i].buf == 0)
			{
				break;
			}
		}
		fh = &task->fhandle[i];
		reg[7] = 0;
		if (i < 8)
		{
			finfo = file_search((char* )ebx + ds_base, (struct FILEINFO* )(ADR_DISKIMG + 0x002600), 224);
			if (finfo != 0)
			{
				reg[7] = (int)fh;
				fh->buf = (char* )memman_alloc_4k(memman, finfo->size);
				fh->size = finfo->size;
				fh->pos = 0;
				file_loadfile(finfo->clustno, finfo->size, fh->buf, task->fat, (char* )(ADR_DISKIMG + 0x003e00));
			}
		}
	}
	else if (edx == 22)
	{
		fh = (struct FILEHANDLE* )eax;
		memman_free_4k(memman, (int)fh->buf, fh->size);
		fh->buf = 0;
	}
	else if (edx == 23)
	{
		fh = (struct FILEHANDLE* )eax;
		if (ecx == 0)
		{
			fh->pos = ebx;
		}
		else if (ecx == 1)
		{
			fh->pos += ebx;
		}
		else if (ecx == 2)
		{
			fh->pos = fh->size + ebx;
		}
		if (fh->pos < 0)
		{
			fh->pos = 0;
		}
		if (fh->pos > fh->size)
		{
			fh->pos = fh->size;
		}
	}
	else if (edx == 24)
	{
		fh = (struct FILEHANDLE* )eax;
		if (ecx == 0)
		{
			reg[7] = fh->size;
		}
		else if (ecx == 1)
		{
			reg[7] = fh->pos;
		}
		else if (ecx == 2)
		{
			reg[7] = fh->pos - fh->size;
		}
	}
	else if (edx == 25)
	{
		fh = (struct FILEHANDLE* )eax;
		for (i = 0; i < ecx; i ++)
		{
			if (fh->pos == fh->size)
			{
				break;
			}
			*((char* )ebx + ds_base + i) = fh->buf[fh->pos];
			fh->pos ++;
		}
		reg[7] = i;
	}
	else if (edx == 26)
	{
		i = 0;
		for (; ;)
		{
			*((char* )ebx + ds_base + i) = task->cmdline[i];
			if (task->cmdline[i] == 0)
			{
				break;
			}
			if (i >= ecx)
			{
				break;
			}
			i ++;
		}
		reg[7] = i;
	}
	return 0;
}

/* 处理异常中断 */
int* inthandler0d(int *esp)
{
	char s[30];
	struct TASK* task = task_now();
	struct CONSOLE* cons = task->cons;
	/* 将EIP栈中的元素显示出来 */
	sprintf(s, "EIP = %08X\n", esp[11]);
	cons_putstr0(cons, s);
	cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
	return &(task->tss.esp0);
}

/* 处理堆栈异常 */
int* inthandler0c(int* esp)
{
	char s[30];
	struct TASK* task = task_now();
	struct CONSOLE* cons = task->cons;
	cons_putstr0(cons, "\nINT 0C :\n Stack Exception.\n");
	/* 将EIP栈中的元素显示出来 */
	sprintf(s, "EIP = %08X\n", esp[11]);
	cons_putstr0(cons, s);
	/* 强制程序结束 */
	return &(task->tss.esp0);
}

/* 画线API */
void hrb_api_linewin(struct SHEET* sht, int x0, int y0, int x1, int y1, int col)
{
	int i;
	int x;
	int y;
	int len;
	int dx;
	int dy;
	
	dx = x1 - x0;
	dy = y1 - y0;
	x = x0 << 10;
	y = y0 << 10;
	if (dx < 0)
	{
		dx = -dy;
	}
	if (dy < 0)
	{
		dy = -dy;
	}
	if (dx >= dy)
	{
		len = dx + 1;
		if (x0 > x1)
		{
			dx = -1024;
		}
		else
		{
			dx = 1024;
		}
		
		if (y0 <= y1)
		{
			dy = ((y1 - y0 + 1) << 10) / len;
		}
		else
		{
			dy = ((y1 - y0 - 1) << 10) / len;
		}
	}
	else
	{
		len = dy + 1;
		if (y0 > y1)
		{
			dy = -1024;
		}
		else
		{
			dy = 1024;
		}
		if (x0 <= x1)
		{
			dx = ((x1 - x0 + 1) << 10) / len;
		}
		else
		{
			dx = ((x1 - x0 - 1) << 10) / len;
		}
	}
	
	for (i = 0; i < len; i ++)
	{
		sht->buf[(y >> 10) * sht->bxsize + (x >> 10)] = col;
		x += dx;
		y += dy;
	}
	return;
}
