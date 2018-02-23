[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api023.nas"]

	GLOBAL	_api_fseek

[SECTION .text]

; int api_fseek(int fhandle, int offset, int mode)
_api_fseek:
	PUSH	EBX
	MOV	EDX, 23
	MOV	EAX, [ESP + 8]
	MOV	EBX, [ESP + 12]
	MOV	ECX, [ESP + 16]
	INT	0x40
	POP	EBX
	RET