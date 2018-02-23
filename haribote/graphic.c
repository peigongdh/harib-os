/* 初始化操作系统图形界面 */

#include "bootpack.h"

/* 初始化调色板 */
void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,		/* 0: 黑 */
		0xff, 0x00, 0x00,		/* 1: 亮红 */
		0x00, 0xff, 0x00,		/* 2: 亮绿 */
		0xff, 0xff, 0x00,		/* 3: 亮黄 */
		0x00, 0x00, 0xff,		/* 4: 亮蓝 */
		0xff, 0x00, 0xff,		/* 5: 亮紫 */
		0x00, 0xff, 0xff,		/* 6: 浅亮蓝 */
		0xff, 0xff, 0xff,		/* 7: 白 */
		0xc6, 0xc6, 0xc6,		/* 8: 亮灰 */
		0x84, 0x00, 0x00,		/* 9: 暗红 */
		0x00, 0x84, 0x00,		/* 10: 暗绿 */
		0x84, 0x84, 0x00,		/* 11: 暗黄 */
		0x00, 0x00, 0x84,		/* 12: 暗亲 */
		0x84, 0x00, 0x84,		/* 13: 暗紫 */
		0x00, 0x84, 0x84,		/* 14: 浅暗蓝 */
		0x84, 0x84, 0x84		/* 15: 暗灰 */
	};
	int r;
	int g;
	int b;
	unsigned char table2[216 * 3];
	for (b = 0; b < 6; b ++)
	{
		for (g = 0; g < 6; g ++)
		{
			for (r = 0; r < 6; r ++)
			{
				table2[(r + g * 6 + b * 36) * 3 + 0] = r * 51;
				table2[(r + g * 6 + b * 36) * 3 + 1] = g * 51;
				table2[(r + g * 6 + b * 36) * 3 + 2] = b * 51;
			}
		}
	}
	set_palette(0, 15, table_rgb);
	set_palette(16, 231, table2);
	return;
}

/* 设定调色板颜色 */ 
void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();	/* 记录中断许可标志的值 */
	io_cli();					/* 将中断许可标志置为0，禁止中断 */
	
	io_out8(0x03c8, start);
	for (i = start; i <= end; i ++)
	{
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	
	io_store_eflags(eflags);	/* 复原中断许可标志 */
	return;
}

/* 用指定颜色填充矩形 */
void boxfill8(unsigned char *vram, int xsize, unsigned char c, 
	int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y ++)
	{
		for (x = x0; x <= x1; x ++)
		{
			vram[y * xsize + x] = c;
		}
	}
	return;
}

/* 绘制操作系统的图形界面 */
void init_screen8(char *vram, int x, int y)
{
	boxfill8(vram, x, COL8_008484,  0,     0,      x -  1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 28, x -  1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF,  0,     y - 27, x -  1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y -  1);
	
	boxfill8(vram, x, COL8_FFFFFF,  3,     y - 24, 59,     y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2,     y - 24,  2,     y -  4);
	boxfill8(vram, x, COL8_848484,  3,     y -  4, 59,     y -  4);
	boxfill8(vram, x, COL8_848484, 59,     y - 23, 59,     y -  5);
	boxfill8(vram, x, COL8_000000,  2,     y -  3, 59,     y -  3);
	boxfill8(vram, x, COL8_000000, 60,     y - 24, 60,     y -  3);
	
	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);
	return;
}

/* 输出字符 */
void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d;	/* data */
	for (i = 0; i < 16; i ++)
	{
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}

/* 输出汉字 */
void putfontHZK(char* vram, int xsize, int x, int y, char c, char* font)
{
	static char key[8] = {
		0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
	};
	int i, j, k;
	char d;
	char* p;
	
	for (i = 0; i < 8; i ++)
	{
		p = vram + (y + i) * xsize + x;
		for (j = 0; j < 2; j ++)
		{
			d = font[i * 2 + j];
			for (k = 0; k < 8; k ++)
			{
				if ((d & key[k]) != 0)
				{
					p[j * 8 + k] = c;
				}
			}
		}
	}
	return;
}

/* 已ACSII格式显示字符串 */ 

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	extern char hankaku[4096];
	struct TASK* task = task_now();
	char* hzk = (char* )*((int* )0x0fe8);
	char* font;
	/* langbyte1, langbyte2分别表示为区号和位号 */
	int langbyte1, langbyte2;
	
	if (task->langmode == 0)
	{
		for (; *s != 0x00; s ++)
		{
			putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
			x += 8;
		}	
	}
 	else if (task->langmode == 1)
 	{
		for (; *s != 0x00; s ++)
		{
			if (task->langbyte1 == 0)
			{
			 	if (*s >= 0x81 && *s <= 0xfe)
			 	{
		 			task->langbyte1 = *s;
		 		}
		 		else
		 		{
			 		putfont8(vram, xsize, x, y, c, hzk + *s * 16);
			 	}
		 	}
		 	else
		 	{
		 		langbyte1 = task->langbyte1 - 0xa1;
		 		langbyte2 = *s - 0xa1;
		 		font = hzk + 256 * 16 + (langbyte1 * 94 + langbyte2) * 32;
		 		task->langbyte1 = 0;
		 		putfontHZK(vram, xsize, x - 8, y, c, font);
		 		putfontHZK(vram, xsize, x - 8, y + 8, c, font + 16);
		 	}
		 	x += 8;
		}
	}
	return;
}

/* 初始化鼠标指针 */ 
void init_mouse_cursor8(char *mouse, char bc)
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	
	int x, y;
	
	for (y = 0; y < 16; y ++)
	{
		for (x = 0; x < 16; x ++)
		{
			if (cursor[y][x] == '*')
			{
				mouse[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O')
			{
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.')
			{
				/* bc为背景色 */ 
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}

/* 显示鼠标的背景色 */ 
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize,
	int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for (y = 0; y < pysize; y ++)
	{
		for (x = 0; x < pxsize; x ++)
		{
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y *bxsize + x];
		}
	}
}

