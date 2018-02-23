/* asmhead.nas */
/* 0x0ff0-0x0fff */
struct BOOTINFO
{
	char cyls;	/* 启动区读硬盘读到何处为止 */ 
	char leds;	/* 启动时键盘LED的状态 */ 
	char vmode;	/* 显卡模式为多少位彩色 */ 
	char reserve;
	short scrnx, scrny;	/* 画面分辨率 */ 
	char *vram;
};

#define ADR_BOOTINFO	0x00000ff0
#define ADR_DISKIMG		0x00100000

/* naskfunc.nas */
void io_hlt(void);					/* HLT */
void io_cli(void);					/* 标志寄存器中断标志置为0，禁止中断*/
void io_sti(void);					/* cli的逆指令，标志寄存器中断标志置为1，允许中断 */
void io_stihlt(void);				/* 允许中断+HLT */
int io_in8(int port);				/* 从端口port读数据 */
void io_out8(int port, int data);	/* 往端口port中写入data数据 */ 
int io_load_eflags(void);			/* 载入标志寄存器，即返回值 */	
void io_store_eflags(int eflags);	/* 保存标志寄存器，压如栈顶 */
void load_gdtr(int limit, int addr);/* 将指定段上限和地址赋值给GDTR寄存器 */ 
void load_idtr(int limit, int addr);/* 将指定段上限和地址赋值给IDTR寄存器 */
int load_cr0(void);					/* 载入寄存器cr0的值 */ 
void store_cr0(int cr0);			/* 保存寄存器cr的值 */
void asm_inthandler20(void);		/* 来自定时器的中断 */
void asm_inthandler21(void);		/* 来自PS/2键盘的21号中断 */
void asm_inthandler27(void);		/* 特定的27号中断 */
void asm_inthandler2c(void);		/* 来自PS/2鼠标的2c号中断 */
unsigned int memtest_sub(unsigned int start, unsigned int end);
									/* 内存检查 */ 
void load_tr(int tr);				/* 修改tr的值 */
void farjmp(int eip, int cs);		/* 执行far跳转 */
void farcall(int eip, int cs);		/* 执行farcall */
void asm_hrb_api(void);				/* 封装hrb_api */
void start_app(int eip, int cs, int esp, int ds, int* tss_esp0);				
									/* 开始应用程序 */
void asm_end_app(void);				/* 结束应用程序 */
void asm_inthandler0d(void);		/* 关于保护操作系统中断 */
void asm_inthandler0c(void);		/* 堆栈异常中断 */

/* graphic.c */
void init_palette(void);			/* 初始化调色板 */ 
void set_palette(int start, int end, unsigned char *rgb);	
									/* 设定调色板颜色 */ 
void boxfill8(unsigned char *vram, int xsize, unsigned char c, 
	int x0, int y0, int x1, int y1);/* 用指定颜色填充矩形 */
void init_screen8(char *vram, int x, int y);
									/* 绘制操作系统的图形界面 */
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
									/* 输出字符 */ 
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
									/* 已ACSII格式显示字符串 */ 
void init_mouse_cursor8(char *mouse, char bc);
									/* 初始化鼠标指针 */ 
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize,
int px0, int py0, char *buf, int bxsize);
									/* 显示鼠标的背景色 */ 
									
/* 定义调色板颜色 */
#define COL8_000000		0 
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define	COL8_008484		14
#define	COL8_848484		15

/* dsctbl.c */
struct SEGMENT_DESCRIPTOR
{
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

struct GATE_DESCRIPTOR
{
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);

#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_LDT			0x0082
#define AR_TSS32		0x0089
#define AR_INTGATE32	0x008e

/* int.c */
/* 记录键盘中断编码的缓冲区 */
struct KEYBUF
{
	unsigned char data[32];
	int next_r, next_w, len;
			/* len是指缓冲区记录数据的长度 */
};
void init_pic(void);
			/* PIC-(programmable interrupt controller)可编程中断控制器的初始化 */ 
void inthandler21(int *esp);
			/* 来自PS/2键盘的21号中断 */
void inthandler27(int *esp);
			/* 特定的27号中断 */
void inthandler2c(int *esp);
			/* 来自PS/2鼠标的2c号中断 */

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

/* fifo.c */
struct FIFO8
{
	unsigned char *buf;
	int p, q, size, free, flags;
};

struct FIFO32
{
	int *buf;
	int p, q, size, free, flags;
	struct TASK* task;
};

/* 初始化FIFO32缓冲区 */
void fifo32_init(struct FIFO32 *fifo, int size, int* buf, struct TASK* task);
/* 向FIFO32传送数据并保存，返回-1表示溢出，0表示没有溢出 */
int fifo32_put(struct FIFO32 *fifo, int data);
/* 从FIFO32取得一个数据，若缓存区为空则返回-1 */
int fifo32_get(struct FIFO32 *fifo);
/* 报告FIFO32缓存区积攒数据量 */
int fifo32_status(struct FIFO32* fifo);

/* keyboard.c */
#define PORT_KEYDAT				0x0060
#define PORT_KEYCMD				0x0064

extern struct FIFO32* keyfifo;
/* 等待键盘控制电路准备完毕 */
void wait_KBC_sendready(void);
/* 初始化键盘控制电路 */
void init_keyboard(struct FIFO32* fifo, int data0);
/* 来自PS/2键盘的21号中断 */
void inthandler21(int *esp);

/* mouse.c */
struct MOUSE_DEC
{
	unsigned char buf[3], phase;
	int x, y, btn;
};

extern struct FIFO32* mousefifo;
/* 激活鼠标 */
void enable_mouse(struct FIFO32* fifo, int data0, struct MOUSE_DEC *mdec);
/* 根据phase的值依次获取鼠标的3个字节 */
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
/* 来自PS/2鼠标的2c号中断 */
void inthandler2c(int *esp);

/* memory.c */
#define MEMMAN_ADDR		0x003c0000
#define MEMMAN_FREES	4090

/* 可用信息 */
struct FREEINFO
{
	unsigned int addr, size;	
};

/* 内存管理 */
struct MEMMAN
{
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};

/* 确认CPU是386还是486以上的 */
unsigned int memtest(unsigned int start, unsigned int end);
/* 内存检测 */ 
unsigned int memtest_sub(unsigned int start, unsigned int end);
/* 内存处理初始化 */
void memman_init(struct MEMMAN *man);
/* 报告空余内存大小的合计 */
unsigned int memman_total(struct MEMMAN *man);
/* 分配内存 */
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
/* 释放内存 */
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
/* 以4KB字节为单位分配内存 */
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
/* 以4KB字节为单位释放内存 */
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);

/* sheet.c */
/* 透明图层 */ 
struct SHEET
{
	unsigned char *buf;		/* 图层地址 */
	int bxsize, bysize;		/* 图层的大小 */
	int vx0, vy0;			/* 图层坐标 */
	int col_inv;			/* 透明色色号 */
	int height;				/* 图层高度 */
	int flags;				/* 设定信息 */
	struct SHTCTL *ctl;		/* 包括图层管理 */
	struct TASK* task;		/* 图层所对应的任务 */
};

/* 设定能够管理的最大图层数 */
#define MAX_SHEETS		256

/* 图层管理 */
struct SHTCTL
{
	unsigned char *vram;
	unsigned char *map;
	int xsize, ysize;
	int top;					/* 最上面图层的高度 */
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};

/* 图层管理初始化 */ 
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
/* 取得新生成的未使用图层 */
struct SHEET *sheet_alloc(struct SHTCTL *ctl); 
/* 设置图层属性 */
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
/* 图层上下移动 */ 
void sheet_updown(struct SHEET *sht, int height); 
/* 采用局部刷新的方法重写sheet_refresh */
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
/* 以地图的方式做局部刷新 */
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0); 
/* 从h0以上h1以下的图层开始局部刷新 */
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
/* 图层高度不变，水平平移 */
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
/* 释放已使用的图层 */
void sheet_free(struct SHEET *sht); 

/* timer.c */

#define MAX_TIMER	500

struct TIMER
{
	/* 超时剩余时间 */
	unsigned int timeout;
	char flags;
	/* 区分该定时器是否需要在应用程序结束时自动取消 */
	char flags2;
	struct FIFO32* fifo;
	int data;
	struct TIMER* next;
};

struct TIMERCTL
{
	/* 时间累计器 */
	unsigned int count;
	/* 下一个定时器到期时间 */
	unsigned int next;
	/* 记录活动中定时器的数量 */
	unsigned int using;
	struct TIMER* t0;
	struct TIMER timers0[MAX_TIMER];
};

extern struct TIMERCTL timerctl;

/* 初始化定时器 */ 
void init_pit(void);
/* 申请定时器，返回一个定时器指针类型，0表示没有找到 */
struct TIMER* timer_alloc(void);
/* 释放定时器 */
void timer_free(struct TIMER* timer);
/* 定时器初始化 */
void timer_init(struct TIMER* timer, struct FIFO32* fifo, int data);
/* 设定定时器时间 */
void timer_settime(struct TIMER* timer, unsigned int timeout);
/* 定时器中断处理 */
void inthandler20(int *esp);
/* 取消指定定时器 */
int timer_cancel(struct TIMER* timer);
/* 取消flags2标识不为2的所有定时器 */
void timer_cancelall(struct FIFO32* fifo);

/* mtask.c */

/* 最大任务数量 */
#define MAX_TASKS	1000
/* 定义从CDT的几号开始分配给TSS */
#define TASK_GDT0	3
/* 每个任务级别最多运行100个任务 */
#define MAX_TASKS_LV	100
/* 任务优先级分为10个级别 */
#define MAX_TASKLEVELS	10

/* TSS-task status segment 任务状态段 */
struct TSS32
{
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};

struct TASK
{
	/* 用来存放GDT的编号 */
	int sel;
	/* 对应空闲-休眠-运行3种状态 */
	int flags;
	/* 优先级， 以任务运行的时间来区分 */
	int priority;
	/* 层，更高的优先级策略 */
	int level;
	struct FIFO32 fifo;
	struct TSS32 tss;
	struct SEGMENT_DESCRIPTOR ldt[2];
	struct CONSOLE* cons;
	int ds_base;
	/* 保存栈地址 */
	int cons_stack;
	struct FILEHANDLE* fhandle;
	int* fat;
	/* 命令行 */
	char* cmdline;
	/* 语言模式 */
	unsigned char langmode;
	/* 区号 */ 
	unsigned char langbyte1;
};

struct FILEHANDLE
{
	char* buf;
	int size;
	int pos;
};

struct TASKLEVEL
{
	/* 正在运行的任务数量 */
	int running;
	/* 正在运行的任务编号 */
	int now;
	struct TASK* tasks[MAX_TASKS_LV];
};

struct TASKCTL
{
	/* 现在活动中的level */
	int now_lv;
	/* 在下次任务切换时是否需要改变LEVEL */
	char lv_change; 
	struct TASKLEVEL level[MAX_TASKLEVELS];
	struct TASK tasks0[MAX_TASKS];
};

extern struct TASKCTL* taskctl;
extern struct TIMER* task_timer;

/* 任务管理器初始化，返回任务管理器内存地址 */
struct TASK* task_init(struct MEMMAN* memman);
/* 任务申请，返回0表示任务上限已满 */
struct TASK* task_alloc(void);
/* 任务运行 */
void task_run(struct TASK* task, int level, int priority); 
/* 对当前任务列表中的任务进行切换 */
void task_switch(void);
/* 任务休眠 */
void task_sleep(struct TASK* task);
/* 返回当前活动着的任务指针 */
struct TASK* task_now(void);

/* console.c */

struct CONSOLE
{
	struct SHEET* sht;
	int cur_x, cur_y, cur_c;
	struct TIMER* timer;
};

/* 控制台任务 */
void console_task(struct SHEET* sheet, int memtotal);
/* 在cons控制台打印字符chr，move为1时光标后移一位 */
void cons_putchar(struct CONSOLE* cons, int chr, char move);
/* 换行函数，当到达最后一行的时候会自动滚动 */
void cons_newline(struct CONSOLE* cons);
/* 根据cmdline匹配运行特定的命令 */
void cons_runcmd(char* cmdline, struct CONSOLE* cons, int* fat, int memtotal);
/* mem显示内存命令 */
void cmd_mem(struct CONSOLE* cons, int memtotal);
/* cls清屏命令 */
void cmd_cls(struct CONSOLE* cons);
/* dir显示目录命令 */
void cmd_dir(struct CONSOLE* cons);
/* type命令，显示文件内容 */ 
void cmd_type(struct CONSOLE* cons, int* fat, char* cmdline);
/* exit命令 */
void cmd_exit(struct CONSOLE* cons, int* fat);
/* 设定语言模式 */
void cmd_langmode(struct CONSOLE* cons, char* cmdline);
/* 启动应用程序 */
void cmd_start(struct CONSOLE* cons, char* cmdline, int memtotal);
/* 不用命令行启动应用程序 */
void cmd_ncst(struct CONSOLE* cons, char* cmdline, int memtotal);
/* 应用程序 */
int cmd_app(struct CONSOLE* cons, int* fat, char* cmdline);
/* 打印字符串s，以0结束 */
void cons_putstr0(struct CONSOLE* cons, char* s);
/* 根据字符串长度l来打印字符串 */
void cons_putstr1(struct CONSOLE* cons, char* s, int l); 
/* 操作系统API */
int* hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax); 
/* 处理异常中断 */
int* inthandler0d(int *esp);
/* 处理堆栈异常 */
int* inthandler0c(int* esp);
/* 画线API */
void hrb_api_linewin(struct SHEET* sht, int x0, int y0, int x1, int y1, int col);
 
/*file.c*/

/* 文件信息 */
struct FILEINFO
{
	/* 文件名，后缀，类型 */
	unsigned char name[8], ext[3], type;
	/* 微软保留字 */
	char reserve[10];
	/* 日期及时间 */
	unsigned short time, date, clustno;
	/* 文件大小 */ 
	unsigned int size;
};

/* 将磁盘映像中的FAT解压缩 */
void file_readfat(int* fat, unsigned char* img);
/* 将文件的内容读入内存 */
void file_loadfile(int clustno, int size, char* buf, int* fat, char* img);
/* 根据name查找文件, 返回文件信息，0为没有找到 */
struct FILEINFO* file_search(char* name, struct FILEINFO* finfo, int max);

/* window.c */

/* 绘制窗口 */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
/* 在窗口中添加标题 */
void make_wtitle8(unsigned char* buf, int xsize, char* title, char act);
/* 绘制编辑框 */
void make_textbox8(struct SHEET* sht, int x0, int y0, int sx, int sy, int c);
/* 涂背景，写字符，刷新 */
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
/* 根据窗口是否活动改变标题颜色 */
void change_wtitle8(struct SHEET* sht, char act);
 
/* bootpack.c */

/* 打开一个任务 */
struct TASK* open_constask(struct SHEET* sht, unsigned int memtotal);
/* 新开打一个命令行 */
struct SHEET* open_console(struct SHTCTL* shtctl, unsigned int memtotal);

