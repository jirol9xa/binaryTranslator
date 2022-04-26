section .text
global	_start

_start:
	mov     rax, 3
        mov     rbx, 10
Movs:
        mov     rax, rbx

pushes:
        mov     r15, 0x123456

        push    r13

        mov     r15, [r13]
        ;push    [r13]

        add     r13, r15
        sub     r13, r15
        ;mul     r13, r15
        ;div     r13, r15

        nop

        ret