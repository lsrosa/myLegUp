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
	.file	"prof.arm.bc"
	.globl	legup_start_counter
	.align	2
	.type	legup_start_counter,%function
legup_start_counter:                    @ @legup_start_counter
	.fnstart
.Leh_func_begin0:
@ BB#0:                                 @ %entry
	movw	r1, :lower16:legup_counters
	@APP
	mrc	p15, #0, r2, c9, c13, #0
	@NO_APP
	movt	r1, :upper16:legup_counters
	str	r2, [r1, r0, lsl #2]
	bx	lr
.Ltmp0:
	.size	legup_start_counter, .Ltmp0-legup_start_counter
	.cantunwind
	.fnend

	.globl	legup_stop_counter
	.align	2
	.type	legup_stop_counter,%function
legup_stop_counter:                     @ @legup_stop_counter
	.fnstart
.Leh_func_begin1:
@ BB#0:                                 @ %entry
	movw	r2, :lower16:legup_counters
	@APP
	mrc	p15, #0, r1, c9, c13, #0
	@NO_APP
	movt	r2, :upper16:legup_counters
	ldr	r0, [r2, r0, lsl #2]
	sub	r0, r1, r0
	bx	lr
.Ltmp1:
	.size	legup_stop_counter, .Ltmp1-legup_stop_counter
	.cantunwind
	.fnend

	.globl	__legup_prof_init
	.align	2
	.type	__legup_prof_init,%function
__legup_prof_init:                      @ @__legup_prof_init
	.fnstart
.Leh_func_begin2:
@ BB#0:                                 @ %entry
	push	{r4, r5, lr}
	movw	r0, #36868
	mov	r1, #251658240
	movt	r0, #65392
	mvn	r2, #0
	str	r1, [r0]
	movw	r1, #20508
	movt	r1, #65488
	mov	r0, #0
	str	r0, [r1]
	movw	r1, #0
	movt	r1, #65535
	mov	r12, #164
	str	r0, [r1]
	movw	r1, :lower16:CURRENT
	movt	r1, :upper16:CURRENT
	movw	lr, #1023
	str	r2, [r1]
	movw	r1, :lower16:NEXT
	movt	r1, :upper16:NEXT
	mov	r3, #0
	str	r0, [r1]
	movw	r1, :lower16:entries
	movt	r1, :upper16:entries
	mov	r2, r1
.LBB2_1:                                @ %for.cond1.preheader
                                        @ =>This Loop Header: Depth=1
                                        @     Child Loop BB2_2 Depth 2
	mov	r4, #44
	mov	r5, r2
.LBB2_2:                                @ %for.body3
                                        @   Parent Loop BB2_1 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	strb	r0, [r5], #1
	subs	r4, r4, #1
	bne	.LBB2_2
@ BB#3:                                 @ %for.end
                                        @   in Loop: Header=BB2_1 Depth=1
	mla	r4, r3, r12, r1
	add	r2, r2, #164
	cmp	r3, lr
	str	r0, [r4, #88]
	str	r0, [r4, #92]
	str	r0, [r4, #96]
	str	r0, [r4, #100]
	str	r0, [r4, #124]
	str	r0, [r4, #104]
	str	r0, [r4, #128]
	str	r0, [r4, #108]
	str	r0, [r4, #132]
	str	r0, [r4, #112]
	str	r0, [r4, #136]
	str	r0, [r4, #116]
	str	r0, [r4, #140]
	str	r0, [r4, #120]
	str	r0, [r4, #144]
	str	r0, [r4, #148]
	str	r0, [r4, #152]
	str	r0, [r4, #156]
	str	r0, [r4, #160]
	add	r4, r3, #1
	mov	r3, r4
	bne	.LBB2_1
@ BB#4:                                 @ %for.cond31.preheader
	mov	r0, #0
	mov	r2, #3
	@APP
	mcr	p15, #0, r0, c9, c12, #5
	@NO_APP
	movw	r0, :lower16:events
	movt	r0, :upper16:events
	ldr	r1, [r0]
	@APP
	mcr	p15, #0, r1, c9, c13, #1
	@NO_APP
	mov	r1, #1
	@APP
	mcr	p15, #0, r1, c9, c12, #1
	@NO_APP
	@APP
	mcr	p15, #0, r1, c9, c12, #5
	@NO_APP
	ldr	r1, [r0, #4]
	@APP
	mcr	p15, #0, r1, c9, c13, #1
	@NO_APP
	mov	r1, #2
	@APP
	mcr	p15, #0, r1, c9, c12, #1
	@NO_APP
	@APP
	mcr	p15, #0, r1, c9, c12, #5
	@NO_APP
	ldr	r1, [r0, #8]
	@APP
	mcr	p15, #0, r1, c9, c13, #1
	@NO_APP
	mov	r1, #4
	@APP
	mcr	p15, #0, r1, c9, c12, #1
	@NO_APP
	@APP
	mcr	p15, #0, r2, c9, c12, #5
	@NO_APP
	ldr	r2, [r0, #12]
	@APP
	mcr	p15, #0, r2, c9, c13, #1
	@NO_APP
	mov	r2, #8
	@APP
	mcr	p15, #0, r2, c9, c12, #1
	@NO_APP
	@APP
	mcr	p15, #0, r1, c9, c12, #5
	@NO_APP
	ldr	r1, [r0, #16]
	movw	r2, :lower16:L2C_EV_COUNTER0_CFG
	movt	r2, :upper16:L2C_EV_COUNTER0_CFG
	@APP
	mcr	p15, #0, r1, c9, c13, #1
	@NO_APP
	mov	r1, #16
	@APP
	mcr	p15, #0, r1, c9, c12, #1
	@NO_APP
	mov	r1, #5
	@APP
	mcr	p15, #0, r1, c9, c12, #5
	@NO_APP
	ldr	r0, [r0, #20]
	@APP
	mcr	p15, #0, r0, c9, c13, #1
	@NO_APP
	mov	r0, #32
	@APP
	mcr	p15, #0, r0, c9, c12, #1
	@NO_APP
	movw	r0, :lower16:l2c_events
	ldr	r2, [r2]
	movt	r0, :upper16:l2c_events
	ldr	r1, [r0]
	lsl	r1, r1, #2
	str	r1, [r2]
	movw	r1, :lower16:L2C_EV_COUNTER1_CFG
	ldr	r0, [r0, #4]
	movt	r1, :upper16:L2C_EV_COUNTER1_CFG
	ldr	r1, [r1]
	lsl	r0, r0, #2
	str	r0, [r1]
	movw	r0, :lower16:L2C_EV_COUNTER_CTRL
	movt	r0, :upper16:L2C_EV_COUNTER_CTRL
	mov	r1, #7
	ldr	r0, [r0]
	str	r1, [r0]
	mov	r0, #-2147483648
	@APP
	mcr	p15, #0, r0, c9, c12, #1
	@NO_APP
	@APP
	mrc	p15, #0, r0, c9, c12, #0
	@NO_APP
	orr	r0, r0, #7
	@APP
	mcr	p15, #0, r0, c9, c12, #0
	@NO_APP
	pop	{r4, r5, pc}
.Ltmp2:
	.size	__legup_prof_init, .Ltmp2-__legup_prof_init
	.cantunwind
	.fnend

	.globl	update_counts
	.align	2
	.type	update_counts,%function
update_counts:                          @ @update_counts
	.fnstart
.Leh_func_begin3:
@ BB#0:                                 @ %entry
	push	{r4, r5, r6, r7, r8, r9, r10, r11, lr}
	sub	sp, sp, #24
	@APP
	mrc	p15, #0, r0, c9, c12, #0
	@NO_APP
	mov	r1, #0
	movw	r12, :lower16:entries
	movt	r12, :upper16:entries
	mov	lr, #164
	bfc	r0, #0, #1
	@APP
	mcr	p15, #0, r0, c9, c12, #0
	@NO_APP
	movw	r0, :lower16:L2C_EV_COUNTER_CTRL
	movt	r0, :upper16:L2C_EV_COUNTER_CTRL
	ldr	r0, [r0]
	str	r1, [r0]
	@APP
	mrc	p15, #0, r8, c9, c13, #0
	@NO_APP
	@APP
	mcr	p15, #0, r1, c9, c12, #5
	@NO_APP
	@APP
	mrc	p15, #0, r0, c9, c13, #2
	@NO_APP
	str	r0, [sp]
	mov	r0, #1
	@APP
	mcr	p15, #0, r0, c9, c12, #5
	@NO_APP
	@APP
	mrc	p15, #0, r0, c9, c13, #2
	@NO_APP
	str	r0, [sp, #4]
	mov	r0, #2
	@APP
	mcr	p15, #0, r0, c9, c12, #5
	@NO_APP
	@APP
	mrc	p15, #0, r0, c9, c13, #2
	@NO_APP
	str	r0, [sp, #8]
	mov	r0, #3
	@APP
	mcr	p15, #0, r0, c9, c12, #5
	@NO_APP
	@APP
	mrc	p15, #0, r0, c9, c13, #2
	@NO_APP
	str	r0, [sp, #12]
	mov	r0, #4
	@APP
	mcr	p15, #0, r0, c9, c12, #5
	@NO_APP
	@APP
	mrc	p15, #0, r0, c9, c13, #2
	@NO_APP
	str	r0, [sp, #16]
	mov	r0, #5
	@APP
	mcr	p15, #0, r0, c9, c12, #5
	@NO_APP
	@APP
	mrc	p15, #0, r0, c9, c13, #2
	@NO_APP
	str	r0, [sp, #20]
	movw	r0, :lower16:L2C_EV_COUNTER0
	movt	r0, :upper16:L2C_EV_COUNTER0
	ldr	r0, [r0]
	ldr	r9, [r0]
	movw	r0, :lower16:L2C_EV_COUNTER1
	movt	r0, :upper16:L2C_EV_COUNTER1
	ldr	r0, [r0]
	ldr	r10, [r0]
	movw	r0, :lower16:CURRENT
	movt	r0, :upper16:CURRENT
	ldr	r11, [sp]
	ldr	r7, [r0]
	ldr	r4, [sp, #4]
	ldr	r5, [sp, #8]
	mla	r2, r7, lr, r12
	ldr	r6, [sp, #12]
	ldr	r0, [r2, #148]
	add	r0, r0, r9
	str	r0, [r2, #148]
	ldr	r0, [r2, #152]
	add	r0, r0, r10
	str	r0, [r2, #152]
	ldr	r0, [r2, #92]
	add	r0, r0, r8
	str	r0, [r2, #92]
	ldr	r0, [r2, #100]
	add	r0, r0, r11
	str	r0, [r2, #100]
	ldr	r0, [r2, #104]
	add	r0, r0, r4
	str	r0, [r2, #104]
	ldr	r0, [r2, #108]
	add	r0, r0, r5
	str	r0, [r2, #108]
	ldr	r0, [r2, #112]
	add	r0, r0, r6
	str	r0, [r2, #112]
	ldr	r1, [r2, #116]
	ldr	r0, [sp, #16]
	add	r1, r1, r0
	str	r1, [r2, #116]
	ldr	r3, [r2, #120]
	ldr	r1, [sp, #20]
	add	r3, r3, r1
	str	r3, [r2, #120]
	b	.LBB3_2
.LBB3_1:                                @ %while.body
                                        @   in Loop: Header=BB3_2 Depth=1
	mla	r2, r7, lr, r12
	ldr	r3, [r2, #96]
	add	r3, r3, r8
	str	r3, [r2, #96]
	ldr	r3, [r2, #124]
	add	r3, r3, r11
	str	r3, [r2, #124]
	ldr	r3, [r2, #128]
	add	r3, r3, r4
	str	r3, [r2, #128]
	ldr	r3, [r2, #132]
	add	r3, r3, r5
	str	r3, [r2, #132]
	ldr	r3, [r2, #136]
	add	r3, r3, r6
	str	r3, [r2, #136]
	ldr	r3, [r2, #140]
	add	r3, r3, r0
	str	r3, [r2, #140]
	ldr	r3, [r2, #144]
	add	r3, r3, r1
	str	r3, [r2, #144]
	ldr	r3, [r2, #156]
	add	r3, r3, r9
	str	r3, [r2, #156]
	ldr	r3, [r2, #160]
	add	r3, r3, r10
	str	r3, [r2, #160]
	ldr	r7, [r2, #84]
.LBB3_2:                                @ %while.body
                                        @ =>This Inner Loop Header: Depth=1
	cmp	r7, #0
	bge	.LBB3_1
@ BB#3:                                 @ %while.end
	add	sp, sp, #24
	pop	{r4, r5, r6, r7, r8, r9, r10, r11, pc}
.Ltmp3:
	.size	update_counts, .Ltmp3-update_counts
	.cantunwind
	.fnend

	.globl	reset_counts
	.align	2
	.type	reset_counts,%function
reset_counts:                           @ @reset_counts
	.fnstart
.Leh_func_begin4:
@ BB#0:                                 @ %entry
	@APP
	mrc	p15, #0, r0, c9, c12, #0
	@NO_APP
	mov	r1, #7
	orr	r0, r0, #7
	@APP
	mcr	p15, #0, r0, c9, c12, #0
	@NO_APP
	movw	r0, :lower16:L2C_EV_COUNTER_CTRL
	movt	r0, :upper16:L2C_EV_COUNTER_CTRL
	ldr	r0, [r0]
	str	r1, [r0]
	bx	lr
.Ltmp4:
	.size	reset_counts, .Ltmp4-reset_counts
	.cantunwind
	.fnend

	.globl	__legup_prof_begin
	.align	2
	.type	__legup_prof_begin,%function
__legup_prof_begin:                     @ @__legup_prof_begin
	.fnstart
.Leh_func_begin5:
@ BB#0:                                 @ %entry
	push	{r4, r5, r6, r7, r8, r9, r10, r11, lr}
	mov	r4, r0
	bl	update_counts
	movw	r0, :lower16:NEXT
	movw	lr, :lower16:CURRENT
	movt	r0, :upper16:NEXT
	movt	lr, :upper16:CURRENT
	ldr	r2, [r0]
	ldr	r10, [lr]
	cmp	r2, #1
	blt	.LBB5_18
@ BB#1:                                 @ %while.cond.preheader.lr.ph
	ldrb	r11, [r4]
	movw	r7, :lower16:entries
	movt	r7, :upper16:entries
	mov	r0, #0
	cmp	r11, #0
	beq	.LBB5_14
@ BB#2:
	movw	r8, :lower16:entries
	mov	r9, #164
	movt	r8, :upper16:entries
.LBB5_3:                                @ %while.body.lr.ph.us
                                        @ =>This Loop Header: Depth=1
                                        @     Child Loop BB5_4 Depth 2
	mov	r3, #0
	mov	r1, r7
	mov	r5, r11
.LBB5_4:                                @ %while.body.us
                                        @   Parent Loop BB5_3 Depth=1
                                        @ =>  This Inner Loop Header: Depth=2
	ldrb	r6, [r1]
	uxtb	r5, r5
	cmp	r5, r6
	bne	.LBB5_7
@ BB#5:                                 @ %while.cond.us
                                        @   in Loop: Header=BB5_4 Depth=2
	add	r12, r3, #1
	add	r3, r4, r3
	ldrb	r5, [r3, #1]
	mov	r6, #1
	cmp	r5, #0
	beq	.LBB5_8
@ BB#6:                                 @ %while.cond.us
                                        @   in Loop: Header=BB5_4 Depth=2
	add	r1, r1, #1
	mov	r3, r12
	cmp	r12, #82
	ble	.LBB5_4
	b	.LBB5_8
.LBB5_7:                                @ %while.endthread-pre-split.us
                                        @   in Loop: Header=BB5_3 Depth=1
	ldrb	r5, [r4, r3]
	mov	r6, #0
	mov	r12, r3
.LBB5_8:                                @ %while.end.us
                                        @   in Loop: Header=BB5_3 Depth=1
	cmp	r5, #0
	bne	.LBB5_11
@ BB#9:                                 @ %land.lhs.true.us
                                        @   in Loop: Header=BB5_3 Depth=1
	mla	r1, r0, r9, r8
	ldrb	r1, [r1, r12]
	cmp	r1, #32
	beq	.LBB5_12
@ BB#10:                                @ %land.lhs.true.us
                                        @   in Loop: Header=BB5_3 Depth=1
	cmp	r1, #0
	bne	.LBB5_13
.LBB5_11:                               @ %if.end32.us
                                        @   in Loop: Header=BB5_3 Depth=1
	cmp	r6, #0
	beq	.LBB5_13
.LBB5_12:                               @ %if.then33.us
                                        @   in Loop: Header=BB5_3 Depth=1
	mla	r1, r0, r9, r8
	ldr	r1, [r1, #84]
	cmp	r1, r10
	beq	.LBB5_19
.LBB5_13:                               @ %for.inc.us
                                        @   in Loop: Header=BB5_3 Depth=1
	add	r0, r0, #1
	add	r7, r7, #164
	cmp	r0, r2
	blt	.LBB5_3
	b	.LBB5_18
.LBB5_14:
	add	r1, r7, #84
.LBB5_15:                               @ %land.lhs.true
                                        @ =>This Inner Loop Header: Depth=1
	sub	r3, r1, #84
	ldrb	r3, [r3]
	orr	r3, r3, #32
	uxtb	r3, r3
	cmp	r3, #32
	bne	.LBB5_17
@ BB#16:                                @ %if.then33
                                        @   in Loop: Header=BB5_15 Depth=1
	ldr	r3, [r1]
	cmp	r3, r10
	beq	.LBB5_19
.LBB5_17:                               @ %for.inc
                                        @   in Loop: Header=BB5_15 Depth=1
	add	r0, r0, #1
	add	r1, r1, #164
	cmp	r0, r2
	blt	.LBB5_15
.LBB5_18:                               @ %if.then42
	movw	r0, :lower16:entries
	mov	r1, #164
	movt	r0, :upper16:entries
	str	r2, [lr]
	mla	r0, r2, r1, r0
	movw	r1, :lower16:NEXT
	movt	r1, :upper16:NEXT
	str	r10, [r0, #84]
	add	r0, r2, #1
	str	r0, [r1]
	b	.LBB5_20
.LBB5_19:
	mov	r2, r0
	str	r2, [lr]
.LBB5_20:                               @ %do.body.preheader
	movw	r0, :lower16:entries
	mov	r1, #164
	movt	r0, :upper16:entries
	ldrb	r7, [r4], #1
	mla	r3, r2, r1, r0
	mov	r1, #0
.LBB5_21:                               @ %do.body
                                        @ =>This Inner Loop Header: Depth=1
	strb	r7, [r3, r1]
	ldrb	r7, [r4, r1]
	add	r1, r1, #1
	cmp	r7, #0
	beq	.LBB5_23
@ BB#22:                                @ %do.body
                                        @   in Loop: Header=BB5_21 Depth=1
	cmp	r1, #83
	blt	.LBB5_21
.LBB5_23:                               @ %while.cond61.preheader
	mov	r3, #164
	cmp	r1, #15
	mla	r3, r2, r3, r0
	bgt	.LBB5_27
@ BB#24:                                @ %while.body64.preheader
	mov	r7, #32
.LBB5_25:                               @ %while.body64
                                        @ =>This Inner Loop Header: Depth=1
	strb	r7, [r3, r1]
	add	r1, r1, #1
	cmp	r1, #16
	bne	.LBB5_25
@ BB#26:                                @ %while.cond61.while.end69_crit_edge
	mov	r1, #164
	mla	r0, r2, r1, r0
	add	r0, r0, #16
	b	.LBB5_28
.LBB5_27:
	add	r0, r3, r1
.LBB5_28:                               @ %while.end69
	mov	r1, #0
	strb	r1, [r0]
	mov	r1, #7
	ldr	r0, [r3, #88]
	add	r0, r0, #1
	str	r0, [r3, #88]
	@APP
	mrc	p15, #0, r0, c9, c12, #0
	@NO_APP
	orr	r0, r0, #7
	@APP
	mcr	p15, #0, r0, c9, c12, #0
	@NO_APP
	movw	r0, :lower16:L2C_EV_COUNTER_CTRL
	movt	r0, :upper16:L2C_EV_COUNTER_CTRL
	ldr	r0, [r0]
	str	r1, [r0]
	pop	{r4, r5, r6, r7, r8, r9, r10, r11, pc}
.Ltmp5:
	.size	__legup_prof_begin, .Ltmp5-__legup_prof_begin
	.cantunwind
	.fnend

	.globl	__legup_prof_end
	.align	2
	.type	__legup_prof_end,%function
__legup_prof_end:                       @ @__legup_prof_end
	.fnstart
.Leh_func_begin6:
@ BB#0:                                 @ %entry
	push	{lr}
	bl	update_counts
	movw	r0, :lower16:CURRENT
	movw	r2, :lower16:entries
	movt	r0, :upper16:CURRENT
	movt	r2, :upper16:entries
	ldr	r1, [r0]
	mov	r3, #164
	mla	r1, r1, r3, r2
	ldr	r1, [r1, #84]
	str	r1, [r0]
	@APP
	mrc	p15, #0, r0, c9, c12, #0
	@NO_APP
	mov	r1, #7
	orr	r0, r0, #7
	@APP
	mcr	p15, #0, r0, c9, c12, #0
	@NO_APP
	movw	r0, :lower16:L2C_EV_COUNTER_CTRL
	movt	r0, :upper16:L2C_EV_COUNTER_CTRL
	ldr	r0, [r0]
	str	r1, [r0]
	pop	{lr}
	bx	lr
.Ltmp6:
	.size	__legup_prof_end, .Ltmp6-__legup_prof_end
	.cantunwind
	.fnend

	.globl	__legup_prof_print
	.align	2
	.type	__legup_prof_print,%function
__legup_prof_print:                     @ @__legup_prof_print
	.fnstart
.Leh_func_begin7:
@ BB#0:                                 @ %entry
	push	{r4, r5, r6, r7, r8, r9, r10, r11, lr}
	movw	r0, :lower16:.L.str
	movt	r0, :upper16:.L.str
	bl	printf
	movw	r0, :lower16:.L.str1
	movt	r0, :upper16:.L.str1
	bl	printf
	movw	r6, :lower16:events
	movw	r4, :lower16:.L.str2
	movt	r6, :upper16:events
	movt	r4, :upper16:.L.str2
	ldr	r1, [r6]
	mov	r0, r4
	bl	printf
	movw	r5, :lower16:.L.str3
	movt	r5, :upper16:.L.str3
	mov	r0, r5
	bl	printf
	ldr	r1, [r6, #4]
	mov	r0, r4
	bl	printf
	mov	r0, r5
	bl	printf
	ldr	r1, [r6, #8]
	mov	r0, r4
	bl	printf
	mov	r0, r5
	bl	printf
	ldr	r1, [r6, #12]
	mov	r0, r4
	bl	printf
	mov	r0, r5
	bl	printf
	ldr	r1, [r6, #16]
	mov	r0, r4
	bl	printf
	mov	r0, r5
	bl	printf
	ldr	r1, [r6, #20]
	mov	r0, r4
	bl	printf
	mov	r0, r5
	bl	printf
	movw	r4, :lower16:l2c_events
	movw	r0, :lower16:.L.str4
	movt	r4, :upper16:l2c_events
	movt	r0, :upper16:.L.str4
	ldr	r1, [r4]
	bl	printf
	mov	r0, r5
	bl	printf
	ldr	r1, [r4, #4]
	movw	r0, :lower16:.L.str5
	movt	r0, :upper16:.L.str5
	bl	printf
	movw	r0, :lower16:.L.str6
	movt	r0, :upper16:.L.str6
	bl	printf
	movw	r4, :lower16:.L.str7
	mov	r5, #200
	movt	r4, :upper16:.L.str7
.LBB7_1:                                @ %for.body13
                                        @ =>This Inner Loop Header: Depth=1
	mov	r0, r4
	bl	printf
	subs	r5, r5, #1
	bne	.LBB7_1
@ BB#2:                                 @ %for.end17
	movw	r0, :lower16:.L.str6
	movt	r0, :upper16:.L.str6
	bl	printf
	movw	r9, :lower16:NEXT
	movt	r9, :upper16:NEXT
	ldr	r0, [r9]
	cmp	r0, #0
	beq	.LBB7_8
@ BB#3:
	movw	r10, :lower16:entries
	movw	r5, :lower16:.L.str8
	movw	r6, :lower16:.L.str9
	movw	r11, :lower16:.L.str6
	movt	r10, :upper16:entries
	mov	r4, #0
	add	r7, r10, #160
	movt	r5, :upper16:.L.str8
	movt	r6, :upper16:.L.str9
	mov	r8, #164
	movt	r11, :upper16:.L.str6
.LBB7_4:                                @ %while.body
                                        @ =>This Inner Loop Header: Depth=1
	mov	r0, r5
	mov	r1, r4
	bl	printf
	sub	r1, r7, #160
	mov	r0, r6
	bl	printf
	sub	r0, r7, #76
	ldr	r0, [r0]
	cmp	r0, #0
	blt	.LBB7_6
@ BB#5:                                 @ %if.then25
                                        @   in Loop: Header=BB7_4 Depth=1
	mla	r1, r0, r8, r10
	mov	r0, r6
	bl	printf
	b	.LBB7_7
.LBB7_6:                                @ %if.else
                                        @   in Loop: Header=BB7_4 Depth=1
	movw	r0, :lower16:.L.str10
	movt	r0, :upper16:.L.str10
	bl	printf
.LBB7_7:                                @ %if.end33
                                        @   in Loop: Header=BB7_4 Depth=1
	sub	r0, r7, #72
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #68
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #64
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #60
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #36
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #56
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #32
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #52
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #28
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #48
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #24
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #44
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #20
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #40
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #16
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #12
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #4
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	sub	r0, r7, #8
	ldr	r1, [r0]
	mov	r0, r5
	bl	printf
	ldr	r1, [r7]
	mov	r0, r5
	bl	printf
	mov	r0, r11
	bl	printf
	ldr	r0, [r9]
	add	r4, r4, #1
	add	r7, r7, #164
	cmp	r4, r0
	bne	.LBB7_4
.LBB7_8:                                @ %while.end
	pop	{r4, r5, r6, r7, r8, r9, r10, r11, pc}
.Ltmp7:
	.size	__legup_prof_print, .Ltmp7-__legup_prof_print
	.cantunwind
	.fnend

	.type	events,%object          @ @events
	.data
	.globl	events
	.align	2
events:
	.long	16                      @ 0x10
	.long	18                      @ 0x12
	.long	96                      @ 0x60
	.long	97                      @ 0x61
	.long	104                     @ 0x68
	.long	112                     @ 0x70
	.size	events, 24

	.type	l2c_events,%object      @ @l2c_events
	.globl	l2c_events
	.align	2
l2c_events:
	.long	2                       @ 0x2
	.long	3                       @ 0x3
	.size	l2c_events, 8

	.type	L2C_BASE,%object        @ @L2C_BASE
	.globl	L2C_BASE
	.align	2
L2C_BASE:
	.long	4294897664
	.size	L2C_BASE, 4

	.type	L2C_EV_COUNTER_CTRL,%object @ @L2C_EV_COUNTER_CTRL
	.globl	L2C_EV_COUNTER_CTRL
	.align	2
L2C_EV_COUNTER_CTRL:
	.long	4294898176
	.size	L2C_EV_COUNTER_CTRL, 4

	.type	L2C_EV_COUNTER0_CFG,%object @ @L2C_EV_COUNTER0_CFG
	.globl	L2C_EV_COUNTER0_CFG
	.align	2
L2C_EV_COUNTER0_CFG:
	.long	4294898180
	.size	L2C_EV_COUNTER0_CFG, 4

	.type	L2C_EV_COUNTER1_CFG,%object @ @L2C_EV_COUNTER1_CFG
	.globl	L2C_EV_COUNTER1_CFG
	.align	2
L2C_EV_COUNTER1_CFG:
	.long	4294898184
	.size	L2C_EV_COUNTER1_CFG, 4

	.type	L2C_EV_COUNTER0,%object @ @L2C_EV_COUNTER0
	.globl	L2C_EV_COUNTER0
	.align	2
L2C_EV_COUNTER0:
	.long	4294898188
	.size	L2C_EV_COUNTER0, 4

	.type	L2C_EV_COUNTER1,%object @ @L2C_EV_COUNTER1
	.globl	L2C_EV_COUNTER1
	.align	2
L2C_EV_COUNTER1:
	.long	4294898192
	.size	L2C_EV_COUNTER1, 4

	.type	legup_counters,%object  @ @legup_counters
	.comm	legup_counters,256,4
	.type	CURRENT,%object         @ @CURRENT
	.comm	CURRENT,4,4
	.type	NEXT,%object            @ @NEXT
	.comm	NEXT,4,4
	.type	entries,%object         @ @entries
	.comm	entries,167936,4
	.type	.L.str,%object          @ @.str
	.section	.rodata.str1.1,"aMS",%progbits,1
.L.str:
	.asciz	"\t\t\t\t\t\t\t\t    Cycles\t\t\t  Events (hier)\n"
	.size	.L.str, 38

	.type	.L.str1,%object         @ @.str1
.L.str1:
	.asciz	"index\tfunction\t\tparent\t\t\tcalls\tself\thier\t"
	.size	.L.str1, 42

	.type	.L.str2,%object         @ @.str2
.L.str2:
	.asciz	"ev 0x%x\t"
	.size	.L.str2, 9

	.type	.L.str3,%object         @ @.str3
.L.str3:
	.asciz	"\t"
	.size	.L.str3, 2

	.type	.L.str4,%object         @ @.str4
.L.str4:
	.asciz	"L2C 0x%x\t"
	.size	.L.str4, 10

	.type	.L.str5,%object         @ @.str5
.L.str5:
	.asciz	"L2C 0x%x"
	.size	.L.str5, 9

	.type	.L.str6,%object         @ @.str6
.L.str6:
	.asciz	"\n"
	.size	.L.str6, 2

	.type	.L.str7,%object         @ @.str7
.L.str7:
	.asciz	"="
	.size	.L.str7, 2

	.type	.L.str8,%object         @ @.str8
.L.str8:
	.asciz	"%d\t"
	.size	.L.str8, 4

	.type	.L.str9,%object         @ @.str9
.L.str9:
	.asciz	"%s\t"
	.size	.L.str9, 4

	.type	.L.str10,%object        @ @.str10
.L.str10:
	.asciz	"---\t\t\t"
	.size	.L.str10, 7


	.ident	"clang version 3.5.2 (tags/RELEASE_352/final)"
