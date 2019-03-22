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

	movq type(%rsi), %rax
	movq %rax, type(%rdx)
	movq data(%rsi), %rax
	movq %rax, data(%rdx)

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

	.global lequalOP
	.type lequalOP, %function
lequalOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	// Verify %rdi & %rsi
	call verifyNumber

	// Extract a whole number as the exponent
	movsd data(%rdi), %xmm0
	movsd data(%rsi), %xmm1

	cmplesd %xmm1, %xmm0
	// if xmm0 is all 1's, then true, if is all 0's then false
	cvtsd2si %xmm0, %rax

	// Save the result
	movq %rax, data(%rdx)
	movq $'b', type(%rdx)
	leave
	retq
	.size ., .-lequalOP

	.global callOP
	.type callOP, %function
callOP: // function = rdi, arg1 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	mov type(%rdi), %rax
	cmpq $'f', %rax
	je 1f
	mov $argIsNotFunction, %rdi
	call runtimeError
1:
	call *data(%rdi)

	leave
	retq
	.size ., .-callOP

	.global indexofOP
	.type indexofOP, %function
indexofOP: // table = rdi, index = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp
	sub $24, %rsp
	and $-16, %rsp

	// TODO: implement array and array indexing
	mov type(%rdi), %rax
	cmpq $'o', %rax
	je .indexObject
	cmpq $'a', %rax
	je .indexArray
	mov $argIsNotObjectOrArray, %rdi
	call runtimeError
.indexObject:
	//call *data(%rdi)

	mov type(%rsi), %rax
	cmpq $'s', %rax
	je 1f
	mov $indexIsNotString, %rdi
	call runtimeError
1:
	mov data(%rsi), %rsi

	mov data(%rdi), %rdi
	mov obj_size(%rdi), %rcx
	lea obj_data(%rdi), %rdi

	// rdi = obj_data, rsi = str index, rcx = obj entries count

1:
	mov %rdi, -8(%rbp)
	mov %rsi, -16(%rbp)
	mov %rcx, -24(%rbp)

	mov obj_data_name(%rdi), %rdi

	call strcmp

	mov -24(%rbp), %rcx
	mov -16(%rbp), %rsi
	mov -8(%rbp), %rdi

	// obj_data_name == index
	test %rax, %rax
	jnz 2f

	lea obj_data_sizeof(%rdi), %rdi
	loop 1b

	mov $objectMemberMissing, %rdi
	call runtimeError
2:
	mov obj_data_var(%rdi), %rdi

	movq type(%rdi), %rax
	movq %rax, type(%rdx)
	movq data(%rdi), %rax
	movq %rax, data(%rdx)

	jmp .return
.indexArray:
	mov $argIsNotObjectOrArray, %rdi
	call runtimeError
.return:
	leave
	retq
	.size ., .-indexofOP

	.global intToStr
	.type intToStr, %function
intToStr:	// rdi = number
	push %rbp
	mov %rsp, %rbp
	sub $8, %rsp
	and $-16, %rsp

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
	mov %rax, -8(%rbp)
	mov %rdx, %rax
	movb %al, (%rdi)
	mov -8(%rbp), %rax

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
	sub $8, %rsp
	and $-16, %rsp

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

	.global strcmp
	.type strcmp, %function
strcmp: // arg1 = rdi, arg2 = rsi
	push %rbp
	mov %rsp, %rbp
	sub $8, %rsp
	and $-16, %rsp
	xor %rax, %rax

	mov %rdi, -8(%rbp)
	call strlen
	mov -8(%rbp), %rdi

	mov %rax, %rcx
	rep cmpsb
	jne 1f

	mov $1, %rax
	jmp 2f
1:
	mov $0, %rax
2:
	leave
	retq
	.size ., .-strcmp

	.section .rodata
notImplemented:	.asciz "Not implemented!\n"
unknownType:	 .asciz "<Unknown type>"
nil:	 .asciz "<NIL>"
newl:	 .asciz "\n"
true:	 .asciz "true"
false:	 .asciz "false"
argIsNotString:	.asciz "Argument is not a string!\n"
argIsNotNumber:	.asciz "Argument is not a number!\n"
argIsNotFunction: .asciz "Argument is not a function!\n"
argIsNotObjectOrArray:	 .asciz "Argument is not a object or array!\n"
objectMemberMissing: .asciz "Object member could not be found!\n"
indexIsNotString:	.asciz "Object index is not a string!\n"

	.bss
	.global print_buffer
print_buffer:
	// This will be more than enought to store a UINT64_MAX number.
	.space 32
print_buffer_end:
	// The '\0' at the end of the string!
	.space 1
	// </EPILOGUE>
