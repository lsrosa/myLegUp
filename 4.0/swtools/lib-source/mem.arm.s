	.text
	.syntax unified
	.cpu	cortex-a9
	.eabi_attribute	6, 10	@ Tag_CPU_arch
	.eabi_attribute	7, 65	@ Tag_CPU_arch_profile
	.eabi_attribute	8, 1	@ Tag_ARM_ISA_use
	.eabi_attribute	9, 2	@ Tag_THUMB_ISA_use
	.fpu	neon
	.eabi_attribute	17, 1	@ Tag_ABI_PCS_GOT_use
	.eabi_attribute	20, 1	@ Tag_ABI_FP_denormal
	.eabi_attribute	21, 1	@ Tag_ABI_FP_exceptions
	.eabi_attribute	23, 3	@ Tag_ABI_FP_number_model
	.eabi_attribute	24, 1	@ Tag_ABI_align_needed
	.eabi_attribute	25, 1	@ Tag_ABI_align_preserved
	.eabi_attribute	36, 1	@ Tag_FP_HP_extension
	.eabi_attribute	68, 1	@ Tag_Virtualization_use
	.file	"mem.arm.bc"
	.globl	memset
	.align	2
	.type	memset,%function
memset:                                 @ @memset
	.fnstart
.Leh_func_begin0:
@ BB#0:                                 @ %entry
	cmp	r2, #0
	bxeq	lr
	mov	r3, r0
.LBB0_1:                                @ %while.body
                                        @ =>This Inner Loop Header: Depth=1
	strb	r1, [r3], #1
	subs	r2, r2, #1
	bne	.LBB0_1
@ BB#2:                                 @ %while.end
	bx	lr
.Ltmp0:
	.size	memset, .Ltmp0-memset
	.cantunwind
	.fnend

	.globl	memcpy
	.align	2
	.type	memcpy,%function
memcpy:                                 @ @memcpy
	.fnstart
.Leh_func_begin1:
@ BB#0:                                 @ %entry
	cmp	r2, #0
	bxeq	lr
	mov	r12, r0
.LBB1_1:                                @ %while.body
                                        @ =>This Inner Loop Header: Depth=1
	ldrb	r3, [r1], #1
	subs	r2, r2, #1
	strb	r3, [r12], #1
	bne	.LBB1_1
@ BB#2:                                 @ %while.end
	bx	lr
.Ltmp1:
	.size	memcpy, .Ltmp1-memcpy
	.cantunwind
	.fnend


	.ident	"clang version 3.5.2 (tags/RELEASE_352/final)"
