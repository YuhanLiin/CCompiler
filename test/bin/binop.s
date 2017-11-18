.text
.globl add
add:
	pushq %rbp
	movq %rsp, %rbp
	movq %rcx, 16(%rbp)
	movq %rdx, 24(%rbp)
	pushq 16(%rbp)
	movq 24(%rbp), %r10
	addq %r10, -8(%rbp)
	pushq -8(%rbp)
	addq $5, -16(%rbp)
	movq -16(%rbp), %rax
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
	pushq -8(%rbp)
	movq 16(%rbp), %r10
	subq %r10, -48(%rbp)
	movq -48(%rbp), %rax
	jmp .L1
.L1:
	leave
	ret
.globl multi
multi:
	pushq %rbp
	movq %rsp, %rbp
	movq %rcx, 16(%rbp)
	movq %rdx, 24(%rbp)
	movq %r8, 32(%rbp)
	movq %r9, 40(%rbp)
	pushq 16(%rbp)
	pushq 24(%rbp)
	pushq 32(%rbp)
	movq 40(%rbp), %r10
	addq %r10, -24(%rbp)
	movq -24(%rbp), %r10
	movq -16(%rbp), %rax
	imulq %r10
	movq %rax, %r10
	addq %r10, -8(%rbp)
	movq -8(%rbp), %rax
	jmp .L2
.L2:
	leave
	ret
.globl div
div:
	pushq %rbp
	movq %rsp, %rbp
	pushq $100
	movq $1, %rdx
	movq $1, %rcx
	subq $32, %rsp
	call add
	pushq %rax
	movq $1, %rcx
	subq $32, %rsp
	call minus
	pushq %rax
	pushq -88(%rbp)
	movq $3, %r9
	movq -48(%rbp), %r8
	movq $5, %rdx
	movq $0, %rcx
	subq $32, %rsp
	call multi
	movq %rax, %r10
	movq -8(%rbp), %rax
	cqto
	idivq %r10
	movq %rax, %rax
	jmp .L3
.L3:
	leave
	ret
.globl idiv
idiv:
	pushq %rbp
	movq %rsp, %rbp
	pushq $100
	movq $1, %rdx
	movq $1, %rcx
	subq $32, %rsp
	call add
	pushq %rax
	movq $1, %rcx
	subq $32, %rsp
	call minus
	pushq %rax
	pushq -88(%rbp)
	movq $3, %r9
	movq -48(%rbp), %r8
	movq $5, %rdx
	movq $0, %rcx
	subq $32, %rsp
	call multi
	movq %rax, %r10
	movq -8(%rbp), %rax
	movq $0, %rdx
	divq %r10
	movq %rax, %rax
	jmp .L4
.L4:
	leave
	ret
.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $32, %rsp
	call __main
	subq $32, %rsp
	call div
	pushq %rax
	movq -72(%rbp), %rcx
	subq $32, %rsp
	call minus
	pushq %rax
	movq -112(%rbp), %rdx
	movq $1, %rcx
	subq $32, %rsp
	call add
	movq %rax, %rax
	jmp .L5
.L5:
	leave
	ret
