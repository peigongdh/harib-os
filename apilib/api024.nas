[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api024.nas"]

	GLOBAL	_api_fsize

[SECTION .text]

; int api_fsize(int fhandle, int mode)
_api_fsize:
	MOV	EDX, 24
	MOV	EAX, [ESP + 4]
	MOV	ECX, [ESP + 8]
	INT	0x40
	RET