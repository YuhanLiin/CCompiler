	.file	"empty.c"
	.text
	.globl	empty
	.def	empty;	.scl	2;	.type	32;	.endef
empty:
	pushq	%rbp
	movq	%rsp, %rbp
	nop
	popq	%rbp
	ret
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.1.0"
