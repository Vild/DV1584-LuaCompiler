	// <BUILTIN>

	.code64
	.text

	.global __builtin_print
	.type __builtin_print, %function
__builtin_print: // thisFunction = rdi, text = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp

	mov $__builtin_io_write, %rdi
	call *%rdi

	// new line
	mov $1, %rdx
	mov $newl, %rsi
  mov $__NR_write, %rax
  mov $1, %rdi
  syscall

	leave
	retq
	.size ., .-__builtin_print

	.global __builtin_io_write
	.type __builtin_io_write, %function
__builtin_io_write: // thisFunction = rdi, text = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp
	sub $16, %rsp
	and $-16, %rsp
	// Backup rdi
	mov %rsi, -16(%rbp)

	mov type(%rsi), %rax

	// %rdi will be set to the string
	cmpq $'N', %rax
	je .LprintNil
	cmpq $'s', %rax
	je .LprintString
	cmpq $'n', %rax
	je .LprintNumber
	cmpq $'b', %rax
	je .LprintBool

	mov $unknownType, %rdi
	jmp .LprintIt

.LprintNil:
	mov $nil, %rdi
	jmp .LprintIt

.LprintString:
	mov data(%rsi), %rdi
	jmp .LprintIt

.LprintNumber:
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

	jmp .LprintIt

.LprintBool:
	mov data(%rsi), %rax
	test %rax, %rax
	jz 1f
	mov $true, %rdi
	jmp 2f
1:
	mov $false, %rdi
2:
	jmp .LprintIt

.LprintIt:
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
	.size ., .-__builtin_io_write

	.global __builtin_io_read
	.type __builtin_io_read, %function
__builtin_io_read: // thisFunction = rdi, readmode = rsi, output = rdx
	push %rbp
	mov %rsp, %rbp
	sub $16, %rsp
	and $-16, %rsp
	mov %rdx, -8(%rbp)

	movq type(%rsi), %rax
	cmpq $'s', %rax
	je 1f
	mov $argIsNotString, %r8
	call runtimeError
1:
	mov data(%rsi), %rsi

	mov $c__number, %rdi
	call strcmp
	test %rax, %rax
	jnz .LreadNumber

	mov $unknownReadMode, %r8
	call runtimeError
.LreadNumber:
	mov $__NR_read, %rax
	mov $0, %rdi
	mov $read_buffer, %rsi
	mov $(read_buffer_end - read_buffer), %rdx
	syscall

	mov $read_buffer, %rdi
	xor %rax, %rax
	xor %rsi, %rsi

	xor %r8, %r8
	inc %r8
	cmpb $'-', (%rdi)
	jne .LparseStart
	mov $-1, %r8
	inc %rdi

	// This loop will do the following thing:
	// 1. If a dot is found
	// 1.1 If this is the first dot, set rsi to 1, go back to start.
	// 1.2 If this is a second dot, break
	// 2. If the character is not a number, break
	// 3. Do rax = rax * 10 + number
	// 4. rsi = rsi * 10
.LparseStart:
	cmpb $'.', (%rdi)
	jne 2f

	test %rsi, %rsi
	jnz .LloopDone

	inc %rsi
	jmp .LparseContinue
2:
	cmpb $'0', (%rdi)
	jl .LloopDone
	cmpb $'9', (%rdi)
	jg .LloopDone

	// Here we know it is a valid number

	mov %rax, -16(%rbp)
	movb (%rdi), %al
	mov %rax, %rdx
	sub $'0', %rdx
	mov -16(%rbp), %rax

	imul $10, %rax
	add %rdx, %rax

	imul $10, %rsi
.LparseContinue:
	inc %rdi
	jmp .LparseStart
.LloopDone:
	// rsi = max(rsi, 1)
	test %rsi, %rsi
	jnz 1f
	inc %rsi
1:
	imul %r8, %rax

	cvtsi2sd %rax, %xmm0
	cvtsi2sd %rsi, %xmm1
	divsd %xmm1, %xmm0

	mov -8(%rbp), %rdx
	movq $'n', type(%rdx)
	movsd %xmm0, data(%rdx)

	leave
	retq
	.size ., .-__builtin_io_read

	.section .rodata
unknownReadMode:	.asciz "Unknown read mode!\n"
c_write:	.asciz "write"
c_read:	 .asciz "read"
c__number:	 .asciz "*number"

	// Here are the built in global variables:
	.align 8
print:
	.quad 'f'
	.quad __builtin_print

	.align 8
io:
	.quad 'o'
	.quad __io_object

	.align 8
__io_object:
	// The number of items
	.quad ((__io_object_end - . - 8) / 8) / 2
	// pair<char*, Variable>
	.quad c_write
	.quad __io_write
	.quad c_read
	.quad __io_read
__io_object_end:

	.align 8
__io_write:
	.quad 'f'
	.quad __builtin_io_write

	.align 8
__io_read:
	.quad 'f'
	.quad __builtin_io_read

	.bss
	.global read_buffer
read_buffer:
	// This will be more than enought to store a UINT64_MAX number.
	.space 32
read_buffer_end:
	// The '\0' at the end of the string!
	.space 1

	// </BUILTIN>
