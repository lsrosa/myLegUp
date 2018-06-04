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
	.file	"simuart.arm.bc"
	.globl	printc_uart
	.align	2
	.type	printc_uart,%function
printc_uart:                            @ @printc_uart
	.fnstart
.Leh_func_begin0:
@ BB#0:
	movw	r1, #36864
	movt	r1, #4096
	str	r0, [r1]
	bx	lr
.Ltmp0:
	.size	printc_uart, .Ltmp0-printc_uart
	.cantunwind
	.fnend

	.globl	print_uart
	.align	2
	.type	print_uart,%function
print_uart:                             @ @print_uart
	.fnstart
.Leh_func_begin1:
@ BB#0:
	ldrb	r1, [r0]
	cmp	r1, #0
	bxeq	lr
	movw	r2, #36864
	add	r0, r0, #1
	movt	r2, #4096
.LBB1_1:                                @ %.lr.ph
                                        @ =>This Inner Loop Header: Depth=1
	sxtb	r1, r1
	str	r1, [r2]
	ldrb	r1, [r0], #1
	cmp	r1, #0
	bne	.LBB1_1
@ BB#2:                                 @ %._crit_edge
	bx	lr
.Ltmp1:
	.size	print_uart, .Ltmp1-print_uart
	.cantunwind
	.fnend

	.globl	_i2h
	.align	2
	.type	_i2h,%function
_i2h:                                   @ @_i2h
	.fnstart
.Leh_func_begin2:
@ BB#0:
	mov	r1, #87
	cmp	r0, #10
	movwlt	r1, #48
	add	r0, r1, r0
	sxtb	r0, r0
	bx	lr
.Ltmp2:
	.size	_i2h, .Ltmp2-_i2h
	.cantunwind
	.fnend

	.globl	_i2H
	.align	2
	.type	_i2H,%function
_i2H:                                   @ @_i2H
	.fnstart
.Leh_func_begin3:
@ BB#0:
	mov	r1, #55
	cmp	r0, #10
	movwlt	r1, #48
	add	r0, r1, r0
	sxtb	r0, r0
	bx	lr
.Ltmp3:
	.size	_i2H, .Ltmp3-_i2H
	.cantunwind
	.fnend

	.globl	i2h
	.align	2
	.type	i2h,%function
i2h:                                    @ @i2h
	.fnstart
.Leh_func_begin4:
@ BB#0:
	push	{r4, r5, r6, r7, r8, lr}
	sub	sp, sp, #12
	movw	r6, :lower16:_i2H
	mov	r4, r0
	movw	r0, :lower16:_i2h
	movt	r6, :upper16:_i2H
	movt	r0, :upper16:_i2h
	cmp	r1, #0
	moveq	r6, r0
	cmp	r4, #0
	beq	.LBB4_7
@ BB#1:
	mov	r1, #0
	add	r8, sp, #3
	mov	r7, #0
.LBB4_2:                                @ %.lr.ph5
                                        @ =>This Inner Loop Header: Depth=1
	and	r0, r4, #15
	mov	r5, r1
	blx	r6
	strb	r0, [r8, r5]
	lsr	r0, r4, #4
	cmp	r7, r4, lsr #4
	add	r1, r5, #1
	mov	r4, r0
	bne	.LBB4_2
@ BB#3:                                 @ %._crit_edge
	movw	r0, :lower16:_MergedGlobals
	mov	r2, #0
	movt	r0, :upper16:_MergedGlobals
	cmp	r5, #0
	strb	r2, [r0, r1]
	blt	.LBB4_6
@ BB#4:                                 @ %.lr.ph.preheader
	movw	r0, :lower16:_MergedGlobals
	sub	r2, r8, #1
	movt	r0, :upper16:_MergedGlobals
	mov	r3, r0
.LBB4_5:                                @ %.lr.ph
                                        @ =>This Inner Loop Header: Depth=1
	ldrb	r7, [r2, r1]
	subs	r1, r1, #1
	strb	r7, [r3], #1
	bne	.LBB4_5
.LBB4_6:                                @ %.loopexit
	add	sp, sp, #12
	pop	{r4, r5, r6, r7, r8, pc}
.LBB4_7:
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
	add	sp, sp, #12
	pop	{r4, r5, r6, r7, r8, pc}
.Ltmp4:
	.size	i2h, .Ltmp4-i2h
	.cantunwind
	.fnend

	.globl	l2h
	.align	2
	.type	l2h,%function
l2h:                                    @ @l2h
	.fnstart
.Leh_func_begin5:
@ BB#0:
	push	{r4, r5, r6, r7, r8, lr}
	sub	sp, sp, #20
	movw	r7, :lower16:_i2H
	mov	r5, r0
	movw	r0, :lower16:_i2h
	movt	r7, :upper16:_i2H
	movt	r0, :upper16:_i2h
	cmp	r2, #0
	mov	r4, r1
	moveq	r7, r0
	orrs	r0, r5, r4
	beq	.LBB5_7
@ BB#1:
	mov	r1, #0
	add	r8, sp, #3
.LBB5_2:                                @ %.lr.ph5
                                        @ =>This Inner Loop Header: Depth=1
	and	r0, r5, #15
	mov	r6, r1
	blx	r7
	lsr	r1, r5, #4
	strb	r0, [r8, r6]
	orr	r5, r1, r4, lsl #28
	add	r1, r6, #1
	orr	r2, r5, r4, lsr #4
	lsr	r4, r4, #4
	cmp	r2, #0
	bne	.LBB5_2
@ BB#3:                                 @ %._crit_edge
	movw	r2, :lower16:_MergedGlobals
	mov	r3, #0
	movt	r2, :upper16:_MergedGlobals
	cmp	r6, #0
	add	r0, r2, #53
	strb	r3, [r0, r1]
	blt	.LBB5_6
@ BB#4:                                 @ %.lr.ph.preheader
	sub	r7, r8, #1
	mov	r6, #0
.LBB5_5:                                @ %.lr.ph
                                        @ =>This Inner Loop Header: Depth=1
	ldrb	r5, [r7, r1]
	add	r0, r2, #53
	sub	r1, r1, #1
	strb	r5, [r0, r3]
	adds	r3, r3, #1
	adc	r6, r6, #0
	cmp	r1, #0
	bgt	.LBB5_5
.LBB5_6:                                @ %.loopexit
	add	sp, sp, #20
	pop	{r4, r5, r6, r7, r8, pc}
.LBB5_7:
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
	add	sp, sp, #20
	pop	{r4, r5, r6, r7, r8, pc}
.Ltmp5:
	.size	l2h, .Ltmp5-l2h
	.cantunwind
	.fnend

	.globl	itoa
	.align	2
	.type	itoa,%function
itoa:                                   @ @itoa
	.fnstart
.Leh_func_begin6:
@ BB#0:
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	cmp	r0, #0
	blt	.LBB6_3
@ BB#1:
	mov	lr, #0
	bne	.LBB6_4
@ BB#2:
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.LBB6_3:
	rsb	r0, r0, #0
	mov	lr, #1
.LBB6_4:                                @ %.preheader27
	movw	r1, #15241
	movw	r12, #26215
	movt	r1, #21990
	movt	r12, #26214
	smmul	r1, r0, r1
	movw	r3, #12193
	movt	r3, #17592
	smmul	r2, r0, r12
	asr	r4, r1, #25
	smmul	r3, r0, r3
	asr	r5, r2, #2
	add	r1, r4, r1, lsr #31
	add	r4, r5, r2, lsr #31
	smmul	r2, r1, r12
	add	r6, r4, r4, lsl #2
	asr	r5, r3, #28
	add	r5, r5, r3, lsr #31
	sub	r3, r0, r6, lsl #1
	asr	r6, r2, #2
	add	r7, r3, #48
	add	r6, r6, r2, lsr #31
	movw	r2, :lower16:_MergedGlobals
	movt	r2, :upper16:_MergedGlobals
	mov	r3, #0
	strb	r3, [r2, #19]
	add	r8, r6, r6, lsl #2
	strb	r7, [r2, #18]
	smmul	r7, r5, r12
	sub	r8, r1, r8, lsl #1
	asr	r6, r7, #2
	add	r6, r6, r7, lsr #31
	add	r6, r6, r6, lsl #2
	sub	r9, r5, r6, lsl #1
	movw	r6, #51819
	movt	r6, #27487
	smmul	r6, r0, r6
	asr	r7, r6, #22
	add	r6, r7, r6, lsr #31
	smmul	r7, r6, r12
	lsr	r5, r7, #2
	add	r5, r5, r7, lsr #31
	add	r5, r5, r5, lsl #2
	sub	r10, r6, r5, lsl #1
	movw	r6, #56963
	movt	r6, #17179
	smmul	r6, r0, r6
	asr	r7, r6, #18
	add	r6, r7, r6, lsr #31
	smmul	r7, r6, r12
	asr	r1, r7, #2
	add	r1, r1, r7, lsr #31
	add	r1, r1, r1, lsl #2
	sub	r1, r6, r1, lsl #1
	smmul	r6, r4, r12
	lsr	r7, r6, #2
	add	r6, r7, r6, lsr #31
	add	r6, r6, r6, lsl #2
	sub	r4, r4, r6, lsl #1
	add	r4, r4, #48
	strb	r4, [r2, #17]
	movw	r4, #34079
	movt	r4, #20971
	smmul	r4, r0, r4
	asr	r6, r4, #5
	add	r4, r6, r4, lsr #31
	smmul	r6, r4, r12
	asr	r7, r6, #2
	add	r6, r7, r6, lsr #31
	add	r6, r6, r6, lsl #2
	sub	r4, r4, r6, lsl #1
	add	r4, r4, #48
	strb	r4, [r2, #16]
	movw	r4, #46473
	movt	r4, #5368
	smmul	r4, r0, r4
	asr	r6, r4, #13
	add	r4, r6, r4, lsr #31
	smmul	r6, r4, r12
	lsr	r7, r6, #2
	add	r6, r7, r6, lsr #31
	add	r6, r6, r6, lsl #2
	sub	r4, r4, r6, lsl #1
	movw	r6, #35757
	movt	r6, #26843
	smmul	r6, r0, r6
	asr	r7, r6, #12
	add	r6, r7, r6, lsr #31
	smmul	r7, r6, r12
	asr	r5, r7, #2
	add	r5, r5, r7, lsr #31
	add	r5, r5, r5, lsl #2
	sub	r5, r6, r5, lsl #1
	movw	r6, #19923
	movt	r6, #4194
	smmul	r0, r0, r6
	asr	r6, r0, #6
	add	r0, r6, r0, lsr #31
	smmul	r7, r0, r12
	lsr	r6, r7, #2
	add	r7, r6, r7, lsr #31
	add	r7, r7, r7, lsl #2
	sub	r0, r0, r7, lsl #1
	add	r0, r0, #48
	strb	r0, [r2, #15]
	add	r0, r5, #48
	strb	r0, [r2, #14]
	add	r0, r4, #48
	strb	r0, [r2, #13]
	add	r0, r1, #48
	strb	r0, [r2, #12]
	add	r0, r10, #48
	strb	r0, [r2, #11]
	add	r0, r8, #48
	strb	r0, [r2, #10]
	add	r0, r9, #48
	strb	r0, [r2, #9]
	b	.LBB6_6
.LBB6_5:                                @ %._crit_edge8
                                        @   in Loop: Header=BB6_6 Depth=1
	add	r0, r2, r3
	mov	r3, r1
	ldrb	r0, [r0, #10]
.LBB6_6:                                @ =>This Inner Loop Header: Depth=1
	uxtb	r0, r0
	cmp	r0, #48
	bne	.LBB6_9
@ BB#7:                                 @   in Loop: Header=BB6_6 Depth=1
	add	r1, r3, #1
	cmp	r1, #9
	ble	.LBB6_5
@ BB#8:                                 @ %split
	add	r3, r3, #1
.LBB6_9:
	cmp	lr, #0
	beq	.LBB6_11
@ BB#10:
	mov	r0, #45
	strb	r0, [r2, #20]
.LBB6_11:                               @ %.preheader
	rsb	r0, r3, #10
	mov	r12, #0
	mov	r1, #0
	cmp	r0, #1
	blt	.LBB6_15
@ BB#12:                                @ %.lr.ph.preheader
	mvn	r1, #8
	sub	r3, r1, r3
	add	r1, lr, r2
	add	r1, r1, #20
.LBB6_13:                               @ %.lr.ph
                                        @ =>This Inner Loop Header: Depth=1
	ldrb	r4, [r2, -r3]
	sub	r3, r3, #1
	cmn	r3, #19
	strb	r4, [r1], #1
	bne	.LBB6_13
@ BB#14:
	mov	r1, r0
.LBB6_15:                               @ %._crit_edge
	add	r0, r2, #20
	add	r1, r1, lr
	strb	r12, [r0, r1]
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.Ltmp6:
	.size	itoa, .Ltmp6-itoa
	.cantunwind
	.fnend

	.globl	utoa
	.align	2
	.type	utoa,%function
utoa:                                   @ @utoa
	.fnstart
.Leh_func_begin7:
@ BB#0:
	push	{r4, r5, r6, r7, r8, r9, lr}
	cmp	r0, #0
	beq	.LBB7_11
@ BB#1:                                 @ %.preheader27
	movw	r1, #15241
	movw	r12, #52429
	movt	r1, #21990
	movt	r12, #52428
	umull	r1, r2, r0, r1
	umull	r1, r3, r0, r12
	movw	r1, #51819
	movt	r1, #27487
	lsr	r2, r2, #25
	umull	r1, lr, r0, r1
	lsr	r3, r3, #3
	umull	r1, r7, r2, r12
	add	r1, r3, r3, lsl #2
	lsr	r4, lr, #22
	sub	r1, r0, r1, lsl #1
	lsr	r7, r7, #3
	add	r7, r7, r7, lsl #2
	umull	r5, r6, r4, r12
	orr	r5, r1, #48
	movw	r1, :lower16:_MergedGlobals
	sub	lr, r2, r7, lsl #1
	movt	r1, :upper16:_MergedGlobals
	mov	r2, #0
	movw	r7, #19331
	strb	r2, [r1, #41]
	movt	r7, #4
	lsr	r6, r6, #3
	strb	r5, [r1, #40]
	lsr	r5, r0, #9
	add	r6, r6, r6, lsl #2
	umull	r5, r7, r5, r7
	sub	r8, r4, r6, lsl #1
	lsr	r5, r7, #7
	umull	r6, r7, r5, r12
	lsr	r6, r7, #3
	add	r6, r6, r6, lsl #2
	sub	r9, r5, r6, lsl #1
	movw	r6, #56963
	movt	r6, #17179
	umull	r6, r7, r0, r6
	lsr	r6, r7, #18
	umull	r7, r4, r6, r12
	lsr	r4, r4, #3
	add	r4, r4, r4, lsl #2
	sub	r4, r6, r4, lsl #1
	umull	r6, r7, r3, r12
	lsr	r6, r7, #3
	add	r6, r6, r6, lsl #2
	sub	r3, r3, r6, lsl #1
	orr	r3, r3, #48
	strb	r3, [r1, #39]
	movw	r3, #34079
	movt	r3, #20971
	umull	r3, r6, r0, r3
	lsr	r3, r6, #5
	umull	r6, r7, r3, r12
	lsr	r6, r7, #3
	add	r6, r6, r6, lsl #2
	sub	r3, r3, r6, lsl #1
	movw	r6, #23237
	movt	r6, #2684
	orr	r3, r3, #48
	strb	r3, [r1, #38]
	lsr	r3, r0, #5
	umull	r3, r6, r3, r6
	lsr	r3, r6, #7
	umull	r6, r7, r3, r12
	lsr	r6, r7, #3
	add	r6, r6, r6, lsl #2
	sub	r3, r3, r6, lsl #1
	movw	r6, #5977
	movt	r6, #53687
	umull	r6, r7, r0, r6
	lsr	r6, r7, #13
	umull	r7, r5, r6, r12
	lsr	r5, r5, #3
	add	r5, r5, r5, lsl #2
	sub	r5, r6, r5, lsl #1
	movw	r6, #19923
	movt	r6, #4194
	umull	r0, r6, r0, r6
	lsr	r0, r6, #6
	umull	r7, r6, r0, r12
	lsr	r7, r6, #3
	add	r7, r7, r7, lsl #2
	sub	r0, r0, r7, lsl #1
	orr	r0, r0, #48
	strb	r0, [r1, #37]
	orr	r0, r5, #48
	strb	r0, [r1, #36]
	orr	r0, r3, #48
	strb	r0, [r1, #35]
	orr	r0, r4, #48
	strb	r0, [r1, #34]
	orr	r0, r8, #48
	strb	r0, [r1, #33]
	orr	r0, lr, #48
	strb	r0, [r1, #32]
	orr	r0, r9, #48
	strb	r0, [r1, #31]
	b	.LBB7_3
.LBB7_2:                                @ %._crit_edge8
                                        @   in Loop: Header=BB7_3 Depth=1
	add	r0, r1, r2
	mov	r2, r3
	ldrb	r0, [r0, #32]
.LBB7_3:                                @ =>This Inner Loop Header: Depth=1
	uxtb	r0, r0
	cmp	r0, #48
	bne	.LBB7_6
@ BB#4:                                 @   in Loop: Header=BB7_3 Depth=1
	add	r3, r2, #1
	cmp	r3, #9
	ble	.LBB7_2
@ BB#5:                                 @ %..preheader_crit_edge
	add	r2, r2, #1
.LBB7_6:                                @ %.preheader
	rsb	lr, r2, #10
	mov	r12, #0
	mov	r3, #0
	cmp	lr, #1
	blt	.LBB7_10
@ BB#7:                                 @ %.lr.ph.preheader
	add	r3, r1, r2
	sub	r2, r2, #10
	mov	r0, #0
.LBB7_8:                                @ %.lr.ph
                                        @ =>This Inner Loop Header: Depth=1
	sub	r4, r3, r0
	sub	r5, r1, r0
	ldrb	r4, [r4, #31]
	sub	r0, r0, #1
	cmp	r2, r0
	strb	r4, [r5, #42]
	bne	.LBB7_8
@ BB#9:
	mov	r3, lr
.LBB7_10:                               @ %._crit_edge
	add	r0, r1, #42
	strb	r12, [r0, r3]
	pop	{r4, r5, r6, r7, r8, r9, pc}
.LBB7_11:
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
	pop	{r4, r5, r6, r7, r8, r9, pc}
.Ltmp7:
	.size	utoa, .Ltmp7-utoa
	.cantunwind
	.fnend

	.globl	ltoa
	.align	2
	.type	ltoa,%function
ltoa:                                   @ @ltoa
	.fnstart
.Leh_func_begin8:
@ BB#0:
	push	{r4, r5, r6, r7, r8, r9, lr}
	mov	r4, r1
	mov	r5, r0
	cmp	r4, #0
	blt	.LBB8_3
@ BB#1:
	mov	r8, #0
	orrs	r0, r5, r4
	bne	.LBB8_4
@ BB#2:
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
	pop	{r4, r5, r6, r7, r8, r9, pc}
.LBB8_3:
	rsbs	r5, r5, #0
	mov	r8, #1
	rsc	r4, r4, #0
.LBB8_4:
	movw	r9, :lower16:_MergedGlobals
	mov	r6, #0
	movt	r9, :upper16:_MergedGlobals
	mov	r7, #93
	strb	r6, [r9, #94]
.LBB8_5:                                @ =>This Inner Loop Header: Depth=1
	mov	r0, r5
	mov	r1, r4
	mov	r2, #10
	mov	r3, #0
	bl	__moddi3
	add	r0, r0, #48
	mov	r1, r4
	strb	r0, [r9, r7]
	mov	r0, r5
	mov	r2, #10
	mov	r3, #0
	bl	__divdi3
	mov	r5, r0
	sub	r0, r7, #70
	mov	r4, r1
	sub	r7, r7, #1
	cmp	r0, #0
	bgt	.LBB8_5
.LBB8_6:                                @ %.preheader2
                                        @ =>This Inner Loop Header: Depth=1
	add	r0, r9, r6
	ldrb	r0, [r0, #70]
	cmp	r0, #48
	bne	.LBB8_8
@ BB#7:                                 @   in Loop: Header=BB8_6 Depth=1
	add	r6, r6, #1
	cmp	r6, #23
	ble	.LBB8_6
.LBB8_8:
	cmp	r8, #0
	beq	.LBB8_10
@ BB#9:
	mov	r0, #45
	strb	r0, [r9, #95]
.LBB8_10:                               @ %.preheader
	rsb	r0, r6, #24
	mov	r1, #0
	mov	r2, #0
	cmp	r0, #1
	blt	.LBB8_14
@ BB#11:                                @ %.lr.ph.preheader
	mvn	r2, #69
	add	r3, r8, r9
	sub	r2, r2, r6
	add	r3, r3, #95
.LBB8_12:                               @ %.lr.ph
                                        @ =>This Inner Loop Header: Depth=1
	ldrb	r7, [r9, -r2]
	sub	r2, r2, #1
	cmn	r2, #94
	strb	r7, [r3], #1
	bne	.LBB8_12
@ BB#13:
	mov	r2, r0
.LBB8_14:                               @ %._crit_edge
	add	r0, r9, #95
	add	r2, r2, r8
	strb	r1, [r0, r2]
	pop	{r4, r5, r6, r7, r8, r9, pc}
.Ltmp8:
	.size	ltoa, .Ltmp8-ltoa
	.cantunwind
	.fnend

	.globl	puts
	.align	2
	.type	puts,%function
puts:                                   @ @puts
	.fnstart
.Leh_func_begin9:
@ BB#0:
	ldrb	r1, [r0]
	cmp	r1, #0
	beq	.LBB9_3
@ BB#1:                                 @ %.lr.ph.preheader
	movw	r2, #36864
	add	r0, r0, #1
	movt	r2, #4096
.LBB9_2:                                @ %.lr.ph
                                        @ =>This Inner Loop Header: Depth=1
	sxtb	r1, r1
	str	r1, [r2]
	ldrb	r1, [r0], #1
	cmp	r1, #0
	bne	.LBB9_2
.LBB9_3:                                @ %._crit_edge
	movw	r0, #36864
	mov	r1, #10
	movt	r0, #4096
	str	r1, [r0]
	mov	r0, #0
	bx	lr
.Ltmp9:
	.size	puts, .Ltmp9-puts
	.cantunwind
	.fnend

	.globl	printf
	.align	2
	.type	printf,%function
printf:                                 @ @printf
	.fnstart
.Leh_func_begin10:
@ BB#0:
	sub	sp, sp, #12
	push	{r4, r5, r6, r7, r8, r9, r10, r11, lr}
	sub	sp, sp, #44
	movw	r10, #36864
	movw	lr, :lower16:_MergedGlobals
	add	r5, sp, #80
	add	r7, sp, #18
	mov	r4, r0
	stm	r5, {r1, r2, r3}
	add	r0, sp, #80
	sub	r8, r7, #1
	mov	r9, #0
	movt	r10, #4096
	mov	r11, #37
	add	r12, sp, #27
	movt	lr, :upper16:_MergedGlobals
	mov	r5, #0
	str	r0, [sp, #12]
	str	r8, [sp, #8]            @ 4-byte Spill
	b	.LBB10_2
.LBB10_1:                               @ %.backedge
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r5, r5, #1
.LBB10_2:                               @ =>This Loop Header: Depth=1
                                        @     Child Loop BB10_4 Depth 2
                                        @     Child Loop BB10_33 Depth 2
                                        @     Child Loop BB10_35 Depth 2
                                        @     Child Loop BB10_60 Depth 2
                                        @     Child Loop BB10_23 Depth 2
                                        @     Child Loop BB10_30 Depth 2
                                        @     Child Loop BB10_101 Depth 2
                                        @     Child Loop BB10_102 Depth 2
                                        @     Child Loop BB10_108 Depth 2
                                        @     Child Loop BB10_113 Depth 2
                                        @     Child Loop BB10_52 Depth 2
                                        @     Child Loop BB10_56 Depth 2
                                        @     Child Loop BB10_89 Depth 2
                                        @     Child Loop BB10_73 Depth 2
                                        @     Child Loop BB10_77 Depth 2
                                        @     Child Loop BB10_97 Depth 2
                                        @     Child Loop BB10_70 Depth 2
                                        @     Child Loop BB10_46 Depth 2
                                        @     Child Loop BB10_48 Depth 2
                                        @     Child Loop BB10_85 Depth 2
                                        @     Child Loop BB10_64 Depth 2
                                        @     Child Loop BB10_66 Depth 2
                                        @     Child Loop BB10_93 Depth 2
                                        @     Child Loop BB10_41 Depth 2
                                        @     Child Loop BB10_12 Depth 2
                                        @     Child Loop BB10_14 Depth 2
                                        @     Child Loop BB10_27 Depth 2
	ldrb	r0, [r4, r5]
	cmp	r0, #0
	beq	.LBB10_115
@ BB#3:                                 @   in Loop: Header=BB10_2 Depth=1
	cmp	r0, #37
	bne	.LBB10_8
.LBB10_4:                               @ %.preheader
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	mov	r0, r5
	add	r1, r4, r0
	add	r5, r0, #1
	ldrb	r1, [r1, #1]
	sub	r2, r1, #48
	uxtb	r2, r2
	cmp	r2, #10
	blo	.LBB10_4
@ BB#5:                                 @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #108
	addeq	r5, r0, #2
	add	r0, r4, r5
	ldrsb	r1, [r0]
	cmp	r1, #87
	bgt	.LBB10_9
@ BB#6:                                 @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #37
	bne	.LBB10_37
@ BB#7:                                 @   in Loop: Header=BB10_2 Depth=1
	str	r11, [r10]
	add	r5, r5, #1
	b	.LBB10_2
.LBB10_8:                               @   in Loop: Header=BB10_2 Depth=1
	sxtb	r0, r0
	b	.LBB10_38
.LBB10_9:                               @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #98
	bgt	.LBB10_15
@ BB#10:                                @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #88
	bne	.LBB10_37
@ BB#11:                                @   in Loop: Header=BB10_2 Depth=1
	ldr	r1, [sp, #12]
	add	r0, r1, #4
	str	r0, [sp, #12]
	mov	r0, #0
	ldr	r1, [r1]
	cmp	r1, #0
	beq	.LBB10_24
.LBB10_12:                              @ %.lr.ph5.i82
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	mov	r2, r0
	and	r0, r1, #15
	cmp	r0, #10
	mov	r3, #55
	movwlo	r3, #48
	cmp	r9, r1, lsr #4
	add	r0, r3, r0
	lsr	r3, r1, #4
	strb	r0, [r7, r2]
	add	r0, r2, #1
	mov	r1, r3
	bne	.LBB10_12
@ BB#13:                                @ %._crit_edge.i83
                                        @   in Loop: Header=BB10_2 Depth=1
	movw	r1, :lower16:_MergedGlobals
	cmp	r2, #0
	movt	r1, :upper16:_MergedGlobals
	mov	r2, r1
	strb	r9, [r1, r0]
	blt	.LBB10_25
.LBB10_14:                              @ %.lr.ph.i87
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r3, [r8, r0]
	subs	r0, r0, #1
	strb	r3, [r2], #1
	bne	.LBB10_14
	b	.LBB10_25
.LBB10_15:                              @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #114
	bgt	.LBB10_19
@ BB#16:                                @   in Loop: Header=BB10_2 Depth=1
	sub	r1, r1, #99
	cmp	r1, #9
	bhi	.LBB10_37
@ BB#17:                                @   in Loop: Header=BB10_2 Depth=1
	lsl	r1, r1, #2
	adr	r2, .LJTI10_0_0
	ldr	pc, [r1, r2]
.LJTI10_0_0:
	.long	.LBB10_18
	.long	.LBB10_39
	.long	.LBB10_37
	.long	.LBB10_37
	.long	.LBB10_37
	.long	.LBB10_43
	.long	.LBB10_37
	.long	.LBB10_37
	.long	.LBB10_37
	.long	.LBB10_49
.LBB10_18:                              @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #12]
	add	r1, r0, #4
	str	r1, [sp, #12]
	ldrsb	r0, [r0]
	b	.LBB10_38
.LBB10_19:                              @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #115
	beq	.LBB10_28
@ BB#20:                                @   in Loop: Header=BB10_2 Depth=1
	mov	r6, r12
	cmp	r1, #117
	bne	.LBB10_31
@ BB#21:                                @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #12]
	mov	r11, r8
	mov	r8, lr
	add	r1, r0, #4
	str	r1, [sp, #12]
	ldr	r0, [r0]
	bl	utoa
	ldrb	r1, [r0]
	cmp	r1, #0
	beq	.LBB10_71
@ BB#22:                                @ %.lr.ph.i29.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r0, r0, #1
.LBB10_23:                              @ %.lr.ph.i29
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	sxtb	r1, r1
	str	r1, [r10]
	ldrb	r1, [r0], #1
	cmp	r1, #0
	bne	.LBB10_23
	b	.LBB10_71
.LBB10_24:                              @   in Loop: Header=BB10_2 Depth=1
	movw	r1, :lower16:.L.str
	movt	r1, :upper16:.L.str
.LBB10_25:                              @ %i2h.exit89
                                        @   in Loop: Header=BB10_2 Depth=1
	ldrb	r0, [r1]
	cmp	r0, #0
	beq	.LBB10_1
@ BB#26:                                @ %.lr.ph.i76.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r1, r1, #1
.LBB10_27:                              @ %.lr.ph.i76
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	sxtb	r0, r0
	str	r0, [r10]
	ldrb	r0, [r1], #1
	cmp	r0, #0
	bne	.LBB10_27
	b	.LBB10_1
.LBB10_28:                              @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #12]
	add	r1, r0, #4
	str	r1, [sp, #12]
	ldr	r1, [r0]
	ldrb	r0, [r1]
	cmp	r0, #0
	beq	.LBB10_1
@ BB#29:                                @ %.lr.ph.i112.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r1, r1, #1
.LBB10_30:                              @ %.lr.ph.i112
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	sxtb	r0, r0
	str	r0, [r10]
	ldrb	r0, [r1], #1
	cmp	r0, #0
	bne	.LBB10_30
	b	.LBB10_1
.LBB10_31:                              @   in Loop: Header=BB10_2 Depth=1
	mov	r12, r6
	cmp	r1, #120
	bne	.LBB10_37
@ BB#32:                                @   in Loop: Header=BB10_2 Depth=1
	ldr	r1, [sp, #12]
	add	r0, r1, #4
	str	r0, [sp, #12]
	mov	r0, #0
	ldr	r1, [r1]
	cmp	r1, #0
	beq	.LBB10_57
.LBB10_33:                              @ %.lr.ph5.i100
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	mov	r2, r0
	and	r0, r1, #15
	cmp	r0, #10
	mov	r3, #87
	movwlo	r3, #48
	cmp	r9, r1, lsr #4
	add	r0, r3, r0
	lsr	r3, r1, #4
	strb	r0, [r7, r2]
	add	r0, r2, #1
	mov	r1, r3
	bne	.LBB10_33
@ BB#34:                                @ %._crit_edge.i101
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r2, #0
	mov	r1, lr
	mov	r2, lr
	strb	r9, [lr, r0]
	blt	.LBB10_58
.LBB10_35:                              @ %.lr.ph.i105
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r2, [r8, r0]
	subs	r0, r0, #1
	strb	r2, [r1], #1
	bne	.LBB10_35
@ BB#36:                                @   in Loop: Header=BB10_2 Depth=1
	mov	r2, lr
	b	.LBB10_58
.LBB10_37:                              @   in Loop: Header=BB10_2 Depth=1
	str	r11, [r10]
	ldrsb	r0, [r0]
.LBB10_38:                              @ %.backedge
                                        @   in Loop: Header=BB10_2 Depth=1
	str	r0, [r10]
	b	.LBB10_1
.LBB10_39:                              @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #12]
	mov	r6, lr
	mov	r8, r12
	add	r1, r0, #4
	str	r1, [sp, #12]
	ldr	r0, [r0]
	bl	itoa
	ldrb	r1, [r0]
	cmp	r1, #0
	beq	.LBB10_61
@ BB#40:                                @ %.lr.ph.i.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r0, r0, #1
	mov	lr, r6
.LBB10_41:                              @ %.lr.ph.i
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	sxtb	r1, r1
	str	r1, [r10]
	ldrb	r1, [r0], #1
	cmp	r1, #0
	bne	.LBB10_41
@ BB#42:                                @   in Loop: Header=BB10_2 Depth=1
	mov	r12, r8
	b	.LBB10_62
.LBB10_43:                              @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #12]
	add	r1, r0, #4
	str	r1, [sp, #12]
	add	r1, r5, #1
	ldrb	r2, [r4, r1]
	ldr	r0, [r0]
	cmp	r2, #104
	movw	r2, #65535
	moveq	r5, r1
	movweq	r2, #255
	add	r5, r5, #1
	and	r0, r2, r0
	add	r1, r4, r5
	ldrsb	r1, [r1]
	cmp	r1, #88
	beq	.LBB10_63
@ BB#44:                                @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #120
	bne	.LBB10_67
@ BB#45:                                @   in Loop: Header=BB10_2 Depth=1
	mov	r1, #0
	cmp	r0, #0
	beq	.LBB10_82
.LBB10_46:                              @ %.lr.ph5.i13
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	mov	r2, r1
	and	r1, r0, #15
	cmp	r1, #10
	mov	r3, #87
	movwlo	r3, #48
	cmp	r9, r0, lsr #4
	add	r1, r3, r1
	lsr	r3, r0, #4
	strb	r1, [r7, r2]
	add	r1, r2, #1
	mov	r0, r3
	bne	.LBB10_46
@ BB#47:                                @ %._crit_edge.i14
                                        @   in Loop: Header=BB10_2 Depth=1
	movw	r0, :lower16:_MergedGlobals
	cmp	r2, #0
	movt	r0, :upper16:_MergedGlobals
	mov	r2, r0
	strb	r9, [r0, r1]
	blt	.LBB10_83
.LBB10_48:                              @ %.lr.ph.i18
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r3, [r8, r1]
	subs	r1, r1, #1
	strb	r3, [r2], #1
	bne	.LBB10_48
	b	.LBB10_83
.LBB10_49:                              @   in Loop: Header=BB10_2 Depth=1
	add	r2, r5, #1
	add	r0, r4, r2
	ldrsb	r0, [r0]
	cmp	r0, #88
	beq	.LBB10_72
@ BB#50:                                @   in Loop: Header=BB10_2 Depth=1
	cmp	r0, #120
	bne	.LBB10_78
@ BB#51:                                @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #12]
	str	r2, [sp, #4]            @ 4-byte Spill
	add	r1, r0, #4
	str	r1, [sp, #12]
	mov	r1, r0
	ldr	r2, [r1], #8
	str	r1, [sp, #12]
	ldr	r0, [r0, #4]
	orrs	r1, r2, r0
	mov	r1, #0
	beq	.LBB10_86
.LBB10_52:                              @ %.lr.ph5.i56
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	mov	r3, r1
	lsr	r1, r2, #4
	orr	r6, r1, r0, lsl #28
	and	r1, r2, #15
	cmp	r1, #10
	mov	r2, #87
	orr	r5, r6, r0, lsr #4
	movwlo	r2, #48
	add	r1, r2, r1
	lsr	r0, r0, #4
	strb	r1, [r12, r3]
	add	r1, r3, #1
	mov	r2, r6
	cmp	r5, #0
	bne	.LBB10_52
@ BB#53:                                @ %._crit_edge.i57
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r2, lr, #53
	cmp	r3, #0
	strb	r9, [r2, r1]
	blt	.LBB10_87
@ BB#54:                                @ %.lr.ph.i60.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	sub	r0, r1, #1
	mov	r1, #0
	mov	r3, #0
	b	.LBB10_56
.LBB10_55:                              @ %.lr.ph.i60..lr.ph.i60_crit_edge
                                        @   in Loop: Header=BB10_56 Depth=2
	adds	r1, r1, #1
	sub	r0, r0, #1
	adc	r3, r3, #0
.LBB10_56:                              @ %.lr.ph.i60
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r6, [r12, r0]
	add	r2, lr, #53
	cmp	r0, #1
	strb	r6, [r2, r1]
	bge	.LBB10_55
	b	.LBB10_87
.LBB10_57:                              @   in Loop: Header=BB10_2 Depth=1
	movw	r2, :lower16:.L.str
	movt	r2, :upper16:.L.str
.LBB10_58:                              @ %i2h.exit107
                                        @   in Loop: Header=BB10_2 Depth=1
	ldrb	r0, [r2]
	cmp	r0, #0
	beq	.LBB10_1
@ BB#59:                                @ %.lr.ph.i94.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r1, r2, #1
.LBB10_60:                              @ %.lr.ph.i94
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	sxtb	r0, r0
	str	r0, [r10]
	ldrb	r0, [r1], #1
	cmp	r0, #0
	bne	.LBB10_60
	b	.LBB10_1
.LBB10_61:                              @   in Loop: Header=BB10_2 Depth=1
	mov	r12, r8
	mov	lr, r6
.LBB10_62:                              @   in Loop: Header=BB10_2 Depth=1
	ldr	r8, [sp, #8]            @ 4-byte Reload
	add	r5, r5, #1
	b	.LBB10_2
.LBB10_63:                              @   in Loop: Header=BB10_2 Depth=1
	mov	r1, #0
	cmp	r0, #0
	beq	.LBB10_90
.LBB10_64:                              @ %.lr.ph5.i
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	mov	r2, r1
	and	r1, r0, #15
	cmp	r1, #10
	mov	r3, #55
	movwlo	r3, #48
	cmp	r9, r0, lsr #4
	add	r1, r3, r1
	lsr	r3, r0, #4
	strb	r1, [r7, r2]
	add	r1, r2, #1
	mov	r0, r3
	bne	.LBB10_64
@ BB#65:                                @ %._crit_edge.i
                                        @   in Loop: Header=BB10_2 Depth=1
	movw	r0, :lower16:_MergedGlobals
	cmp	r2, #0
	movt	r0, :upper16:_MergedGlobals
	mov	r2, r0
	strb	r9, [r0, r1]
	blt	.LBB10_91
.LBB10_66:                              @ %.lr.ph.i4
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r3, [r8, r1]
	subs	r1, r1, #1
	strb	r3, [r2], #1
	bne	.LBB10_66
	b	.LBB10_91
.LBB10_67:                              @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #100
	bne	.LBB10_1
@ BB#68:                                @   in Loop: Header=BB10_2 Depth=1
	mov	r11, r8
	mov	r8, lr
	mov	r6, r12
	bl	itoa
	ldrb	r1, [r0]
	cmp	r1, #0
	beq	.LBB10_71
@ BB#69:                                @ %.lr.ph.i25.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r0, r0, #1
.LBB10_70:                              @ %.lr.ph.i25
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	sxtb	r1, r1
	str	r1, [r10]
	ldrb	r1, [r0], #1
	cmp	r1, #0
	bne	.LBB10_70
.LBB10_71:                              @   in Loop: Header=BB10_2 Depth=1
	mov	lr, r8
	mov	r8, r11
	mov	r12, r6
	mov	r11, #37
	add	r5, r5, #1
	b	.LBB10_2
.LBB10_72:                              @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #12]
	str	r2, [sp, #4]            @ 4-byte Spill
	add	r1, r0, #4
	str	r1, [sp, #12]
	mov	r1, r0
	ldr	r2, [r1], #8
	str	r1, [sp, #12]
	ldr	r0, [r0, #4]
	orrs	r1, r2, r0
	mov	r1, #0
	beq	.LBB10_94
.LBB10_73:                              @ %.lr.ph5.i39
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	mov	r3, r1
	lsr	r1, r2, #4
	orr	r6, r1, r0, lsl #28
	and	r1, r2, #15
	cmp	r1, #10
	mov	r2, #55
	orr	r5, r6, r0, lsr #4
	movwlo	r2, #48
	add	r1, r2, r1
	lsr	r0, r0, #4
	strb	r1, [r12, r3]
	add	r1, r3, #1
	mov	r2, r6
	cmp	r5, #0
	bne	.LBB10_73
@ BB#74:                                @ %._crit_edge.i40
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r2, lr, #53
	cmp	r3, #0
	strb	r9, [r2, r1]
	blt	.LBB10_95
@ BB#75:                                @ %.lr.ph.i43.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	sub	r0, r1, #1
	mov	r1, #0
	mov	r3, #0
	b	.LBB10_77
.LBB10_76:                              @ %.lr.ph.i43..lr.ph.i43_crit_edge
                                        @   in Loop: Header=BB10_77 Depth=2
	adds	r1, r1, #1
	sub	r0, r0, #1
	adc	r3, r3, #0
.LBB10_77:                              @ %.lr.ph.i43
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r6, [r12, r0]
	add	r2, lr, #53
	cmp	r0, #1
	strb	r6, [r2, r1]
	bge	.LBB10_76
	b	.LBB10_95
.LBB10_78:                              @   in Loop: Header=BB10_2 Depth=1
	cmp	r0, #100
	bne	.LBB10_1
@ BB#79:                                @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #12]
	add	r1, r0, #8
	str	r1, [sp, #12]
	ldm	r0, {r5, r6}
	cmp	r6, #0
	blt	.LBB10_99
@ BB#80:                                @   in Loop: Header=BB10_2 Depth=1
	orrs	r0, r5, r6
	mov	r0, #0
	str	r0, [sp]                @ 4-byte Spill
	bne	.LBB10_100
@ BB#81:                                @   in Loop: Header=BB10_2 Depth=1
	movw	r1, :lower16:.L.str
	movt	r1, :upper16:.L.str
	b	.LBB10_111
.LBB10_82:                              @   in Loop: Header=BB10_2 Depth=1
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
.LBB10_83:                              @ %i2h.exit20
                                        @   in Loop: Header=BB10_2 Depth=1
	ldrb	r1, [r0]
	cmp	r1, #0
	beq	.LBB10_1
@ BB#84:                                @ %.lr.ph.i7.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r0, r0, #1
.LBB10_85:                              @ %.lr.ph.i7
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	sxtb	r1, r1
	str	r1, [r10]
	ldrb	r1, [r0], #1
	cmp	r1, #0
	bne	.LBB10_85
	b	.LBB10_1
.LBB10_86:                              @   in Loop: Header=BB10_2 Depth=1
	movw	r2, :lower16:.L.str
	movt	r2, :upper16:.L.str
.LBB10_87:                              @ %l2h.exit62
                                        @   in Loop: Header=BB10_2 Depth=1
	ldrb	r0, [r2]
	cmp	r0, #0
	beq	.LBB10_98
@ BB#88:                                @ %.lr.ph.i49.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r1, r2, #1
.LBB10_89:                              @ %.lr.ph.i49
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	sxtb	r0, r0
	str	r0, [r10]
	ldrb	r0, [r1], #1
	cmp	r0, #0
	bne	.LBB10_89
	b	.LBB10_98
.LBB10_90:                              @   in Loop: Header=BB10_2 Depth=1
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
.LBB10_91:                              @ %i2h.exit
                                        @   in Loop: Header=BB10_2 Depth=1
	ldrb	r1, [r0]
	cmp	r1, #0
	beq	.LBB10_1
@ BB#92:                                @ %.lr.ph.i2.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r0, r0, #1
.LBB10_93:                              @ %.lr.ph.i2
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	sxtb	r1, r1
	str	r1, [r10]
	ldrb	r1, [r0], #1
	cmp	r1, #0
	bne	.LBB10_93
	b	.LBB10_1
.LBB10_94:                              @   in Loop: Header=BB10_2 Depth=1
	movw	r2, :lower16:.L.str
	movt	r2, :upper16:.L.str
.LBB10_95:                              @ %l2h.exit
                                        @   in Loop: Header=BB10_2 Depth=1
	ldrb	r0, [r2]
	cmp	r0, #0
	beq	.LBB10_98
@ BB#96:                                @ %.lr.ph.i33.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r1, r2, #1
.LBB10_97:                              @ %.lr.ph.i33
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	sxtb	r0, r0
	str	r0, [r10]
	ldrb	r0, [r1], #1
	cmp	r0, #0
	bne	.LBB10_97
.LBB10_98:                              @   in Loop: Header=BB10_2 Depth=1
	ldr	r5, [sp, #4]            @ 4-byte Reload
	add	r5, r5, #1
	b	.LBB10_2
.LBB10_99:                              @   in Loop: Header=BB10_2 Depth=1
	rsbs	r5, r5, #0
	mov	r0, #1
	rsc	r6, r6, #0
	str	r0, [sp]                @ 4-byte Spill
.LBB10_100:                             @   in Loop: Header=BB10_2 Depth=1
	strb	r9, [lr, #94]
	mov	r9, #93
	mov	r8, lr
	str	r2, [sp, #4]            @ 4-byte Spill
.LBB10_101:                             @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	mov	r0, r5
	mov	r1, r6
	mov	r2, #10
	mov	r3, #0
	mov	r11, #0
	bl	__moddi3
	add	r0, r0, #48
	mov	r1, r6
	strb	r0, [r8, r9]
	mov	r0, r5
	mov	r2, #10
	mov	r3, #0
	bl	__divdi3
	mov	r5, r0
	sub	r0, r9, #70
	mov	r6, r1
	sub	r9, r9, #1
	cmp	r0, #0
	bgt	.LBB10_101
.LBB10_102:                             @ %.preheader2.i
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	add	r0, r8, r11
	ldrb	r0, [r0, #70]
	cmp	r0, #48
	bne	.LBB10_104
@ BB#103:                               @   in Loop: Header=BB10_102 Depth=2
	add	r11, r11, #1
	cmp	r11, #23
	ble	.LBB10_102
.LBB10_104:                             @   in Loop: Header=BB10_2 Depth=1
	ldr	r5, [sp]                @ 4-byte Reload
	mov	lr, r8
	cmp	r5, #0
	beq	.LBB10_106
@ BB#105:                               @   in Loop: Header=BB10_2 Depth=1
	mov	r0, #45
	strb	r0, [lr, #95]
.LBB10_106:                             @ %.preheader.i
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	r8, [sp, #8]            @ 4-byte Reload
	rsb	r0, r11, #24
	mov	r1, #0
	mov	r9, #0
	add	r12, sp, #27
	cmp	r0, #1
	blt	.LBB10_110
@ BB#107:                               @ %.lr.ph.i72.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	mvn	r1, #69
	add	r2, lr, r5
	sub	r1, r1, r11
	add	r2, r2, #95
.LBB10_108:                             @ %.lr.ph.i72
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r3, [lr, -r1]
	sub	r1, r1, #1
	cmn	r1, #94
	strb	r3, [r2], #1
	bne	.LBB10_108
@ BB#109:                               @   in Loop: Header=BB10_2 Depth=1
	mov	r1, r0
.LBB10_110:                             @ %._crit_edge.i73
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	r2, [sp, #4]            @ 4-byte Reload
	add	r0, r1, r5
	add	r1, lr, #95
	mov	r11, #37
	strb	r9, [r1, r0]
.LBB10_111:                             @ %ltoa.exit
                                        @   in Loop: Header=BB10_2 Depth=1
	ldrb	r0, [r1]
	cmp	r0, #0
	beq	.LBB10_114
@ BB#112:                               @ %.lr.ph.i67.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r1, r1, #1
.LBB10_113:                             @ %.lr.ph.i67
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	sxtb	r0, r0
	str	r0, [r10]
	ldrb	r0, [r1], #1
	cmp	r0, #0
	bne	.LBB10_113
.LBB10_114:                             @   in Loop: Header=BB10_2 Depth=1
	mov	r5, r2
	add	r5, r5, #1
	b	.LBB10_2
.LBB10_115:
	mov	r0, #0
	add	sp, sp, #44
	pop	{r4, r5, r6, r7, r8, r9, r10, r11, lr}
	add	sp, sp, #12
	bx	lr
.Ltmp10:
	.size	printf, .Ltmp10-printf
	.cantunwind
	.fnend

	.globl	exit
	.align	2
	.type	exit,%function
exit:                                   @ @exit
	.fnstart
.Leh_func_begin11:
@ BB#0:
	push	{lr}
	mov	r1, r0
	movw	r0, :lower16:.L.str1
	movt	r0, :upper16:.L.str1
	bl	printf
.LBB11_1:                               @ =>This Inner Loop Header: Depth=1
	b	.LBB11_1
.Ltmp11:
	.size	exit, .Ltmp11-exit
	.cantunwind
	.fnend

	.globl	abort
	.align	2
	.type	abort,%function
abort:                                  @ @abort
	.fnstart
.Leh_func_begin12:
@ BB#0:
	mov	lr, pc
	b	exit
.Ltmp12:
	.size	abort, .Ltmp12-abort
	.cantunwind
	.fnend

	.type	.L.str,%object          @ @.str
	.section	.rodata.str1.1,"aMS",%progbits,1
.L.str:
	.asciz	"0"
	.size	.L.str, 2

	.type	.L.str1,%object         @ @.str1
.L.str1:
	.asciz	"program returned %d exit status\n"
	.size	.L.str1, 33

	.type	_MergedGlobals,%object  @ @_MergedGlobals
	.local	_MergedGlobals
	.comm	_MergedGlobals,120,16

	.ident	"Ubuntu clang version 3.5.0-4ubuntu2~trusty2 (tags/RELEASE_350/final) (based on LLVM 3.5.0)"
