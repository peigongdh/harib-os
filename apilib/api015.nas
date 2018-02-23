[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api015.nas"]

	GLOBAL	_api_getkey

[SECTION .text]

; void api_getkey(int mode)
_api_getkey:
	MOV	EDX, 15
	MOV	EAX, [ESP + 4]	; mode
	INT	0x40
	RET