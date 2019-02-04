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
	.file	"uart.arm.bc"
	.globl	printc_uart
	.align	2
	.type	printc_uart,%function
printc_uart:                            @ @printc_uart
	.fnstart
.Leh_func_begin0:
@ BB#0:                                 @ %entry
	movw	r1, #4100
	movt	r1, #65312
.LBB0_1:                                @ %do.body
                                        @ =>This Inner Loop Header: Depth=1
	ldr	r2, [r1]
	cmp	r2, #65536
	blo	.LBB0_1
@ BB#2:                                 @ %do.end
	movw	r1, #4096
	movt	r1, #65312
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
@ BB#0:                                 @ %entry
	ldrb	r1, [r0]
	cmp	r1, #0
	bxeq	lr
	movw	r2, #4100
	movw	r12, #4096
	movt	r2, #65312
	movt	r12, #65312
.LBB1_1:                                @ %do.body.i
                                        @ =>This Inner Loop Header: Depth=1
	ldr	r3, [r2]
	cmp	r3, #65536
	blo	.LBB1_1
@ BB#2:                                 @ %printc_uart.exit
                                        @   in Loop: Header=BB1_1 Depth=1
	sxtb	r1, r1
	str	r1, [r12]
	ldrb	r1, [r0, #1]!
	cmp	r1, #0
	bne	.LBB1_1
@ BB#3:                                 @ %while.end
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
@ BB#0:                                 @ %entry
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
@ BB#0:                                 @ %entry
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
@ BB#0:                                 @ %entry
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
.LBB4_2:                                @ %while.body
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
@ BB#3:                                 @ %while.end
	movw	r0, :lower16:_MergedGlobals
	mov	r2, #0
	movt	r0, :upper16:_MergedGlobals
	cmp	r5, #0
	strb	r2, [r0, r1]
	blt	.LBB4_6
@ BB#4:                                 @ %while.body7.preheader
	movw	r0, :lower16:_MergedGlobals
	sub	r2, r8, #1
	movt	r0, :upper16:_MergedGlobals
	mov	r3, r0
.LBB4_5:                                @ %while.body7
                                        @ =>This Inner Loop Header: Depth=1
	ldrb	r7, [r2, r1]
	subs	r1, r1, #1
	strb	r7, [r3], #1
	bne	.LBB4_5
.LBB4_6:                                @ %return
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
@ BB#0:                                 @ %entry
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
.LBB5_2:                                @ %while.body
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
@ BB#3:                                 @ %while.end
	movw	r2, :lower16:_MergedGlobals
	mov	r3, #0
	movt	r2, :upper16:_MergedGlobals
	cmp	r6, #0
	add	r0, r2, #53
	strb	r3, [r0, r1]
	blt	.LBB5_6
@ BB#4:                                 @ %while.body8.preheader
	sub	r7, r8, #1
	mov	r6, #0
.LBB5_5:                                @ %while.body8
                                        @ =>This Inner Loop Header: Depth=1
	ldrb	r5, [r7, r1]
	add	r0, r2, #53
	sub	r1, r1, #1
	strb	r5, [r0, r3]
	adds	r3, r3, #1
	adc	r6, r6, #0
	cmp	r1, #0
	bgt	.LBB5_5
.LBB5_6:                                @ %return
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
@ BB#0:                                 @ %entry
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	cmp	r0, #0
	blt	.LBB6_3
@ BB#1:                                 @ %if.else
	mov	lr, #0
	bne	.LBB6_4
@ BB#2:
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.LBB6_3:                                @ %if.then
	rsb	r0, r0, #0
	mov	lr, #1
.LBB6_4:                                @ %if.end3
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
.LBB6_5:                                @ %for.cond5.for.body8_crit_edge
                                        @   in Loop: Header=BB6_6 Depth=1
	add	r0, r2, r3
	mov	r3, r1
	ldrb	r0, [r0, #10]
.LBB6_6:                                @ %for.body8
                                        @ =>This Inner Loop Header: Depth=1
	uxtb	r0, r0
	cmp	r0, #48
	bne	.LBB6_9
@ BB#7:                                 @ %for.cond5
                                        @   in Loop: Header=BB6_6 Depth=1
	add	r1, r3, #1
	cmp	r1, #9
	ble	.LBB6_5
@ BB#8:                                 @ %for.end16split
	add	r3, r3, #1
.LBB6_9:                                @ %for.end16
	cmp	lr, #0
	beq	.LBB6_11
@ BB#10:                                @ %if.then17
	mov	r0, #45
	strb	r0, [r2, #20]
.LBB6_11:                               @ %for.cond19.preheader
	rsb	r0, r3, #10
	mov	r12, #0
	mov	r1, #0
	cmp	r0, #1
	blt	.LBB6_15
@ BB#12:                                @ %for.body22.lr.ph
	mvn	r1, #8
	sub	r3, r1, r3
	add	r1, lr, r2
	add	r1, r1, #20
.LBB6_13:                               @ %for.body22
                                        @ =>This Inner Loop Header: Depth=1
	ldrb	r4, [r2, -r3]
	sub	r3, r3, #1
	cmn	r3, #19
	strb	r4, [r1], #1
	bne	.LBB6_13
@ BB#14:
	mov	r1, r0
.LBB6_15:                               @ %for.end29
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
@ BB#0:                                 @ %entry
	push	{r4, r5, r6, r7, r8, r9, lr}
	cmp	r0, #0
	beq	.LBB7_11
@ BB#1:                                 @ %if.end
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
.LBB7_2:                                @ %for.cond2.for.body5_crit_edge
                                        @   in Loop: Header=BB7_3 Depth=1
	add	r0, r1, r2
	mov	r2, r3
	ldrb	r0, [r0, #32]
.LBB7_3:                                @ %for.body5
                                        @ =>This Inner Loop Header: Depth=1
	uxtb	r0, r0
	cmp	r0, #48
	bne	.LBB7_6
@ BB#4:                                 @ %for.cond2
                                        @   in Loop: Header=BB7_3 Depth=1
	add	r3, r2, #1
	cmp	r3, #9
	ble	.LBB7_2
@ BB#5:                                 @ %for.cond2.for.cond14.preheader_crit_edge
	add	r2, r2, #1
.LBB7_6:                                @ %for.cond14.preheader
	rsb	lr, r2, #10
	mov	r12, #0
	mov	r3, #0
	cmp	lr, #1
	blt	.LBB7_10
@ BB#7:                                 @ %for.body17.lr.ph
	add	r3, r1, r2
	sub	r2, r2, #10
	mov	r0, #0
.LBB7_8:                                @ %for.body17
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
.LBB7_10:                               @ %for.end23
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
@ BB#0:                                 @ %entry
	push	{r4, r5, r6, r7, r8, r9, lr}
	mov	r4, r1
	mov	r5, r0
	cmp	r4, #0
	blt	.LBB8_3
@ BB#1:                                 @ %if.else
	mov	r8, #0
	orrs	r0, r5, r4
	bne	.LBB8_4
@ BB#2:
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
	pop	{r4, r5, r6, r7, r8, r9, pc}
.LBB8_3:                                @ %if.then
	rsbs	r5, r5, #0
	mov	r8, #1
	rsc	r4, r4, #0
.LBB8_4:                                @ %if.end3
	movw	r9, :lower16:_MergedGlobals
	mov	r6, #0
	movt	r9, :upper16:_MergedGlobals
	mov	r7, #93
	strb	r6, [r9, #94]
.LBB8_5:                                @ %for.body
                                        @ =>This Inner Loop Header: Depth=1
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
.LBB8_6:                                @ %for.body8
                                        @ =>This Inner Loop Header: Depth=1
	add	r0, r9, r6
	ldrb	r0, [r0, #70]
	cmp	r0, #48
	bne	.LBB8_8
@ BB#7:                                 @ %for.cond5
                                        @   in Loop: Header=BB8_6 Depth=1
	add	r6, r6, #1
	cmp	r6, #23
	ble	.LBB8_6
.LBB8_8:                                @ %for.end16
	cmp	r8, #0
	beq	.LBB8_10
@ BB#9:                                 @ %if.then17
	mov	r0, #45
	strb	r0, [r9, #95]
.LBB8_10:                               @ %for.cond19.preheader
	rsb	r0, r6, #24
	mov	r1, #0
	mov	r2, #0
	cmp	r0, #1
	blt	.LBB8_14
@ BB#11:                                @ %for.body22.lr.ph
	mvn	r2, #69
	add	r3, r8, r9
	sub	r2, r2, r6
	add	r3, r3, #95
.LBB8_12:                               @ %for.body22
                                        @ =>This Inner Loop Header: Depth=1
	ldrb	r7, [r9, -r2]
	sub	r2, r2, #1
	cmn	r2, #94
	strb	r7, [r3], #1
	bne	.LBB8_12
@ BB#13:
	mov	r2, r0
.LBB8_14:                               @ %for.end29
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
@ BB#0:                                 @ %entry
	push	{r4, r5, r6, lr}
	movw	r2, #4100
	movw	lr, #4096
	mov	r3, #0
	movt	r2, #65312
	movt	lr, #65312
	mov	r12, #13
	mov	r4, #0
	b	.LBB9_2
.LBB9_1:                                @ %printc_uart.exit25
                                        @   in Loop: Header=BB9_2 Depth=1
	sxtb	r4, r5
	add	r3, r3, #1
	str	r4, [lr]
	mov	r4, r1
.LBB9_2:                                @ %while.cond
                                        @ =>This Loop Header: Depth=1
                                        @     Child Loop BB9_7 Depth 2
                                        @     Child Loop BB9_3 Depth 2
	ldrb	r6, [r0, r3]
	mov	r1, #1
	mov	r5, #13
	cmp	r6, #13
	bne	.LBB9_4
.LBB9_3:                                @ %do.body.i24
                                        @   Parent Loop BB9_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldr	r4, [r2]
	cmp	r4, #65536
	blo	.LBB9_3
	b	.LBB9_1
.LBB9_4:                                @ %while.cond
                                        @   in Loop: Header=BB9_2 Depth=1
	cmp	r6, #0
	beq	.LBB9_10
@ BB#5:                                 @ %if.end
                                        @   in Loop: Header=BB9_2 Depth=1
	cmp	r6, #10
	bne	.LBB9_9
@ BB#6:                                 @ %if.end
                                        @   in Loop: Header=BB9_2 Depth=1
	mov	r5, r6
	mov	r1, r4
	cmp	r4, #0
	bne	.LBB9_3
.LBB9_7:                                @ %do.body.i
                                        @   Parent Loop BB9_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldr	r1, [r2]
	cmp	r1, #65536
	blo	.LBB9_7
@ BB#8:                                 @ %printc_uart.exit
                                        @   in Loop: Header=BB9_2 Depth=1
	str	r12, [lr]
	mov	r1, #0
	ldrb	r5, [r0, r3]
	b	.LBB9_3
.LBB9_9:                                @   in Loop: Header=BB9_2 Depth=1
	mov	r5, r6
	mov	r1, r4
	b	.LBB9_3
.LBB9_10:                               @ %do.body.i21
                                        @ =>This Inner Loop Header: Depth=1
	ldr	r0, [r2]
	cmp	r0, #65536
	blo	.LBB9_10
@ BB#11:                                @ %printc_uart.exit22
	mov	r0, #10
	str	r0, [lr]
	mov	r0, #0
	pop	{r4, r5, r6, pc}
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
@ BB#0:                                 @ %entry
	sub	sp, sp, #12
	push	{r4, r5, r6, r7, r8, r9, r10, r11, lr}
	sub	sp, sp, #52
	movw	r7, #4100
	movw	r11, #4096
	mov	r4, r0
	add	r5, sp, #88
	add	r8, sp, #26
	add	r0, sp, #88
	stm	r5, {r1, r2, r3}
	sub	r10, r8, #1
	mov	r9, #0
	movt	r7, #65312
	str	r0, [sp, #20]
	mov	r0, #0
	movt	r11, #65312
	mov	r6, #37
	add	r12, sp, #35
	mov	r5, #0
	str	r0, [sp, #16]           @ 4-byte Spill
	b	.LBB10_2
.LBB10_1:                               @ %sw.epilog99
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r5, r5, #1
.LBB10_2:                               @ %while.cond
                                        @ =>This Loop Header: Depth=1
                                        @     Child Loop BB10_68 Depth 2
                                        @     Child Loop BB10_5 Depth 2
                                        @     Child Loop BB10_39 Depth 2
                                        @     Child Loop BB10_41 Depth 2
                                        @     Child Loop BB10_76 Depth 2
                                        @       Child Loop BB10_77 Depth 3
                                        @     Child Loop BB10_26 Depth 2
                                        @       Child Loop BB10_27 Depth 3
                                        @     Child Loop BB10_35 Depth 2
                                        @       Child Loop BB10_36 Depth 3
                                        @     Child Loop BB10_119 Depth 2
                                        @     Child Loop BB10_120 Depth 2
                                        @     Child Loop BB10_126 Depth 2
                                        @     Child Loop BB10_131 Depth 2
                                        @       Child Loop BB10_132 Depth 3
                                        @     Child Loop BB10_60 Depth 2
                                        @     Child Loop BB10_64 Depth 2
                                        @     Child Loop BB10_106 Depth 2
                                        @     Child Loop BB10_85 Depth 2
                                        @     Child Loop BB10_89 Depth 2
                                        @     Child Loop BB10_115 Depth 2
                                        @     Child Loop BB10_95 Depth 2
                                        @       Child Loop BB10_96 Depth 3
                                        @     Child Loop BB10_54 Depth 2
                                        @     Child Loop BB10_56 Depth 2
                                        @     Child Loop BB10_102 Depth 2
                                        @       Child Loop BB10_103 Depth 3
                                        @     Child Loop BB10_79 Depth 2
                                        @     Child Loop BB10_81 Depth 2
                                        @     Child Loop BB10_111 Depth 2
                                        @       Child Loop BB10_112 Depth 3
                                        @     Child Loop BB10_49 Depth 2
                                        @     Child Loop BB10_20 Depth 2
                                        @     Child Loop BB10_13 Depth 2
                                        @     Child Loop BB10_15 Depth 2
                                        @     Child Loop BB10_31 Depth 2
                                        @       Child Loop BB10_32 Depth 3
                                        @     Child Loop BB10_43 Depth 2
                                        @     Child Loop BB10_45 Depth 2
                                        @     Child Loop BB10_8 Depth 2
                                        @     Child Loop BB10_71 Depth 2
	ldrb	r0, [r4, r5]
	cmp	r0, #13
	beq	.LBB10_65
@ BB#3:                                 @ %while.cond
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r0, #0
	beq	.LBB10_134
@ BB#4:                                 @ %while.cond
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r0, #37
	bne	.LBB10_66
.LBB10_5:                               @ %while.cond7
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	mov	r0, r5
	add	r1, r4, r0
	add	r5, r0, #1
	ldrb	r1, [r1, #1]
	sub	r2, r1, #48
	uxtb	r2, r2
	cmp	r2, #10
	blo	.LBB10_5
@ BB#6:                                 @ %while.end
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #108
	addeq	r5, r0, #2
	add	r0, r4, r5
	ldrsb	r1, [r0]
	cmp	r1, #87
	bgt	.LBB10_10
@ BB#7:                                 @ %while.end
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #37
	bne	.LBB10_43
.LBB10_8:                               @ %do.body.i178
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldr	r0, [r7]
	cmp	r0, #65536
	blo	.LBB10_8
@ BB#9:                                 @ %printc_uart.exit179
                                        @   in Loop: Header=BB10_2 Depth=1
	str	r6, [r11]
	add	r5, r5, #1
	b	.LBB10_2
.LBB10_10:                              @ %while.end
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #98
	bgt	.LBB10_16
@ BB#11:                                @ %while.end
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #88
	bne	.LBB10_43
@ BB#12:                                @ %sw.bb41
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #20]
	add	r1, r0, #4
	str	r1, [sp, #20]
	mov	r1, #0
	ldr	r0, [r0]
	cmp	r0, #0
	beq	.LBB10_28
.LBB10_13:                              @ %while.body.i362
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
	strb	r1, [r8, r2]
	add	r1, r2, #1
	mov	r0, r3
	bne	.LBB10_13
@ BB#14:                                @ %while.end.i365
                                        @   in Loop: Header=BB10_2 Depth=1
	movw	r0, :lower16:_MergedGlobals
	cmp	r2, #0
	movt	r0, :upper16:_MergedGlobals
	mov	r2, r0
	strb	r9, [r0, r1]
	blt	.LBB10_29
.LBB10_15:                              @ %while.body7.i373
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r3, [r10, r1]
	subs	r1, r1, #1
	strb	r3, [r2], #1
	bne	.LBB10_15
	b	.LBB10_29
.LBB10_16:                              @ %while.end
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #114
	bgt	.LBB10_22
@ BB#17:                                @ %while.end
                                        @   in Loop: Header=BB10_2 Depth=1
	sub	r1, r1, #99
	cmp	r1, #9
	bhi	.LBB10_43
@ BB#18:                                @ %while.end
                                        @   in Loop: Header=BB10_2 Depth=1
	lsl	r1, r1, #2
	adr	r2, .LJTI10_0_0
	ldr	pc, [r1, r2]
.LJTI10_0_0:
	.long	.LBB10_19
	.long	.LBB10_47
	.long	.LBB10_43
	.long	.LBB10_43
	.long	.LBB10_43
	.long	.LBB10_51
	.long	.LBB10_43
	.long	.LBB10_43
	.long	.LBB10_43
	.long	.LBB10_57
.LBB10_19:                              @ %sw.bb33
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #20]
	add	r1, r0, #4
	str	r1, [sp, #20]
	ldr	r0, [r0]
.LBB10_20:                              @ %do.body.i419
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldr	r1, [r7]
	cmp	r1, #65536
	blo	.LBB10_20
@ BB#21:                                @ %printc_uart.exit421
                                        @   in Loop: Header=BB10_2 Depth=1
	sxtb	r0, r0
	b	.LBB10_46
.LBB10_22:                              @ %while.end
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #115
	beq	.LBB10_33
@ BB#23:                                @ %while.end
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #117
	bne	.LBB10_37
@ BB#24:                                @ %sw.bb26
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #20]
	mov	r6, r12
	add	r1, r0, #4
	str	r1, [sp, #20]
	ldr	r0, [r0]
	bl	utoa
	ldrb	r1, [r0]
	b	.LBB10_26
.LBB10_25:                              @ %printc_uart.exit.i251
                                        @   in Loop: Header=BB10_26 Depth=2
	sxtb	r1, r1
	str	r1, [r11]
	ldrb	r1, [r0, #1]!
.LBB10_26:                              @ %sw.bb26
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Loop Header: Depth=2
                                        @       Child Loop BB10_27 Depth 3
	cmp	r1, #0
	beq	.LBB10_97
.LBB10_27:                              @ %do.body.i.i247
                                        @   Parent Loop BB10_2 Depth=1
                                        @     Parent Loop BB10_26 Depth=2
                                        @ =>    This Inner Loop Header: Depth=3
	ldr	r2, [r7]
	cmp	r2, #65536
	blo	.LBB10_27
	b	.LBB10_25
.LBB10_28:                              @   in Loop: Header=BB10_2 Depth=1
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
.LBB10_29:                              @ %i2h.exit375
                                        @   in Loop: Header=BB10_2 Depth=1
	ldrb	r1, [r0]
	b	.LBB10_31
.LBB10_30:                              @ %printc_uart.exit.i350
                                        @   in Loop: Header=BB10_31 Depth=2
	sxtb	r1, r1
	str	r1, [r11]
	ldrb	r1, [r0, #1]!
.LBB10_31:                              @ %i2h.exit375
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Loop Header: Depth=2
                                        @       Child Loop BB10_32 Depth 3
	cmp	r1, #0
	beq	.LBB10_1
.LBB10_32:                              @ %do.body.i.i346
                                        @   Parent Loop BB10_2 Depth=1
                                        @     Parent Loop BB10_31 Depth=2
                                        @ =>    This Inner Loop Header: Depth=3
	ldr	r2, [r7]
	cmp	r2, #65536
	blo	.LBB10_32
	b	.LBB10_30
.LBB10_33:                              @ %sw.bb30
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #20]
	add	r1, r0, #4
	str	r1, [sp, #20]
	ldr	r0, [r0]
	ldrb	r1, [r0]
	b	.LBB10_35
.LBB10_34:                              @ %printc_uart.exit.i430
                                        @   in Loop: Header=BB10_35 Depth=2
	sxtb	r1, r1
	str	r1, [r11]
	ldrb	r1, [r0, #1]!
.LBB10_35:                              @ %sw.bb30
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Loop Header: Depth=2
                                        @       Child Loop BB10_36 Depth 3
	cmp	r1, #0
	beq	.LBB10_1
.LBB10_36:                              @ %do.body.i.i426
                                        @   Parent Loop BB10_2 Depth=1
                                        @     Parent Loop BB10_35 Depth=2
                                        @ =>    This Inner Loop Header: Depth=3
	ldr	r2, [r7]
	cmp	r2, #65536
	blo	.LBB10_36
	b	.LBB10_34
.LBB10_37:                              @ %while.end
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #120
	bne	.LBB10_43
@ BB#38:                                @ %sw.bb37
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	r1, [sp, #20]
	add	r0, r1, #4
	str	r0, [sp, #20]
	mov	r0, #0
	ldr	r1, [r1]
	cmp	r1, #0
	beq	.LBB10_73
.LBB10_39:                              @ %while.body.i400
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
	strb	r0, [r8, r2]
	add	r0, r2, #1
	mov	r1, r3
	bne	.LBB10_39
@ BB#40:                                @ %while.end.i403
                                        @   in Loop: Header=BB10_2 Depth=1
	movw	r1, :lower16:_MergedGlobals
	cmp	r2, #0
	movt	r1, :upper16:_MergedGlobals
	mov	r2, r1
	strb	r9, [r1, r0]
	blt	.LBB10_74
.LBB10_41:                              @ %while.body7.i411
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r1, [r10, r0]
	subs	r0, r0, #1
	strb	r1, [r2], #1
	bne	.LBB10_41
@ BB#42:                                @   in Loop: Header=BB10_2 Depth=1
	movw	r1, :lower16:_MergedGlobals
	movt	r1, :upper16:_MergedGlobals
	b	.LBB10_74
.LBB10_43:                              @ %do.body.i175
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldr	r1, [r7]
	cmp	r1, #65536
	blo	.LBB10_43
@ BB#44:                                @ %printc_uart.exit176
                                        @   in Loop: Header=BB10_2 Depth=1
	str	r6, [r11]
	ldrsb	r0, [r0]
.LBB10_45:                              @ %do.body.i171
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldr	r1, [r7]
	cmp	r1, #65536
	blo	.LBB10_45
.LBB10_46:                              @ %printc_uart.exit173
                                        @   in Loop: Header=BB10_2 Depth=1
	str	r0, [r11]
	b	.LBB10_1
.LBB10_47:                              @ %sw.bb
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #20]
	mov	r6, r12
	add	r1, r0, #4
	str	r1, [sp, #20]
	ldr	r0, [r0]
	bl	itoa
	ldrb	r1, [r0]
	cmp	r1, #0
	beq	.LBB10_97
@ BB#48:                                @   in Loop: Header=BB10_2 Depth=1
	mov	r12, r6
.LBB10_49:                              @ %do.body.i.i
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldr	r2, [r7]
	cmp	r2, #65536
	blo	.LBB10_49
@ BB#50:                                @ %printc_uart.exit.i
                                        @   in Loop: Header=BB10_49 Depth=2
	sxtb	r1, r1
	str	r1, [r11]
	ldrb	r1, [r0, #1]!
	cmp	r1, #0
	bne	.LBB10_49
	b	.LBB10_98
.LBB10_51:                              @ %sw.bb74
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #20]
	add	r1, r0, #4
	str	r1, [sp, #20]
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
	beq	.LBB10_78
@ BB#52:                                @ %sw.bb74
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #120
	bne	.LBB10_82
@ BB#53:                                @ %sw.bb92
                                        @   in Loop: Header=BB10_2 Depth=1
	mov	r1, #0
	cmp	r0, #0
	beq	.LBB10_99
.LBB10_54:                              @ %while.body.i214
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
	strb	r1, [r8, r2]
	add	r1, r2, #1
	mov	r0, r3
	bne	.LBB10_54
@ BB#55:                                @ %while.end.i217
                                        @   in Loop: Header=BB10_2 Depth=1
	movw	r0, :lower16:_MergedGlobals
	cmp	r2, #0
	movt	r0, :upper16:_MergedGlobals
	mov	r2, r0
	strb	r9, [r0, r1]
	blt	.LBB10_100
.LBB10_56:                              @ %while.body7.i225
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r3, [r10, r1]
	subs	r1, r1, #1
	strb	r3, [r2], #1
	bne	.LBB10_56
	b	.LBB10_100
.LBB10_57:                              @ %sw.bb45
                                        @   in Loop: Header=BB10_2 Depth=1
	add	lr, r5, #1
	add	r0, r4, lr
	ldrsb	r0, [r0]
	cmp	r0, #88
	beq	.LBB10_84
@ BB#58:                                @ %sw.bb45
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r0, #120
	bne	.LBB10_90
@ BB#59:                                @ %sw.bb53
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #20]
	add	r1, r0, #4
	str	r1, [sp, #20]
	mov	r1, r0
	ldr	r3, [r1], #8
	str	r1, [sp, #20]
	ldr	r0, [r0, #4]
	orrs	r1, r3, r0
	mov	r1, #0
	beq	.LBB10_104
.LBB10_60:                              @ %while.body.i301
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	mov	r2, r1
	lsr	r1, r3, #4
	orr	r6, r1, r0, lsl #28
	and	r1, r3, #15
	cmp	r1, #10
	mov	r3, #87
	orr	r5, r6, r0, lsr #4
	movwlo	r3, #48
	add	r1, r3, r1
	lsr	r0, r0, #4
	strb	r1, [r12, r2]
	add	r1, r2, #1
	mov	r3, r6
	cmp	r5, #0
	bne	.LBB10_60
@ BB#61:                                @ %while.end.i304
                                        @   in Loop: Header=BB10_2 Depth=1
	movw	r5, :lower16:_MergedGlobals
	cmp	r2, #0
	movt	r5, :upper16:_MergedGlobals
	add	r0, r5, #53
	strb	r9, [r0, r1]
	blt	.LBB10_105
@ BB#62:                                @ %while.body8.i313.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	sub	r1, r1, #1
	mov	r2, #0
	mov	r3, #0
	b	.LBB10_64
.LBB10_63:                              @ %while.body8.i313.while.body8.i313_crit_edge
                                        @   in Loop: Header=BB10_64 Depth=2
	adds	r2, r2, #1
	sub	r1, r1, #1
	adc	r3, r3, #0
.LBB10_64:                              @ %while.body8.i313
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r6, [r12, r1]
	add	r0, r5, #53
	cmp	r1, #1
	strb	r6, [r0, r2]
	bge	.LBB10_63
	b	.LBB10_105
.LBB10_65:                              @   in Loop: Header=BB10_2 Depth=1
	mov	r0, #13
	mov	r1, #1
	b	.LBB10_70
.LBB10_66:                              @ %if.end107
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r0, #10
	bne	.LBB10_71
@ BB#67:                                @ %if.end107
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	r1, [sp, #16]           @ 4-byte Reload
	cmp	r1, #0
	bne	.LBB10_71
.LBB10_68:                              @ %do.body.i168
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldr	r0, [r7]
	cmp	r0, #65536
	blo	.LBB10_68
@ BB#69:                                @ %printc_uart.exit169
                                        @   in Loop: Header=BB10_2 Depth=1
	mov	r0, #13
	mov	r1, #0
	str	r0, [r11]
	ldrb	r0, [r4, r5]
.LBB10_70:                              @ %if.end113
                                        @   in Loop: Header=BB10_2 Depth=1
	str	r1, [sp, #16]           @ 4-byte Spill
.LBB10_71:                              @ %do.body.i
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldr	r1, [r7]
	cmp	r1, #65536
	blo	.LBB10_71
@ BB#72:                                @ %printc_uart.exit
                                        @   in Loop: Header=BB10_2 Depth=1
	sxtb	r0, r0
	add	r5, r5, #1
	str	r0, [r11]
	b	.LBB10_2
.LBB10_73:                              @   in Loop: Header=BB10_2 Depth=1
	movw	r1, :lower16:.L.str
	movt	r1, :upper16:.L.str
.LBB10_74:                              @ %i2h.exit413
                                        @   in Loop: Header=BB10_2 Depth=1
	ldrb	r0, [r1]
	b	.LBB10_76
.LBB10_75:                              @ %printc_uart.exit.i388
                                        @   in Loop: Header=BB10_76 Depth=2
	sxtb	r0, r0
	str	r0, [r11]
	ldrb	r0, [r1, #1]!
.LBB10_76:                              @ %i2h.exit413
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Loop Header: Depth=2
                                        @       Child Loop BB10_77 Depth 3
	cmp	r0, #0
	beq	.LBB10_1
.LBB10_77:                              @ %do.body.i.i384
                                        @   Parent Loop BB10_2 Depth=1
                                        @     Parent Loop BB10_76 Depth=2
                                        @ =>    This Inner Loop Header: Depth=3
	ldr	r2, [r7]
	cmp	r2, #65536
	blo	.LBB10_77
	b	.LBB10_75
.LBB10_78:                              @ %sw.bb94
                                        @   in Loop: Header=BB10_2 Depth=1
	mov	r1, #0
	cmp	r0, #0
	beq	.LBB10_108
.LBB10_79:                              @ %while.body.i191
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
	strb	r1, [r8, r2]
	add	r1, r2, #1
	mov	r0, r3
	bne	.LBB10_79
@ BB#80:                                @ %while.end.i
                                        @   in Loop: Header=BB10_2 Depth=1
	movw	r0, :lower16:_MergedGlobals
	cmp	r2, #0
	movt	r0, :upper16:_MergedGlobals
	mov	r2, r0
	strb	r9, [r0, r1]
	blt	.LBB10_109
.LBB10_81:                              @ %while.body7.i
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r3, [r10, r1]
	subs	r1, r1, #1
	strb	r3, [r2], #1
	bne	.LBB10_81
	b	.LBB10_109
.LBB10_82:                              @ %sw.bb74
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r1, #100
	bne	.LBB10_1
@ BB#83:                                @ %sw.bb90
                                        @   in Loop: Header=BB10_2 Depth=1
	mov	r6, r12
	bl	itoa
	ldrb	r1, [r0]
	b	.LBB10_95
.LBB10_84:                              @ %sw.bb62
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #20]
	add	r1, r0, #4
	str	r1, [sp, #20]
	mov	r1, r0
	ldr	r3, [r1], #8
	str	r1, [sp, #20]
	ldr	r0, [r0, #4]
	orrs	r1, r3, r0
	mov	r1, #0
	beq	.LBB10_113
.LBB10_85:                              @ %while.body.i272
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	mov	r2, r1
	lsr	r1, r3, #4
	orr	r6, r1, r0, lsl #28
	and	r1, r3, #15
	cmp	r1, #10
	mov	r3, #55
	orr	r5, r6, r0, lsr #4
	movwlo	r3, #48
	add	r1, r3, r1
	lsr	r0, r0, #4
	strb	r1, [r12, r2]
	add	r1, r2, #1
	mov	r3, r6
	cmp	r5, #0
	bne	.LBB10_85
@ BB#86:                                @ %while.end.i274
                                        @   in Loop: Header=BB10_2 Depth=1
	movw	r5, :lower16:_MergedGlobals
	cmp	r2, #0
	movt	r5, :upper16:_MergedGlobals
	add	r0, r5, #53
	strb	r9, [r0, r1]
	blt	.LBB10_114
@ BB#87:                                @ %while.body8.i.preheader
                                        @   in Loop: Header=BB10_2 Depth=1
	sub	r1, r1, #1
	mov	r2, #0
	mov	r3, #0
	b	.LBB10_89
.LBB10_88:                              @ %while.body8.i.while.body8.i_crit_edge
                                        @   in Loop: Header=BB10_89 Depth=2
	adds	r2, r2, #1
	sub	r1, r1, #1
	adc	r3, r3, #0
.LBB10_89:                              @ %while.body8.i
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r6, [r12, r1]
	add	r0, r5, #53
	cmp	r1, #1
	strb	r6, [r0, r2]
	bge	.LBB10_88
	b	.LBB10_114
.LBB10_90:                              @ %sw.bb45
                                        @   in Loop: Header=BB10_2 Depth=1
	cmp	r0, #100
	bne	.LBB10_1
@ BB#91:                                @ %sw.bb48
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	r0, [sp, #20]
	add	r1, r0, #8
	str	r1, [sp, #20]
	ldm	r0, {r5, r6}
	cmp	r6, #0
	blt	.LBB10_117
@ BB#92:                                @ %if.else.i
                                        @   in Loop: Header=BB10_2 Depth=1
	orrs	r0, r5, r6
	mov	r0, #0
	str	r0, [sp, #12]           @ 4-byte Spill
	bne	.LBB10_118
@ BB#93:                                @   in Loop: Header=BB10_2 Depth=1
	movw	r0, :lower16:.L.str
	mov	r6, #37
	movt	r0, :upper16:.L.str
	b	.LBB10_129
.LBB10_94:                              @ %printc_uart.exit.i240
                                        @   in Loop: Header=BB10_95 Depth=2
	sxtb	r1, r1
	str	r1, [r11]
	ldrb	r1, [r0, #1]!
.LBB10_95:                              @ %sw.bb90
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Loop Header: Depth=2
                                        @       Child Loop BB10_96 Depth 3
	cmp	r1, #0
	beq	.LBB10_97
.LBB10_96:                              @ %do.body.i.i236
                                        @   Parent Loop BB10_2 Depth=1
                                        @     Parent Loop BB10_95 Depth=2
                                        @ =>    This Inner Loop Header: Depth=3
	ldr	r2, [r7]
	cmp	r2, #65536
	blo	.LBB10_96
	b	.LBB10_94
.LBB10_97:                              @   in Loop: Header=BB10_2 Depth=1
	mov	r12, r6
.LBB10_98:                              @   in Loop: Header=BB10_2 Depth=1
	mov	r6, #37
	add	r5, r5, #1
	b	.LBB10_2
.LBB10_99:                              @   in Loop: Header=BB10_2 Depth=1
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
.LBB10_100:                             @ %i2h.exit227
                                        @   in Loop: Header=BB10_2 Depth=1
	ldrb	r1, [r0]
	b	.LBB10_102
.LBB10_101:                             @ %printc_uart.exit.i202
                                        @   in Loop: Header=BB10_102 Depth=2
	sxtb	r1, r1
	str	r1, [r11]
	ldrb	r1, [r0, #1]!
.LBB10_102:                             @ %i2h.exit227
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Loop Header: Depth=2
                                        @       Child Loop BB10_103 Depth 3
	cmp	r1, #0
	beq	.LBB10_1
.LBB10_103:                             @ %do.body.i.i198
                                        @   Parent Loop BB10_2 Depth=1
                                        @     Parent Loop BB10_102 Depth=2
                                        @ =>    This Inner Loop Header: Depth=3
	ldr	r2, [r7]
	cmp	r2, #65536
	blo	.LBB10_103
	b	.LBB10_101
.LBB10_104:                             @   in Loop: Header=BB10_2 Depth=1
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
.LBB10_105:                             @ %l2h.exit315
                                        @   in Loop: Header=BB10_2 Depth=1
	ldrb	r1, [r0]
	mov	r6, #37
	cmp	r1, #0
	beq	.LBB10_133
.LBB10_106:                             @ %do.body.i.i284
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldr	r2, [r7]
	cmp	r2, #65536
	blo	.LBB10_106
@ BB#107:                               @ %printc_uart.exit.i288
                                        @   in Loop: Header=BB10_106 Depth=2
	sxtb	r1, r1
	str	r1, [r11]
	ldrb	r1, [r0, #1]!
	cmp	r1, #0
	bne	.LBB10_106
	b	.LBB10_133
.LBB10_108:                             @   in Loop: Header=BB10_2 Depth=1
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
.LBB10_109:                             @ %i2h.exit
                                        @   in Loop: Header=BB10_2 Depth=1
	ldrb	r1, [r0]
	b	.LBB10_111
.LBB10_110:                             @ %printc_uart.exit.i188
                                        @   in Loop: Header=BB10_111 Depth=2
	sxtb	r1, r1
	str	r1, [r11]
	ldrb	r1, [r0, #1]!
.LBB10_111:                             @ %i2h.exit
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Loop Header: Depth=2
                                        @       Child Loop BB10_112 Depth 3
	cmp	r1, #0
	beq	.LBB10_1
.LBB10_112:                             @ %do.body.i.i184
                                        @   Parent Loop BB10_2 Depth=1
                                        @     Parent Loop BB10_111 Depth=2
                                        @ =>    This Inner Loop Header: Depth=3
	ldr	r2, [r7]
	cmp	r2, #65536
	blo	.LBB10_112
	b	.LBB10_110
.LBB10_113:                             @   in Loop: Header=BB10_2 Depth=1
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
.LBB10_114:                             @ %l2h.exit
                                        @   in Loop: Header=BB10_2 Depth=1
	ldrb	r1, [r0]
	mov	r6, #37
	cmp	r1, #0
	beq	.LBB10_133
.LBB10_115:                             @ %do.body.i.i258
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldr	r2, [r7]
	cmp	r2, #65536
	blo	.LBB10_115
@ BB#116:                               @ %printc_uart.exit.i262
                                        @   in Loop: Header=BB10_115 Depth=2
	sxtb	r1, r1
	str	r1, [r11]
	ldrb	r1, [r0, #1]!
	cmp	r1, #0
	bne	.LBB10_115
	b	.LBB10_133
.LBB10_117:                             @ %if.then.i
                                        @   in Loop: Header=BB10_2 Depth=1
	rsbs	r5, r5, #0
	mov	r0, #1
	rsc	r6, r6, #0
	str	r0, [sp, #12]           @ 4-byte Spill
.LBB10_118:                             @ %if.end3.i
                                        @   in Loop: Header=BB10_2 Depth=1
	str	r10, [sp, #8]           @ 4-byte Spill
	movw	r10, :lower16:_MergedGlobals
	movt	r10, :upper16:_MergedGlobals
	str	lr, [sp, #4]            @ 4-byte Spill
	strb	r9, [r10, #94]
	mov	r9, #93
.LBB10_119:                             @ %for.body.i
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	mov	r0, r5
	mov	r1, r6
	mov	r2, #10
	mov	r3, #0
	mov	r8, #0
	bl	__moddi3
	add	r0, r0, #48
	mov	r1, r6
	strb	r0, [r10, r9]
	mov	r0, r5
	mov	r2, #10
	mov	r3, #0
	bl	__divdi3
	mov	r5, r0
	sub	r0, r9, #70
	mov	r6, r1
	sub	r9, r9, #1
	cmp	r0, #0
	bgt	.LBB10_119
.LBB10_120:                             @ %for.body8.i
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	add	r0, r10, r8
	ldrb	r0, [r0, #70]
	cmp	r0, #48
	bne	.LBB10_122
@ BB#121:                               @ %for.cond5.i
                                        @   in Loop: Header=BB10_120 Depth=2
	add	r8, r8, #1
	cmp	r8, #23
	ble	.LBB10_120
.LBB10_122:                             @ %for.end16.i
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	r5, [sp, #12]           @ 4-byte Reload
	mov	r6, #37
	cmp	r5, #0
	beq	.LBB10_124
@ BB#123:                               @ %if.then17.i
                                        @   in Loop: Header=BB10_2 Depth=1
	mov	r0, #45
	strb	r0, [r10, #95]
.LBB10_124:                             @ %for.cond19.preheader.i
                                        @   in Loop: Header=BB10_2 Depth=1
	ldr	lr, [sp, #4]            @ 4-byte Reload
	rsb	r0, r8, #24
	mov	r1, #0
	mov	r9, #0
	add	r12, sp, #35
	cmp	r0, #1
	blt	.LBB10_128
@ BB#125:                               @ %for.body22.lr.ph.i
                                        @   in Loop: Header=BB10_2 Depth=1
	mvn	r1, #69
	add	r2, r10, r5
	sub	r1, r1, r8
	add	r2, r2, #95
.LBB10_126:                             @ %for.body22.i
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r3, [r10, -r1]
	sub	r1, r1, #1
	cmn	r1, #94
	strb	r3, [r2], #1
	bne	.LBB10_126
@ BB#127:                               @   in Loop: Header=BB10_2 Depth=1
	mov	r1, r0
.LBB10_128:                             @ %for.end29.i
                                        @   in Loop: Header=BB10_2 Depth=1
	add	r0, r10, #95
	ldr	r10, [sp, #8]           @ 4-byte Reload
	add	r1, r1, r5
	add	r8, sp, #26
	strb	r9, [r0, r1]
.LBB10_129:                             @ %ltoa.exit
                                        @   in Loop: Header=BB10_2 Depth=1
	ldrb	r1, [r0]
	b	.LBB10_131
.LBB10_130:                             @ %printc_uart.exit.i328
                                        @   in Loop: Header=BB10_131 Depth=2
	sxtb	r1, r1
	str	r1, [r11]
	ldrb	r1, [r0, #1]!
.LBB10_131:                             @ %ltoa.exit
                                        @   Parent Loop BB10_2 Depth=1
                                        @ =>  This Loop Header: Depth=2
                                        @       Child Loop BB10_132 Depth 3
	cmp	r1, #0
	beq	.LBB10_133
.LBB10_132:                             @ %do.body.i.i324
                                        @   Parent Loop BB10_2 Depth=1
                                        @     Parent Loop BB10_131 Depth=2
                                        @ =>    This Inner Loop Header: Depth=3
	ldr	r2, [r7]
	cmp	r2, #65536
	blo	.LBB10_132
	b	.LBB10_130
.LBB10_133:                             @   in Loop: Header=BB10_2 Depth=1
	mov	r5, lr
	add	r5, r5, #1
	b	.LBB10_2
.LBB10_134:                             @ %while.end117
	mov	r0, #0
	add	sp, sp, #52
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
@ BB#0:                                 @ %entry
	push	{lr}
	mov	r1, r0
	movw	r0, :lower16:.L.str1
	movt	r0, :upper16:.L.str1
	bl	printf
.LBB11_1:                               @ %while.body
                                        @ =>This Inner Loop Header: Depth=1
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
@ BB#0:                                 @ %entry
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

	.ident	"clang version 3.5.2 (tags/RELEASE_352/final)"
