[INSTRSET "i486p"]
;	PIC关闭一切中断
;	根据AT兼容机的规格，如果要初始化PIC，
;	必须在CLI之前进行，否则有时会挂起
;	随后进行PIC的初始化

	BOTPAK	EQU		0x00280000
	DSKCAC	EQU		0x00100000
	DSKCAC0	EQU		0x00008000
	VBEMODE	EQU		0x105
	
	; BOOT_INDO
	CYLS	EQU		0x0ff0
	LEDS	EQU		0x0ff1
	VMODE	EQU		0x0ff2
	SCRNX	EQU		0x0ff4
	SCRNY	EQU		0x0ff6
	VRAM	EQU		0x0ff8
	
	ORG		0xc200

	; 确认VBE是否存在
	MOV		AX, 0x9000
	MOV		ES, AX
	MOV		DI, 0
	MOV		AX, 0x4f00
	INT		0x10
	CMP		AX, 0x004f
	JNE		scrn320

	; 检查VBE的版本
	MOV		AX, [ES:DI + 4]
	CMP		AX, 0x0200
	JB		scrn320

	; 取得画面模式信息
	MOV		CX, VBEMODE
	MOV		AX, 0x4f01
	INT		0x10
	CMP		AX, 0x004f
	JNE		scrn320

	; 画面模式信息的确认
	CMP		BYTE [ES:DI + 0x19], 8
	JNE		scrn320
	CMP		BYTE [ES:DI + 0x1b], 4
	JNE		scrn320
	MOV		AX, [ES:DI + 0x00]
	AND		AX, 0x0080
	JZ		scrn320

	; 画面模式的切换
	MOV		BX, VBEMODE + 0x4000		; VBE的640x480x8bit彩色
	MOV		AX, 0x4f02
	INT		0x10
	MOV		BYTE [VMODE], 8
	MOV		AX, [ES:DI + 0x12]
	MOV		[SCRNX], AX
	MOV		AX, [ES:DI + 0x14]
	MOV		[SCRNY], AX
	MOV		EAX, [ES:DI + 0x28]
	MOV		[VRAM], EAX
	JMP		keystatus

scrn320:
	; 跳转到320 * 200的分辨率
	MOV		AL,0x13
	MOV		AH,0x00
	INT		0x10
	MOV		BYTE [VMODE], 8
	MOV		WORD [SCRNX], 320
	MOV		WORD [SCRNY], 200
	MOV		DWORD [VRAM], 0x000a0000

keystatus:
	MOV		AH, 0x02
	INT		0x16			; keyboard BIOS
	MOV		[LEDS], AL
	

	MOV		AL, 0xff
	OUT		0x21, AL	;	禁止主PIC的全部中断
	NOP							;	如果连续执行OUT指令，有些机种会无法正常运行
	OUT		0xa1, AL	;	禁止从PIC的全部中断
	
	CLI							; 禁止CPU级别的中断

;	为了让CPU能够访问1MB以上的内存空间，设定A20GATE

	CALL	waitkbdout
	MOV		AL, 0xd1
	OUT		0x64, AL
	CALL	waitkbdout
	MOV		AL, 0xdf
	OUT		0x60, AL
	CALL	waitkbdout

;	以上部分等同于下C语言
;	#define KEYCMD_WRITE_OUOTPORT		0xd1
;	#define KBC_OUTPORT_A20G_ENABLE	0xdf
;	
;	wait_KBC_sendready();
;	io_out8(PORT_KEYCMD, KEYCMD_WRITE_OUTPORT);
;	wait_KBC_sendready();
;	io_out8(PORT_KEYDAT, KBC_OUTPORT_A20G_ENABLE);
;	wait_KBC_sendready();

;	切换到保护模式

	[INSTRSET "i486p"]			;	"想要使用486指令"的叙述
	LGDT	[GDTR0]						;	设定临时GDT
	MOV		EAX, CR0
	AND		EAX, 0x7fffffff		;	设bit31为0（为了禁止颁）
	OR		EAX, 0x00000001		;	设bit0为1（为了切换到保护模式）
	MOV		CR0, EAX
	JMP		pipelineflush

pipelineflush:
	MOV		AX, 1*8						;	可读写的段32bit
	MOV		DS, AX
	MOV		ES, AX
	MOV		FS, AX
	MOV		GS, AX
	MOV		SS, AX
	
;	bootpack的传送
	MOV		ESI, bootpack			;	转送源
	MOV		EDI, BOTPAK				;	转送目的地
	MOV		ECX, 512*1024/4
	CALL	memcpy
	
;	磁盘数据追踪转送到它本来的位置去

;	首先从启动扇区开始

	MOV		ESI, 0x7c00				;	转送源
	MOV		EDI, DSKCAC				; 转送目的地
	MOV		ECX, 512/4
	CALL	memcpy
	
;	所有剩下的
	MOV		ESI, DSKCAC0+512	;	转送源
	MOV		EDI, DSKCAC+512		; 转送目的地
	MOV		ECX, 0
	MOV		CL, BYTE[CYLS]
	IMUL	ECX, 512*18*2/4		;	从柱面数变换为字节数/4
	SUB		ECX, 512/4				;	减去IPL
	CALL	memcpy

;	必须由asmhead来完成的工作，至此全部完毕
;	以后就交由bootpack来完成

;	bootpack的启动
	MOV		EBX, BOTPAK
	MOV		ECX, [EBX+16]
	ADD		ECX, 3
	SHR		ECX, 2						;	ECX /= 4;
	JZ		skip							;	没有要转送的东西时
	MOV		ESI, [EBX+20]			;	转送源
	ADD		ESI, EBX
	MOV		EDI, [EBX+12]			; 转送目的地
	CALL	memcpy

skip:
	MOV		ESP, [EBX+12]
	JMP		DWORD 2*8:0x0000001b
	
waitkbdout:
	IN		AL, 0x64
	AND		AL, 0x02
	IN		AL, 0x60					;	空读（为了清空数据接收缓冲区中的垃圾数据）
	JNZ		waitkbdout				;	AND的结果如果不是0，就跳到waitkbdout
	RET
	
memcpy:
	MOV		EAX, [ESI]
	ADD		ESI, 4
	MOV		[EDI], EAX
	ADD		EDI, 4
	SUB		ECX, 1
	JNZ		memcpy						;	减法运算的结果如果不是0，就跳转到memcpy
	RET
	
	ALIGNB	16	
GDT0:
	RESB		8								;	NULL selector
	DW			0xffff, 0x0000, 0x9200, 0x00cf
													;	可以读写的段（segment）32bit
	DW			0xffff, 0x0000, 0x9a28, 0x0047
													;	可以执行的段（segment）32bit（bootpack用）												
	DW			0
GDTR0:
	DW			8*3-1
	DD			GDT0
	
	ALIGNB	16
bootpack:
