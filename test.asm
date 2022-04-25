section .text
global	_start

_start:
	mov     rax, 3
        mov     rbx, 10
Movs:
        mov     rax, rbx

pushes:
        mov     r13, [rax]
        mov     r13, [0x123]
        mov     r13, [0x1234]
        mov     r13, [0x123456]

        push    r13

        push    rax
        push    rbx
        push    rcx
        push    rdx

        pop     rax
        pop     rbx

        jmp     Movs
        jmp     pushes

        nop

        ret