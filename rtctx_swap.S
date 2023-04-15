# 这是一个汇编代码实现的函数rtctx_swap，用于在协程之间切换上下文。代码中包含了两个不同体系结构的实现：i386和x86_64。
# 在i386实现中，代码首先将当前上下文中的寄存器值保存到第一个参数指向的内存区域中。
# 然后，代码将第二个参数指向的内存区域中保存的寄存器值恢复到CPU寄存器中。最后，使用ret指令返回到调用rtctx_swap函数的位置。
# 在x86_64实现中，代码首先将当前上下文中的寄存器值保存到第一个参数指向的内存区域中。
# 然后，代码将第二个参数指向的内存区域中保存的寄存器值恢复到CPU寄存器中。此外，代码还需要调整堆栈指针和保存额外的寄存器值。最后，使用ret指令返回到调用rtctx_swap函数的位置。
# 需要注意的是，这段汇编代码使用了一些特定于体系结构的指令，因此不能直接在其他体系结构上运行。此外，这段汇编代码实现了协程上下文切换的功能，但它只是一个底层的实现，通常需要在C/C++代码中进行封装和调用。

.globl rtctx_swap
#if !defined( __APPLE__ )
.type  rtctx_swap, @function
#endif
rtctx_swap:

#if defined(__i386__)
    # 指令将堆栈偏移量为4的位置的值复制到eax寄存器中
    movl 4(%esp), %eax
    movl %esp,  28(%eax)
    movl %ebp, 24(%eax)
    movl %esi, 20(%eax)
    movl %edi, 16(%eax)
    movl %edx, 12(%eax)
    movl %ecx, 8(%eax)
    movl %ebx, 4(%eax)


    movl 8(%esp), %eax
    movl 4(%eax), %ebx
    movl 8(%eax), %ecx
    movl 12(%eax), %edx
    movl 16(%eax), %edi
    movl 20(%eax), %esi
    movl 24(%eax), %ebp
    movl 28(%eax), %esp

	ret

#elif defined(__x86_64__)
  # lea指令可以将一个有效地址计算结果加载到寄存器中
	# 将当前栈指针rsp的值（64位）作为有效地址，计算出其值并加载到rax寄存器中
	leaq (%rsp),%rax
    # movq指令可以将64位数据从一个位置复制到另一个位置
		# 将rax寄存器中的值（64位）复制到rdi寄存器指向的内存区域偏移量为104的位置中
		movq %rax, 104(%rdi)
    movq %rbx, 96(%rdi)
    movq %rcx, 88(%rdi)
    movq %rdx, 80(%rdi)
	  movq 0(%rax), %rax
	  movq %rax, 72(%rdi) 
    movq %rsi, 64(%rdi)
	  movq %rdi, 56(%rdi)
    movq %rbp, 48(%rdi)
    movq %r8, 40(%rdi)
    movq %r9, 32(%rdi)
    movq %r12, 24(%rdi)
    movq %r13, 16(%rdi)
    movq %r14, 8(%rdi)
    movq %r15, (%rdi)
	  xorq %rax, %rax

    movq 48(%rsi), %rbp
    movq 104(%rsi), %rsp
    movq (%rsi), %r15
    movq 8(%rsi), %r14
    movq 16(%rsi), %r13
    movq 24(%rsi), %r12
    movq 32(%rsi), %r9
    movq 40(%rsi), %r8
    movq 56(%rsi), %rdi
    movq 80(%rsi), %rdx
    movq 88(%rsi), %rcx
    movq 96(%rsi), %rbx
		leaq 8(%rsp), %rsp
		pushq 72(%rsi)

    movq 64(%rsi), %rsi
	ret
#endif
