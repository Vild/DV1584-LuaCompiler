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
	sub $16, %rsp
	and $-16, %rsp

	mov $_cN_NIL, %rsi
	mov $__main, %rdi
	lea -16(%rbp), %rdx
	call callOP

	leave
	ret
	.size ., .-main

	// </PROLOGUE>
