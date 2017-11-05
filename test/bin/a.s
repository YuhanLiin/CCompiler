.text
.globl b
b:
	pushq %rbp
	movq %rsp, %rbp
	movq %rcx, 16(%rbp)
	movq %rdx, 24(%rbp)
	movq %r8, 32(%rbp)
	movq %r9, 40(%rbp)
	pushq 48(%rbp)
	subq $32, %rsp
	movq 40(%rbp), %r9
	movq 32(%rbp), %r8
	movq 24(%rbp), %rdx
	movq 16(%rbp), %rcx
	call a
	movq %rax, %rax
	jmp .L0
.L0:
	leave
	ret
