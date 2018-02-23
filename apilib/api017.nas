[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api017.nas"]

	GLOBAL	_api_inittimer

[SECTION .text]

; void api_inittimer(int timer, int data)
_api_inittimer:
	PUSH	EBX
	MOV	EDX, 17
	MOV	EBX, [ESP + 8]	; timer
	MOV	EAX, [ESP + 12]	; data
	INT	0x40
	POP	EBX
	RET