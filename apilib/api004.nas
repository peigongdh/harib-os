[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api004.nas"]

	GLOBAL	_api_end

[SECTION .text]

; void api_end(void)
_api_end:
	MOV	EDX, 4
	INT	0x40