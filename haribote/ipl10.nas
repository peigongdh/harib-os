; haribote-ipl
; TAB = 4

CYLS	EQU	10		; 声明常数cylinders(柱面)=20

ORG	0x7c00			; 指明程序的装载地址

; 以下的记述用途与标准FAT12格式的软盘

	JMP		entry
	DB		0x90
	DB		"HARIBOTE"	; 启动区的名称可以是任意的字符串
	DW		512		; 每个扇区的大小
	DB		1		; 簇的大小(1个扇区)
	DW		1		; FAT的起始位置(一般从第一个扇区开始)
	DB		2		; FAT的个数
	DW		224		; 根目录的大小
	DW		2880		; 该磁盘的大小
	DB		0xf0		; 该磁盘的种类
	DW		9		; FAT的长度
	DW		18		; 1个磁道有18个扇区
	DW		2		; 磁头数为2
	DD		0		; 不使用分区
	DD		2880		; 重写一次磁盘大小
	DB		0, 0, 0x29	; 意义不明
	DD		0xffffffff	; 卷标号码
	DB		"HARIBOTEOS "	; 磁盘格式化名称(11字节)
	DB		"FAT12   "	; 磁盘格式名称(8字节)
	RESB	18			; 先空出18字节


; 程序核心

entry:
		MOV 	AX, 0		; 初始化寄存器
		MOV 	SS, AX
		MOV 	SP, 0x7c00
		MOV 	DS, AX
	
; 初始化磁盘

		MOV 	AX, 0x0820
		MOV 	ES, AX
		MOV 	CH, 0		; 柱面0
		MOV 	DH, 0		; 磁头0
		MOV 	CL, 2		; 扇区2
readloop:
		MOV	SI, 0		; 记录失败次数的寄存器
retry:
		MOV 	AH, 0x02	; AH = 0x02 : 读盘
		MOV 	AL, 1		; 所读盘的扇区
		MOV 	BX, 0
		MOV 	DL, 0x00	; A驱动器
		INT	0x13		; 调用磁盘BIOS的0x13函数，读盘
		JNC	next		; 如果读盘未出错，则跳转至next
		
		ADD	SI, 1		; 往SI加1
		CMP	SI, 5		; 比较SI和5
		JAE	error		; SI >= 5时，放弃重试，跳转至error
		
		MOV	AH, 0x00	; 系统复位
		MOV	DL, 0x00
		INT 	0x13		; 调用磁盘BIOS的0x13函数，读盘
		JMP	retry
next:
		MOV	AX, ES		; 把内存地址后移0x200,即512字节
		ADD	AX, 0x0020
		MOV	ES, AX		; 因为没有ADD ES, 0x020指令，所以这里稍微绕个弯
		ADD	CL, 1		; CL加1
		CMP	CL, 18		; 比较CL与18
		JBE	readloop	; 如果CL <= 18 跳转至readloop
		MOV	CL, 1
		ADD	DH, 1
		CMP	DH, 2
		JB	readloop	; 如果DH < 2，则跳转至readloop
		MOV	DH, 0
		ADD	CH, 1
		CMP	CH, CYLS
		JB	readloop	; 如果CH < CYLS，则跳转至readloop
		
		MOV	[0x0ff0], CH	; 将CYLS的值写到内存地址0x0ff中
		JMP	0xc200		; 跳转至0xc200地址，即操作系统入口
		
error:
		MOV 	SI, msg
putloop:
		MOV 	Al, [SI]
		ADD 	SI, 1		; 给SI加1
		CMP 	AL, 0
		
		JE	fin
		MOV 	AH, 0x0e	; 显示一个文字
		MOV 	BX, 15		; 指定字符串颜色
		INT 	0x10		; 调用显卡BIOS
		JMP 	putloop
fin:
		HLT			; 让CPU停止，等待指令
		JMP 	fin		; 无限循环
msg:
		DB	0x0a, 0x0a	; 换行两次
		DB	"load error"
		DB	0x0a
		DB	0
		
		RESB	0x7dfe-$	; 将指定地址填充为0

		DB	0x55, 0xaa
