	.section .mdebug.abi32
	.previous
	.file	"mem.mips.ll"
	.text
	.globl	memset
	.align	2
	.type	memset,@function
	.ent	memset                  # @memset
memset:
$tmp0:
	.cfi_startproc
	.frame	$sp,0,$ra
	.mask 	0x00000000,0
	.fmask	0x00000000,0
# BB#0:
	.set	noreorder
	.set	nomacro
	beq	$6, $zero, $BB0_3
	nop
# BB#1:                                 # %.lr.ph
	addu	$2, $zero, $4
$BB0_2:                                 # =>This Inner Loop Header: Depth=1
	addiu	$6, $6, -1
	addiu	$3, $2, 1
	sb	$5, 0($2)
	addu	$2, $zero, $3
	bne	$6, $zero, $BB0_2
	nop
$BB0_3:                                 # %._crit_edge
	addu	$2, $zero, $4
	jr	$ra
	nop
	.set	macro
	.set	reorder
	.end	memset
$tmp1:
	.size	memset, ($tmp1)-memset
$tmp2:
	.cfi_endproc
$eh_func_end0:

	.globl	memcpy
	.align	2
	.type	memcpy,@function
	.ent	memcpy                  # @memcpy
memcpy:
$tmp3:
	.cfi_startproc
	.frame	$sp,0,$ra
	.mask 	0x00000000,0
	.fmask	0x00000000,0
# BB#0:
	.set	noreorder
	.set	nomacro
	beq	$6, $zero, $BB1_3
	nop
# BB#1:
	addu	$2, $zero, $4
$BB1_2:                                 # %.lr.ph
                                        # =>This Inner Loop Header: Depth=1
	lbu	$3, 0($5)
	nop
	addiu	$6, $6, -1
	addiu	$5, $5, 1
	addiu	$7, $2, 1
	sb	$3, 0($2)
	addu	$2, $zero, $7
	bne	$6, $zero, $BB1_2
	nop
$BB1_3:                                 # %._crit_edge
	addu	$2, $zero, $4
	jr	$ra
	nop
	.set	macro
	.set	reorder
	.end	memcpy
$tmp4:
	.size	memcpy, ($tmp4)-memcpy
$tmp5:
	.cfi_endproc
$eh_func_end1:


