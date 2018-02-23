/* asmhead.nas */
/* 0x0ff0-0x0fff */
struct BOOTINFO
{
	char cyls;	/* ��������Ӳ�̶����δ�Ϊֹ */ 
	char leds;	/* ����ʱ����LED��״̬ */ 
	char vmode;	/* �Կ�ģʽΪ����λ��ɫ */ 
	char reserve;
	short scrnx, scrny;	/* ����ֱ��� */ 
	char *vram;
};

#define ADR_BOOTINFO	0x00000ff0
#define ADR_DISKIMG		0x00100000

/* naskfunc.nas */
void io_hlt(void);					/* HLT */
void io_cli(void);					/* ��־�Ĵ����жϱ�־��Ϊ0����ֹ�ж�*/
void io_sti(void);					/* cli����ָ���־�Ĵ����жϱ�־��Ϊ1�������ж� */
void io_stihlt(void);				/* �����ж�+HLT */
int io_in8(int port);				/* �Ӷ˿�port������ */
void io_out8(int port, int data);	/* ���˿�port��д��data���� */ 
int io_load_eflags(void);			/* �����־�Ĵ�����������ֵ */	
void io_store_eflags(int eflags);	/* �����־�Ĵ�����ѹ��ջ�� */
void load_gdtr(int limit, int addr);/* ��ָ�������޺͵�ַ��ֵ��GDTR�Ĵ��� */ 
void load_idtr(int limit, int addr);/* ��ָ�������޺͵�ַ��ֵ��IDTR�Ĵ��� */
int load_cr0(void);					/* ����Ĵ���cr0��ֵ */ 
void store_cr0(int cr0);			/* ����Ĵ���cr��ֵ */
void asm_inthandler20(void);		/* ���Զ�ʱ�����ж� */
void asm_inthandler21(void);		/* ����PS/2���̵�21���ж� */
void asm_inthandler27(void);		/* �ض���27���ж� */
void asm_inthandler2c(void);		/* ����PS/2����2c���ж� */
unsigned int memtest_sub(unsigned int start, unsigned int end);
									/* �ڴ��� */ 
void load_tr(int tr);				/* �޸�tr��ֵ */
void farjmp(int eip, int cs);		/* ִ��far��ת */
void farcall(int eip, int cs);		/* ִ��farcall */
void asm_hrb_api(void);				/* ��װhrb_api */
void start_app(int eip, int cs, int esp, int ds, int* tss_esp0);				
									/* ��ʼӦ�ó��� */
void asm_end_app(void);				/* ����Ӧ�ó��� */
void asm_inthandler0d(void);		/* ���ڱ�������ϵͳ�ж� */
void asm_inthandler0c(void);		/* ��ջ�쳣�ж� */

/* graphic.c */
void init_palette(void);			/* ��ʼ����ɫ�� */ 
void set_palette(int start, int end, unsigned char *rgb);	
									/* �趨��ɫ����ɫ */ 
void boxfill8(unsigned char *vram, int xsize, unsigned char c, 
	int x0, int y0, int x1, int y1);/* ��ָ����ɫ������ */
void init_screen8(char *vram, int x, int y);
									/* ���Ʋ���ϵͳ��ͼ�ν��� */
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
									/* ����ַ� */ 
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
									/* ��ACSII��ʽ��ʾ�ַ��� */ 
void init_mouse_cursor8(char *mouse, char bc);
									/* ��ʼ�����ָ�� */ 
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize,
int px0, int py0, char *buf, int bxsize);
									/* ��ʾ���ı���ɫ */ 
									
/* �����ɫ����ɫ */
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
/* ��¼�����жϱ���Ļ����� */
struct KEYBUF
{
	unsigned char data[32];
	int next_r, next_w, len;
			/* len��ָ��������¼���ݵĳ��� */
};
void init_pic(void);
			/* PIC-(programmable interrupt controller)�ɱ���жϿ������ĳ�ʼ�� */ 
void inthandler21(int *esp);
			/* ����PS/2���̵�21���ж� */
void inthandler27(int *esp);
			/* �ض���27���ж� */
void inthandler2c(int *esp);
			/* ����PS/2����2c���ж� */

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

/* ��ʼ��FIFO32������ */
void fifo32_init(struct FIFO32 *fifo, int size, int* buf, struct TASK* task);
/* ��FIFO32�������ݲ����棬����-1��ʾ�����0��ʾû����� */
int fifo32_put(struct FIFO32 *fifo, int data);
/* ��FIFO32ȡ��һ�����ݣ���������Ϊ���򷵻�-1 */
int fifo32_get(struct FIFO32 *fifo);
/* ����FIFO32���������������� */
int fifo32_status(struct FIFO32* fifo);

/* keyboard.c */
#define PORT_KEYDAT				0x0060
#define PORT_KEYCMD				0x0064

extern struct FIFO32* keyfifo;
/* �ȴ����̿��Ƶ�·׼����� */
void wait_KBC_sendready(void);
/* ��ʼ�����̿��Ƶ�· */
void init_keyboard(struct FIFO32* fifo, int data0);
/* ����PS/2���̵�21���ж� */
void inthandler21(int *esp);

/* mouse.c */
struct MOUSE_DEC
{
	unsigned char buf[3], phase;
	int x, y, btn;
};

extern struct FIFO32* mousefifo;
/* ������� */
void enable_mouse(struct FIFO32* fifo, int data0, struct MOUSE_DEC *mdec);
/* ����phase��ֵ���λ�ȡ����3���ֽ� */
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
/* ����PS/2����2c���ж� */
void inthandler2c(int *esp);

/* memory.c */
#define MEMMAN_ADDR		0x003c0000
#define MEMMAN_FREES	4090

/* ������Ϣ */
struct FREEINFO
{
	unsigned int addr, size;	
};

/* �ڴ���� */
struct MEMMAN
{
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};

/* ȷ��CPU��386����486���ϵ� */
unsigned int memtest(unsigned int start, unsigned int end);
/* �ڴ��� */ 
unsigned int memtest_sub(unsigned int start, unsigned int end);
/* �ڴ洦���ʼ�� */
void memman_init(struct MEMMAN *man);
/* ��������ڴ��С�ĺϼ� */
unsigned int memman_total(struct MEMMAN *man);
/* �����ڴ� */
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
/* �ͷ��ڴ� */
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
/* ��4KB�ֽ�Ϊ��λ�����ڴ� */
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
/* ��4KB�ֽ�Ϊ��λ�ͷ��ڴ� */
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);

/* sheet.c */
/* ͸��ͼ�� */ 
struct SHEET
{
	unsigned char *buf;		/* ͼ���ַ */
	int bxsize, bysize;		/* ͼ��Ĵ�С */
	int vx0, vy0;			/* ͼ������ */
	int col_inv;			/* ͸��ɫɫ�� */
	int height;				/* ͼ��߶� */
	int flags;				/* �趨��Ϣ */
	struct SHTCTL *ctl;		/* ����ͼ����� */
	struct TASK* task;		/* ͼ������Ӧ������ */
};

/* �趨�ܹ���������ͼ���� */
#define MAX_SHEETS		256

/* ͼ����� */
struct SHTCTL
{
	unsigned char *vram;
	unsigned char *map;
	int xsize, ysize;
	int top;					/* ������ͼ��ĸ߶� */
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};

/* ͼ������ʼ�� */ 
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
/* ȡ�������ɵ�δʹ��ͼ�� */
struct SHEET *sheet_alloc(struct SHTCTL *ctl); 
/* ����ͼ������ */
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
/* ͼ�������ƶ� */ 
void sheet_updown(struct SHEET *sht, int height); 
/* ���þֲ�ˢ�µķ�����дsheet_refresh */
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
/* �Ե�ͼ�ķ�ʽ���ֲ�ˢ�� */
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0); 
/* ��h0����h1���µ�ͼ�㿪ʼ�ֲ�ˢ�� */
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
/* ͼ��߶Ȳ��䣬ˮƽƽ�� */
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
/* �ͷ���ʹ�õ�ͼ�� */
void sheet_free(struct SHEET *sht); 

/* timer.c */

#define MAX_TIMER	500

struct TIMER
{
	/* ��ʱʣ��ʱ�� */
	unsigned int timeout;
	char flags;
	/* ���ָö�ʱ���Ƿ���Ҫ��Ӧ�ó������ʱ�Զ�ȡ�� */
	char flags2;
	struct FIFO32* fifo;
	int data;
	struct TIMER* next;
};

struct TIMERCTL
{
	/* ʱ���ۼ��� */
	unsigned int count;
	/* ��һ����ʱ������ʱ�� */
	unsigned int next;
	/* ��¼��ж�ʱ�������� */
	unsigned int using;
	struct TIMER* t0;
	struct TIMER timers0[MAX_TIMER];
};

extern struct TIMERCTL timerctl;

/* ��ʼ����ʱ�� */ 
void init_pit(void);
/* ���붨ʱ��������һ����ʱ��ָ�����ͣ�0��ʾû���ҵ� */
struct TIMER* timer_alloc(void);
/* �ͷŶ�ʱ�� */
void timer_free(struct TIMER* timer);
/* ��ʱ����ʼ�� */
void timer_init(struct TIMER* timer, struct FIFO32* fifo, int data);
/* �趨��ʱ��ʱ�� */
void timer_settime(struct TIMER* timer, unsigned int timeout);
/* ��ʱ���жϴ��� */
void inthandler20(int *esp);
/* ȡ��ָ����ʱ�� */
int timer_cancel(struct TIMER* timer);
/* ȡ��flags2��ʶ��Ϊ2�����ж�ʱ�� */
void timer_cancelall(struct FIFO32* fifo);

/* mtask.c */

/* ����������� */
#define MAX_TASKS	1000
/* �����CDT�ļ��ſ�ʼ�����TSS */
#define TASK_GDT0	3
/* ÿ�����񼶱��������100������ */
#define MAX_TASKS_LV	100
/* �������ȼ���Ϊ10������ */
#define MAX_TASKLEVELS	10

/* TSS-task status segment ����״̬�� */
struct TSS32
{
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};

struct TASK
{
	/* �������GDT�ı�� */
	int sel;
	/* ��Ӧ����-����-����3��״̬ */
	int flags;
	/* ���ȼ��� ���������е�ʱ�������� */
	int priority;
	/* �㣬���ߵ����ȼ����� */
	int level;
	struct FIFO32 fifo;
	struct TSS32 tss;
	struct SEGMENT_DESCRIPTOR ldt[2];
	struct CONSOLE* cons;
	int ds_base;
	/* ����ջ��ַ */
	int cons_stack;
	struct FILEHANDLE* fhandle;
	int* fat;
	/* ������ */
	char* cmdline;
	/* ����ģʽ */
	unsigned char langmode;
	/* ���� */ 
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
	/* �������е��������� */
	int running;
	/* �������е������� */
	int now;
	struct TASK* tasks[MAX_TASKS_LV];
};

struct TASKCTL
{
	/* ���ڻ�е�level */
	int now_lv;
	/* ���´������л�ʱ�Ƿ���Ҫ�ı�LEVEL */
	char lv_change; 
	struct TASKLEVEL level[MAX_TASKLEVELS];
	struct TASK tasks0[MAX_TASKS];
};

extern struct TASKCTL* taskctl;
extern struct TIMER* task_timer;

/* �����������ʼ������������������ڴ��ַ */
struct TASK* task_init(struct MEMMAN* memman);
/* �������룬����0��ʾ������������ */
struct TASK* task_alloc(void);
/* �������� */
void task_run(struct TASK* task, int level, int priority); 
/* �Ե�ǰ�����б��е���������л� */
void task_switch(void);
/* �������� */
void task_sleep(struct TASK* task);
/* ���ص�ǰ��ŵ�����ָ�� */
struct TASK* task_now(void);

/* console.c */

struct CONSOLE
{
	struct SHEET* sht;
	int cur_x, cur_y, cur_c;
	struct TIMER* timer;
};

/* ����̨���� */
void console_task(struct SHEET* sheet, int memtotal);
/* ��cons����̨��ӡ�ַ�chr��moveΪ1ʱ������һλ */
void cons_putchar(struct CONSOLE* cons, int chr, char move);
/* ���к��������������һ�е�ʱ����Զ����� */
void cons_newline(struct CONSOLE* cons);
/* ����cmdlineƥ�������ض������� */
void cons_runcmd(char* cmdline, struct CONSOLE* cons, int* fat, int memtotal);
/* mem��ʾ�ڴ����� */
void cmd_mem(struct CONSOLE* cons, int memtotal);
/* cls�������� */
void cmd_cls(struct CONSOLE* cons);
/* dir��ʾĿ¼���� */
void cmd_dir(struct CONSOLE* cons);
/* type�����ʾ�ļ����� */ 
void cmd_type(struct CONSOLE* cons, int* fat, char* cmdline);
/* exit���� */
void cmd_exit(struct CONSOLE* cons, int* fat);
/* �趨����ģʽ */
void cmd_langmode(struct CONSOLE* cons, char* cmdline);
/* ����Ӧ�ó��� */
void cmd_start(struct CONSOLE* cons, char* cmdline, int memtotal);
/* ��������������Ӧ�ó��� */
void cmd_ncst(struct CONSOLE* cons, char* cmdline, int memtotal);
/* Ӧ�ó��� */
int cmd_app(struct CONSOLE* cons, int* fat, char* cmdline);
/* ��ӡ�ַ���s����0���� */
void cons_putstr0(struct CONSOLE* cons, char* s);
/* �����ַ�������l����ӡ�ַ��� */
void cons_putstr1(struct CONSOLE* cons, char* s, int l); 
/* ����ϵͳAPI */
int* hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax); 
/* �����쳣�ж� */
int* inthandler0d(int *esp);
/* �����ջ�쳣 */
int* inthandler0c(int* esp);
/* ����API */
void hrb_api_linewin(struct SHEET* sht, int x0, int y0, int x1, int y1, int col);
 
/*file.c*/

/* �ļ���Ϣ */
struct FILEINFO
{
	/* �ļ�������׺������ */
	unsigned char name[8], ext[3], type;
	/* ΢������ */
	char reserve[10];
	/* ���ڼ�ʱ�� */
	unsigned short time, date, clustno;
	/* �ļ���С */ 
	unsigned int size;
};

/* ������ӳ���е�FAT��ѹ�� */
void file_readfat(int* fat, unsigned char* img);
/* ���ļ������ݶ����ڴ� */
void file_loadfile(int clustno, int size, char* buf, int* fat, char* img);
/* ����name�����ļ�, �����ļ���Ϣ��0Ϊû���ҵ� */
struct FILEINFO* file_search(char* name, struct FILEINFO* finfo, int max);

/* window.c */

/* ���ƴ��� */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
/* �ڴ�������ӱ��� */
void make_wtitle8(unsigned char* buf, int xsize, char* title, char act);
/* ���Ʊ༭�� */
void make_textbox8(struct SHEET* sht, int x0, int y0, int sx, int sy, int c);
/* Ϳ������д�ַ���ˢ�� */
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
/* ���ݴ����Ƿ��ı������ɫ */
void change_wtitle8(struct SHEET* sht, char act);
 
/* bootpack.c */

/* ��һ������ */
struct TASK* open_constask(struct SHEET* sht, unsigned int memtotal);
/* �¿���һ�������� */
struct SHEET* open_console(struct SHTCTL* shtctl, unsigned int memtotal);

