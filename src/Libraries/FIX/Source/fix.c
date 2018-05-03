/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
*/
/*
** fix.c
**
** $Header: r:/prj/lib/src/fix/RCS/fix.c 1.21 1994/08/11 12:11:09 dfan Exp $
** $Log: fix.c $
 * Revision 1.21  1994/08/11  12:11:09  dfan
 * move some stuff out
 * 
 * Revision 1.20  1994/03/16  10:29:08  dfan
 * fix_safe_pyth_dist etc.
 * 
 * Revision 1.19  1994/03/01  11:28:44  dfan
 * Do it for fix24's too
 * 
 * Revision 1.18  1994/03/01  11:27:58  dfan
 * Don't need quadrant/placing code in fix_atan, and besides,
 * it's buggy
 * 
 * Revision 1.17  1994/02/16  17:33:47  dfan
 * allow arbitrarily small arguments to fix_exp
 * 
 * Revision 1.16  1993/11/30  13:02:42  dfan
 * safe fix_mul
 * 
 * Revision 1.15  1993/11/04  11:06:09  rex
 * Moved fix_sprintf() stuff out into fixsprnt.c
 * 
 * Revision 1.14  1993/10/20  13:08:22  dfan
 * some more fast_pyth_dists
 * 
 * Revision 1.13  1993/09/17  13:03:30  dfan
 * fast_pyth_dist
 * 
 * Revision 1.12  1993/07/30  12:42:55  dfan
 * fix_exp
 * 
 * Revision 1.11  1993/06/27  03:09:43  dc
 * pretty up hexprint formatting, sorry, whoops. etc.
 * 
 * Revision 1.10  1993/06/27  02:30:24  dc
 * fix_sprint now returns the buffer string, also added fix_sprint_hex
 * 
 * Revision 1.9  1993/06/01  14:14:43  dfan
 * Include trigtab.h, generated by fmaketab
 * Lose a bit of precision, but now all trig functions work everywhere
 * 
 * Revision 1.8  1993/04/19  13:31:33  dfan
 * individual sin & cos functions
 * 
 * Revision 1.7  1993/03/17  12:46:45  matt
 * Removed 'const' from sincos declaration so could be ref'd in fix_asm.asm
 * 
 * Revision 1.6  1993/03/02  10:51:30  dfan
 * atan2: comparisons of sin should have been unsigned, not signed
 * 
 * Revision 1.5  1993/02/15  12:15:21  dfan
 * more fix24 functions
 * 
 * Revision 1.4  1993/02/15  11:39:57  dfan
 * fix24 support, not for all fuctions though
 * 
 * Revision 1.3  1993/01/29  16:37:22  dfan
 * maybe we shouldn't printf an error message in a library, what say
 * 
 * Revision 1.2  1993/01/29  11:10:06  dfan
 * Changed fix_pyth_dist to be straightforward (and twice as fast)
 * The arctrig functions worked all along
 * 
 * Revision 1.1  1993/01/22  09:57:39  dfan
 * Initial revision
 * 
*/

#include <stdlib.h>
// #include <FixMath.h>
//#include <stdio.h>
//#include <lg.h>
#include "fix.h"
#include "trigtab.h"

// #include <Carbon/Carbon.h>

int	gOVResult;


//----------------------------------------------------------------------------
// fix_mul: Multiply two fixed numbers.
//----------------------------------------------------------------------------
#if defined(powerc) || defined(__powerc)
fix fix_mul(fix a, fix b)
{
	float af = fix_float(a);
	float bf = fix_float(b);
	return fix_from_float(af * bf);
}
#else
fix asm fix_mul(fix a, fix b)
{
 	move.l	4(sp),d0
	dc.w		0x4c2f,0x0c01,0x0008		// 	muls.l	8(sp),d1:d0
	move.w	d1,d0
	swap		d0
	rts
}
#endif


//----------------------------------------------------------------------------
// fast_fix_mul_int: Return the high word of a fixed multiply.
//----------------------------------------------------------------------------
#if defined(powerc) || defined(__powerc)
fix fast_fix_mul_int(fix a, fix b)
{
	float af = fix_float(a);
	float bf = fix_float(b);
	fix f = fix_from_float(af * bf);
	return fix_int(f);
}
#else
fix asm fast_fix_mul_int(fix a, fix b)
{
 	move.l	4(sp),d0
	dc.w		0x4c2f,0x0c01,0x0008		// 	muls.l	8(sp),d1:d0
	move.l	d1,d0
	rts
}
#endif


//----------------------------------------------------------------------------
// fix_mul_asm_safe: Multiply two fixed numbers, checking for -1/0 problems
//----------------------------------------------------------------------------
#if defined(powerc) || defined(__powerc)
fix fix_mul_asm_safe(fix a, fix b)
{
	float af = fix_float(a);
	float bf = fix_float(b);
	float val = af * bf;
	return fix_from_float(af * bf);
}
#else
fix asm fix_mul_asm_safe(fix a, fix b)
{
 	move.l	4(sp),d0
	dc.w		0x4c2f,0x0c01,0x0008		// 	muls.l	8(sp),d1:d0
	move.w	d0,d2
	move.w	d1,d0
	swap		d0
	
	cmp.l		#-1,d0
	beq.s		@MaybeBad
	rts
	
@MaybeBad:	
	cmp.w		#-1,d2
	beq.s		@OK
	
	moveq		#0,d0		// zero it
	
@OK:
	rts
}
#endif

//----------------------------------------------------------------------------
// fix_div: Divide two fixed numbers.
//----------------------------------------------------------------------------
#if defined(powerc) || defined(__powerc)
fix fix_div(fix a, fix b)
{
	float af = fix_float(a);
	float bf = fix_float(b);
	return fix_from_float(af / bf);
}
#else
fix asm fix_div(fix a, fix b)
{
	clr.l		gOVResult
	move.l	8(sp),d2
	beq.s		@DivZero
 	moveq		#0,d1
 	move.l	4(sp),d0
 	swap		d0
 	move.w	d0,d1
 	ext.l		d1
 	clr.w		d0
 	dc.l		0x4C420C01          // DIVS.L    D2,D1:D0
 	bvs.s		@DivOverflow
 	rts
 	
@DivOverflow:
	move.l	#1,gOVResult
	move.l	#0x7FFFFFFF,d0
	tst.b		4(sp)
	bpl.s		@noNeg
	neg.l		d0
@noNeg:
	rts

@DivZero:
	move.l	#2,gOVResult
	move.l	#0x7FFFFFFF,d0
	tst.b		4(sp)
	bpl.s		@noNeg2
	neg.l		d0
@noNeg2:
	rts
 }
#endif

//----------------------------------------------------------------------------
// Multiply two numbers, and divide by a third. Used to be in asm, but
// now in C.
//----------------------------------------------------------------------------
#if defined(powerc) || defined(__powerc)
fix fix_mul_div(fix m0, fix m1, fix d)
{
	float af = fix_float(m0);
	float bf = fix_float(m1);
	float df = fix_float(d);
	return fix_from_float((af * bf) / df);
}
#else
fix asm fix_mul_div (fix m0, fix m1, fix d)
 {
	clr.l		gOVResult
 	move.l	4(sp),d0
	dc.w		0x4c2f,0x0c01,0x0008		// 	muls.l	8(sp),d1:d0
	
  move.l	12(sp),d2
	beq.s		@DivZero
	dc.l		0x4C420C01         			//  DIVS.L  D2,D1:D0
	bvs.s		@DivOverflow
	rts

@DivOverflow:
	move.l	#1,gOVResult
	move.l	#0x7FFFFFFF,d0
	tst.l		d1
	bpl.s		@noNeg
	neg.l		d0
@noNeg:
	rts
	
@DivZero:
	move.l	#2,gOVResult
	move.l	#0x7FFFFFFF,d0
	tst.l		d1									// sign of high double long from multiply
	bpl.s		@noNeg2
	neg.l		d0
@noNeg2:
	rts
 }
#endif


int blah;

//----------------------------------------------------------------------------
// Returns the distance from (0,0) to (a,b)
//----------------------------------------------------------------------------
fix fix_pyth_dist (fix a, fix b)
{	
	// ¥¥¥should check for overflow!
 	return fix_sqrt(fix_mul(a, a) + fix_mul(b, b));
}


//----------------------------------------------------------------------------
// Returns an approximation to the distance from (0,0) to (a,b)
//----------------------------------------------------------------------------
fix fix_fast_pyth_dist (fix a, fix b)
{
   if (a < 0) a = -a;
   if (b < 0) b = -b;
   if (a > b) return (a + b/2);
         else return (b + a/2);
}


//----------------------------------------------------------------------------
// We can use the fix function because the difference in scale doesn't matter.
//----------------------------------------------------------------------------
long long_fast_pyth_dist (long a, long b)
{
   return (fix_fast_pyth_dist (a, b));
}

//----------------------------------------------------------------------------
// This function is safer than the other fix_pyth_dist because we don't
// have to worry about overflow.
//
// Uses algorithm from METAFONT involving reflecting (a,b) through
// line from (0,0) to (a,b/2), which keeps a^2+b^2 invariant but
// greatly reduces b.  When b reaches 0, a is the distance.
//
// Knuth credits it to Moler & Morrison, IBM Journal of Research and
// Development 27 (1983).  Good for them.
//----------------------------------------------------------------------------
fix fix_safe_pyth_dist (fix a, fix b)
{
	fix tmp;
			
	a = abs(a); b = abs(b);					// works fine since they're really longs
	if (a < b)
	{
		tmp = a; a = b; b = tmp;
	}												// now 0 <= b <= a
	if (a > 0)
	{
		if (a > 0x2fffffff)
		{
//			Warning (("Overflow in fix_safe_pyth_dist\n"));
			DebugString("Overflow in fix_safe_pyth_dist");
			return 0;
		}
		for (;;)
		{
			// This is a quick way of doing the reflection
			tmp = fix_div (b, a);
			tmp = fix_mul (tmp, tmp);
			if (tmp == 0) break;
			tmp = fix_div (tmp, tmp + fix_make(4,0));
			a += fix_mul (2*a, tmp);
			b = fix_mul (b, tmp);
		}
	}
	return a;
}

//----------------------------------------------------------------------------
// We can use the fix function because the difference in scale doesn't matter.
//----------------------------------------------------------------------------
long long_safe_pyth_dist (long a, long b)
{
   return (fix_safe_pyth_dist (a, b));
}


//----------------------------------------------------------------------------
// Computes sin and cos of theta
//----------------------------------------------------------------------------
void fix_sincos (fixang theta, fix *sin, fix *cos)
{
	uchar baseth, fracth;					// high and low bytes of the
	ushort lowsin, lowcos, hisin, hicos; 	// table lookups

	// divide the angle into high and low bytes
	// we will do a table lookup with the high byte and
	// interpolate with the low byte
	baseth = theta >> 8;
	fracth = theta & 0xff;
	
	// use the identity [cos x = sin (x + PI/2)] to look up
	// cosines in the sine table
	lowsin = sintab[baseth];
	hisin = sintab[baseth + 1];
	lowcos = sintab[baseth + 64];
	hicos = sintab[baseth + 65];
	
	// interpolate between low___ and hi___ according to fracth
	*sin = ((short)(lowsin + (short)((((short)hisin - (short)lowsin) * (uchar)fracth) >> 8))) << 2;
	*cos = ((short)(lowcos + (short)((((short)hicos - (short)lowcos) * (uchar)fracth) >> 8))) << 2;

	return;
}

//----------------------------------------------------------------------------
// Computes sin of theta
//----------------------------------------------------------------------------
fix fix_sin (fixang theta)
{
	uchar baseth, fracth;
	ushort lowsin, hisin;

	baseth = theta >> 8;
	fracth = theta & 0xff;
	lowsin = sintab[baseth];
	hisin = sintab[baseth + 1];
	return ((short) (lowsin + (short) (( ( (short) hisin - (short) lowsin ) * (uchar) fracth ) >> 8))) << 2;
}

//----------------------------------------------------------------------------
// Computes cos of theta
//----------------------------------------------------------------------------
fix fix_cos (fixang theta)
{
	uchar baseth, fracth;
	ushort lowcos, hicos;

	baseth = theta >> 8;
	fracth = theta & 0xff;
	lowcos = sintab[baseth + 64];
	hicos = sintab[baseth + 65];
	return ((short) (lowcos + (short) (( ( (short) hicos - (short) lowcos ) * (uchar) fracth ) >> 8))) << 2;
}

//----------------------------------------------------------------------------
// Computes sin and cos of theta
// Faster than fix_sincos() but not as accurate (does not interpolate)
//----------------------------------------------------------------------------
void fix_fastsincos (fixang theta, fix *sin, fix *cos)
{
	// use the identity [cos x = sin (x + PI/2)] to look up
	// cosines in the sine table
	*sin = (((short) (sintab[theta >> 8])) << 2);
	*cos = (((short) (sintab[(theta >> 8) + 64])) << 2);

	return;
}

//----------------------------------------------------------------------------
// Fast sin of theta
//----------------------------------------------------------------------------
fix fix_fastsin (fixang theta)
{
	return (((short) (sintab[theta >> 8])) << 2);
}

//----------------------------------------------------------------------------
// Fast cos of theta
//----------------------------------------------------------------------------
fix fix_fastcos (fixang theta)
{
	return (((short) (sintab[(theta >> 8) + 64])) << 2);
}


//----------------------------------------------------------------------------
// Computes the arcsin of x
// Assumes -1 <= x <= 1
// Returns 0xc000..0x4000 (-PI/2..PI/2)
//----------------------------------------------------------------------------
fixang fix_asin (fix x)
{
	uchar basex, fracx;								// high and low bytes of x
	fixang lowy, hiy;							// table lookups

	// divide x into high and low bytes
	// lookup with the high byte, interpolate with the low
	// We shift basex around to make it continuous; see trigtab.h

	basex = ((x >> 2) >> 8) + 0x40;
	fracx = (x >> 2) & 0xff;

	lowy = asintab[basex];
	hiy = asintab[basex+1];

	// interpolate between lowy and hiy according to fracx
	return (lowy + (short) (( ( (short) hiy - (short) lowy ) * (uchar) fracx ) >> 8));
}



//----------------------------------------------------------------------------
// Computes the arccos of x
// Returns 0x0000..0x8000 (0..PI)
//----------------------------------------------------------------------------
fixang fix_acos (fix x)
{
	uchar basex, fracx;
	ushort lowy, hiy;
	fixang asin_answer;

	// acos(x) = PI/2 - asin(x)

	basex = ((x >> 2) >> 8) + 0x40;
	fracx = (x >> 2) & 0xff;

	lowy = asintab[basex];
	hiy = asintab[basex+1];

	asin_answer = (lowy + (short) (( ( (short) hiy - (short) lowy ) * (uchar) fracx ) >> 8));
	return (0x4000 - asin_answer);
}

//----------------------------------------------------------------------------
// Computes the atan of y/x, in the correct quadrant and everything
//----------------------------------------------------------------------------
fixang fix_atan2 (fix y, fix x)
{
	fix hyp;										// hypotenuse
	fix s, c;									// sine, cosine
	fixang th;									// our answer

	// Get special cases out of the way so we don't have to deal
	// with things like making sure 1 gets converted to 0x7fff and
	// not 0x8000.  Note that we grab the y = x = 0 case here
	if (y == 0)
	{
		if (x >= 0) return 0x0000;
		else return 0x8000;
	}
	else if (x == 0)
	{
		if (y >= 0) return 0x4000;
		else return 0xc000;
	}

	if ((hyp = fix_pyth_dist (x, y)) == 0)
	{
//		printf ("hey, dist was 0\n");

		return 0;
	}

	// Use fix_asin or fix_acos depending on where we are.  We don't want to use
	// fix_asin if the sin is close to 1 or -1
	s = fix_div (y, hyp);
	if ((ulong) s < 0x00004000 || (ulong) s > 0xffffc000)
	{												// range is good, use asin
		th = fix_asin (s);
		if (x < 0)
		{
			if (th < 0x4000) th = 0x8000 - th;
			else th = ~th + 0x8000;			// that is, 0xffff - th + 0x8000
		}
	}
	else
	{												// use acos instead
		c = fix_div (x, hyp);
		th = fix_acos (c);
		if (y < 0)
		{
			th = ~th;							// that is, 0xffff - th
		}
	}

// The above (x < 0) and (y < 0) conditionals should take care of placing us in
// the correct quadrant, so we shouldn't need the code below.  Additionally,
// the code below can cause rounding errors when (th & 0x3fff == 0).  So let's
// try omitting it.

#ifdef NO_NEED
	// set high bits based on what quadrant we are in
	th &= 0x3fff;
	th |= (y > 0 ? ( x > 0 ? 0x0000 : 0x4000)
		          : ( x > 0 ? 0xc000 : 0x8000));
#endif

	return th;
}

//----------------------------------------------------------------------------
// fix24_div: Divide two fix24 numbers.
//----------------------------------------------------------------------------
#if defined(powerc) || defined(__powerc)
fix fix24_div(fix24 a, fix24 b)
{
	printf("fix24_div not implemented.\n");
	return a;
}

fix fix24_mul(fix24 a, fix24 b)
{
	printf("fix24_mul not implemented.\n");
	return a;
}

fix fix24_pow(fix24 a, fix24 b)
{
	printf("fix24_pow not implemented.\n");
	return a;
}
#else
fix24 asm fix24_div(fix24 a, fix24 b)
 {
	clr.l		gOVResult
	tst.l		8(sp)
	beq.s		@DivZero

 	moveq		#0,d1
 	move.l	4(sp),d0
 	move.b	4(sp),d1								// get high byte of A in low byte of d1
	ext.w		d1
	ext.l		d1
	lsl.l		#8,d0										// shift rest of A up 8 bits
	dc.w		0x4C6F,0x0C01,0x0008		// 	divs.l	8(sp),d1:d0
	bvs.s		@DivOverflow
 	rts

@DivOverflow:
	move.l	#1,gOVResult
	move.l	#0x7FFFFFFF,d0
	tst.b		4(sp)
	bpl.s		@noNeg
	neg.l		d0
@noNeg:
	rts

@DivZero:
	move.l	#2,gOVResult
	move.l	#0x7FFFFFFF,d0
	tst.b		4(sp)
	bpl.s		@noNeg2
	neg.l		d0
@noNeg2:
	rts
 }

fix24 asm fix24_mul(fix24 a, fix24 b)
 {
 	move.l	4(sp),d0
	dc.w		0x4c2f,0x0c01,0x0008		// 	muls.l	8(sp),d1:d0
	lsr.l		#8,d0						// shift down 8
	ror.l		#8,d1						// get low 8 bits of d1 into high 8 bits
	andi.l	#0xff000000,d1	// mask everything else off
	or.l		d1,d0						// OR result into d0
	rts
 }

#endif


fix fix_pow(fix x,fix y)
{
   int i;
   fix ans;
   fix rh, rl;
   ushort yh,yl;

   ans = FIX_UNIT;
   yh = fix_int(y);
   yl = fix_frac(y);
   rh = rl = x;

   // calculate hi part, leave when done
   for (i=0;i<16;++i) {
      if (yh & 1) ans = fix_mul(ans,rh);
      if (yh!=0) rh = fix_mul(rh,rh);
      yh = yh >> 1;
      if (yl!=0) rl = fix_sqrt(rl);
      if (yl & 0x8000) ans = fix_mul(ans,rl);
      yl = yl << 1;
   }
   return ans;
}

//----------------------------------------------------------------------------
// AsmWideDivide: Divide a 64 bit long by a 32 bit long, return 32 bit result.
//----------------------------------------------------------------------------

#if defined(powerc) || defined(__powerc)
AWide *AsmWideAdd(AWide *target, AWide *source)
{
	*target = (*target + *source);
	return target;
}

AWide *AsmWideSub(AWide *target, AWide *source)
{
	*target = *target - *source;
	return target;
}

AWide *AsmWideMultiply(fix multiplicand, fix multiplier, AWide *target)
{
	float val = fix_float(multiplicand) * fix_float(multiplier);
	*target = val;
	return target;
}

fix AsmWideDivide(AWide src, fix divisor)
{
	float val = fix_float(divisor) / src;
	return fix_from_float(val);
}

AWide *AsmWideNegate(AWide *target)
{
	*target = -(*target);
	return target;
}

AWide *AsmWideBitShift(AWide *src, long shift)
{
	printf("AsmWideBitShift\n");
	*src = ((long)*src) << shift;
	return src;
}

/*AWide *WideSquareRoot(AWide *src)
{
	printf("WideSquareRoot\n");

	float ff = src->hi + src->lo * 0.1;
	sqrtf(ff);

	src->hi = (int)ff;
	src->lo = (ff - (int)ff) * 10;

	return src;
}*/
#else
 
asm AWide *AsmWideSub(AWide *target, AWide *source)
 {
 	move.l	4(sp),a0		// target
 	move.l	8(sp),a1		// source
 	
 	move.l	(a0),d1			// get high bytes
 	move.l	(a1),d2
 	
 	move.l	4(a1),d0		// get low byte
 	sub.l		d0,4(a0)		// target.lo -= source.lo
 	
 	subx.l	d2,d1				// target.hi -= source.hi + X
 	move.l	d1,(a0)			// save it
 	
	rts
 }


asm AWide *AsmWideMultiply(long multiplicand, long multiplier, AWide *target)
 {
 	move.l	4(sp),d0
	dc.w		0x4c2f,0x0c01,0x0008		// 	muls.l	8(sp),d1:d0
 	move.l	12(sp),a0
 	move.l	d1,(a0)
 	move.l	d0,4(a0)
 	rts
 }


asm long AsmWideDivide(long hi, long lo, long divisor)
 {
	clr.l		gOVResult
	move.l	12(sp),d2
	beq.s		@DivZero
	move.l	4(sp),d1
	move.l	8(sp),d0
 	dc.l		0x4C420C01          // DIVS.L    D2,D1:D0
	bvs.s		@DivOverflow
 	rts

@DivOverflow:
	move.l	#1,gOVResult
	move.l	#0x7FFFFFFF,d0
	tst.b		4(sp)
	bpl.s		@noNeg
	neg.l		d0
@noNeg:
	rts

@DivZero:
	move.l	#2,gOVResult
	move.l	#0x7FFFFFFF,d0
	tst.b		4(sp)
	bpl.s		@noNeg2
	neg.l		d0
@noNeg2:
	rts
 }

asm AWide *AsmWideNegate(AWide *target)
 {
 	move.l	4(a7),a0
 	move.l	(a0)+,d0
 	move.l	(a0),d1
 	moveq		#0,d2
 	
 	not.l		d0			// not high reg
 	not.l		d1			// not low reg
 	addq.l	#1,d1
 	addx.l	d2,d0		// +1
 	
 	move.l	d1,(a0)		// save result
 	move.l	d0,-(a0)
 
@Done:
 	rts
 }
 
asm AWide *AsmWideBitShift(AWide *src, long shift)
 {
 	move.l	4(a7),a0
 	move.l	8(a7),d2
 	beq.s		@exit
 	bmi.s		@negshift
 	
 	move.l	(a0),d1		// get high
 	move.l	4(a0),d0	// get low 
	subq.w	#1,d2			// for dbra
	
@Loop:
	asr.l		#1,d1
  roxr.l	#1,d0				// shift low down with carry
	dbra		d2,@Loop
	bra.s		@done
 	
@negshift:
	neg.l		d2
	subq.w	#1,d2				// for dbra
	
@Loop2:
	lsl.l		#1,d0				// shift low up 1
	roxl.l	#1,d1				// shift high up with carry
	dbra		d2,@Loop2

@done:
	move.l	d1,(a0)
	move.l	d0,4(a0)

@exit:
 	rts
 }
 
 
#endif
