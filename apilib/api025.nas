[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api024.nas"]

	GLOBAL	_api_fread

[SECTION .text]

; int api_fread(char* buf, int maxsize, int fhandle)
_api_fread:
	PUSH	EBX
	MOV	EDX, 25
	MOV	EBX, [ESP + 8]
	MOV	ECX, [ESP + 12]
	MOV	EAX, [ESP + 16]
	INT	0x40
	POP	EBX
	RET