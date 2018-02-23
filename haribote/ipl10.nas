; haribote-ipl
; TAB = 4

CYLS	EQU	10		; ��������cylinders(����)=20

ORG	0x7c00			; ָ�������װ�ص�ַ

; ���µļ�����;���׼FAT12��ʽ������

	JMP		entry
	DB		0x90
	DB		"HARIBOTE"	; �����������ƿ�����������ַ���
	DW		512		; ÿ�������Ĵ�С
	DB		1		; �صĴ�С(1������)
	DW		1		; FAT����ʼλ��(һ��ӵ�һ��������ʼ)
	DB		2		; FAT�ĸ���
	DW		224		; ��Ŀ¼�Ĵ�С
	DW		2880		; �ô��̵Ĵ�С
	DB		0xf0		; �ô��̵�����
	DW		9		; FAT�ĳ���
	DW		18		; 1���ŵ���18������
	DW		2		; ��ͷ��Ϊ2
	DD		0		; ��ʹ�÷���
	DD		2880		; ��дһ�δ��̴�С
	DB		0, 0, 0x29	; ���岻��
	DD		0xffffffff	; ������
	DB		"HARIBOTEOS "	; ���̸�ʽ������(11�ֽ�)
	DB		"FAT12   "	; ���̸�ʽ����(8�ֽ�)
	RESB	18			; �ȿճ�18�ֽ�


; �������

entry:
		MOV 	AX, 0		; ��ʼ���Ĵ���
		MOV 	SS, AX
		MOV 	SP, 0x7c00
		MOV 	DS, AX
	
; ��ʼ������

		MOV 	AX, 0x0820
		MOV 	ES, AX
		MOV 	CH, 0		; ����0
		MOV 	DH, 0		; ��ͷ0
		MOV 	CL, 2		; ����2
readloop:
		MOV	SI, 0		; ��¼ʧ�ܴ����ļĴ���
retry:
		MOV 	AH, 0x02	; AH = 0x02 : ����
		MOV 	AL, 1		; �����̵�����
		MOV 	BX, 0
		MOV 	DL, 0x00	; A������
		INT	0x13		; ���ô���BIOS��0x13����������
		JNC	next		; �������δ��������ת��next
		
		ADD	SI, 1		; ��SI��1
		CMP	SI, 5		; �Ƚ�SI��5
		JAE	error		; SI >= 5ʱ���������ԣ���ת��error
		
		MOV	AH, 0x00	; ϵͳ��λ
		MOV	DL, 0x00
		INT 	0x13		; ���ô���BIOS��0x13����������
		JMP	retry
next:
		MOV	AX, ES		; ���ڴ��ַ����0x200,��512�ֽ�
		ADD	AX, 0x0020
		MOV	ES, AX		; ��Ϊû��ADD ES, 0x020ָ�����������΢�Ƹ���
		ADD	CL, 1		; CL��1
		CMP	CL, 18		; �Ƚ�CL��18
		JBE	readloop	; ���CL <= 18 ��ת��readloop
		MOV	CL, 1
		ADD	DH, 1
		CMP	DH, 2
		JB	readloop	; ���DH < 2������ת��readloop
		MOV	DH, 0
		ADD	CH, 1
		CMP	CH, CYLS
		JB	readloop	; ���CH < CYLS������ת��readloop
		
		MOV	[0x0ff0], CH	; ��CYLS��ֵд���ڴ��ַ0x0ff��
		JMP	0xc200		; ��ת��0xc200��ַ��������ϵͳ���
		
error:
		MOV 	SI, msg
putloop:
		MOV 	Al, [SI]
		ADD 	SI, 1		; ��SI��1
		CMP 	AL, 0
		
		JE	fin
		MOV 	AH, 0x0e	; ��ʾһ������
		MOV 	BX, 15		; ָ���ַ�����ɫ
		INT 	0x10		; �����Կ�BIOS
		JMP 	putloop
fin:
		HLT			; ��CPUֹͣ���ȴ�ָ��
		JMP 	fin		; ����ѭ��
msg:
		DB	0x0a, 0x0a	; ��������
		DB	"load error"
		DB	0x0a
		DB	0
		
		RESB	0x7dfe-$	; ��ָ����ַ���Ϊ0

		DB	0x55, 0xaa
