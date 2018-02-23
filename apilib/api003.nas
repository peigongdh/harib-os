[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api003.nas"]

	GLOBAL	_api_putstr1

[SECTION .text]

; void api_putstr1(char *s, int l);
_api_putstr1:
	PUSH	EBX
	MOV	EDX, 3
	MOV	EBX, [ESP+ 8]	; s
	MOV	ECX, [ESP+12]	; l
	INT	0x40
	POP	EBX
	RET