.text
.globl add
add:
	pushq %rbp
	movq %rsp, %rbp
	movq %rcx, 16(%rbp)
	movq %rdx, 24(%rbp)
	movq 24(%rbp), %r10
	addq %r10, 16(%rbp)
	addq $5, 16(%rbp)
	movq 16(%rbp), %rax
	jmp .L0
.L0:
	leave
	ret
.globl minus
minus:
	pushq %rbp
	movq %rsp, %rbp
	movq %rcx, 16(%rbp)
	pushq $50
	movq $2, %rdx
	movq $1, %rcx
	subq $32, %rsp
	call add
	movq %rax, %r10
	subq %r10, -8(%rbp)
	movq 16(%rbp), %r10
	subq %r10, -8(%rbp)
	movq -8(%rbp), %rax
	jmp .L1
.L1:
	leave
	ret
.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $32, %rsp
	call __main
	movq $1, %rdx
	movq $1, %rcx
	subq $32, %rsp
	call add
	movq %rax, %rcx
	subq $32, %rsp
	call minus
	movq %rax, %rax
	jmp .L2
.L2:
	leave
	ret
