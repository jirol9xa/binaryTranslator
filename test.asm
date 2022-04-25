section .text
global	_start

_start:
	mov     rax, 3
        mov     rbx, 10
Movs:
        mov     rax, rbx

pushes:
        push    4
        push    rcx
        pop     rax
        pop     rbx

        jmp     Movs
        jmp     pushes

        nop

        ret