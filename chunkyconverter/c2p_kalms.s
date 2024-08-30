
;
; Date: 2016-09-01			Mikael Kalms
;					Email: mikael@kalms.org
;
; 1x1 4bpl cpu5 C2P for arbitrary BitMaps
;
; Features:
; Performs CPU-only C2P conversion
; Different routines for non-modulo and modulo C2P conversions
; Handles bitmaps of virtually any size (>4096x4096)
; Position-independent (PC-relative) code
;
; Restrictions:
; Chunky-buffer must be an even multiple of 32 pixels wide
; X-Offset must be set to an even multiple of 8
; If these conditions not are met, the routine will abort.
; If incorrect/invalid parameters are specified, the routine will
; most probably crash.
;
; c2p1x1_4_c5_bm

STRUCTURE   MACRO		; structure name, initial offset
\1	    EQU     0
SOFFSET     SET     \2
	    ENDM

FPTR	    MACRO		; function pointer (32 bits - all bits valid)
\1	    EQU     SOFFSET
SOFFSET     SET     SOFFSET+4
	    ENDM

BOOL	    MACRO		; boolean (16 bits)
\1	    EQU     SOFFSET
SOFFSET     SET     SOFFSET+2
	    ENDM

BYTE	    MACRO		; byte (8 bits)
\1	    EQU     SOFFSET
SOFFSET     SET     SOFFSET+1
	    ENDM

UBYTE	    MACRO		; unsigned byte (8 bits)
\1	    EQU     SOFFSET
SOFFSET     SET     SOFFSET+1
	    ENDM

WORD	    MACRO		; word (16 bits)
\1	    EQU     SOFFSET
SOFFSET     SET     SOFFSET+2
	    ENDM

UWORD	    MACRO		; unsigned word (16 bits)
\1	    EQU     SOFFSET
SOFFSET     SET     SOFFSET+2
	    ENDM

SHORT	    MACRO		; obsolete - use WORD
\1	    EQU     SOFFSET
SOFFSET     SET     SOFFSET+2
	    ENDM

USHORT	    MACRO		; obsolete - use UWORD
\1	    EQU     SOFFSET
SOFFSET     SET     SOFFSET+2
	    ENDM

LONG	    MACRO		; long (32 bits)
\1	    EQU     SOFFSET
SOFFSET     SET     SOFFSET+4
	    ENDM

ULONG	    MACRO		; unsigned long (32 bits)
\1	    EQU     SOFFSET
SOFFSET     SET     SOFFSET+4
	    ENDM

LABEL	    MACRO		; Define a label without bumping the offset
\1	    EQU     SOFFSET
	    ENDM

STRUCT	    MACRO		; Define a sub-structure
\1	    EQU     SOFFSET
SOFFSET     SET     SOFFSET+\2
	    ENDM

   STRUCTURE	BitMap,0
    WORD    bm_BytesPerRow
    WORD    bm_Rows
    BYTE    bm_Flags
    BYTE    bm_Depth
    WORD    bm_Pad
    STRUCT  bm_Planes,8*4
    LABEL   bm_SIZEOF

		rsreset
C2P1X1_4_C5_BM_WORD_CHUNKYX	rs.w	1
C2P1X1_4_C5_BM_WORD_CHUNKYY	rs.w	1
C2P1X1_4_C5_BM_WORD_ROWMOD	rs.l	1
C2P1X1_4_C5_BM_WORD_SIZEOF	rs.b	0


	section	code,code

; d0.w	chunkyx [chunky-pixels]
; d1.w	chunkyy [chunky-pixels]
; d2.w	offsx [screen-pixels]
; d3.w	offsy [screen-pixels]
; a0	chunkyscreen
; a1	BitMap

	XDEF	_c2p1x1_4_c5_bm_word
	XDEF	c2p1x1_4_c5_bm_word
_c2p1x1_4_c5_bm_word
c2p1x1_4_c5_bm_word
	movem.l	d2-d7/a2-a6,-(sp)
	subq.l	#C2P1X1_4_C5_BM_WORD_SIZEOF,sp
					; A few sanity checks
	cmpi.b	#4,bm_Depth(a1)		; At least 4 valid bplptrs?
	blo	.exit
	move.w	d0,d4
	move.w	d2,d5
	andi.w	#$1f,d4			; Even 32-pixel width?
	bne	.exit
	andi.w	#$7,d5			; Even 8-pixel xoffset?
	bne	.exit
	moveq	#0,d4
	move.w	bm_BytesPerRow(a1),d4

	move.w	d0,C2P1X1_4_C5_BM_WORD_CHUNKYX(sp) ; Skip if 0 pixels to convert
	beq	.exit
	move.w	d1,C2P1X1_4_C5_BM_WORD_CHUNKYY(sp)
	beq	.exit

	ext.l	d2			; Offs to first pixel to draw in bpl
	mulu.w	d4,d3
	lsr.l	#3,d2
	add.l	d2,d3

	lsl.w	#3,d4			; Modulo c2p required?
	sub.w	d0,d4
	bmi	.exit
	bne	.c2p_mod

	mulu.w	d0,d1
	add.l	a0,d1
	move.l	d1,a2			; Ptr to end of chunkybuffer

	movem.l	a0-a1/d3,-(sp)

	movem.l	bm_Planes(a1),a3-a6	; Setup ptrs to bpl0-3
	add.l	d3,a3
	add.l	d3,a4
	add.l	d3,a5
	add.l	d3,a6

	move.l	#$0f0f0f0f,d4
	move.l	#$00ff00ff,d5

	move.l	(a0)+,d0
	move.l	(a0)+,d2
	move.l	(a0)+,d1
	move.l	(a0)+,d3

	and.l	d4,d0
	and.l	d4,d1
	and.l	d4,d2
	and.l	d4,d3
	lsl.l	#4,d0
	lsl.l	#4,d1
	or.l	d2,d0
	or.l	d3,d1

; a3a2a1a0e3e2e1e0 b3b2b1b0f3f2f1f0 c3c2c1c0g3g2g1g0 d3d2d1d0h3h2h1h0
; i3i2i1i0m3m2m1m0 j3j2j1j0n3n2n1n0 k3k2k1k0o3o2o1o0 l3l2l1l0p3p2p1p0

	move.l	d1,d2
	lsr.l	#8,d2
	eor.l	d0,d2
	and.l	d5,d2
	eor.l	d2,d0
	lsl.l	#8,d2
	eor.l	d2,d1

; a3a2a1a0e3e2e1e0 i3i2i1i0m3m2m1m0 c3c2c1c0g3g2g1g0 k3k2k1k0o3o2o1o0
; b3b2b1b0f3f2f1f0 j3j2j1j0n3n2n1n0 d3d2d1d0h3h2h1h0 l3l2l1l0p3p2p1p0

	move.l	d1,d2
	lsr.l	#1,d2
	eor.l	d0,d2
	and.l	#$55555555,d2
	eor.l	d2,d0
	add.l	d2,d2
	eor.l	d2,d1

; a3b3a1b1e3f3e1f1 i3j3i1j1m3n3m1n1 c3d3c1d1g3h3g1h1 k3l3k1l1o3p3o1p1
; a2b2a0b0e2f2f0f0 i2j2i0j0m2n2m0n0 c2d2c0d0g2h2g0h0 k2l2k0l0o2p2o0p0

	move.w	d1,d2
	move.w	d0,d1
	swap	d1
	move.w	d1,d0
	move.w	d2,d1

; a3b3a1b1e3f3e1f1 i3j3i1j1m3n3m1n1 a2b2a0b0e2f2f0f0 i2j2i0j0m2n2m0n0
; c3d3c1d1g3h3g1h1 k3l3k1l1o3p3o1p1 c2d2c0d0g2h2g0h0 k2l2k0l0o2p2o0p0

	move.l	d1,d2

	bra	.x1start

.x1
	move.l	(a0)+,d0
	move.l	(a0)+,d2
	move.l	(a0)+,d1
	move.l	(a0)+,d3

	move.w	d6,(a5)+
	swap	d6

	and.l	d4,d0
	and.l	d4,d1
	and.l	d4,d2
	and.l	d4,d3
	lsl.l	#4,d0
	lsl.l	#4,d1
	or.l	d2,d0
	or.l	d3,d1

; a3a2a1a0e3e2e1e0 b3b2b1b0f3f2f1f0 c3c2c1c0g3g2g1g0 d3d2d1d0h3h2h1h0
; i3i2i1i0m3m2m1m0 j3j2j1j0n3n2n1n0 k3k2k1k0o3o2o1o0 l3l2l1l0p3p2p1p0

	move.w	d7,(a3)+
	swap	d7

	move.l	d1,d2
	lsr.l	#8,d2
	eor.l	d0,d2
	and.l	d5,d2
	eor.l	d2,d0
	lsl.l	#8,d2
	eor.l	d2,d1

; a3a2a1a0e3e2e1e0 i3i2i1i0m3m2m1m0 c3c2c1c0g3g2g1g0 k3k2k1k0o3o2o1o0
; b3b2b1b0f3f2f1f0 j3j2j1j0n3n2n1n0 d3d2d1d0h3h2h1h0 l3l2l1l0p3p2p1p0

	move.l	d1,d2
	lsr.l	#1,d2

	move.w	d6,(a6)+

	eor.l	d0,d2
	and.l	#$55555555,d2
	eor.l	d2,d0
	add.l	d2,d2
	eor.l	d2,d1

; a3b3a1b1e3f3e1f1 i3j3i1j1m3n3m1n1 c3d3c1d1g3h3g1h1 k3l3k1l1o3p3o1p1
; a2b2a0b0e2f2f0f0 i2j2i0j0m2n2m0n0 c2d2c0d0g2h2g0h0 k2l2k0l0o2p2o0p0

	move.w	d1,d2
	move.w	d0,d1
	swap	d1
	move.w	d1,d0
	move.w	d2,d1

; a3b3a1b1e3f3e1f1 i3j3i1j1m3n3m1n1 a2b2a0b0e2f2f0f0 i2j2i0j0m2n2m0n0
; c3d3c1d1g3h3g1h1 k3l3k1l1o3p3o1p1 c2d2c0d0g2h2g0h0 k2l2k0l0o2p2o0p0

	move.l	d1,d2

	move.w	d7,(a4)+
.x1start
	lsr.l	#2,d2
	eor.l	d0,d2
	and.l	#$33333333,d2
	eor.l	d2,d0
	lsl.l	#2,d2
	eor.l	d2,d1

; a3b3c3d3e3f3g3h3 i3j3k3l3m3n3o3p3 a2b2c2d2e2f2g2h2 i2j2k2l2m2n2o2p2
; a1b1c1d1e1f1g1h1 i1j1k1l1m1n1o1p1 a0b0c0d0e0f0g0h0 i0j0k0l0m0n0o0p0

	move.l	d0,d6
	move.l	d1,d7

	cmpa.l	a0,a2
	bne	.x1

	move.w	d6,(a5)+
	swap	d6
	move.w	d7,(a3)+
	swap	d7
	move.w	d6,(a6)+
	move.w	d7,(a4)+

	movem.l	(sp)+,a0-a1/d3

.exit
	addq.l	#C2P1X1_4_C5_BM_WORD_SIZEOF,sp
	movem.l	(sp)+,d2-d7/a2-a6
.earlyexit
	rts

.c2p_mod
	lsr.w	#3,d4
	move.l	d4,C2P1X1_4_C5_BM_WORD_ROWMOD(sp) ; Modulo between two rows

	move.l	a0,a2			; Ptr to end of line + 1 iter
	add.w	C2P1X1_4_C5_BM_WORD_CHUNKYX(sp),a2
	add.w	#16,a2

	movem.l	a0-a2/d1/d3,-(sp)

	movem.l	bm_Planes(a1),a3-a6	; Setup ptrs to bpl0-3
	add.l	d3,a3
	add.l	d3,a4
	add.l	d3,a5
	add.l	d3,a6

	move.l	#$0f0f0f0f,d4
	move.l	#$00ff00ff,d5

	move.l	(a0)+,d0
	move.l	(a0)+,d2
	move.l	(a0)+,d1
	move.l	(a0)+,d3

	and.l	d4,d0
	and.l	d4,d1
	and.l	d4,d2
	and.l	d4,d3
	lsl.l	#4,d0
	lsl.l	#4,d1
	or.l	d2,d0
	or.l	d3,d1

; a3a2a1a0e3e2e1e0 b3b2b1b0f3f2f1f0 c3c2c1c0g3g2g1g0 d3d2d1d0h3h2h1h0
; i3i2i1i0m3m2m1m0 j3j2j1j0n3n2n1n0 k3k2k1k0o3o2o1o0 l3l2l1l0p3p2p1p0

	move.l	d1,d2
	lsr.l	#8,d2
	eor.l	d0,d2
	and.l	d5,d2
	eor.l	d2,d0
	lsl.l	#8,d2
	eor.l	d2,d1

; a3a2a1a0e3e2e1e0 i3i2i1i0m3m2m1m0 c3c2c1c0g3g2g1g0 k3k2k1k0o3o2o1o0
; b3b2b1b0f3f2f1f0 j3j2j1j0n3n2n1n0 d3d2d1d0h3h2h1h0 l3l2l1l0p3p2p1p0

	move.l	d1,d2
	lsr.l	#1,d2
	eor.l	d0,d2
	and.l	#$55555555,d2
	eor.l	d2,d0
	add.l	d2,d2
	eor.l	d2,d1

; a3b3a1b1e3f3e1f1 i3j3i1j1m3n3m1n1 c3d3c1d1g3h3g1h1 k3l3k1l1o3p3o1p1
; a2b2a0b0e2f2f0f0 i2j2i0j0m2n2m0n0 c2d2c0d0g2h2g0h0 k2l2k0l0o2p2o0p0

	move.w	d1,d2
	move.w	d0,d1
	swap	d1
	move.w	d1,d0
	move.w	d2,d1

; a3b3a1b1e3f3e1f1 i3j3i1j1m3n3m1n1 a2b2a0b0e2f2f0f0 i2j2i0j0m2n2m0n0
; c3d3c1d1g3h3g1h1 k3l3k1l1o3p3o1p1 c2d2c0d0g2h2g0h0 k2l2k0l0o2p2o0p0

	move.l	d1,d2
	bra	.modx1start
.modx1y
	add.w	C2P1X1_4_C5_BM_WORD_CHUNKYX+20(sp),a2 ; Skip to end of next
						 ; line + 1 iter
	move.l	C2P1X1_4_C5_BM_WORD_ROWMOD+20(sp),d0  ; Skip to beginning of
	add.l	d0,a3				 ; next line
	add.l	d0,a4
	add.l	d0,a5
	add.l	d0,a6
.modx1
	move.l	(a0)+,d0
	move.l	(a0)+,d2
	move.l	(a0)+,d1
	move.l	(a0)+,d3

	move.w	d6,(a5)+
	swap	d6

	and.l	d4,d0
	and.l	d4,d1
	and.l	d4,d2
	and.l	d4,d3
	lsl.l	#4,d0
	lsl.l	#4,d1
	or.l	d2,d0
	or.l	d3,d1

; a3a2a1a0e3e2e1e0 b3b2b1b0f3f2f1f0 c3c2c1c0g3g2g1g0 d3d2d1d0h3h2h1h0
; i3i2i1i0m3m2m1m0 j3j2j1j0n3n2n1n0 k3k2k1k0o3o2o1o0 l3l2l1l0p3p2p1p0

	move.w	d7,(a3)+
	swap	d7

	move.l	d1,d2
	lsr.l	#8,d2
	eor.l	d0,d2
	and.l	d5,d2
	eor.l	d2,d0
	lsl.l	#8,d2
	eor.l	d2,d1

; a3a2a1a0e3e2e1e0 i3i2i1i0m3m2m1m0 c3c2c1c0g3g2g1g0 k3k2k1k0o3o2o1o0
; b3b2b1b0f3f2f1f0 j3j2j1j0n3n2n1n0 d3d2d1d0h3h2h1h0 l3l2l1l0p3p2p1p0

	move.l	d1,d2
	lsr.l	#1,d2

	move.w	d6,(a6)+

	eor.l	d0,d2
	and.l	#$55555555,d2
	eor.l	d2,d0
	add.l	d2,d2
	eor.l	d2,d1

; a3b3a1b1e3f3e1f1 i3j3i1j1m3n3m1n1 c3d3c1d1g3h3g1h1 k3l3k1l1o3p3o1p1
; a2b2a0b0e2f2f0f0 i2j2i0j0m2n2m0n0 c2d2c0d0g2h2g0h0 k2l2k0l0o2p2o0p0

	move.w	d1,d2
	move.w	d0,d1
	swap	d1
	move.w	d1,d0
	move.w	d2,d1

; a3b3a1b1e3f3e1f1 i3j3i1j1m3n3m1n1 a2b2a0b0e2f2f0f0 i2j2i0j0m2n2m0n0
; c3d3c1d1g3h3g1h1 k3l3k1l1o3p3o1p1 c2d2c0d0g2h2g0h0 k2l2k0l0o2p2o0p0

	move.l	d1,d2

	move.w	d7,(a4)+
.modx1start
	lsr.l	#2,d2
	eor.l	d0,d2
	and.l	#$33333333,d2
	eor.l	d2,d0
	lsl.l	#2,d2
	eor.l	d2,d1

; a3b3c3d3e3f3g3h3 i3j3k3l3m3n3o3p3 a2b2c2d2e2f2g2h2 i2j2k2l2m2n2o2p2
; a1b1c1d1e1f1g1h1 i1j1k1l1m1n1o1p1 a0b0c0d0e0f0g0h0 i0j0k0l0m0n0o0p0

	move.l	d0,d6
	move.l	d1,d7

	cmpa.l	a0,a2
	bne	.modx1

	subq.w	#1,C2P1X1_4_C5_BM_WORD_CHUNKYY+20(sp)
	bne	.modx1y

	movem.l	(sp)+,a0-a2/d1/d3

	move.w	d1,C2P1X1_4_C5_BM_WORD_CHUNKYY(sp)

	bra	.exit


	XDEF	_vline
	XDEF	vline
_vline:
vline:

; a0 = startpos in framebuffer
; d0.w = (color << 8) | color
; d1.w = height
	movem.l	d0-d7/a0-a6,-(a7)

	lsr.w	#1,d1 
	beq.s	.skip1
	move.w	d0,(a0)
	lea	160(a0),a0
.skip1:
	lsr.w	#1,d1
	beq.s	.skip2
	move.w	d0,(a0)
	move.w	d0,160(a0)
	lea	320(a0),a0
.skip2:
	subq.w	#1,d1
	bmi.s	.done
.loop4:
	move.w	d0,(a0)
	move.w	d0,160(a0)
	move.w	d0,320(a0)
	move.w	d0,480(a0)
	lea	640(a0),a0
	dbf	d1,.loop4
.done:
	movem.l	(a7)+,d0-d7/a0-a6

	rts

	XDEF	_c2p2x1_4_c5_bm
	XDEF	c2p2x1_4_c5_bm
_c2p2x1_4_c5_bm
c2p2x1_4_c5_bm
	movem.l	d2-d7/a2-a6,-(sp)
					; A few sanity checks
	cmpi.b	#4,bm_Depth(a1)		; At least 4 valid bplptrs?
	blo	.exitzzz
	move.w	d0,d4
	move.w	d2,d5
	andi.w	#$f,d4			; Even 16-pixel width?
	bne	.exitzzz
	andi.w	#$7,d5			; Even 8-pixel xoffset?
	bne	.exitzzz
	moveq	#0,d4
	move.w	bm_BytesPerRow(a1),d4

	move.w	d1,d5			; Skip if 0 pixels to convert
	beq	.exitzzz
	swap	d5
	move.w	d0,d5
	beq	.exitzzz

	ext.l	d2			; Offs to first pixel to draw in bpl
	mulu.w	d4,d3
	lsr.l	#3,d2
	add.l	d2,d3

	move.w	d4,d2			; Modulo from one line to the next in destination BitMap
	lsr.w	#2,d0
	sub.w	d0,d2
	bmi	.exitzzz

	movem.l	bm_Planes(a1),a3-a6	; Setup ptrs to bpl0-3
	add.l	d3,a3
	add.l	d3,a4
	add.l	d3,a5
	add.l	d3,a6

	move.w	d2,a1

	move.l	a0,a2			; End of current line in input chunkybuffer
	add.w	d5,a2

	move.l	(a0)+,d0
	move.l	(a0)+,d1
	move.l	(a0)+,d2
	move.l	(a0)+,d3

	swap	d2			; Swap 16x2
	swap	d3
	eor.w	d0,d2
	eor.w	d1,d3
	eor.w	d2,d0
	eor.w	d3,d1
	eor.w	d0,d2
	eor.w	d1,d3
	swap	d2
	swap	d3

	lsl.l	#4,d0			; Merge 4x2, part 1a
	lsl.l	#4,d1

	or.l	d2,d0			; Merge 4x2, part 1b
	or.l	d3,d1

	move.l	d1,d4			; Swap 8x1
	lsr.l	#8,d4
	eor.l	d0,d4
	and.l	#$00ff00ff,d4
	eor.l	d4,d0
	lsl.l	#8,d4
	eor.l	d4,d1

	move.l	d1,d4			; Swap 2x1
	lsr.l	#2,d4
	eor.l	d0,d4
	and.l	#$33333333,d4
	eor.l	d4,d0
	lsl.l	#2,d4
	eor.l	d1,d4

	move.l	d0,d7			; Dup 1x1, part 1
	lsr.l	#1,d7
	eor.l	d0,d7
	and.l	#$55555555,d7
	eor.l	d7,d0
	move.l	d0,d6
	eor.l	d7,d0
	add.l	d7,d7
	eor.l	d0,d7

	swap	d5
.y
	swap	d5
.x
	move.l	(a0)+,d0
	move.l	(a0)+,d1
	move.l	(a0)+,d2
	move.l	(a0)+,d3

	move.l	d6,(a6)+
	swap	d2			; Swap 16x2
	swap	d3
	eor.w	d0,d2
	eor.w	d1,d3
	eor.w	d2,d0
	eor.w	d3,d1
	eor.w	d0,d2
	eor.w	d1,d3
	swap	d2
	swap	d3

	move.l	d4,d6			; Dup 1x1, part 2a
	lsr.l	#1,d6
	eor.l	d4,d6
	and.l	#$55555555,d6

	lsl.l	#4,d0			; Merge 4x2, part 1a
	move.l	d7,(a5)+
	lsl.l	#4,d1

	eor.l	d6,d4			; Dup 1x1, part 2b
	move.l	d4,d7
	eor.l	d6,d4
	add.l	d6,d6
	eor.l	d4,d6

	move.l	d7,(a4)+
	or.l	d2,d0			; Merge 4x2, part 1b
	or.l	d3,d1

	move.l	d1,d4			; Swap 8x1
	lsr.l	#8,d4
	eor.l	d0,d4
	and.l	#$00ff00ff,d4
	eor.l	d4,d0
	lsl.l	#8,d4
	eor.l	d4,d1

	move.l	d1,d4			; Swap 2x1
	lsr.l	#2,d4
	eor.l	d0,d4
	and.l	#$33333333,d4
	eor.l	d4,d0
	move.l	d6,(a3)+
	lsl.l	#2,d4
	eor.l	d1,d4

	move.l	d0,d7			; Dup 1x1, part 1
	lsr.l	#1,d7
	eor.l	d0,d7
	and.l	#$55555555,d7
	eor.l	d7,d0
	move.l	d0,d6
	eor.l	d7,d0
	add.l	d7,d7
	eor.l	d0,d7

	cmp.l	a0,a2
	bhs.w	.x

	add.l	a1,a3
	add.l	a1,a4
	add.l	a1,a5
	add.l	a1,a6
	add.w	d5,a2

	swap	d5
	
	subq.w	#1,d5
	bne .y

.exitzzz
	movem.l	(sp)+,d2-d7/a2-a6
	rts


;
; 1999-01-08
;
; c2p2x1_8_c5_030
;
; 0.83vbl [all dma off] on Blizzard 1230-IV@50MHz
;

	IFND	BPLX
BPLX	EQU	320
	ENDC
	IFND	BPLY
BPLY	EQU	256
	ENDC
	IFND	BPLSIZE
BPLSIZE	EQU	BPLX*BPLY/8
	ENDC
	IFND	CHUNKYXMAX
CHUNKYXMAX EQU	BPLX/2
	ENDC
	IFND	CHUNKYYMAX
CHUNKYYMAX EQU	BPLY
	ENDC

	XDEF	_c2p2x1_8_c5_030_init
	XDEF	_c2p2x1_8_c5_030

	section	code,code

; d0.w	chunkyx [chunky-pixels]
; d1.w	chunkyy [chunky-pixels]
; d2.w	(scroffsx) [screen-pixels]
; d3.w	scroffsy [screen-pixels]
; d4.w	(rowlen) [bytes] -- offset between one row and the next in a bpl
; d5.l	(bplsize) [bytes] -- offset between one row in one bpl and the next bpl

_c2p2x1_8_c5_030_init
c2p2x1_8_c5_030_init
	movem.l	d2-d3,-(sp)
	andi.l	#$ffff,d0
	mulu.w	d0,d3
	lsr.l	#3,d3
	move.l	d3,c2p2x1_8_c5_030_scroffs
	mulu.w	d0,d1
	move.l	d1,c2p2x1_8_c5_030_pixels
	movem.l	(sp)+,d2-d3
	rts

; a0	c2pscreen
; a1	bitplanes

_c2p2x1_8_c5_030
c2p2x1_8_c5_030
	movem.l	d2-d7/a2-a6,-(sp)

	add.w	#BPLSIZE,a1
	add.l	c2p2x1_8_c5_030_scroffs,a1

	move.l	#$55555555,d5

	move.l	#$00ff00ff,a6

	move.l	c2p2x1_8_c5_030_pixels,a5
	add.l	a0,a5
	move.l	a1,a2
	add.l	#BPLSIZE*4,a2
	cmpa.l	a0,a5
	beq	.none

	move.l	(a0)+,d0
	move.l	(a0)+,d1
	move.l	(a0)+,d2
	move.l	(a0)+,d3

	move.w	d2,d7			; Swap 16x2
	move.w	d0,d2
	swap	d2
	move.w	d2,d0
	move.w	d7,d2

	move.w	d3,d7
	move.w	d1,d3
	swap	d3
	move.w	d3,d1
	move.w	d7,d3

	move.l	#$0f0f0f0f,d6
	move.l	d2,d7			; Swap 4x2
	lsr.l	#4,d7
	eor.l	d0,d7
	and.l	d6,d7
	eor.l	d7,d0
	lsl.l	#4,d7
	eor.l	d7,d2

	move.l	d3,d7
	lsr.l	#4,d7
	eor.l	d1,d7
	and.l	d6,d7
	eor.l	d7,d1
	lsl.l	#4,d7
	eor.l	d7,d3

	move.l	a6,d6
	move.l	d1,d7			; Swap 8x1, part 1
	lsr.l	#8,d7
	eor.l	d0,d7
	and.l	d6,d7
	eor.l	d7,d0
	lsl.l	#8,d7
	eor.l	d7,d1

	bra	.start
.x
	move.l	(a0)+,d0
	move.l	(a0)+,d1
	move.l	(a0)+,d2
	move.l	(a0)+,d3
	move.l	d7,(a2)+
	eor.l	d4,d7
	add.l	d4,d4
	eor.l	d7,d4

	move.w	d2,d7			; Swap 16x2
	move.w	d0,d2
	swap	d2
	move.w	d2,d0
	move.w	d7,d2

	move.l	d4,-BPLSIZE-4(a2)

	move.w	d3,d7
	move.w	d1,d3
	swap	d3
	move.w	d3,d1
	move.w	d7,d3

	move.l	a3,d4
	move.l	d4,d7
	lsr.l	#1,d7
	eor.l	d4,d7
	and.l	d5,d7
	eor.l	d7,d4
	move.l	d4,BPLSIZE*2(a1)
	eor.l	d7,d4
	add.l	d7,d7
	eor.l	d7,d4

	move.l	#$0f0f0f0f,d6
	move.l	d2,d7			; Swap 4x2
	lsr.l	#4,d7
	eor.l	d0,d7
	and.l	d6,d7
	eor.l	d7,d0
	lsl.l	#4,d7
	eor.l	d7,d2
	move.l	d4,BPLSIZE(a1)

	move.l	d3,d7
	lsr.l	#4,d7
	eor.l	d1,d7
	and.l	d6,d7
	eor.l	d7,d1
	lsl.l	#4,d7
	eor.l	d7,d3

	move.l	a4,d4
	move.l	d4,d7
	lsr.l	#1,d7
	eor.l	d4,d7
	and.l	d5,d7
	eor.l	d7,d4
	move.l	d4,(a1)+
	eor.l	d7,d4
	add.l	d7,d7
	eor.l	d7,d4

	move.l	a6,d6
	move.l	d1,d7			; Swap 8x1, part 1
	lsr.l	#8,d7
	eor.l	d0,d7
	and.l	d6,d7
	eor.l	d7,d0
	lsl.l	#8,d7
	eor.l	d7,d1
	move.l	d4,-BPLSIZE-4(a1)
.start

	move.l	#$33333333,d6
	move.l	d1,d7			; Swap 2x1, part 1
	lsr.l	#2,d7
	eor.l	d0,d7
	and.l	d6,d7
	eor.l	d7,d0
	lsl.l	#2,d7
	eor.l	d1,d7

	move.l	d0,d4
	lsr.l	#1,d4
	eor.l	d0,d4
	and.l	d5,d4
	eor.l	d4,d0
	move.l	d0,BPLSIZE*2(a2)
	eor.l	d4,d0
	add.l	d4,d4
	eor.l	d4,d0

	move.l	a6,d6
	move.l	d3,d1			; Swap 8x1, part 2
	lsr.l	#8,d1
	eor.l	d2,d1
	and.l	d6,d1
	eor.l	d1,d2
	lsl.l	#8,d1
	eor.l	d1,d3

	move.l	d0,BPLSIZE(a2)

	move.l	#$33333333,d6
	move.l	d3,d1			; Swap 2x1, part 2
	lsr.l	#2,d1
	eor.l	d2,d1
	and.l	d6,d1
	eor.l	d1,d2
	lsl.l	#2,d1
	eor.l	d1,d3
	move.l	d2,a3
	move.l	d3,a4

	move.l	d7,d4
	lsr.l	#1,d4
	eor.l	d7,d4
	and.l	d5,d4
	eor.l	d4,d7

	cmp.l	a0,a5
	bne	.x
.x2
	move.l	d7,(a2)+
	eor.l	d4,d7
	add.l	d4,d4
	eor.l	d7,d4
	move.l	d4,-BPLSIZE-4(a2)

	move.l	a3,d4
	move.l	d4,d7
	lsr.l	#1,d7
	eor.l	d4,d7
	and.l	d5,d7
	eor.l	d7,d4
	move.l	d4,BPLSIZE*2(a1)
	eor.l	d7,d4
	add.l	d7,d7
	eor.l	d7,d4
	move.l	d4,BPLSIZE(a1)

	move.l	a4,d4
	move.l	d4,d7
	lsr.l	#1,d7
	eor.l	d4,d7
	and.l	d5,d7
	eor.l	d7,d4
	move.l	d4,(a1)+
	eor.l	d7,d4
	add.l	d7,d7
	eor.l	d7,d4
	move.l	d4,-BPLSIZE-4(a1)

.none
	movem.l	(sp)+,d2-d7/a2-a6
	rts

	section	bss,bss

c2p2x1_8_c5_030_scroffs	ds.l	1
c2p2x1_8_c5_030_pixels	ds.l	1
