[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api020.nas"]

	GLOBAL	_api_beep

[SECTION .text]

; void api_beep(int tone)
_api_beep:
	MOV	EDX, 20
	MOV	EAX, [ESP + 4]
	INT	0x40
	RET