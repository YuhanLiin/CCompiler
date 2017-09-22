	.file	"helloworld.c"
	.section .rdata,"dr"
.LC0:
	.ascii "Helloworld\0"
	.text
	.globl	test
	.def	test;	.scl	2;	.type	32;	.endef
test:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
	leaq	.LC0(%rip), %rcx
	call	puts
	movl	$0, %eax
	leave
	ret
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.1.0"
	.def	puts;	.scl	2;	.type	32;	.endef
