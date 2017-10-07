.text
	.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $32, %rsp
	call __main
	movq $1, %rax
	leave
	ret
