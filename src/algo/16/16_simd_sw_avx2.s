	.file	"16_simd_sw.c"
	.section	.text.unlikely,"ax",@progbits
.LCOLDB3:
	.text
.LHOTB3:
	.p2align 4,,15
	.globl	search_16_sse2_sw
	.type	search_16_sse2_sw, @function
search_16_sse2_sw:
.LFB2252:
	.cfi_startproc
	leaq	8(%rsp), %r10
	.cfi_def_cfa 10, 0
	andq	$-32, %rsp
	movslq	%r8d, %rax
	pxor	%xmm0, %xmm0
	pushq	-8(%r10)
	pushq	%rbp
	.cfi_escape 0x10,0x6,0x2,0x76,0
	movq	%rsp, %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	.cfi_escape 0x10,0xf,0x2,0x76,0x78
	.cfi_escape 0x10,0xe,0x2,0x76,0x70
	.cfi_escape 0x10,0xd,0x2,0x76,0x68
	.cfi_escape 0x10,0xc,0x2,0x76,0x60
	leaq	-240(%rbp), %r13
	pushq	%r10
	.cfi_escape 0xf,0x3,0x76,0x58,0x6
	pushq	%rbx
	leaq	-176(%rbp), %r10
	leaq	-112(%rbp), %r15
	xorl	%r14d, %r14d
	subq	$480, %rsp
	.cfi_escape 0x10,0x3,0x2,0x76,0x50
	movl	%eax, -500(%rbp)
	leaq	(%rdi,%rax,8), %rax
	movq	%rdx, -464(%rbp)
	movq	%rdi, -432(%rbp)
	movq	%rax, -424(%rbp)
	movq	40(%rax), %rax
	movaps	%xmm0, -320(%rbp)
	movq	%rcx, -416(%rbp)
	movl	$8, %ecx
	movq	%rsi, -400(%rbp)
	movq	(%rax), %r8
	movzwl	90(%rdi), %eax
	movq	$0, -408(%rbp)
	movq	$0, -392(%rbp)
	movl	%eax, %edx
	addw	88(%rdi), %dx
	movd	%edx, %xmm0
	punpcklwd	%xmm0, %xmm0
	pshufd	$0, %xmm0, %xmm0
	movaps	%xmm0, -480(%rbp)
	movd	%eax, %xmm0
	movq	(%rdi), %rax
	movq	%r13, %rdi
	punpcklwd	%xmm0, %xmm0
	movq	%rax, -456(%rbp)
	xorl	%eax, %eax
	rep stosq
	movq	%r10, %rdi
	pshufd	$0, %xmm0, %xmm3
	movaps	%xmm3, -496(%rbp)
	movb	$8, %cl
	rep stosq
	movq	%r15, %rdi
	movb	$8, %cl
	rep stosq
	.p2align 4,,10
	.p2align 3
.L42:
	testl	%r14d, %r14d
	je	.L2
	leaq	-304(%rbp), %rax
	movl	$1, %r14d
	leaq	48(%rax), %rdx
	xorl	%eax, %eax
	jmp	.L12
	.p2align 4,,10
	.p2align 3
.L58:
	xorl	%ebx, %ebx
	movw	%bx, -48(%rdx)
.L5:
	xorl	%r11d, %r11d
	movq	%rcx, %rsi
	movw	%r11w, -32(%rdx)
.L7:
	xorl	%r9d, %r9d
	movw	%r9w, -16(%rdx)
.L9:
	xorl	%ecx, %ecx
	movw	%cx, (%rdx)
.L11:
	xorl	%ecx, %ecx
	cmpq	%rsi, %rdi
	setne	%cl
	andl	%ecx, %r14d
.L3:
	addq	$8, %rax
	addq	$2, %rdx
	cmpq	$64, %rax
	je	.L57
.L12:
	cmpq	$0, (%r15,%rax)
	je	.L3
	movq	0(%r13,%rax), %rcx
	movq	(%r10,%rax), %rdi
	cmpq	%rcx, %rdi
	jbe	.L58
	movzbl	(%rcx), %r9d
	leaq	1(%rcx), %rsi
	cmpq	%rsi, %rdi
	movq	%rsi, 0(%r13,%rax)
	movw	%r9w, -48(%rdx)
	jbe	.L59
	movzbl	1(%rcx), %r9d
	leaq	2(%rcx), %rsi
	cmpq	%rsi, %rdi
	movq	%rsi, 0(%r13,%rax)
	movw	%r9w, -32(%rdx)
	jbe	.L7
	movzbl	2(%rcx), %r9d
	leaq	3(%rcx), %rsi
	cmpq	%rsi, %rdi
	movq	%rsi, 0(%r13,%rax)
	movw	%r9w, -16(%rdx)
	jbe	.L9
	leaq	4(%rcx), %rsi
	movzbl	3(%rcx), %ecx
	movq	%rsi, 0(%r13,%rax)
	movw	%cx, (%rdx)
	jmp	.L11
	.p2align 4,,10
	.p2align 3
.L2:
	pxor	%xmm0, %xmm0
	movl	$1, %r14d
	xorl	%ebx, %ebx
	movl	%r14d, -384(%rbp)
	leaq	-304(%rbp), %r12
	movq	%r8, -440(%rbp)
	movq	%rbx, %r14
	movaps	%xmm0, -336(%rbp)
	jmp	.L38
	.p2align 4,,10
	.p2align 3
.L16:
	movq	(%r15,%r14,4), %rsi
	movl	$32767, %eax
	leaq	-352(%rbp), %rbx
	movw	%ax, -336(%rbp,%r14)
	testq	%rsi, %rsi
	je	.L24
	cmpw	$0, -320(%rbp,%r14)
	movswl	-352(%rbp,%r14), %eax
	jne	.L25
	addl	$32768, %eax
	cmpl	$65535, %eax
	jne	.L60
.L25:
	movq	-416(%rbp), %rax
	movq	%r10, -368(%rbp)
	cmpq	$0, (%rax)
	je	.L27
	movq	%rax, %rdi
	call	ll_push
	movq	-368(%rbp), %r10
.L28:
	xorl	%eax, %eax
	movw	%ax, -320(%rbp,%r14)
.L26:
	addq	$1, -408(%rbp)
.L24:
	movl	$-32768, %eax
	movw	%ax, (%rbx,%r14)
	movq	-400(%rbp), %rax
	movq	-392(%rbp), %rbx
	cmpq	%rbx, 16(%rax)
	ja	.L61
	xorl	%eax, %eax
	xorl	%edx, %edx
	xorl	%ecx, %ecx
	xorl	%esi, %esi
	movq	$0, (%r15,%r14,4)
	movq	$0, 0(%r13,%r14,4)
	movq	$0, (%r10,%r14,4)
	movw	%ax, (%r12)
	movw	%dx, 16(%r12)
	movw	%cx, 32(%r12)
	movw	%si, 48(%r12)
.L23:
	addq	$2, %r14
	addq	$2, %r12
	cmpq	$16, %r14
	je	.L62
.L38:
	movq	0(%r13,%r14,4), %rax
	movq	(%r10,%r14,4), %rdx
	cmpq	%rdx, %rax
	jae	.L16
	movzbl	(%rax), %esi
	leaq	1(%rax), %rcx
	cmpq	%rcx, %rdx
	movq	%rcx, 0(%r13,%r14,4)
	movw	%si, (%r12)
	ja	.L17
	xorl	%eax, %eax
	movw	%ax, 16(%r12)
.L18:
	xorl	%eax, %eax
	movw	%ax, 32(%r12)
.L20:
	xorl	%eax, %eax
	movw	%ax, 48(%r12)
.L22:
	xorl	%eax, %eax
	cmpq	%rcx, %rdx
	setne	%al
	addq	$2, %r14
	andl	%eax, -384(%rbp)
	addq	$2, %r12
	cmpq	$16, %r14
	jne	.L38
.L62:
	movq	-400(%rbp), %rax
	movq	-408(%rbp), %rdi
	movq	-440(%rbp), %r8
	movq	%r10, -448(%rbp)
	cmpq	%rdi, 16(%rax)
	movl	-384(%rbp), %r14d
	movq	%r8, -368(%rbp)
	je	.L1
	movq	-432(%rbp), %rax
	leaq	-304(%rbp), %rsi
	movq	8(%rax), %rdi
	call	dprofile_fill_16_sse2
	movq	-424(%rbp), %rax
	movq	-368(%rbp), %r8
	movdqa	-336(%rbp), %xmm11
	movq	-448(%rbp), %r10
	movq	40(%rax), %rax
	testq	%r8, %r8
	movq	8(%rax), %rsi
	je	.L43
	movdqa	.LC0(%rip), %xmm9
	xorl	%ecx, %ecx
	movq	-456(%rbp), %rax
	movaps	%xmm9, -368(%rbp)
	movdqa	%xmm9, %xmm4
	movdqa	%xmm9, %xmm8
	movdqa	%xmm9, %xmm12
	movdqa	%xmm9, %xmm2
	movdqa	-352(%rbp), %xmm7
	movdqa	%xmm9, %xmm13
	movdqa	%xmm9, %xmm14
	movdqa	%xmm9, %xmm0
	movdqa	-480(%rbp), %xmm15
	movdqa	-496(%rbp), %xmm3
	.p2align 4,,10
	.p2align 3
.L41:
	movq	(%rsi,%rcx,8), %rdx
	movdqa	%xmm0, %xmm10
	movdqa	16(%rax), %xmm5
	addq	$1, %rcx
	movdqa	%xmm13, %xmm6
	addq	$32, %rax
	paddsw	(%rdx), %xmm10
	psubsw	%xmm11, %xmm5
	pmaxsw	%xmm13, %xmm10
	psubsw	%xmm11, %xmm5
	pmaxsw	%xmm5, %xmm10
	movdqa	%xmm10, %xmm0
	movdqa	-32(%rax), %xmm1
	psubsw	%xmm3, %xmm6
	pmaxsw	%xmm10, %xmm7
	psubsw	%xmm15, %xmm0
	pmaxsw	%xmm0, %xmm6
	movdqa	%xmm6, %xmm13
	movdqa	-368(%rbp), %xmm6
	psubsw	%xmm11, %xmm1
	psubsw	%xmm3, %xmm5
	psubsw	%xmm11, %xmm1
	pmaxsw	%xmm0, %xmm5
	pmaxsw	%xmm10, %xmm14
	paddsw	16(%rdx), %xmm6
	movaps	%xmm1, -384(%rbp)
	pmaxsw	%xmm2, %xmm6
	movdqa	%xmm7, %xmm1
	pmaxsw	%xmm5, %xmm6
	psubsw	%xmm3, %xmm5
	movdqa	%xmm6, %xmm7
	movdqa	%xmm5, %xmm0
	psubsw	%xmm3, %xmm2
	movdqa	%xmm12, %xmm5
	pmaxsw	%xmm6, %xmm1
	pmaxsw	%xmm6, %xmm14
	psubsw	%xmm15, %xmm7
	pmaxsw	%xmm7, %xmm0
	pmaxsw	%xmm7, %xmm2
	paddsw	32(%rdx), %xmm5
	pmaxsw	%xmm8, %xmm5
	pmaxsw	%xmm0, %xmm5
	psubsw	%xmm3, %xmm0
	movdqa	%xmm5, %xmm7
	pmaxsw	%xmm5, %xmm1
	movdqa	%xmm0, %xmm12
	movdqa	%xmm4, %xmm0
	pmaxsw	%xmm5, %xmm14
	psubsw	%xmm15, %xmm7
	psubsw	%xmm3, %xmm8
	movaps	%xmm10, -368(%rbp)
	pmaxsw	%xmm7, %xmm12
	paddsw	48(%rdx), %xmm0
	pmaxsw	%xmm9, %xmm0
	pmaxsw	%xmm12, %xmm0
	movdqa	%xmm0, %xmm4
	psubsw	%xmm3, %xmm12
	movaps	%xmm0, -32(%rax)
	pmaxsw	%xmm0, %xmm1
	pmaxsw	%xmm7, %xmm8
	psubsw	%xmm15, %xmm4
	pmaxsw	%xmm4, %xmm12
	pmaxsw	%xmm0, %xmm14
	movaps	%xmm12, -16(%rax)
	psubsw	%xmm3, %xmm9
	pmaxsw	%xmm4, %xmm9
	cmpq	%r8, %rcx
	movdqa	%xmm1, %xmm7
	movdqa	%xmm6, %xmm12
	movdqa	%xmm5, %xmm4
	movdqa	-384(%rbp), %xmm0
	jne	.L41
	movaps	%xmm1, -352(%rbp)
	movdqa	%xmm14, %xmm0
	jmp	.L14
	.p2align 4,,10
	.p2align 3
.L61:
	movq	-400(%rbp), %rax
	leaq	1(%rbx), %rsi
	movq	(%rax), %rax
	movq	(%rax,%rbx,8), %rax
	movq	8(%rax), %rdx
	movq	%rax, (%r15,%r14,4)
	movq	%rdx, %rcx
	addq	16(%rax), %rcx
	movq	%rdx, 0(%r13,%r14,4)
	movq	%rdx, %rax
	cmpq	%rdx, %rcx
	movq	%rcx, (%r10,%r14,4)
	ja	.L30
	xorl	%r11d, %r11d
	movw	%r11w, (%r12)
.L31:
	xorl	%r9d, %r9d
	movw	%r9w, 16(%r12)
.L33:
	xorl	%r8d, %r8d
	movw	%r8w, 32(%r12)
.L35:
	xorl	%edi, %edi
	movw	%di, 48(%r12)
.L37:
	cmpq	%rax, %rcx
	movq	%rsi, -392(%rbp)
	setne	%al
	movzbl	%al, %eax
	andl	%eax, -384(%rbp)
	jmp	.L23
.L17:
	movzbl	1(%rax), %esi
	leaq	2(%rax), %rcx
	cmpq	%rcx, %rdx
	movq	%rcx, 0(%r13,%r14,4)
	movw	%si, 16(%r12)
	jbe	.L18
	movzbl	2(%rax), %esi
	leaq	3(%rax), %rcx
	cmpq	%rcx, %rdx
	movq	%rcx, 0(%r13,%r14,4)
	movw	%si, 32(%r12)
	jbe	.L20
	leaq	4(%rax), %rcx
	movzbl	3(%rax), %eax
	movq	%rcx, 0(%r13,%r14,4)
	movw	%ax, 48(%r12)
	jmp	.L22
	.p2align 4,,10
	.p2align 3
.L57:
	movq	-432(%rbp), %rax
	leaq	-304(%rbp), %rsi
	movq	%r10, -440(%rbp)
	movq	%r8, -384(%rbp)
	movaps	%xmm14, -368(%rbp)
	movq	8(%rax), %rdi
	call	dprofile_fill_16_sse2
	movq	-424(%rbp), %rax
	movq	-384(%rbp), %r8
	movdqa	-368(%rbp), %xmm14
	movq	-440(%rbp), %r10
	movq	40(%rax), %rax
	testq	%r8, %r8
	movq	8(%rax), %rsi
	je	.L55
	movdqa	.LC0(%rip), %xmm10
	xorl	%ecx, %ecx
	movq	-456(%rbp), %rax
	movdqa	-352(%rbp), %xmm9
	movdqa	%xmm10, %xmm0
	movdqa	%xmm10, %xmm13
	movdqa	%xmm10, %xmm2
	movdqa	%xmm10, %xmm12
	movdqa	%xmm10, %xmm5
	movdqa	%xmm10, %xmm11
	movdqa	%xmm10, %xmm7
	movdqa	-480(%rbp), %xmm15
	movdqa	-496(%rbp), %xmm3
	.p2align 4,,10
	.p2align 3
.L15:
	movq	(%rsi,%rcx,8), %rdx
	movdqa	%xmm7, %xmm6
	movdqa	(%rax), %xmm4
	addq	$1, %rcx
	movdqa	%xmm9, %xmm8
	addq	$32, %rax
	movaps	%xmm4, -368(%rbp)
	paddsw	(%rdx), %xmm6
	pmaxsw	%xmm11, %xmm6
	psubsw	%xmm3, %xmm11
	paddsw	48(%rdx), %xmm0
	pmaxsw	%xmm10, %xmm0
	psubsw	%xmm3, %xmm10
	movdqa	-16(%rax), %xmm4
	pmaxsw	%xmm4, %xmm6
	movdqa	%xmm6, %xmm1
	psubsw	%xmm3, %xmm4
	pmaxsw	%xmm6, %xmm8
	pmaxsw	%xmm6, %xmm14
	movdqa	-368(%rbp), %xmm7
	psubsw	%xmm15, %xmm1
	pmaxsw	%xmm1, %xmm11
	pmaxsw	%xmm4, %xmm1
	movdqa	%xmm5, %xmm4
	paddsw	16(%rdx), %xmm4
	pmaxsw	%xmm12, %xmm4
	pmaxsw	%xmm1, %xmm4
	movdqa	%xmm4, %xmm5
	psubsw	%xmm3, %xmm1
	psubsw	%xmm3, %xmm12
	pmaxsw	%xmm4, %xmm8
	pmaxsw	%xmm4, %xmm14
	psubsw	%xmm15, %xmm5
	pmaxsw	%xmm5, %xmm12
	pmaxsw	%xmm1, %xmm5
	movdqa	%xmm2, %xmm1
	paddsw	32(%rdx), %xmm1
	pmaxsw	%xmm13, %xmm1
	pmaxsw	%xmm5, %xmm1
	movdqa	%xmm1, %xmm2
	psubsw	%xmm3, %xmm5
	psubsw	%xmm3, %xmm13
	pmaxsw	%xmm1, %xmm8
	pmaxsw	%xmm1, %xmm14
	psubsw	%xmm15, %xmm2
	pmaxsw	%xmm2, %xmm13
	pmaxsw	%xmm5, %xmm2
	pmaxsw	%xmm2, %xmm0
	movdqa	%xmm0, %xmm5
	psubsw	%xmm3, %xmm2
	movaps	%xmm0, -32(%rax)
	pmaxsw	%xmm0, %xmm8
	pmaxsw	%xmm0, %xmm14
	psubsw	%xmm15, %xmm5
	pmaxsw	%xmm5, %xmm10
	pmaxsw	%xmm2, %xmm5
	movaps	%xmm5, -16(%rax)
	cmpq	%r8, %rcx
	movdqa	%xmm8, %xmm9
	movdqa	%xmm6, %xmm5
	movdqa	%xmm4, %xmm2
	movdqa	%xmm1, %xmm0
	jne	.L15
	movaps	%xmm8, -352(%rbp)
.L55:
	movdqa	%xmm14, %xmm0
.L14:
	pcmpeqw	.LC2(%rip), %xmm0
	por	-320(%rbp), %xmm0
	movaps	%xmm0, -320(%rbp)
	jmp	.L42
	.p2align 4,,10
	.p2align 3
.L30:
	movzbl	(%rdx), %r11d
	leaq	1(%rdx), %rdi
	cmpq	%rdi, %rcx
	movq	%rdi, 0(%r13,%r14,4)
	movw	%r11w, (%r12)
	jbe	.L63
	movzbl	1(%rdx), %r11d
	leaq	2(%rdx), %rdi
	cmpq	%rdi, %rcx
	movq	%rdi, 0(%r13,%r14,4)
	movw	%r11w, 16(%r12)
	jbe	.L64
	movzbl	2(%rdx), %r11d
	leaq	3(%rdx), %rdi
	cmpq	%rdi, %rcx
	movq	%rdi, 0(%r13,%r14,4)
	movw	%r11w, 32(%r12)
	jbe	.L65
	movzbl	3(%rdx), %edx
	addq	$4, %rax
	movq	%rax, 0(%r13,%r14,4)
	movw	%dx, 48(%r12)
	jmp	.L37
	.p2align 4,,10
	.p2align 3
.L60:
	movq	%rsi, %rdx
	movq	-464(%rbp), %rdi
	movl	-500(%rbp), %esi
	movslq	%eax, %rcx
	movq	%r10, -368(%rbp)
	call	add_to_minheap
	movq	-368(%rbp), %r10
	jmp	.L26
	.p2align 4,,10
	.p2align 3
.L27:
	movq	%rsi, %rdi
	call	ll_init
	movq	-416(%rbp), %rdi
	movq	-368(%rbp), %r10
	movq	%rax, (%rdi)
	jmp	.L28
.L1:
	addq	$480, %rsp
	popq	%rbx
	popq	%r10
	.cfi_remember_state
	.cfi_def_cfa 10, 0
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	leaq	-8(%r10), %rsp
	.cfi_def_cfa 7, 8
	ret
.L43:
	.cfi_restore_state
	movdqa	.LC1(%rip), %xmm0
	movdqa	.LC0(%rip), %xmm14
	jmp	.L14
.L59:
	movq	%rsi, %rcx
	jmp	.L5
.L65:
	movq	%rdi, %rax
	jmp	.L35
.L64:
	movq	%rdi, %rax
	jmp	.L33
.L63:
	movq	%rdi, %rax
	jmp	.L31
	.cfi_endproc
.LFE2252:
	.size	search_16_sse2_sw, .-search_16_sse2_sw
	.section	.text.unlikely
.LCOLDE3:
	.text
.LHOTE3:
	.section	.rodata.cst16,"aM",@progbits,16
	.align 16
.LC0:
	.quad	-9223231297218904064
	.quad	-9223231297218904064
	.align 16
.LC1:
	.value	-32768
	.value	-32768
	.value	-32768
	.value	-32768
	.value	-32768
	.value	-32768
	.value	-32768
	.value	-32768
	.align 16
.LC2:
	.value	32767
	.value	32767
	.value	32767
	.value	32767
	.value	32767
	.value	32767
	.value	32767
	.value	32767
	.ident	"GCC: (Ubuntu 4.9.2-0ubuntu1~14.04) 4.9.2"
	.section	.note.GNU-stack,"",@progbits
