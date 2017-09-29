	.file	"basic.c"
	.def	__main;	.scl	2;	.type	32;	.endef
	.text
	.globl	main
	.def	main;	.scl	2;	.type	32;	.endef
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
	call	__main
	movl	$1, %eax
	leave
	ret
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.1.0"
