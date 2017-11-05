	.file	"params.c"
	.text
	.globl	p
	.def	p;	.scl	2;	.type	32;	.endef
p:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
	movl	%ecx, 16(%rbp)
	movl	%edx, 24(%rbp)
	movl	%r8d, 32(%rbp)
	movl	%r9d, 40(%rbp)
	movl	48(%rbp), %eax
	movl	%eax, %ecx
	call	b
	leave
	ret
	.globl	b
	.def	b;	.scl	2;	.type	32;	.endef
b:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp
	movl	%ecx, 16(%rbp)
	movl	$5, 32(%rsp)
	movl	$4, %r9d
	movl	$3, %r8d
	movl	$2, %edx
	movl	16(%rbp), %ecx
	call	p
	leave
	ret
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.1.0"
