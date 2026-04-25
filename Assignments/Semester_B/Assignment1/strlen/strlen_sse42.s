# 211388921 Nadav Ziri
.section .data


.section .text
.globl strlen_sse42
.type strlen_sse42, @function

strlen_sse42:
    pxor    %xmm0, %xmm0
    xorq    %rax, %rax

.loop:
    pcmpistri $0x08, (%rdi,%rax), %xmm0
    leaq    16(%rax), %rax
    jnz     .loop                           

    leaq    -16(%rax,%rcx), %rax       # length = (offset - 16) + ecx
    ret
