	// <EPILOGUE>


	.global verifyNumber
	.type verifyNumber, %function
verifyNumber: // arg1 = rdi
	push %rbp
	mov %rsp, %rbp

	movq type(%rdi), %rax
	cmpq $'n', %rax
	je 1f
	mov $argIsNotNumber, %rdi
	call runtimeError
1:
	movq type(%rsi), %rax
	cmpq $'n', %rax
	je 2f
	mov $argIsNotNumber, %rdi
	call runtimeError
2:
	xor %rax, %rax
	leave
	retq
	.size ., .-verifyNumber

	.global copyOP
	.type copyOP, %function
copyOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	movq data(%rsi), %rax
	movq %rax, data(%rdx)
	movq type(%rsi), %rax
	movq %rax, type(%rdx)

	leave
	retq
	.size ., .-copyOP

	.global plusOP
	.type plusOP, %function
plusOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	// Verify %rdi & %rsi
	call verifyNumber

	// Do the operation
	movsd data(%rdi), %xmm0
	movsd data(%rsi), %xmm1
	addsd %xmm1, %xmm0

	// Save the result
	movsd %xmm0, data(%rdx)
	movq $'n', type(%rdx)
	leave
	retq
	.size ., .-plusOP

	.global minusOP
	.type minusOP, %function
minusOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	// Verify %rdi & %rsi
	call verifyNumber

	// Do the operation
	movsd data(%rdi), %xmm0
	movsd data(%rsi), %xmm1
	subsd %xmm1, %xmm0

	// Save the result
	movsd %xmm0, data(%rdx)
	movq $'n', type(%rdx)
	leave
	retq
	.size ., .-minusOP

	.global mulOP
	.type mulOP, %function
mulOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	// Verify %rdi & %rsi
	call verifyNumber

	// Do the operation
	movsd data(%rdi), %xmm0
	movsd data(%rsi), %xmm1
	mulsd %xmm1, %xmm0

	// Save the result
	movsd %xmm0, data(%rdx)
	movq $'n', type(%rdx)
	leave
	retq
	.size ., .-mulOP

	.global divOP
	.type divOP, %function
divOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	// Verify %rdi & %rsi
	call verifyNumber

	// Do the operation
	movsd data(%rdi), %xmm0
	movsd data(%rsi), %xmm1
	divsd %xmm1, %xmm0

	// Save the result
	movsd %xmm0, data(%rdx)
	movq $'n', type(%rdx)
	leave
	retq
	.size ., .-divOP

	.global powOP
	.type powOP, %function
powOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	// Verify %rdi & %rsi
	call verifyNumber

	// Extract a whole number as the exponent
	movsd data(%rsi), %xmm0
	// https://www.felixcloutier.com/x86/cvtsd2si
	cvtsd2si %xmm0, %rcx

	movsd data(%rdi), %xmm0

	mov $1, %rax
	cvtsi2sd %rax, %xmm1

	// Loop until rcx is zero
1:
	mulsd %xmm0, %xmm1
	loop 1b

	// Save the result
	movsd %xmm1, data(%rdx)
	movq $'n', type(%rdx)
	leave
	retq
	.size ., .-powOP

	.global print
	.type print, %function
print: // thisFunction = rdi, text = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	mov $io_write, %rdi
	call *%rdi

	// new line
	mov $1, %rdx
	mov $newl, %rsi
  mov $__NR_write, %rax
  mov $1, %rdi
  syscall

	leave
	retq
	.size ., .-print

	.global io_write
	.type io_write, %function
io_write: // thisFunction = rdi, text = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp
	sub $16, %rsp
	and $-16, %rsp
	// Backup rdi
	mov %rsi, -16(%rbp)

	mov type(%rsi), %rax

	// %rdi will be set to the string
	cmpq $'N', %rax
	je .printNil
	cmpq $'s', %rax
	je .printString
	cmpq $'n', %rax
	je .printNumber
	cmpq $'b', %rax
	je .printBool

	mov $unknownType, %rdi
	jmp .printIt

.printNil:
	mov $nil, %rdi
	jmp .printIt

.printString:
	mov data(%rsi), %rdi
	jmp .printIt

.printNumber:
	// The idea behind this code is to do (int)(val * 10), and then just insert a
	// '.' before the last character. Probably not the best way of doing it, but
	// it is the easiest way, and it will pass the test-cases.

	movsd data(%rsi), %xmm0
	mov $10, %rax
	cvtsi2sd %rax, %xmm1
	mulsd %xmm1, %xmm0
	cvtsd2si %xmm0, %rdi

	call intToStr
	mov %rax, %rdi

	jmp .printIt

.printBool:
	mov data(%rsi), %rax
	test %rax, %rax
	jz 1f
	mov $true, %rdi
	jmp 2f
1:
	mov $false, %rdi
2:
	jmp .printIt

.printIt:
	mov %rdi, -8(%rbp)
	call strlen

	// Length
	mov %rax, %rdx
	// string
	mov -8(%rbp), %rsi

	// sys number
  mov $__NR_write, %rax
	// stdout
  mov $1, %rdi
  syscall

	xor %rax, %rax
	leave
	retq
	.size ., .-io_write

	.global intToStr
	.type intToStr, %function
intToStr:	// rdi = number
	push %rbp
	mov %rsp, %rbp

	xor %rcx, %rcx

	mov %rdi, %rax
	mov $10, %r8
	mov $print_buffer_end, %rdi

	// Is negative? (%r9 = add a '-')
	cmp $0, %rax
	jge 1f

	neg %rax
	mov $1, %r9
	jmp 2f
1:
	mov $0, %r9
2:
	// Counter for adding the '.'
	xor %r10, %r10

	/*
	 * %rcx = Length
	 * %rax = Number to print
	 * %r8 = DIV does not allow imm values
	 * %rdi = String
	 */
1:
	// length++
	inc %rcx
	dec %rdi
	// %rax is now a unsigned value, so just set rdx to zero
	xor %rdx, %rdx
	// %rdx = %rax % 10; %rax = %rax / 10
	div %r8
	add $'0', %rdx
	push %rax
	mov %rdx, %rax
	movb %al, (%rdi)
	pop %rax

	// Add the dot?
	test %r10, %r10
	jnz 4f

	inc %rcx
	dec %rdi
	movb $'.', (%rdi)
	inc %r10
	jmp 1b
4:

	// %rax != 0
	test %rax, %rax
	jnz 1b

	mov %r9, %rax
	test %rax, %rax
	jz 2f

	inc %rcx
	dec %rdi
	movb $'-', (%rdi)
2:

	mov %rdi, %rax
	leave
	ret
	.size ., .-intToStr

	.global runtimeError
	.type runtimeError, %function
runtimeError: // msg = rdi
	push %rbp
	mov %rsp, %rbp

	push %rdi
	call strlen

	// Length
	mov %rax, %rdx
	// string
	pop %rsi


	// sys number
  mov $__NR_write, %rax
	// stdout
  mov $1, %rdi
  syscall

	int $3

	// Exit with errorcode 1
	mov $1, %rdi
	mov $__NR_exit, %rax
	syscall

	leave
	retq
	.size ., .-runtimeError

	.global strlen
	.type strlen, %function
strlen: // arg1 = rdi
	push %rbp
	mov %rsp, %rbp
	xor %rax, %rax

1:
	cmpb $0, (%rdi)
	je 2f
	inc %rdi
	inc %rax
	jmp 1b

2:
	leave
	retq
	.size ., .-strlen

	.section .rodata
notImplemented:	.asciz "Not implemented!\n"
unknownType:	 .asciz "<Unknown type>"
nil:	 .asciz "<NIL>"
newl:	 .asciz "\n"
true:	 .asciz "true"
false:	 .asciz "false"
argIsNotNumber:	.asciz "Argument is not a number!\n"

	.bss
	.global print_buffer
print_buffer:
	// This will be more than enought to store a UINT64_MAX number.
	.space 32
print_buffer_end:
	// </EPILOGUE>
