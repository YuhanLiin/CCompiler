	.file	"basic.c"
	.def	__main;	.scl	2;	.type	32;	.endef
	.text
	.globl	main
	.def	main;	.scl	2;	.type	32;	.endef
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
	movl	%ecx, 16(%rbp)
	call	__main
	cmpl	$5, 16(%rbp)
	jle	.L2
	movl	16(%rbp), %eax
	jmp	.L3
.L2:
	movl	$1, %eax
.L3:
	leave
	ret
	.globl	x
	.def	x;	.scl	2;	.type	32;	.endef
x:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	%ecx, 16(%rbp)
	cmpl	$5, 16(%rbp)
	jle	.L5
	movl	16(%rbp), %eax
	jmp	.L6
.L5:
	movl	$1, %eax
.L6:
	popq	%rbp
	ret
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.1.0"
