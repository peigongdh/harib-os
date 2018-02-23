[INSTRSET "i486p"]
;	PIC�ر�һ���ж�
;	����AT���ݻ��Ĺ�����Ҫ��ʼ��PIC��
;	������CLI֮ǰ���У�������ʱ�����
;	������PIC�ĳ�ʼ��

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

	; ȷ��VBE�Ƿ����
	MOV		AX, 0x9000
	MOV		ES, AX
	MOV		DI, 0
	MOV		AX, 0x4f00
	INT		0x10
	CMP		AX, 0x004f
	JNE		scrn320

	; ���VBE�İ汾
	MOV		AX, [ES:DI + 4]
	CMP		AX, 0x0200
	JB		scrn320

	; ȡ�û���ģʽ��Ϣ
	MOV		CX, VBEMODE
	MOV		AX, 0x4f01
	INT		0x10
	CMP		AX, 0x004f
	JNE		scrn320

	; ����ģʽ��Ϣ��ȷ��
	CMP		BYTE [ES:DI + 0x19], 8
	JNE		scrn320
	CMP		BYTE [ES:DI + 0x1b], 4
	JNE		scrn320
	MOV		AX, [ES:DI + 0x00]
	AND		AX, 0x0080
	JZ		scrn320

	; ����ģʽ���л�
	MOV		BX, VBEMODE + 0x4000		; VBE��640x480x8bit��ɫ
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
	; ��ת��320 * 200�ķֱ���
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
	OUT		0x21, AL	;	��ֹ��PIC��ȫ���ж�
	NOP							;	�������ִ��OUTָ���Щ���ֻ��޷���������
	OUT		0xa1, AL	;	��ֹ��PIC��ȫ���ж�
	
	CLI							; ��ֹCPU������ж�

;	Ϊ����CPU�ܹ�����1MB���ϵ��ڴ�ռ䣬�趨A20GATE

	CALL	waitkbdout
	MOV		AL, 0xd1
	OUT		0x64, AL
	CALL	waitkbdout
	MOV		AL, 0xdf
	OUT		0x60, AL
	CALL	waitkbdout

;	���ϲ��ֵ�ͬ����C����
;	#define KEYCMD_WRITE_OUOTPORT		0xd1
;	#define KBC_OUTPORT_A20G_ENABLE	0xdf
;	
;	wait_KBC_sendready();
;	io_out8(PORT_KEYCMD, KEYCMD_WRITE_OUTPORT);
;	wait_KBC_sendready();
;	io_out8(PORT_KEYDAT, KBC_OUTPORT_A20G_ENABLE);
;	wait_KBC_sendready();

;	�л�������ģʽ

	[INSTRSET "i486p"]			;	"��Ҫʹ��486ָ��"������
	LGDT	[GDTR0]						;	�趨��ʱGDT
	MOV		EAX, CR0
	AND		EAX, 0x7fffffff		;	��bit31Ϊ0��Ϊ�˽�ֹ�䣩
	OR		EAX, 0x00000001		;	��bit0Ϊ1��Ϊ���л�������ģʽ��
	MOV		CR0, EAX
	JMP		pipelineflush

pipelineflush:
	MOV		AX, 1*8						;	�ɶ�д�Ķ�32bit
	MOV		DS, AX
	MOV		ES, AX
	MOV		FS, AX
	MOV		GS, AX
	MOV		SS, AX
	
;	bootpack�Ĵ���
	MOV		ESI, bootpack			;	ת��Դ
	MOV		EDI, BOTPAK				;	ת��Ŀ�ĵ�
	MOV		ECX, 512*1024/4
	CALL	memcpy
	
;	��������׷��ת�͵���������λ��ȥ

;	���ȴ�����������ʼ

	MOV		ESI, 0x7c00				;	ת��Դ
	MOV		EDI, DSKCAC				; ת��Ŀ�ĵ�
	MOV		ECX, 512/4
	CALL	memcpy
	
;	����ʣ�µ�
	MOV		ESI, DSKCAC0+512	;	ת��Դ
	MOV		EDI, DSKCAC+512		; ת��Ŀ�ĵ�
	MOV		ECX, 0
	MOV		CL, BYTE[CYLS]
	IMUL	ECX, 512*18*2/4		;	���������任Ϊ�ֽ���/4
	SUB		ECX, 512/4				;	��ȥIPL
	CALL	memcpy

;	������asmhead����ɵĹ���������ȫ�����
;	�Ժ�ͽ���bootpack�����

;	bootpack������
	MOV		EBX, BOTPAK
	MOV		ECX, [EBX+16]
	ADD		ECX, 3
	SHR		ECX, 2						;	ECX /= 4;
	JZ		skip							;	û��Ҫת�͵Ķ���ʱ
	MOV		ESI, [EBX+20]			;	ת��Դ
	ADD		ESI, EBX
	MOV		EDI, [EBX+12]			; ת��Ŀ�ĵ�
	CALL	memcpy

skip:
	MOV		ESP, [EBX+12]
	JMP		DWORD 2*8:0x0000001b
	
waitkbdout:
	IN		AL, 0x64
	AND		AL, 0x02
	IN		AL, 0x60					;	�ն���Ϊ��������ݽ��ջ������е��������ݣ�
	JNZ		waitkbdout				;	AND�Ľ���������0��������waitkbdout
	RET
	
memcpy:
	MOV		EAX, [ESI]
	ADD		ESI, 4
	MOV		[EDI], EAX
	ADD		EDI, 4
	SUB		ECX, 1
	JNZ		memcpy						;	��������Ľ���������0������ת��memcpy
	RET
	
	ALIGNB	16	
GDT0:
	RESB		8								;	NULL selector
	DW			0xffff, 0x0000, 0x9200, 0x00cf
													;	���Զ�д�ĶΣ�segment��32bit
	DW			0xffff, 0x0000, 0x9a28, 0x0047
													;	����ִ�еĶΣ�segment��32bit��bootpack�ã�												
	DW			0
GDTR0:
	DW			8*3-1
	DD			GDT0
	
	ALIGNB	16
bootpack:
