.section .text
.global pshufb_xmm4_xmm0
.type pshufb_xmm4_xmm0, @function

pshufb_xmm4_xmm0:
    push %eax
    push %ebx
    subl $32, %esp
    movl %esp, %eax
    movdqu %xmm4, (%esp)
    movdqu %xmm0, 16(%esp)
    movl %esp, %ebx
    addl $16, %ebx
    push %ebx
    push %eax
    call pshufb
    pop %eax
    pop %ebx
    movdqu (%esp), %xmm4
    addl $32, %esp
    pop %eax
    pop %ebx
ret
