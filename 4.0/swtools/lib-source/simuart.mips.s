	.section .mdebug.abi32
	.previous
	.file	"simuart.mips.ll"
	.text
	.globl	printc_uart
	.align	2
	.type	printc_uart,@function
	.ent	printc_uart             # @printc_uart
printc_uart:
$tmp0:
	.cfi_startproc
	.frame	$sp,0,$ra
	.mask 	0x00000000,0
	.fmask	0x00000000,0
# BB#0:
	.set	noreorder
	.set	nomacro
	lui	$2, 45056
	ori	$2, $2, 0
	sw	$4, 0($2)
	jr	$ra
	nop
	.set	macro
	.set	reorder
	.end	printc_uart
$tmp1:
	.size	printc_uart, ($tmp1)-printc_uart
$tmp2:
	.cfi_endproc
$eh_func_end0:

	.globl	print_uart
	.align	2
	.type	print_uart,@function
	.ent	print_uart              # @print_uart
print_uart:
$tmp3:
	.cfi_startproc
	.frame	$sp,0,$ra
	.mask 	0x00000000,0
	.fmask	0x00000000,0
# BB#0:
	.set	noreorder
	.set	nomacro
	lbu	$3, 0($4)
	nop
	beq	$3, $zero, $BB1_3
	nop
# BB#1:                                 # %.lr.ph.preheader
	addiu	$2, $4, 1
$BB1_2:                                 # %.lr.ph
                                        # =>This Inner Loop Header: Depth=1
	sll	$3, $3, 24
	lui	$4, 45056
	sra	$3, $3, 24
	ori	$4, $4, 0
	sw	$3, 0($4)
	lbu	$3, 0($2)
	nop
	addiu	$2, $2, 1
	bne	$3, $zero, $BB1_2
	nop
$BB1_3:                                 # %._crit_edge
	jr	$ra
	nop
	.set	macro
	.set	reorder
	.end	print_uart
$tmp4:
	.size	print_uart, ($tmp4)-print_uart
$tmp5:
	.cfi_endproc
$eh_func_end1:

	.globl	_i2h
	.align	2
	.type	_i2h,@function
	.ent	_i2h                    # @_i2h
_i2h:
$tmp6:
	.cfi_startproc
	.frame	$sp,0,$ra
	.mask 	0x00000000,0
	.fmask	0x00000000,0
# BB#0:
	.set	noreorder
	.set	nomacro
	slti	$2, $4, 10
	bne	$2, $zero, $BB2_2
	nop
# BB#1:
	addiu	$2, $zero, 87
	j	$BB2_3
	nop
$BB2_2:
	addiu	$2, $zero, 48
$BB2_3:
	addu	$2, $2, $4
	sll	$2, $2, 24
	sra	$2, $2, 24
	jr	$ra
	nop
	.set	macro
	.set	reorder
	.end	_i2h
$tmp7:
	.size	_i2h, ($tmp7)-_i2h
$tmp8:
	.cfi_endproc
$eh_func_end2:

	.globl	_i2H
	.align	2
	.type	_i2H,@function
	.ent	_i2H                    # @_i2H
_i2H:
$tmp9:
	.cfi_startproc
	.frame	$sp,0,$ra
	.mask 	0x00000000,0
	.fmask	0x00000000,0
# BB#0:
	.set	noreorder
	.set	nomacro
	slti	$2, $4, 10
	bne	$2, $zero, $BB3_2
	nop
# BB#1:
	addiu	$2, $zero, 55
	j	$BB3_3
	nop
$BB3_2:
	addiu	$2, $zero, 48
$BB3_3:
	addu	$2, $2, $4
	sll	$2, $2, 24
	sra	$2, $2, 24
	jr	$ra
	nop
	.set	macro
	.set	reorder
	.end	_i2H
$tmp10:
	.size	_i2H, ($tmp10)-_i2H
$tmp11:
	.cfi_endproc
$eh_func_end3:

	.globl	i2h
	.align	2
	.type	i2h,@function
	.ent	i2h                     # @i2h
i2h:
$tmp14:
	.cfi_startproc
	.frame	$sp,56,$ra
	.mask 	0x801F0000,-4
	.fmask	0x00000000,0
# BB#0:
	.set	noreorder
	.set	nomacro
	addiu	$sp, $sp, -56
$tmp15:
	.cfi_def_cfa_offset 56
	sw	$ra, 52($sp)
	sw	$20, 48($sp)
	sw	$19, 44($sp)
	sw	$18, 40($sp)
	sw	$17, 36($sp)
	sw	$16, 32($sp)
$tmp16:
	.cfi_offset 31, -4
$tmp17:
	.cfi_offset 20, -8
$tmp18:
	.cfi_offset 19, -12
$tmp19:
	.cfi_offset 18, -16
$tmp20:
	.cfi_offset 17, -20
$tmp21:
	.cfi_offset 16, -24
	addu	$16, $zero, $4
	lui	$3, %hi(_i2h)
	lui	$4, %hi(_i2H)
	lui	$2, %hi($.str)
	beq	$5, $zero, $BB4_2
	nop
# BB#1:
	addiu	$18, $4, %lo(_i2H)
	j	$BB4_3
	nop
$BB4_2:
	addiu	$18, $3, %lo(_i2h)
$BB4_3:
	bne	$16, $zero, $BB4_5
	nop
# BB#4:
	addiu	$2, $2, %lo($.str)
	j	$BB4_11
	nop
$BB4_5:
	addiu	$19, $zero, 0
$BB4_6:                                 # %.lr.ph5
                                        # =>This Inner Loop Header: Depth=1
	addu	$20, $zero, $19
	addiu	$17, $sp, 20
	andi	$4, $16, 15
	srl	$16, $16, 4
	addiu	$19, $20, 1
	jalr	$18
	nop
	addu	$3, $17, $20
	sb	$2, 0($3)
	bne	$16, $zero, $BB4_6
	nop
# BB#7:                                 # %._crit_edge
	lui	$2, %hi(i2h.hex)
	addiu	$2, $2, %lo(i2h.hex)
	addiu	$3, $zero, 0
	addu	$4, $2, $19
	sb	$3, 0($4)
	bltz	$20, $BB4_11
	nop
# BB#8:                                 # %.lr.ph.preheader
	lui	$3, %hi(i2h.hex)
	addiu	$2, $zero, 0
	subu	$2, $2, $19
	addiu	$3, $3, %lo(i2h.hex)
$BB4_9:                                 # %.lr.ph
                                        # =>This Inner Loop Header: Depth=1
	lui	$4, %hi(i2h.hex)
	subu	$5, $17, $2
	lbu	$5, -1($5)
	nop
	addiu	$2, $2, 1
	addiu	$6, $3, 1
	sb	$5, 0($3)
	addu	$3, $zero, $6
	bne	$2, $zero, $BB4_9
	nop
# BB#10:
	addiu	$2, $4, %lo(i2h.hex)
$BB4_11:                                # %.loopexit
	lw	$16, 32($sp)
	nop
	lw	$17, 36($sp)
	nop
	lw	$18, 40($sp)
	nop
	lw	$19, 44($sp)
	nop
	lw	$20, 48($sp)
	nop
	lw	$ra, 52($sp)
	nop
	addiu	$sp, $sp, 56
	jr	$ra
	nop
	.set	macro
	.set	reorder
	.end	i2h
$tmp22:
	.size	i2h, ($tmp22)-i2h
$tmp23:
	.cfi_endproc
$eh_func_end4:

	.globl	l2h
	.align	2
	.type	l2h,@function
	.ent	l2h                     # @l2h
l2h:
$tmp26:
	.cfi_startproc
	.frame	$sp,64,$ra
	.mask 	0x803F0000,-4
	.fmask	0x00000000,0
# BB#0:
	.set	noreorder
	.set	nomacro
	addiu	$sp, $sp, -64
$tmp27:
	.cfi_def_cfa_offset 64
	sw	$ra, 60($sp)
	sw	$21, 56($sp)
	sw	$20, 52($sp)
	sw	$19, 48($sp)
	sw	$18, 44($sp)
	sw	$17, 40($sp)
	sw	$16, 36($sp)
$tmp28:
	.cfi_offset 31, -4
$tmp29:
	.cfi_offset 21, -8
$tmp30:
	.cfi_offset 20, -12
$tmp31:
	.cfi_offset 19, -16
$tmp32:
	.cfi_offset 18, -20
$tmp33:
	.cfi_offset 17, -24
$tmp34:
	.cfi_offset 16, -28
	addu	$16, $zero, $5
	addu	$17, $zero, $4
	lui	$3, %hi(_i2h)
	lui	$4, %hi(_i2H)
	lui	$2, %hi($.str)
	beq	$6, $zero, $BB5_2
	nop
# BB#1:
	addiu	$20, $4, %lo(_i2H)
	j	$BB5_3
	nop
$BB5_2:
	addiu	$20, $3, %lo(_i2h)
$BB5_3:
	or	$3, $17, $16
	bne	$3, $zero, $BB5_5
	nop
# BB#4:
	addiu	$2, $2, %lo($.str)
	j	$BB5_10
	nop
$BB5_5:
	addiu	$18, $zero, 0
$BB5_6:                                 # %.lr.ph5
                                        # =>This Inner Loop Header: Depth=1
	addu	$21, $zero, $18
	addiu	$19, $sp, 16
	andi	$4, $17, 15
	srl	$2, $17, 4
	sll	$3, $16, 28
	addiu	$18, $21, 1
	or	$17, $2, $3
	srl	$16, $16, 4
	jalr	$20
	nop
	addu	$4, $19, $21
	or	$3, $17, $16
	sb	$2, 0($4)
	bne	$3, $zero, $BB5_6
	nop
# BB#7:                                 # %._crit_edge
	lui	$2, %hi(l2h.hex)
	addiu	$2, $2, %lo(l2h.hex)
	addiu	$3, $zero, 0
	addu	$4, $2, $18
	sb	$3, 0($4)
	bltz	$21, $BB5_10
	nop
# BB#8:
	addu	$4, $zero, $3
	addiu	$8, $zero, 0
$BB5_9:                                 # %.lr.ph
                                        # =>This Inner Loop Header: Depth=1
	addiu	$5, $3, 1
	addiu	$6, $zero, 1
	lui	$2, %hi(l2h.hex)
	sltu	$7, $5, $6
	addiu	$2, $2, %lo(l2h.hex)
	addu	$6, $19, $18
	addu	$7, $7, $8
	addiu	$18, $18, -1
	addu	$4, $4, $7
	lbu	$6, -1($6)
	nop
	addu	$3, $2, $3
	sb	$6, 0($3)
	addu	$3, $zero, $5
	bgtz	$18, $BB5_9
	nop
$BB5_10:                                # %.loopexit
	lw	$16, 36($sp)
	nop
	lw	$17, 40($sp)
	nop
	lw	$18, 44($sp)
	nop
	lw	$19, 48($sp)
	nop
	lw	$20, 52($sp)
	nop
	lw	$21, 56($sp)
	nop
	lw	$ra, 60($sp)
	nop
	addiu	$sp, $sp, 64
	jr	$ra
	nop
	.set	macro
	.set	reorder
	.end	l2h
$tmp35:
	.size	l2h, ($tmp35)-l2h
$tmp36:
	.cfi_endproc
$eh_func_end5:

	.globl	itoa
	.align	2
	.type	itoa,@function
	.ent	itoa                    # @itoa
itoa:
$tmp39:
	.cfi_startproc
	.frame	$sp,40,$ra
	.mask 	0x00FF0000,-4
	.fmask	0x00000000,0
# BB#0:
	.set	noreorder
	.set	nomacro
	addiu	$sp, $sp, -40
$tmp40:
	.cfi_def_cfa_offset 40
	sw	$23, 36($sp)
	sw	$22, 32($sp)
	sw	$21, 28($sp)
	sw	$20, 24($sp)
	sw	$19, 20($sp)
	sw	$18, 16($sp)
	sw	$17, 12($sp)
	sw	$16, 8($sp)
$tmp41:
	.cfi_offset 23, -4
$tmp42:
	.cfi_offset 22, -8
$tmp43:
	.cfi_offset 21, -12
$tmp44:
	.cfi_offset 20, -16
$tmp45:
	.cfi_offset 19, -20
$tmp46:
	.cfi_offset 18, -24
$tmp47:
	.cfi_offset 17, -28
$tmp48:
	.cfi_offset 16, -32
	addiu	$2, $zero, -1
	slt	$2, $2, $4
	bne	$2, $zero, $BB6_2
	nop
# BB#1:
	addiu	$2, $zero, 0
	subu	$4, $2, $4
	addiu	$2, $zero, 1
	j	$BB6_5
	nop
$BB6_2:
	lui	$2, %hi($.str)
	bne	$4, $zero, $BB6_4
	nop
# BB#3:
	addiu	$2, $2, %lo($.str)
	j	$BB6_17
	nop
$BB6_4:
	addiu	$2, $zero, 0
$BB6_5:
	sw	$2, 4($sp)
	lui	$2, 26214
	lui	$5, 20971
	ori	$2, $2, 26215
	mult	$4, $2
	mfhi	$3
	lui	$6, 4194
	ori	$5, $5, 34079
	mult	$4, $5
	mfhi	$5
	lui	$8, 26843
	ori	$6, $6, 19923
	sra	$9, $3, 2
	srl	$3, $3, 31
	mult	$4, $6
	mfhi	$6
	lui	$7, 5368
	ori	$8, $8, 35757
	addu	$12, $9, $3
	sra	$3, $5, 5
	srl	$10, $5, 31
	mult	$4, $8
	mfhi	$8
	mult	$12, $2
	mfhi	$5
	lui	$9, 17179
	ori	$7, $7, 46473
	addu	$11, $3, $10
	sra	$3, $6, 6
	srl	$6, $6, 31
	mult	$4, $7
	mfhi	$10
	mult	$11, $2
	mfhi	$7
	lui	$15, 27487
	ori	$13, $9, 56963
	addu	$9, $3, $6
	addiu	$6, $zero, 10
	sra	$24, $8, 12
	srl	$8, $8, 31
	srl	$25, $5, 2
	srl	$16, $5, 31
	mult	$4, $13
	mfhi	$14
	mult	$9, $2
	mfhi	$13
	lui	$5, %hi(itoa.buf)
	sw	$5, 0($sp)
	lui	$17, 21990
	ori	$15, $15, 51819
	addu	$8, $24, $8
	addu	$16, $25, $16
	mult	$12, $6
	mflo	$25
	sra	$18, $10, 13
	srl	$10, $10, 31
	srl	$19, $7, 2
	srl	$3, $7, 31
	mult	$4, $15
	mfhi	$24
	mult	$8, $2
	mfhi	$15
	lui	$20, 17592
	ori	$21, $17, 15241
	addu	$10, $18, $10
	addu	$19, $19, $3
	mult	$16, $6
	mflo	$17
	subu	$16, $4, $25
	addiu	$7, $5, %lo(itoa.buf)
	addiu	$18, $zero, 0
	sra	$22, $14, 18
	srl	$23, $14, 31
	srl	$3, $13, 2
	srl	$5, $13, 31
	mult	$4, $21
	mfhi	$25
	mult	$10, $2
	mfhi	$14
	ori	$21, $20, 12193
	addu	$13, $22, $23
	addu	$20, $3, $5
	mult	$19, $6
	mflo	$19
	subu	$17, $12, $17
	sra	$22, $24, 22
	srl	$23, $24, 31
	srl	$24, $15, 2
	srl	$3, $15, 31
	addiu	$16, $16, 48
	sb	$18, 10($7)
	mult	$4, $21
	mfhi	$15
	mult	$13, $2
	mfhi	$12
	addu	$4, $22, $23
	addu	$21, $24, $3
	mult	$20, $6
	mflo	$20
	subu	$24, $11, $19
	sra	$11, $25, 25
	srl	$25, $25, 31
	srl	$3, $14, 2
	srl	$5, $14, 31
	addiu	$18, $17, 48
	sb	$16, 9($7)
	mult	$4, $2
	mfhi	$14
	addu	$11, $11, $25
	addu	$17, $3, $5
	mult	$21, $6
	mflo	$16
	subu	$25, $9, $20
	sra	$9, $15, 28
	srl	$19, $15, 31
	srl	$3, $12, 2
	srl	$5, $12, 31
	addiu	$15, $24, 48
	sb	$18, 8($7)
	mult	$11, $2
	mfhi	$12
	addu	$9, $9, $19
	addu	$18, $3, $5
	mult	$17, $6
	mflo	$24
	subu	$8, $8, $16
	srl	$3, $14, 2
	srl	$5, $14, 31
	addiu	$14, $25, 48
	sb	$15, 7($7)
	mult	$9, $2
	mfhi	$2
	addu	$16, $3, $5
	mult	$18, $6
	mflo	$25
	subu	$10, $10, $24
	srl	$3, $12, 2
	srl	$5, $12, 31
	addiu	$15, $8, 48
	sb	$14, 6($7)
	addu	$14, $3, $5
	mult	$16, $6
	mflo	$12
	subu	$8, $13, $25
	sra	$3, $2, 2
	srl	$5, $2, 31
	addiu	$2, $10, 48
	sb	$15, 5($7)
	addu	$10, $3, $5
	mult	$14, $6
	mflo	$5
	subu	$4, $4, $12
	addiu	$3, $8, 48
	sb	$2, 4($7)
	mult	$10, $6
	mflo	$6
	subu	$5, $11, $5
	addiu	$2, $4, 48
	sb	$3, 3($7)
	subu	$4, $9, $6
	addiu	$3, $5, 48
	sb	$2, 2($7)
	addiu	$2, $zero, 1
	addiu	$4, $4, 48
	sb	$3, 1($7)
	lw	$3, 0($sp)
	nop
	sb	$4, %lo(itoa.buf)($3)
	j	$BB6_8
	nop
$BB6_6:                                 #   in Loop: Header=BB6_8 Depth=1
	addiu	$3, $zero, 9
	slt	$3, $3, $2
	bne	$3, $zero, $BB6_10
	nop
# BB#7:                                 # %._crit_edge8
                                        #   in Loop: Header=BB6_8 Depth=1
	lui	$3, %hi(itoa.buf)
	addiu	$3, $3, %lo(itoa.buf)
	addu	$3, $3, $2
	lbu	$4, 0($3)
	nop
	addiu	$2, $2, 1
$BB6_8:                                 # =>This Inner Loop Header: Depth=1
	andi	$3, $4, 255
	addiu	$4, $zero, 48
	beq	$3, $4, $BB6_6
	nop
# BB#9:                                 # %._crit_edge9
	addiu	$2, $2, -1
$BB6_10:
	lw	$3, 4($sp)
	nop
	beq	$3, $zero, $BB6_12
	nop
# BB#11:
	addiu	$3, $zero, 45
	lui	$4, %hi(itoa.out)
	sb	$3, %lo(itoa.out)($4)
$BB6_12:                                # %.preheader
	addiu	$3, $zero, 10
	subu	$4, $3, $2
	slti	$3, $4, 1
	beq	$3, $zero, $BB6_14
	nop
# BB#13:
	addiu	$4, $zero, 0
	j	$BB6_16
	nop
$BB6_14:                                # %.lr.ph.preheader
	lui	$3, %hi(itoa.out)
	addiu	$5, $zero, 10
	addiu	$3, $3, %lo(itoa.out)
	subu	$2, $5, $2
	lw	$5, 4($sp)
	nop
	addu	$5, $3, $5
$BB6_15:                                # %.lr.ph
                                        # =>This Inner Loop Header: Depth=1
	lui	$3, %hi(itoa.buf)
	addiu	$3, $3, %lo(itoa.buf)
	subu	$3, $3, $2
	lbu	$3, 10($3)
	nop
	addiu	$2, $2, -1
	addiu	$6, $5, 1
	sb	$3, 0($5)
	addu	$5, $zero, $6
	bne	$2, $zero, $BB6_15
	nop
$BB6_16:                                # %._crit_edge
	lui	$2, %hi(itoa.out)
	addiu	$2, $2, %lo(itoa.out)
	lw	$3, 4($sp)
	nop
	addu	$4, $4, $3
	addiu	$3, $zero, 0
	addu	$4, $2, $4
	sb	$3, 0($4)
$BB6_17:
	lw	$16, 8($sp)
	nop
	lw	$17, 12($sp)
	nop
	lw	$18, 16($sp)
	nop
	lw	$19, 20($sp)
	nop
	lw	$20, 24($sp)
	nop
	lw	$21, 28($sp)
	nop
	lw	$22, 32($sp)
	nop
	lw	$23, 36($sp)
	nop
	addiu	$sp, $sp, 40
	jr	$ra
	nop
	.set	macro
	.set	reorder
	.end	itoa
$tmp49:
	.size	itoa, ($tmp49)-itoa
$tmp50:
	.cfi_endproc
$eh_func_end6:

	.globl	utoa
	.align	2
	.type	utoa,@function
	.ent	utoa                    # @utoa
utoa:
$tmp53:
	.cfi_startproc
	.frame	$sp,24,$ra
	.mask 	0x003F0000,-4
	.fmask	0x00000000,0
# BB#0:
	.set	noreorder
	.set	nomacro
	addiu	$sp, $sp, -24
$tmp54:
	.cfi_def_cfa_offset 24
	sw	$21, 20($sp)
	sw	$20, 16($sp)
	sw	$19, 12($sp)
	sw	$18, 8($sp)
	sw	$17, 4($sp)
	sw	$16, 0($sp)
$tmp55:
	.cfi_offset 21, -4
$tmp56:
	.cfi_offset 20, -8
$tmp57:
	.cfi_offset 19, -12
$tmp58:
	.cfi_offset 18, -16
$tmp59:
	.cfi_offset 17, -20
$tmp60:
	.cfi_offset 16, -24
	lui	$2, %hi($.str)
	bne	$4, $zero, $BB7_7
	nop
# BB#1:
	addiu	$2, $2, %lo($.str)
	j	$BB7_12
	nop
$BB7_2:                                 #   in Loop: Header=BB7_8 Depth=1
	addiu	$2, $zero, 9
	slt	$2, $2, $3
	bne	$2, $zero, $BB7_5
	nop
# BB#3:                                 # %._crit_edge8
                                        #   in Loop: Header=BB7_8 Depth=1
	lui	$2, %hi(utoa.buf)
	addiu	$2, $2, %lo(utoa.buf)
	addu	$2, $2, $3
	lbu	$4, 0($2)
	nop
	addiu	$3, $3, 1
	j	$BB7_8
	nop
$BB7_4:                                 # %.preheadersplit
	addiu	$3, $3, -1
$BB7_5:                                 # %.preheader
	addiu	$2, $zero, 10
	subu	$4, $2, $3
	slti	$2, $4, 1
	beq	$2, $zero, $BB7_9
	nop
# BB#6:
	addiu	$4, $zero, 0
	j	$BB7_11
	nop
$BB7_7:                                 # %.preheader27
	lui	$3, 52428
	lui	$2, 20971
	ori	$3, $3, 52429
	lui	$5, 4194
	ori	$2, $2, 34079
	multu	$4, $3
	mfhi	$6
	lui	$7, 53687
	ori	$5, $5, 19923
	multu	$4, $2
	mfhi	$2
	srl	$11, $6, 3
	lui	$6, 2684
	ori	$7, $7, 5977
	multu	$4, $5
	mfhi	$8
	srl	$10, $2, 5
	multu	$11, $3
	mfhi	$12
	addiu	$5, $zero, 10
	lui	$2, %hi(utoa.buf)
	lui	$9, 17179
	multu	$4, $7
	mfhi	$7
	srl	$8, $8, 6
	multu	$10, $3
	mfhi	$14
	srl	$13, $12, 3
	mult	$11, $5
	mflo	$12
	srl	$15, $4, 5
	ori	$6, $6, 23237
	lui	$25, 27487
	ori	$24, $9, 56963
	multu	$15, $6
	mfhi	$9
	srl	$7, $7, 13
	multu	$8, $3
	mfhi	$15
	srl	$14, $14, 3
	mult	$13, $5
	mflo	$13
	subu	$12, $4, $12
	addiu	$6, $2, %lo(utoa.buf)
	addiu	$19, $zero, 0
	lui	$18, 21990
	ori	$17, $25, 51819
	multu	$4, $24
	mfhi	$16
	srl	$9, $9, 7
	multu	$7, $3
	mfhi	$25
	srl	$24, $15, 3
	mult	$14, $5
	mflo	$15
	subu	$14, $11, $13
	ori	$13, $12, 48
	sb	$19, 10($6)
	lui	$12, 4
	ori	$21, $18, 15241
	multu	$4, $17
	mfhi	$20
	srl	$11, $16, 18
	multu	$9, $3
	mfhi	$19
	srl	$18, $25, 3
	mult	$24, $5
	mflo	$17
	subu	$16, $10, $15
	ori	$25, $14, 48
	sb	$13, 9($6)
	multu	$4, $21
	mfhi	$24
	srl	$10, $20, 22
	multu	$11, $3
	mfhi	$15
	srl	$14, $19, 3
	mult	$18, $5
	mflo	$13
	subu	$8, $8, $17
	srl	$4, $4, 9
	ori	$17, $12, 19331
	ori	$12, $16, 48
	sb	$25, 8($6)
	multu	$4, $17
	mfhi	$25
	srl	$4, $24, 25
	multu	$10, $3
	mfhi	$24
	srl	$15, $15, 3
	mult	$14, $5
	mflo	$14
	subu	$13, $7, $13
	ori	$8, $8, 48
	sb	$12, 7($6)
	srl	$7, $25, 7
	multu	$4, $3
	mfhi	$25
	srl	$24, $24, 3
	mult	$15, $5
	mflo	$15
	subu	$12, $9, $14
	ori	$9, $13, 48
	sb	$8, 6($6)
	multu	$7, $3
	mfhi	$16
	srl	$14, $25, 3
	mult	$24, $5
	mflo	$13
	subu	$8, $11, $15
	ori	$3, $12, 48
	sb	$9, 5($6)
	srl	$12, $16, 3
	mult	$14, $5
	mflo	$11
	subu	$9, $10, $13
	ori	$8, $8, 48
	sb	$3, 4($6)
	mult	$12, $5
	mflo	$10
	subu	$5, $4, $11
	ori	$3, $9, 48
	sb	$8, 3($6)
	subu	$4, $7, $10
	ori	$5, $5, 48
	sb	$3, 2($6)
	addiu	$3, $zero, 1
	ori	$4, $4, 48
	sb	$5, 1($6)
	sb	$4, %lo(utoa.buf)($2)
$BB7_8:                                 # =>This Inner Loop Header: Depth=1
	andi	$2, $4, 255
	addiu	$4, $zero, 48
	beq	$2, $4, $BB7_2
	nop
	j	$BB7_4
	nop
$BB7_9:                                 # %.lr.ph.preheader
	lui	$5, %hi(utoa.out)
	addiu	$2, $zero, 10
	subu	$2, $2, $3
	addiu	$3, $5, %lo(utoa.out)
$BB7_10:                                # %.lr.ph
                                        # =>This Inner Loop Header: Depth=1
	lui	$5, %hi(utoa.buf)
	addiu	$5, $5, %lo(utoa.buf)
	subu	$5, $5, $2
	lbu	$5, 10($5)
	nop
	addiu	$2, $2, -1
	addiu	$6, $3, 1
	sb	$5, 0($3)
	addu	$3, $zero, $6
	bne	$2, $zero, $BB7_10
	nop
$BB7_11:                                # %._crit_edge
	lui	$2, %hi(utoa.out)
	addiu	$2, $2, %lo(utoa.out)
	addiu	$3, $zero, 0
	addu	$4, $2, $4
	sb	$3, 0($4)
$BB7_12:
	lw	$16, 0($sp)
	nop
	lw	$17, 4($sp)
	nop
	lw	$18, 8($sp)
	nop
	lw	$19, 12($sp)
	nop
	lw	$20, 16($sp)
	nop
	lw	$21, 20($sp)
	nop
	addiu	$sp, $sp, 24
	jr	$ra
	nop
	.set	macro
	.set	reorder
	.end	utoa
$tmp61:
	.size	utoa, ($tmp61)-utoa
$tmp62:
	.cfi_endproc
$eh_func_end7:

	.globl	puts
	.align	2
	.type	puts,@function
	.ent	puts                    # @puts
puts:
$tmp63:
	.cfi_startproc
	.frame	$sp,0,$ra
	.mask 	0x00000000,0
	.fmask	0x00000000,0
# BB#0:
	.set	noreorder
	.set	nomacro
	lbu	$3, 0($4)
	nop
	beq	$3, $zero, $BB8_3
	nop
# BB#1:                                 # %.lr.ph.preheader
	addiu	$2, $4, 1
$BB8_2:                                 # %.lr.ph
                                        # =>This Inner Loop Header: Depth=1
	sll	$3, $3, 24
	lui	$4, 45056
	sra	$3, $3, 24
	ori	$4, $4, 0
	sw	$3, 0($4)
	lbu	$3, 0($2)
	nop
	addiu	$2, $2, 1
	bne	$3, $zero, $BB8_2
	nop
$BB8_3:                                 # %._crit_edge
	lui	$3, 45056
	addiu	$2, $zero, 10
	ori	$3, $3, 0
	sw	$2, 0($3)
	addiu	$2, $zero, 0
	jr	$ra
	nop
	.set	macro
	.set	reorder
	.end	puts
$tmp64:
	.size	puts, ($tmp64)-puts
$tmp65:
	.cfi_endproc
$eh_func_end8:

	.globl	printf
	.align	2
	.type	printf,@function
	.ent	printf                  # @printf
printf:
$tmp68:
	.cfi_startproc
	.frame	$sp,72,$ra
	.mask 	0x80070000,-4
	.fmask	0x00000000,0
# BB#0:
	.set	noreorder
	.set	nomacro
	addiu	$sp, $sp, -72
$tmp69:
	.cfi_def_cfa_offset 72
	sw	$ra, 68($sp)
	sw	$18, 64($sp)
	sw	$17, 60($sp)
	sw	$16, 56($sp)
$tmp70:
	.cfi_offset 31, -4
$tmp71:
	.cfi_offset 18, -8
$tmp72:
	.cfi_offset 17, -12
$tmp73:
	.cfi_offset 16, -16
	addu	$16, $zero, $4
	addiu	$17, $zero, 0
	addiu	$2, $sp, 76
	sw	$5, 76($sp)
	sw	$6, 80($sp)
	sw	$7, 84($sp)
	sw	$2, 20($sp)
$BB9_1:                                 # =>This Loop Header: Depth=1
                                        #     Child Loop BB9_62 Depth 2
                                        #     Child Loop BB9_4 Depth 2
                                        #     Child Loop BB9_78 Depth 2
                                        #     Child Loop BB9_75 Depth 2
                                        #     Child Loop BB9_69 Depth 2
                                        #     Child Loop BB9_94 Depth 2
                                        #     Child Loop BB9_91 Depth 2
                                        #     Child Loop BB9_85 Depth 2
                                        #     Child Loop BB9_134 Depth 2
                                        #     Child Loop BB9_130 Depth 2
                                        #     Child Loop BB9_124 Depth 2
                                        #     Child Loop BB9_120 Depth 2
                                        #     Child Loop BB9_116 Depth 2
                                        #     Child Loop BB9_110 Depth 2
                                        #     Child Loop BB9_106 Depth 2
                                        #     Child Loop BB9_55 Depth 2
                                        #     Child Loop BB9_51 Depth 2
                                        #     Child Loop BB9_45 Depth 2
                                        #     Child Loop BB9_43 Depth 2
                                        #     Child Loop BB9_39 Depth 2
                                        #     Child Loop BB9_33 Depth 2
                                        #     Child Loop BB9_30 Depth 2
                                        #     Child Loop BB9_27 Depth 2
                                        #     Child Loop BB9_24 Depth 2
	addu	$2, $16, $17
	lbu	$2, 0($2)
	nop
	beq	$2, $zero, $BB9_140
	nop
$BB9_2:                                 #   in Loop: Header=BB9_1 Depth=1
	addiu	$3, $zero, 37
	bne	$2, $3, $BB9_136
	nop
$BB9_3:                                 #   in Loop: Header=BB9_1 Depth=1
	addu	$2, $zero, $17
$BB9_4:                                 # %.preheader
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	addu	$3, $16, $2
	lbu	$4, 1($3)
	nop
	addiu	$3, $4, -48
	andi	$3, $3, 255
	addiu	$17, $2, 1
	sltiu	$5, $3, 10
	addu	$3, $zero, $2
	addu	$2, $zero, $17
	bne	$5, $zero, $BB9_4
	nop
# BB#5:                                 #   in Loop: Header=BB9_1 Depth=1
	andi	$2, $4, 255
	addiu	$4, $zero, 108
	xor	$2, $2, $4
	bne	$2, $zero, $BB9_7
	nop
$BB9_6:                                 #   in Loop: Header=BB9_1 Depth=1
	addiu	$17, $3, 2
$BB9_7:                                 #   in Loop: Header=BB9_1 Depth=1
	addu	$2, $16, $17
	lb	$3, 0($2)
	nop
	addiu	$4, $zero, 87
	slt	$4, $4, $3
	bne	$4, $zero, $BB9_10
	nop
$BB9_8:                                 #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $zero, 37
	bne	$3, $4, $BB9_135
	nop
$BB9_9:                                 #   in Loop: Header=BB9_1 Depth=1
	lui	$3, 45056
	addiu	$2, $zero, 37
	j	$BB9_137
	nop
$BB9_10:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $zero, 98
	slt	$4, $4, $3
	bne	$4, $zero, $BB9_14
	nop
$BB9_11:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $zero, 88
	bne	$3, $4, $BB9_135
	nop
$BB9_12:                                #   in Loop: Header=BB9_1 Depth=1
	lw	$3, 20($sp)
	nop
	addiu	$4, $3, 4
	lui	$2, %hi($.str)
	sw	$4, 20($sp)
	lw	$4, 0($3)
	nop
	bne	$4, $zero, $BB9_44
	nop
# BB#13:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $2, %lo($.str)
	j	$BB9_53
	nop
$BB9_14:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $zero, 114
	slt	$4, $4, $3
	bne	$4, $zero, $BB9_17
	nop
$BB9_15:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$3, $3, -99
	addiu	$4, $zero, 9
	sltu	$4, $4, $3
	bne	$4, $zero, $BB9_135
	nop
$BB9_16:                                #   in Loop: Header=BB9_1 Depth=1
	lui	$4, %hi($JTI9_0)
	sll	$3, $3, 2
	addiu	$4, $4, %lo($JTI9_0)
	addu	$3, $3, $4
	lw	$3, 0($3)
	nop
	jr	$3
	nop
$BB9_17:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $zero, 115
	beq	$3, $4, $BB9_28
	nop
$BB9_18:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $zero, 117
	beq	$3, $4, $BB9_25
	nop
$BB9_19:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $zero, 120
	bne	$3, $4, $BB9_135
	nop
$BB9_20:                                #   in Loop: Header=BB9_1 Depth=1
	lw	$3, 20($sp)
	nop
	addiu	$4, $3, 4
	lui	$2, %hi($.str)
	sw	$4, 20($sp)
	lw	$4, 0($3)
	nop
	bne	$4, $zero, $BB9_32
	nop
# BB#21:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $2, %lo($.str)
	j	$BB9_41
	nop
$BB9_22:                                #   in Loop: Header=BB9_1 Depth=1
	lw	$2, 20($sp)
	nop
	addiu	$3, $2, 4
	sw	$3, 20($sp)
	lw	$4, 0($2)
	nop
	jal	itoa
	nop
	lbu	$3, 0($2)
	nop
	beq	$3, $zero, $BB9_139
	nop
# BB#23:                                # %.lr.ph.i.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	addiu	$2, $2, 1
$BB9_24:                                # %.lr.ph.i
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	sll	$3, $3, 24
	lui	$4, 45056
	sra	$3, $3, 24
	ori	$4, $4, 0
	sw	$3, 0($4)
	lbu	$3, 0($2)
	nop
	addiu	$2, $2, 1
	bne	$3, $zero, $BB9_24
	nop
	j	$BB9_139
	nop
$BB9_25:                                #   in Loop: Header=BB9_1 Depth=1
	lw	$2, 20($sp)
	nop
	addiu	$3, $2, 4
	sw	$3, 20($sp)
	lw	$4, 0($2)
	nop
	jal	utoa
	nop
	lbu	$3, 0($2)
	nop
	beq	$3, $zero, $BB9_139
	nop
# BB#26:                                # %.lr.ph.i29.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	addiu	$2, $2, 1
$BB9_27:                                # %.lr.ph.i29
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	sll	$3, $3, 24
	lui	$4, 45056
	sra	$3, $3, 24
	ori	$4, $4, 0
	sw	$3, 0($4)
	lbu	$3, 0($2)
	nop
	addiu	$2, $2, 1
	bne	$3, $zero, $BB9_27
	nop
	j	$BB9_139
	nop
$BB9_28:                                #   in Loop: Header=BB9_1 Depth=1
	lw	$2, 20($sp)
	nop
	addiu	$3, $2, 4
	sw	$3, 20($sp)
	lw	$2, 0($2)
	nop
	lbu	$3, 0($2)
	nop
	beq	$3, $zero, $BB9_139
	nop
# BB#29:                                # %.lr.ph.i71.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	addiu	$2, $2, 1
$BB9_30:                                # %.lr.ph.i71
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	sll	$3, $3, 24
	lui	$4, 45056
	sra	$3, $3, 24
	ori	$4, $4, 0
	sw	$3, 0($4)
	lbu	$3, 0($2)
	nop
	addiu	$2, $2, 1
	bne	$3, $zero, $BB9_30
	nop
	j	$BB9_139
	nop
$BB9_31:                                #   in Loop: Header=BB9_1 Depth=1
	lw	$2, 20($sp)
	nop
	addiu	$3, $2, 4
	sw	$3, 20($sp)
	lui	$3, 45056
	lb	$2, 0($2)
	nop
	j	$BB9_137
	nop
$BB9_32:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$5, $zero, 0
$BB9_33:                                # %.lr.ph5.i99
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	addu	$3, $zero, $5
	andi	$6, $4, 15
	sltiu	$2, $6, 10
	bne	$2, $zero, $BB9_35
	nop
# BB#34:                                #   in Loop: Header=BB9_33 Depth=2
	addiu	$7, $zero, 87
	j	$BB9_36
	nop
$BB9_35:                                # %.lr.ph5.i99
                                        #   in Loop: Header=BB9_33 Depth=2
	addiu	$7, $zero, 48
$BB9_36:                                # %.lr.ph5.i99
                                        #   in Loop: Header=BB9_33 Depth=2
	addiu	$2, $sp, 24
	srl	$4, $4, 4
	addiu	$5, $3, 1
	addu	$6, $7, $6
	addu	$7, $2, $3
	sb	$6, 0($7)
	bne	$4, $zero, $BB9_33
	nop
# BB#37:                                # %._crit_edge.i100
                                        #   in Loop: Header=BB9_1 Depth=1
	lui	$4, %hi(i2h.hex)
	addiu	$4, $4, %lo(i2h.hex)
	addiu	$6, $zero, 0
	addu	$7, $4, $5
	sb	$6, 0($7)
	bltz	$3, $BB9_41
	nop
# BB#38:                                # %.lr.ph.i104.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	lui	$4, %hi(i2h.hex)
	addiu	$3, $zero, 0
	subu	$3, $3, $5
	addiu	$4, $4, %lo(i2h.hex)
$BB9_39:                                # %.lr.ph.i104
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lui	$5, %hi(i2h.hex)
	subu	$6, $2, $3
	lbu	$6, -1($6)
	nop
	addiu	$3, $3, 1
	addiu	$7, $4, 1
	sb	$6, 0($4)
	addu	$4, $zero, $7
	bne	$3, $zero, $BB9_39
	nop
# BB#40:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $5, %lo(i2h.hex)
$BB9_41:                                # %i2h.exit106
                                        #   in Loop: Header=BB9_1 Depth=1
	lbu	$3, 0($4)
	nop
	beq	$3, $zero, $BB9_139
	nop
# BB#42:                                # %.lr.ph.i93.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	addiu	$2, $4, 1
$BB9_43:                                # %.lr.ph.i93
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	sll	$3, $3, 24
	lui	$4, 45056
	sra	$3, $3, 24
	ori	$4, $4, 0
	sw	$3, 0($4)
	lbu	$3, 0($2)
	nop
	addiu	$2, $2, 1
	bne	$3, $zero, $BB9_43
	nop
	j	$BB9_139
	nop
$BB9_44:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$5, $zero, 0
$BB9_45:                                # %.lr.ph5.i81
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	addu	$3, $zero, $5
	andi	$6, $4, 15
	sltiu	$2, $6, 10
	bne	$2, $zero, $BB9_47
	nop
# BB#46:                                #   in Loop: Header=BB9_45 Depth=2
	addiu	$7, $zero, 55
	j	$BB9_48
	nop
$BB9_47:                                # %.lr.ph5.i81
                                        #   in Loop: Header=BB9_45 Depth=2
	addiu	$7, $zero, 48
$BB9_48:                                # %.lr.ph5.i81
                                        #   in Loop: Header=BB9_45 Depth=2
	addiu	$2, $sp, 24
	srl	$4, $4, 4
	addiu	$5, $3, 1
	addu	$6, $7, $6
	addu	$7, $2, $3
	sb	$6, 0($7)
	bne	$4, $zero, $BB9_45
	nop
# BB#49:                                # %._crit_edge.i82
                                        #   in Loop: Header=BB9_1 Depth=1
	lui	$4, %hi(i2h.hex)
	addiu	$4, $4, %lo(i2h.hex)
	addiu	$6, $zero, 0
	addu	$7, $4, $5
	sb	$6, 0($7)
	bltz	$3, $BB9_53
	nop
# BB#50:                                # %.lr.ph.i86.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	lui	$4, %hi(i2h.hex)
	addiu	$3, $zero, 0
	subu	$3, $3, $5
	addiu	$4, $4, %lo(i2h.hex)
$BB9_51:                                # %.lr.ph.i86
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lui	$5, %hi(i2h.hex)
	subu	$6, $2, $3
	lbu	$6, -1($6)
	nop
	addiu	$3, $3, 1
	addiu	$7, $4, 1
	sb	$6, 0($4)
	addu	$4, $zero, $7
	bne	$3, $zero, $BB9_51
	nop
# BB#52:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $5, %lo(i2h.hex)
$BB9_53:                                # %i2h.exit88
                                        #   in Loop: Header=BB9_1 Depth=1
	lbu	$3, 0($4)
	nop
	beq	$3, $zero, $BB9_139
	nop
# BB#54:                                # %.lr.ph.i75.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	addiu	$2, $4, 1
$BB9_55:                                # %.lr.ph.i75
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	sll	$3, $3, 24
	lui	$4, 45056
	sra	$3, $3, 24
	ori	$4, $4, 0
	sw	$3, 0($4)
	lbu	$3, 0($2)
	nop
	addiu	$2, $2, 1
	bne	$3, $zero, $BB9_55
	nop
	j	$BB9_139
	nop
$BB9_56:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$18, $17, 1
	addu	$2, $16, $18
	lb	$2, 0($2)
	nop
	addiu	$3, $zero, 88
	beq	$2, $3, $BB9_80
	nop
$BB9_57:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$3, $zero, 120
	beq	$2, $3, $BB9_64
	nop
$BB9_58:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$3, $zero, 100
	bne	$2, $3, $BB9_139
	nop
$BB9_59:                                #   in Loop: Header=BB9_1 Depth=1
	lw	$2, 20($sp)
	nop
	addiu	$3, $2, 8
	sw	$3, 20($sp)
	lw	$4, 4($2)
	nop
	jal	itoa
	nop
	lbu	$3, 0($2)
	nop
	bne	$3, $zero, $BB9_61
	nop
# BB#60:                                #   in Loop: Header=BB9_1 Depth=1
	addu	$17, $zero, $18
	addiu	$17, $17, 1
	j	$BB9_1
	nop
$BB9_61:                                # %.lr.ph.i67.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	addiu	$2, $2, 1
$BB9_62:                                # %.lr.ph.i67
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	sll	$3, $3, 24
	lui	$4, 45056
	sra	$3, $3, 24
	ori	$4, $4, 0
	sw	$3, 0($4)
	lbu	$3, 0($2)
	nop
	addiu	$2, $2, 1
	bne	$3, $zero, $BB9_62
	nop
# BB#63:                                #   in Loop: Header=BB9_1 Depth=1
	addu	$17, $zero, $18
	addiu	$17, $17, 1
	j	$BB9_1
	nop
$BB9_64:                                #   in Loop: Header=BB9_1 Depth=1
	lw	$2, 20($sp)
	nop
	addiu	$3, $2, 4
	sw	$3, 20($sp)
	lw	$3, 0($2)
	nop
	addiu	$4, $2, 8
	lui	$5, %hi($.str)
	sw	$4, 20($sp)
	lw	$4, 4($2)
	nop
	or	$2, $3, $4
	bne	$2, $zero, $BB9_68
	nop
# BB#65:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$7, $5, %lo($.str)
$BB9_66:                                # %l2h.exit62
                                        #   in Loop: Header=BB9_1 Depth=1
	lbu	$3, 0($7)
	nop
	bne	$3, $zero, $BB9_77
	nop
# BB#67:                                #   in Loop: Header=BB9_1 Depth=1
	addu	$17, $zero, $18
	addiu	$17, $17, 1
	j	$BB9_1
	nop
$BB9_68:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$6, $zero, 0
$BB9_69:                                # %.lr.ph5.i56
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	addu	$5, $zero, $6
	andi	$7, $3, 15
	sltiu	$2, $7, 10
	bne	$2, $zero, $BB9_71
	nop
# BB#70:                                #   in Loop: Header=BB9_69 Depth=2
	addiu	$8, $zero, 87
	j	$BB9_72
	nop
$BB9_71:                                # %.lr.ph5.i56
                                        #   in Loop: Header=BB9_69 Depth=2
	addiu	$8, $zero, 48
$BB9_72:                                # %.lr.ph5.i56
                                        #   in Loop: Header=BB9_69 Depth=2
	addiu	$2, $sp, 36
	srl	$3, $3, 4
	sll	$9, $4, 28
	addiu	$6, $5, 1
	or	$3, $3, $9
	srl	$4, $4, 4
	addu	$8, $8, $7
	addu	$9, $2, $5
	or	$7, $3, $4
	sb	$8, 0($9)
	bne	$7, $zero, $BB9_69
	nop
# BB#73:                                # %._crit_edge.i57
                                        #   in Loop: Header=BB9_1 Depth=1
	lui	$3, %hi(l2h.hex)
	addiu	$7, $3, %lo(l2h.hex)
	addiu	$3, $zero, 0
	addu	$4, $7, $6
	sb	$3, 0($4)
	bltz	$5, $BB9_66
	nop
# BB#74:                                # %.lr.ph.i60.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	addiu	$3, $zero, 0
	addiu	$4, $6, -1
	addu	$5, $zero, $3
$BB9_75:                                # %.lr.ph.i60
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lui	$7, %hi(l2h.hex)
	addu	$6, $2, $4
	addiu	$7, $7, %lo(l2h.hex)
	lbu	$8, 0($6)
	nop
	addu	$9, $7, $3
	slti	$6, $4, 1
	sb	$8, 0($9)
	bne	$6, $zero, $BB9_66
	nop
# BB#76:                                # %.lr.ph.i60..lr.ph.i60_crit_edge
                                        #   in Loop: Header=BB9_75 Depth=2
	addiu	$3, $3, 1
	addiu	$6, $zero, 1
	sltu	$6, $3, $6
	addiu	$7, $zero, 0
	addu	$6, $6, $7
	addiu	$4, $4, -1
	addu	$5, $5, $6
	j	$BB9_75
	nop
$BB9_77:                                # %.lr.ph.i49.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	addiu	$2, $7, 1
$BB9_78:                                # %.lr.ph.i49
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	sll	$3, $3, 24
	lui	$4, 45056
	sra	$3, $3, 24
	ori	$4, $4, 0
	sw	$3, 0($4)
	lbu	$3, 0($2)
	nop
	addiu	$2, $2, 1
	bne	$3, $zero, $BB9_78
	nop
# BB#79:                                #   in Loop: Header=BB9_1 Depth=1
	addu	$17, $zero, $18
	addiu	$17, $17, 1
	j	$BB9_1
	nop
$BB9_80:                                #   in Loop: Header=BB9_1 Depth=1
	lw	$2, 20($sp)
	nop
	addiu	$3, $2, 4
	sw	$3, 20($sp)
	lw	$3, 0($2)
	nop
	addiu	$4, $2, 8
	lui	$5, %hi($.str)
	sw	$4, 20($sp)
	lw	$4, 4($2)
	nop
	or	$2, $3, $4
	bne	$2, $zero, $BB9_84
	nop
# BB#81:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$7, $5, %lo($.str)
$BB9_82:                                # %l2h.exit
                                        #   in Loop: Header=BB9_1 Depth=1
	lbu	$3, 0($7)
	nop
	bne	$3, $zero, $BB9_93
	nop
# BB#83:                                #   in Loop: Header=BB9_1 Depth=1
	addu	$17, $zero, $18
	addiu	$17, $17, 1
	j	$BB9_1
	nop
$BB9_84:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$6, $zero, 0
$BB9_85:                                # %.lr.ph5.i39
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	addu	$5, $zero, $6
	andi	$7, $3, 15
	sltiu	$2, $7, 10
	bne	$2, $zero, $BB9_87
	nop
# BB#86:                                #   in Loop: Header=BB9_85 Depth=2
	addiu	$8, $zero, 55
	j	$BB9_88
	nop
$BB9_87:                                # %.lr.ph5.i39
                                        #   in Loop: Header=BB9_85 Depth=2
	addiu	$8, $zero, 48
$BB9_88:                                # %.lr.ph5.i39
                                        #   in Loop: Header=BB9_85 Depth=2
	addiu	$2, $sp, 36
	srl	$3, $3, 4
	sll	$9, $4, 28
	addiu	$6, $5, 1
	or	$3, $3, $9
	srl	$4, $4, 4
	addu	$8, $8, $7
	addu	$9, $2, $5
	or	$7, $3, $4
	sb	$8, 0($9)
	bne	$7, $zero, $BB9_85
	nop
# BB#89:                                # %._crit_edge.i40
                                        #   in Loop: Header=BB9_1 Depth=1
	lui	$3, %hi(l2h.hex)
	addiu	$7, $3, %lo(l2h.hex)
	addiu	$3, $zero, 0
	addu	$4, $7, $6
	sb	$3, 0($4)
	bltz	$5, $BB9_82
	nop
# BB#90:                                # %.lr.ph.i43.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	addiu	$3, $zero, 0
	addiu	$4, $6, -1
	addu	$5, $zero, $3
$BB9_91:                                # %.lr.ph.i43
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lui	$7, %hi(l2h.hex)
	addu	$6, $2, $4
	addiu	$7, $7, %lo(l2h.hex)
	lbu	$8, 0($6)
	nop
	addu	$9, $7, $3
	slti	$6, $4, 1
	sb	$8, 0($9)
	bne	$6, $zero, $BB9_82
	nop
# BB#92:                                # %.lr.ph.i43..lr.ph.i43_crit_edge
                                        #   in Loop: Header=BB9_91 Depth=2
	addiu	$3, $3, 1
	addiu	$6, $zero, 1
	sltu	$6, $3, $6
	addiu	$7, $zero, 0
	addu	$6, $6, $7
	addiu	$4, $4, -1
	addu	$5, $5, $6
	j	$BB9_91
	nop
$BB9_93:                                # %.lr.ph.i33.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	addiu	$2, $7, 1
$BB9_94:                                # %.lr.ph.i33
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	sll	$3, $3, 24
	lui	$4, 45056
	sra	$3, $3, 24
	ori	$4, $4, 0
	sw	$3, 0($4)
	lbu	$3, 0($2)
	nop
	addiu	$2, $2, 1
	bne	$3, $zero, $BB9_94
	nop
# BB#95:                                #   in Loop: Header=BB9_1 Depth=1
	addu	$17, $zero, $18
	addiu	$17, $17, 1
	j	$BB9_1
	nop
$BB9_96:                                #   in Loop: Header=BB9_1 Depth=1
	lw	$2, 20($sp)
	nop
	addiu	$4, $2, 4
	addiu	$3, $17, 1
	sw	$4, 20($sp)
	addu	$4, $16, $3
	lbu	$4, 0($4)
	nop
	addiu	$5, $zero, 104
	xor	$4, $4, $5
	bne	$4, $zero, $BB9_98
	nop
$BB9_97:                                #   in Loop: Header=BB9_1 Depth=1
	addu	$17, $zero, $3
$BB9_98:                                #   in Loop: Header=BB9_1 Depth=1
	addiu	$17, $17, 1
	beq	$4, $zero, $BB9_100
	nop
$BB9_99:                                #   in Loop: Header=BB9_1 Depth=1
	ori	$3, $zero, 65535
	j	$BB9_101
	nop
$BB9_100:                               #   in Loop: Header=BB9_1 Depth=1
	addiu	$3, $zero, 255
$BB9_101:                               #   in Loop: Header=BB9_1 Depth=1
	addu	$5, $16, $17
	lw	$4, 0($2)
	nop
	lb	$2, 0($5)
	nop
	and	$4, $3, $4
	addiu	$3, $zero, 88
	beq	$2, $3, $BB9_121
	nop
$BB9_102:                               #   in Loop: Header=BB9_1 Depth=1
	addiu	$3, $zero, 120
	beq	$2, $3, $BB9_107
	nop
$BB9_103:                               #   in Loop: Header=BB9_1 Depth=1
	addiu	$3, $zero, 100
	bne	$2, $3, $BB9_139
	nop
$BB9_104:                               #   in Loop: Header=BB9_1 Depth=1
	jal	itoa
	nop
	lbu	$3, 0($2)
	nop
	beq	$3, $zero, $BB9_139
	nop
# BB#105:                               # %.lr.ph.i25.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	addiu	$2, $2, 1
$BB9_106:                               # %.lr.ph.i25
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	sll	$3, $3, 24
	lui	$4, 45056
	sra	$3, $3, 24
	ori	$4, $4, 0
	sw	$3, 0($4)
	lbu	$3, 0($2)
	nop
	addiu	$2, $2, 1
	bne	$3, $zero, $BB9_106
	nop
	j	$BB9_139
	nop
$BB9_107:                               #   in Loop: Header=BB9_1 Depth=1
	lui	$2, %hi($.str)
	bne	$4, $zero, $BB9_109
	nop
# BB#108:                               #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $2, %lo($.str)
	j	$BB9_118
	nop
$BB9_109:                               #   in Loop: Header=BB9_1 Depth=1
	addiu	$5, $zero, 0
$BB9_110:                               # %.lr.ph5.i13
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	addu	$3, $zero, $5
	andi	$6, $4, 15
	sltiu	$2, $6, 10
	bne	$2, $zero, $BB9_112
	nop
# BB#111:                               #   in Loop: Header=BB9_110 Depth=2
	addiu	$7, $zero, 87
	j	$BB9_113
	nop
$BB9_112:                               # %.lr.ph5.i13
                                        #   in Loop: Header=BB9_110 Depth=2
	addiu	$7, $zero, 48
$BB9_113:                               # %.lr.ph5.i13
                                        #   in Loop: Header=BB9_110 Depth=2
	addiu	$2, $sp, 24
	srl	$4, $4, 4
	addiu	$5, $3, 1
	addu	$6, $7, $6
	addu	$7, $2, $3
	sb	$6, 0($7)
	bne	$4, $zero, $BB9_110
	nop
# BB#114:                               # %._crit_edge.i14
                                        #   in Loop: Header=BB9_1 Depth=1
	lui	$4, %hi(i2h.hex)
	addiu	$4, $4, %lo(i2h.hex)
	addiu	$6, $zero, 0
	addu	$7, $4, $5
	sb	$6, 0($7)
	bltz	$3, $BB9_118
	nop
# BB#115:                               # %.lr.ph.i18.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	lui	$4, %hi(i2h.hex)
	addiu	$3, $zero, 0
	subu	$3, $3, $5
	addiu	$4, $4, %lo(i2h.hex)
$BB9_116:                               # %.lr.ph.i18
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lui	$5, %hi(i2h.hex)
	subu	$6, $2, $3
	lbu	$6, -1($6)
	nop
	addiu	$3, $3, 1
	addiu	$7, $4, 1
	sb	$6, 0($4)
	addu	$4, $zero, $7
	bne	$3, $zero, $BB9_116
	nop
# BB#117:                               #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $5, %lo(i2h.hex)
$BB9_118:                               # %i2h.exit20
                                        #   in Loop: Header=BB9_1 Depth=1
	lbu	$3, 0($4)
	nop
	beq	$3, $zero, $BB9_139
	nop
# BB#119:                               # %.lr.ph.i7.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	addiu	$2, $4, 1
$BB9_120:                               # %.lr.ph.i7
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	sll	$3, $3, 24
	lui	$4, 45056
	sra	$3, $3, 24
	ori	$4, $4, 0
	sw	$3, 0($4)
	lbu	$3, 0($2)
	nop
	addiu	$2, $2, 1
	bne	$3, $zero, $BB9_120
	nop
	j	$BB9_139
	nop
$BB9_121:                               #   in Loop: Header=BB9_1 Depth=1
	lui	$2, %hi($.str)
	bne	$4, $zero, $BB9_123
	nop
# BB#122:                               #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $2, %lo($.str)
	j	$BB9_132
	nop
$BB9_123:                               #   in Loop: Header=BB9_1 Depth=1
	addiu	$5, $zero, 0
$BB9_124:                               # %.lr.ph5.i
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	addu	$3, $zero, $5
	andi	$6, $4, 15
	sltiu	$2, $6, 10
	bne	$2, $zero, $BB9_126
	nop
# BB#125:                               #   in Loop: Header=BB9_124 Depth=2
	addiu	$7, $zero, 55
	j	$BB9_127
	nop
$BB9_126:                               # %.lr.ph5.i
                                        #   in Loop: Header=BB9_124 Depth=2
	addiu	$7, $zero, 48
$BB9_127:                               # %.lr.ph5.i
                                        #   in Loop: Header=BB9_124 Depth=2
	addiu	$2, $sp, 24
	srl	$4, $4, 4
	addiu	$5, $3, 1
	addu	$6, $7, $6
	addu	$7, $2, $3
	sb	$6, 0($7)
	bne	$4, $zero, $BB9_124
	nop
# BB#128:                               # %._crit_edge.i
                                        #   in Loop: Header=BB9_1 Depth=1
	lui	$4, %hi(i2h.hex)
	addiu	$4, $4, %lo(i2h.hex)
	addiu	$6, $zero, 0
	addu	$7, $4, $5
	sb	$6, 0($7)
	bltz	$3, $BB9_132
	nop
# BB#129:                               # %.lr.ph.i4.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	lui	$4, %hi(i2h.hex)
	addiu	$3, $zero, 0
	subu	$3, $3, $5
	addiu	$4, $4, %lo(i2h.hex)
$BB9_130:                               # %.lr.ph.i4
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lui	$5, %hi(i2h.hex)
	subu	$6, $2, $3
	lbu	$6, -1($6)
	nop
	addiu	$3, $3, 1
	addiu	$7, $4, 1
	sb	$6, 0($4)
	addu	$4, $zero, $7
	bne	$3, $zero, $BB9_130
	nop
# BB#131:                               #   in Loop: Header=BB9_1 Depth=1
	addiu	$4, $5, %lo(i2h.hex)
$BB9_132:                               # %i2h.exit
                                        #   in Loop: Header=BB9_1 Depth=1
	lbu	$3, 0($4)
	nop
	beq	$3, $zero, $BB9_139
	nop
# BB#133:                               # %.lr.ph.i2.preheader
                                        #   in Loop: Header=BB9_1 Depth=1
	addiu	$2, $4, 1
$BB9_134:                               # %.lr.ph.i2
                                        #   Parent Loop BB9_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	sll	$3, $3, 24
	lui	$4, 45056
	sra	$3, $3, 24
	ori	$4, $4, 0
	sw	$3, 0($4)
	lbu	$3, 0($2)
	nop
	addiu	$2, $2, 1
	bne	$3, $zero, $BB9_134
	nop
	j	$BB9_139
	nop
$BB9_135:                               #   in Loop: Header=BB9_1 Depth=1
	lui	$3, 45056
	addiu	$4, $zero, 37
	ori	$3, $3, 0
	sw	$4, 0($3)
	lb	$2, 0($2)
	nop
	j	$BB9_138
	nop
$BB9_136:                               #   in Loop: Header=BB9_1 Depth=1
	sll	$2, $2, 24
	sra	$2, $2, 24
	lui	$3, 45056
$BB9_137:                               #   in Loop: Header=BB9_1 Depth=1
	ori	$3, $3, 0
$BB9_138:                               #   in Loop: Header=BB9_1 Depth=1
	sw	$2, 0($3)
$BB9_139:                               # %.backedge
                                        #   in Loop: Header=BB9_1 Depth=1
	addiu	$17, $17, 1
	j	$BB9_1
	nop
$BB9_140:
	addiu	$2, $zero, 0
	lw	$16, 56($sp)
	nop
	lw	$17, 60($sp)
	nop
	lw	$18, 64($sp)
	nop
	lw	$ra, 68($sp)
	nop
	addiu	$sp, $sp, 72
	jr	$ra
	nop
	.set	macro
	.set	reorder
	.end	printf
$tmp74:
	.size	printf, ($tmp74)-printf
$tmp75:
	.cfi_endproc
$eh_func_end9:
	.section	.rodata,"a",@progbits
	.align	2
$JTI9_0:
	.4byte	($BB9_31)
	.4byte	($BB9_22)
	.4byte	($BB9_135)
	.4byte	($BB9_135)
	.4byte	($BB9_135)
	.4byte	($BB9_96)
	.4byte	($BB9_135)
	.4byte	($BB9_135)
	.4byte	($BB9_135)
	.4byte	($BB9_56)

	.text
	.globl	exit
	.align	2
	.type	exit,@function
	.ent	exit                    # @exit
exit:
$tmp78:
	.cfi_startproc
	.frame	$sp,24,$ra
	.mask 	0x80000000,-4
	.fmask	0x00000000,0
# BB#0:
	.set	noreorder
	.set	nomacro
	addiu	$sp, $sp, -24
$tmp79:
	.cfi_def_cfa_offset 24
	sw	$ra, 20($sp)
$tmp80:
	.cfi_offset 31, -4
	addu	$2, $zero, $4
	lui	$3, %hi($.str1)
	addiu	$4, $3, %lo($.str1)
	addu	$5, $zero, $2
	jal	printf
	nop
$BB10_1:                                # =>This Inner Loop Header: Depth=1
	j	$BB10_1
	nop
	.set	macro
	.set	reorder
	.end	exit
$tmp81:
	.size	exit, ($tmp81)-exit
$tmp82:
	.cfi_endproc
$eh_func_end10:

	.type	i2h.hex,@object         # @i2h.hex
	.local	i2h.hex
	.comm	i2h.hex,9,1
	.type	$.str,@object           # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
$.str:
	.asciz	 "0"
	.size	$.str, 2

	.type	l2h.hex,@object         # @l2h.hex
	.local	l2h.hex
	.comm	l2h.hex,17,1
	.type	itoa.buf,@object        # @itoa.buf
	.local	itoa.buf
	.comm	itoa.buf,11,1
	.type	itoa.out,@object        # @itoa.out
	.local	itoa.out
	.comm	itoa.out,11,1
	.type	utoa.buf,@object        # @utoa.buf
	.local	utoa.buf
	.comm	utoa.buf,11,1
	.type	utoa.out,@object        # @utoa.out
	.local	utoa.out
	.comm	utoa.out,11,1
	.type	$.str1,@object          # @.str1
$.str1:
	.asciz	 "program returned %d exit status\n"
	.size	$.str1, 33


