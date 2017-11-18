	.file	"arith.c"
	.text
	.globl	sub
	.def	sub;	.scl	2;	.type	32;	.endef
sub:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	%ecx, 16(%rbp)
	movl	%edx, 24(%rbp)
	movl	16(%rbp), %eax
	subl	24(%rbp), %eax
	popq	%rbp
	ret
	.globl	mul
	.def	mul;	.scl	2;	.type	32;	.endef
mul:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	%ecx, 16(%rbp)
	movl	%edx, 24(%rbp)
	movl	16(%rbp), %eax
	imull	24(%rbp), %eax
	popq	%rbp
	ret
	.globl	imul
	.def	imul;	.scl	2;	.type	32;	.endef
imul:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	%ecx, 16(%rbp)
	movl	%edx, 24(%rbp)
	movl	16(%rbp), %eax
	imull	24(%rbp), %eax
	popq	%rbp
	ret
	.globl	div
	.def	div;	.scl	2;	.type	32;	.endef
div:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	%rcx, 16(%rbp)
	movq	%rdx, 24(%rbp)
	movq	16(%rbp), %rax
	cqto
	idivq	24(%rbp)
	popq	%rbp
	ret
	.globl	idiv
	.def	idiv;	.scl	2;	.type	32;	.endef
idiv:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	%ecx, 16(%rbp)
	movl	%edx, 24(%rbp)
	movl	16(%rbp), %eax
	movl	$0, %edx
	divl	24(%rbp)
	popq	%rbp
	ret
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.1.0"
