.text
.globl a
a:
	pushq %rbp
	movq %rsp, %rbp
	movq %rcx, 16(%rbp)
	movq %rdx, 24(%rbp)
	movq %r8, 32(%rbp)
	movq %r9, 40(%rbp)
	movq 16(%rbp), %rax
	jmp .L0
.L0:
	leave
	ret
.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $32, %rsp
	call __main
	pushq $5
	subq $32, %rsp
	movq $4, %r9
	movq $3, %r8
	movq $2, %rdx
	movq $1, %rcx
	call a
	movq %rax, %rax
	jmp .L1
.L1:
	leave
	ret
