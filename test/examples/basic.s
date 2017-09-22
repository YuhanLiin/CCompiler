
	.text
	.globl	main
	#.seh_proc	main
main:
	# pushq	%rbp
	# movq	%rsp, %rbp
	# subq	$32, %rsp
	# call	__main
	movl	$1, %eax
	# addq	$32, %rsp
	# popq	%rbp
	ret
	#.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.1.0"
