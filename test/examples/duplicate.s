	.file	"duplicate.c"
	.text
	.globl	a
	.def	a;	.scl	2;	.type	32;	.endef
a:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	.refptr.b(%rip), %rax
	movl	$6, (%rax)
	nop
	popq	%rbp
	ret
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.1.0"
	.section	.rdata$.refptr.b, "dr"
	.globl	.refptr.b
	.linkonce	discard
.refptr.b:
	.quad	b
