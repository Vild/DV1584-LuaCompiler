	// <EPILOGUE>


	.global verifyNumber
	.type verifyNumber, %function
verifyNumber: // arg1 = rdi
	push %rbp
	mov %rsp, %rbp

	movq type(%rdi), %rax
	cmpq $'n', %rax
	je 1f
	mov $argIsNotNumber, %r8
	call runtimeError
1:
	movq type(%rsi), %rax
	cmpq $'n', %rax
	je 2f
	mov $argIsNotNumber, %r8
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

	.global poundOP
	.type poundOP, %function
poundOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	movq type(%rdi), %rax
	cmpq $'a', %rax
	je 1f
	mov $argIsNotArray, %r8
	call runtimeError
1:
	mov data(%rdi), %rdi
	mov arr_size(%rdi), %rax

	cvtsi2sd %rax, %xmm0

	movq $'n', type(%rdx)
	movsd %xmm0, data(%rdx)

	leave
	retq
	.size ., .-poundOP

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

	.global modOP
	.type modOP, %function
modOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	// Verify %rdi & %rsi
	call verifyNumber

	// Algorithm: value - truncate(value/modulus)*modulus;

	movsd data(%rdi), %xmm0
	movsd data(%rsi), %xmm1

	// xmm0 = value/modulus
	divsd %xmm1, %xmm0

	// xmm0 = truncate(xmm9)
	cvttsd2si %xmm0, %rax
	cvtsi2sd %rax, %xmm0

	// xmm0 = xmm0 * modulus
	mulsd %xmm1, %xmm0

	// xmm1 = value
	movsd data(%rdi), %xmm1

	// xmm1 = xmm1 - xmm0
	subsd %xmm0, %xmm1

	// Save the result
	movsd %xmm1, data(%rdx)
	movq $'n', type(%rdx)
	leave
	retq
	.size ., .-modOP

	.global lessOP
	.type lessOP, %function
lessOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	// Swap rdi and rsi
	mov %rdi, %rax
	mov %rsi, %rdi
	mov %rax, %rsi

	call greaterOP

	leave
	retq
	.size ., .-lessOP

	.global lequalOP
	.type lequalOP, %function
lequalOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	// Swap rdi and rsi
	mov %rdi, %rax
	mov %rsi, %rdi
	mov %rax, %rsi

	call gequalOP

	leave
	retq
	.size ., .-lequalOP

	.global greaterOP
	.type greaterOP, %function
greaterOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	// Verify %rdi & %rsi
	call verifyNumber

	// Extract a whole number as the exponent
	movsd data(%rdi), %xmm0
	movsd data(%rsi), %xmm1

	comisd %xmm1, %xmm0

	// arg1 > arg2 (set byte if above)
	seta %al

	// Expand a byte to a whole register
	movzb %al, %rax

	// Save the result
	movq %rax, data(%rdx)
	movq $'b', type(%rdx)

	leave
	retq
	.size ., .-greaterOP

	.global gequalOP
	.type gequalOP, %function
gequalOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	// Verify %rdi & %rsi
	call verifyNumber

	// Extract a whole number as the exponent
	movsd data(%rdi), %xmm0
	movsd data(%rsi), %xmm1

	comisd %xmm1, %xmm0

	// arg1 >= arg2 (set byte if above or equal)
	setae %al

	// Expand a byte to a whole register
	movzb %al, %rax

	// Save the result
	movq %rax, data(%rdx)
	movq $'b', type(%rdx)

	leave
	retq
	.size ., .-gequalOP

	.global equalOP
	.type equalOP, %function
equalOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	movq type(%rdi), %rax
	cmpq type(%rsi), %rax
	je 1f

	mov $typesAreNotEqual, %r8
	call runtimeError
1:
	cmpq $'n', %rax
	je .LequalNumber
	cmpq $'b', %rax
	je .LequalBool

	mov $varIsNotNumberOrBool, %r8
	call runtimeError
.LequalNumber:
	// Verify %rdi & %rsi
	call verifyNumber

	// Extract a whole number as the exponent
	movsd data(%rdi), %xmm0
	movsd data(%rsi), %xmm1

	ucomisd %xmm1, %xmm0

	// a == b (set byte if equal)
	sete %al

	// Expand a byte to a whole register
	movzb %al, %rax

	// Save the result
	movq %rax, data(%rdx)
	movq $'b', type(%rdx)

	jmp .LequalReturn
.LequalBool:
	cmpq $'b', type(%rsi)
	je 1f
	mov $varIsNotNumberOrBool, %r8
	call runtimeError
1:
	xor %rcx, %rcx

	mov data(%rdi), %rax
	cmpq data(%rsi), %rax
	jne 1f
	inc %rcx
1:
	movq $'b', type(%rdx)
	movq %rcx, data(%rdx)

	jmp .LequalReturn
.LequalReturn:
	leave
	retq
	.size ., .-equalOP

	.global notequalOP
	.type notequalOP, %function
notequalOP: // arg1 = rdi, arg2 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	call equalOP

	// Invert result
	movq data(%rdx), %rax

	// Flip the bit
	xorq $0x1, %rax

	movq %rax, data(%rdx)

	leave
	retq
	.size ., .-notequalOP

	.global callOP
	.type callOP, %function
callOP: // function = rdi, arg1 = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp
	sub $8, %rsp
	and $-16, %rsp
	movq %rdx, -8(%rbp)

	mov type(%rdi), %rax
	cmpq $'f', %rax
	je 1f
	mov $argIsNotFunction, %r8
	call runtimeError
1:
	call *data(%rdi)

	movq -8(%rbp), %rdi
	movq %rax, type(%rdi)
	movq %rdx, data(%rdi)

	leave
	retq
	.size ., .-callOP

	.global indexofOP
	.type indexofOP, %function
indexofOP: // table = rdi, index = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp
	sub $8, %rsp
	and $-16, %rsp

	mov %rdx, -8(%rbp)

	call indexofRefOP

	mov -8(%rbp), %rdx
	mov data(%rdx), %rdi

	mov type(%rdi), %rax
	mov %rax, type(%rdx)
	mov data(%rdi), %rax
	mov %rax, data(%rdx)

	leave
	retq
	.size ., .-indexofOP

	.global indexofRefOP
	.type indexofRefOP, %function
indexofRefOP: // table = rdi, index = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp
	sub $24, %rsp
	and $-16, %rsp

	// TODO: implement array and array indexing
	mov type(%rdi), %rax
	cmpq $'o', %rax
	je .LindexObject
	cmpq $'a', %rax
	je .LindexArray
	mov $argIsNotObjectOrArray, %r8
	call runtimeError
.LindexObject:
	mov type(%rsi), %rax
	cmpq $'s', %rax
	je 1f
	mov $indexIsNotString, %r8
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

	mov $objectMemberMissing, %r8
	call runtimeError
2:
	mov obj_data_var(%rdi), %rdi

	movq $'r', type(%rdx)
	mov %rdi, data(%rdx)

	jmp .Lreturn

.LindexArray:
	mov type(%rsi), %rax
	cmpq $'n', %rax
	je 1f
	mov $indexIsNotNumber, %r8
	call runtimeError
1:
	mov data(%rdi), %rdi

	// Convert index to integer
	movsd data(%rsi), %xmm0
	cvtsd2si %xmm0, %rax

	// Because LUA like to be special and uses indices that start at one,
	// this is needed to fix that
	dec %rax

	cmpq obj_size(%rdi), %rax
	jl 1f
	mov $indexOutOfRange, %r8
	call runtimeError
1:
	lea arr_data(%rdi), %rdi

	mov %rdx, %r8
	// rdi = &rdi[16 * rax]
	mov $2, %r9
	mulq %r9
	lea (%rdi, %rax, 8), %rdi
	mov %r8, %rdx

	movq $'r', type(%rdx)
	mov %rdi, data(%rdx)
	jmp .Lreturn

.Lreturn:
	leave
	retq
	.size ., .-indexofRefOP

	.global setValueOP
	.type setValueOP, %function
setValueOP: // refVar = rdi, value = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	mov type(%rdi), %rax
	cmpq $'r', %rax
	je 1f

	mov $varIsNotRef, %r8
	call runtimeError

1:
	mov data(%rdi), %rdx
	mov %rsi, %rdi

	call copyOP

	leave
	retq
	.size ., .-setValueOP

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
runtimeError: // msg = r8
	push %rbp
	mov %rsp, %rbp
	sub $32, %rsp
	and $-16, %rsp

	mov %r8, -8(%rbp)
	mov %rdi, -16(%rbp)
	mov %rsi, -24(%rbp)
	mov %rdx, -32(%rbp)

	mov %r8, %rdi

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

	mov -16(%rbp), %rdi
	mov -24(%rbp), %rsi
	mov -32(%rbp), %rdx
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
argIsNotArray:	 .asciz "Argument is not a array!\n"
objectMemberMissing: .asciz "Object member could not be found!\n"
indexIsNotString:	.asciz "Index is not a string!\n"
indexIsNotNumber:	.asciz "Index is not a number!\n"
indexOutOfRange:	.asciz "Index is out of range!\n"
varIsNotRef:		.asciz "Variable is not a reference variable!\n"
typesAreNotEqual:	 .asciz "Can not compare missmatching types!\n"
varIsNotNumberOrBool:	 .asciz "Can only compare numbers and booleans\n"

	.bss
	.global print_buffer
print_buffer:
	// This will be more than enought to store a UINT64_MAX number.
	.space 32
print_buffer_end:
	// The '\0' at the end of the string!
	.space 1
	// </EPILOGUE>
