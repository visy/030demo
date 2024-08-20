
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



	XDEF	_ChunkyLine
	XDEF	ChunkyLine
;
;ChunkyLine
;a0=ChunkyScreen, d0=x0, d1=y0, d2=x1, d3=y1, d4=Color, d5=pixelwidth, d6=pixelheight
;without clipping pixelheight is not needed

_ChunkyLine:
ChunkyLine:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	d4,a1	;Color
	move.l	d5,a4	;pixelwidth

	cmp	d1,d3
	bne.b	.YNotEqual

	cmp	d0,d2
	beq.b	.dontdraw

.YNotEqual:
	move	d2,d4		;int dx =  abs(x1 - x0);
	sub	d0,d4
	beq.b	.VLine
	bpl.b	.abs1

	neg	d4
.abs1:	moveq	#1,d5		;int sx = x0 < x1 ? 1 : -1;
	cmp	d0,d2		;int sx = x0 < x1 ? 1 : -1;
	bgt.b	.sxpos

	moveq	#-1,d5
.sxpos:
	move	d3,d6		;int dy = -abs(y1 - y0);
	sub	d1,d6
	beq.w	.HLine
	bmi.b	.abs2

	neg	d6
.abs2:	move.l	a4,d7		;int sy = y0 < y1 ? 1 : -1; (pixelwidth)
	cmp	d1,d3		;int sy = y0 < y1 ? 1 : -1;
	bgt.b	.sypos

	neg.l	d7		;-pixelwidth
.sypos:	move.l	d5,a5		;a5 = sx
	move.l	d7,a6		;a6 = sy
	move	d2,a2		;a2 = x1
	move	d4,d2		;int err = dx [+ dy];

	move.l	a4,d5		;pixelwidth für setpixel
	mulu	d3,d5
	add.l	a0,d5
	move.l	d5,a3		;a3 = y1
	
	add	d6,d2		;+ dy;

	move.l	a4,d5		;pixelwidth für setpixel
	mulu	d1,d5		;für setpixel
	add.l	d5,a0

	move.l	a1,d7		;color

	move	d2,d3		;e2 = [2 *] err;
	bra.b	.PixelLoop

	cnop	0,16

.PixelLoop:
	add	d3,d3		;2 * err;
	move.b	d7,(a0,d0.w)	;setpixel
	cmp	d3,d6		;if (e2 > dy) {
	bgt.b	.e2dy

	add	d6,d2		;err += dy;
	add	a5,d0		;x0 += sx;
.e2dy:
	cmp	d3,d4		;if (e2 < dx) {
	blt.b	.e2dx

	add	d4,d2		;err += dx;
	add.l	a6,a0		;y0 += sy; (erspart mul)
.e2dx:
	move	d2,d3		;e2 = [2 *] err;
	cmp	d0,a2		;if (x0 == x1 && [y0 == y1]) break;
	bne.b	.PixelLoop

	cmp.l	a0,a3		;y0 == y1 ?
	bne.b	.PixelLoop

.dontdraw:
	movem.l	(a7)+,d0-d7/a0-a6
	rts

	cnop	2,4
	
.VLine:	cmp	d1,d3
	bgt.b	.DontSwapY

	exg	d1,d3
.DontSwapY:
	move	d3,d6
	move.l	a1,d7	;color
	sub	d1,d6
	move.l	a4,d5	;pixelwidth für setpixel
	subq	#1,d6
	mulu	d1,d5	;für setpixel
	add.l	d5,a0
	add	d0,a0

.VLineL:move.b	d7,(a0)
	add.l	a4,a0
	dbf	d6,.VLineL

	movem.l	(a7)+,d0-d7/a0-a6
	rts

	cnop	0,4
	
.HLine:	cmp	d0,d2
	bgt.b	.DontSwapX

	exg	d0,d2
.DontSwapX:
	move	d2,d4
	move.l	a1,d7	;color
	sub	d0,d4
	move.l	a4,d5	;pixelwidth für setpixel
	move	d7,d6
	lsl.l	#8,d7
	mulu	d1,d5	;für setpixel
	move.b	d6,d7
	add.l	d5,a0
	move	d7,d6
	swap	d7
	add	d0,a0
	move	d6,d7

	btst	#0,d0
	beq.b	.NotOdd

	subq	#1,d4
	bmi.b	.IllegalLine

	move.b	d7,(a0)+
.NotOdd:
	lsr	#1,d4
	beq.b	.NoWordRemain

	bcs.b	.DrawLongByteR

	lsr	#1,d4
	beq.b	.WordRemain

	bcs.b	.DrawLongWordR

	subq	#1,d4
.DrawLong1:
	move.l	d7,(a0)+
	dbf	d4,.DrawLong1

	bra.b	.IllegalLine

	cnop	2,4

.DrawLongWordR:
	subq	#1,d4
.DrawLong2:
	move.l	d7,(a0)+
	dbf	d4,.DrawLong2

.WordRemain:
	move	d7,(a0)
	bra.b	.IllegalLine

	cnop	0,4

.DrawLongByteR:
	lsr	#1,d4
	beq.b	.ThreeBytes

	bcs.b	.DrawLongThreeR

	subq	#1,d4
.DrawLong3:
	move.l	d7,(a0)+
	dbf	d4,.DrawLong3

.ByteRemain:
	move.b	d7,(a0)
	bra.b	.IllegalLine

.DrawLongThreeR:
	subq	#1,d4
.DrawLong4:
	move.l	d7,(a0)+
	dbf	d4,.DrawLong4

.ThreeBytes:
	move	d7,(a0)+
	move.b	d7,(a0)
	bra.b	.IllegalLine

.NoWordRemain:
	bcs.b	.ByteRemain

.IllegalLine:
	movem.l	(a7)+,d0-d7/a0-a6
	rts