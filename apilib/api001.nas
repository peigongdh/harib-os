[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api001.nas"]

	GLOBAL	_api_putchar

[SECTION .text]

; void api_putchar(int c)
_api_putchar:
	MOV	EDX, 1
	MOV	AL, [ESP + 4]	; c
	INT	0x40
	RET