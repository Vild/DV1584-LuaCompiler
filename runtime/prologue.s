	// <PROLOGUE>

	.code64
	.text

	.global _start
	.type _start, %function
start:
_start:
	movq $0, %rbp
	call main
	mov %rax, %rdi
	movq $__NR_exit, %rax
	syscall
	.size ., .-_start

	.global main
	.type main, %function
main:
	pushq %rbp
	mov %rsp, %rbp
	call __main
	xor %rax, %rax
	leave
	ret
	.size ., .-main

	// </PROLOGUE>
