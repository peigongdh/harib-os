[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api010.nas"]

	GLOBAL	_api_free

[SECTION .text]

; void api_free(char* addr, int size)
_api_free:
	PUSH	EBX
	MOV	EDX, 10
	MOV	EBX, [CS:0x0020]; memmanµÄµØÖ·
	MOV	EAX, [ESP + 8]	; addr
	MOV	ECX, [ESP + 12]	; size
	INT	0x40
	POP	EBX
	RET