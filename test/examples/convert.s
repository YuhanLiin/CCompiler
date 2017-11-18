	.file	"convert.c"
	.text
	.globl	i2ll
	.def	i2ll;	.scl	2;	.type	32;	.endef
i2ll:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	%ecx, 16(%rbp)
	movl	%edx, 24(%rbp)
	movl	16(%rbp), %edx
	movl	24(%rbp), %eax
	addl	%edx, %eax
	cltq
	popq	%rbp
	ret
	.globl	ll2i
	.def	ll2i;	.scl	2;	.type	32;	.endef
ll2i:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	%rcx, 16(%rbp)
	movq	%rdx, 24(%rbp)
	movq	16(%rbp), %rax
	movl	%eax, %edx
	movq	24(%rbp), %rax
	addl	%edx, %eax
	popq	%rbp
	ret
	.globl	ll2ui
	.def	ll2ui;	.scl	2;	.type	32;	.endef
ll2ui:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	%rcx, 16(%rbp)
	movq	%rdx, 24(%rbp)
	movq	16(%rbp), %rax
	movl	%eax, %edx
	movq	24(%rbp), %rax
	addl	%edx, %eax
	popq	%rbp
	ret
	.globl	i2ull
	.def	i2ull;	.scl	2;	.type	32;	.endef
i2ull:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	%ecx, 16(%rbp)
	movl	%edx, 24(%rbp)
	movl	16(%rbp), %edx
	movl	24(%rbp), %eax
	addl	%edx, %eax
	cltq
	popq	%rbp
	ret
	.globl	ull2i
	.def	ull2i;	.scl	2;	.type	32;	.endef
ull2i:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	%rcx, 16(%rbp)
	movq	%rdx, 24(%rbp)
	movq	16(%rbp), %rax
	movl	%eax, %edx
	movq	24(%rbp), %rax
	addl	%edx, %eax
	popq	%rbp
	ret
	.globl	ui2ll
	.def	ui2ll;	.scl	2;	.type	32;	.endef
ui2ll:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	%ecx, 16(%rbp)
	movl	%edx, 24(%rbp)
	movl	16(%rbp), %edx
	movl	24(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, %eax
	popq	%rbp
	ret
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.1.0"
