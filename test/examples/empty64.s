	.file	"empty64.c"
	.text
	.globl	hey
	.def	hey;	.scl	2;	.type	32;	.endef
hey:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	%ecx, 16(%rbp)
	movl	%edx, 24(%rbp)
	movl	16(%rbp), %eax
	popq	%rbp
	ret
	.globl	hey2
	.def	hey2;	.scl	2;	.type	32;	.endef
hey2:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	%ecx, 16(%rbp)
	movl	%edx, 24(%rbp)
	movl	24(%rbp), %eax
	popq	%rbp
	ret
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.1.0"
