	.file	"empty64.c"
	.text
	.globl	hey
	.def	hey;	.scl	2;	.type	32;	.endef
hey:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	%rcx, 16(%rbp)
	movq	16(%rbp), %rax
	popq	%rbp
	ret
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.1.0"
