[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api002.nas"]

	GLOBAL	_api_putstr0

[SECTION .text]

; void api_putstr0(char* s)
_api_putstr0:
	PUSH	EBX
	MOV	EDX, 2
	MOV	EBX, [ESP + 8]	; s
	INT	0x40
	POP	EBX
	RET