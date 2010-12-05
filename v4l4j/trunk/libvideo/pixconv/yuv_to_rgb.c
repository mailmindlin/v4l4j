/*
* Copyright (C) 2010 Gilles Gigan (gilles.gigan@gmail.com)
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public  License as published by the
* Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#include <emmintrin.h>
#include "common.h"
#include "libvideo.h"
#include "log.h"
#include "yuv_to_rgb.h"

#ifdef DEBUG

void print_xmm16(char *name, __m128i *reg) {
	// print register as 16bit signed
	short*	array = (short *)reg;
	printf("%s:\n %4hd %4hd %4hd %4hd %4hd %4hd %4hd %4hd\n", name,
			array[0], array[1], array[2], array[3],
			array[4], array[5], array[6], array[7]);
}

void print_xmm8u(char *name, __m128i *reg) {
	// print register as 8bit signed
	char*	array = (char *)reg;
	printf("%s:\n %4hhu %4hhu %4hhu %4hhu %4hhu %4hhu %4hhu %4hhu "
			"%4hhu %4hhu %4hhu %4hhu %4hhu %4hhu %4hhu %4hhu\n", name,
			array[0], array[1], array[2], array[3],
			array[4], array[5], array[6], array[7],
			array[8], array[9], array[10], array[11],
			array[12], array[13], array[14], array[15]);
}
#else
#define print_xmm8u(...)
#define print_xmm16(...)
#endif



static void unpack_yuyv_to_y_u_v_vectors(__m128i in8Pixels, __m128i* yVect, __m128i* uVect, __m128i* vVect)
{
	__v16qi 	v16i_mask_off_chromas = {0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0};

	// TOTAL LATENCY:	16 clock cycles
	//
	// INPUT:
	//
	// 1 vector of 16 char (in8Pixels)
	// Y1 U12	Y2 V12	Y3 U34	Y4 V34	Y5 U56	Y6 V56	Y7 U78	Y8 V78
	//
	// OUTPUT:
	//
	// 3 vectors of 8 short
	// yVect
	// Y1  0	Y2  0	Y3  0	Y4  0	Y5  0	Y6  0	Y7  0	Y8  0
	//
	// uVect
	// U12 0	U12 0	U34 0	U34 0	U56 0	U56 0	U78 0	U78 0
	//
	// vVect
	// V12 0	V12 0	V34 0	V34 0	V56 0	V56 0	V78 0	V78 0
	//

	// Y unpacking
	*yVect = _mm_and_si128(in8Pixels, (__m128i) v16i_mask_off_chromas);		// PAND				2	2
	// Y1 0		Y2 0	Y3 0	Y4 0	Y5 0	Y6 0	Y7 0	Y8 0

	//
	// Cb unpacking
	__m128i uv_v16i = _mm_srli_epi16(in8Pixels,  8);						// PSRLW			2	2
	// U12 0	V12 0	U34 0	V34 0	U56 0	V56 0	U78 0	V78 0

	__m128i v16i_uLo =  _mm_and_si128(uv_v16i, (__m128i) v16i_mask_off_chromas);// PAND			2	2
	// U12 0	0 0		U34 0	0 0		U56 0	0 0		U78 0	0 0

	__m128i v16i_uHi = _mm_slli_epi32(v16i_uLo, 16);						// PSLLD			2	2
	// 0 0		U12 0	0 0		U34 0	0 0		U56 0	0 0		U78 0

	*uVect = _mm_or_si128(v16i_uLo, v16i_uHi);								// POR				2	2
	// U12 0	U12 0	U34 0	U34 0	U56 0	U56 0	U78 0	U78 0


	//
	// Cr unpacking
	__v16qi v16i_mask_cb = {0, 0, 0xFF, 0, 0, 0, 0xFF, 0, 0, 0, 0xFF, 0,0,0, 0xFF, 0};

	__m128i v16i_vHi =  _mm_and_si128(uv_v16i, (__m128i)v16i_mask_cb);		// PAND				2	2
	// 0 0		V12 0	0 0		V34 0	0 0		V56 0	0 0		V78 0

	__m128i v16i_vLo = _mm_srli_epi32(v16i_vHi, 16);						// PSRLD			2	2
	// V12 0	0 0		V34 0	0 0		V56 0	0 0		V78 0	0 0

	*vVect = _mm_or_si128(v16i_vLo, v16i_vHi);								// POR				2	2
	// V12 0	V12 0	V34 0	V34 0	V56 0	V56 0	V78 0	V78 0
}


static void convert_yuv_vectors_to_rgb_vectors(__m128i v16i_yVect, __m128i v16i_uVect, __m128i v16i_vVect,
		__m128i* v16i_rVect, __m128i* v16i_gVect, __m128i* v16i_bVect)
{
	// Conversion
	//
	// Total latency:
	//
	//	R = 	[ 1		0		1.4		]	( Y )
	//	G = 	[ 1		-0.343	-0.711	]	( U - 128 )
	//	B = 	[ 1		1.765	0		]	( V - 128 )
	//
	//
	//			[ 256	0		358		]	left shift 8
	//			[ 256	-88		-182	]
	//			[ 256	452		0		]
	//
	//
	// INPUT:
	//
	// 3 vectors of 8 short:
	// yVect
	// Y1 0		Y2 0	Y3 0	Y4 0	Y5 0	Y6 0	Y7 0	Y8 0
	//
	// uVect
	// U12 0	U12 0	U34 0	U34 0	U56 0	U56 0	U78 0	U78 0
	//
	// vVect
	// V12 0	V12 0	V34 0	V34 0	V56 0	V56 0	V78 0	V78 0
	//
	// OUTPUT:
	//
	// 3 vectors of 8 short:
	// rVect
	// R1 0		R2 0	R3 0	R4 0	R5 0	R6 0	R7 0	R8 0
	//
	// gVect
	// G1 0		G2 0	G3 0	G4 0	G5 0	G6 0	G7 0	G8 0
	//
	// bVect
	// B1 0		B2 0	B3 0	B4 0	B5 0	B6 0	B7 0	B8 0

#if 0

	// TODO: finish implementing this function
	// U - 128
	__m128i	v16_sub128 = {-128, -128, -128, -128, -128, -128, -128, -128};
	uVect = _mm_add_epi16(uVect,  v16_sub128);									// PADDW		2	2

	// Y * YCoef
	// less clock cycles than multiplication
	yVect = mm_slli_epi16(yVect, 8);											// PSLLW		2	2


	// V - 128
	vVect = _mm_add_epi16(vVect,  v16_sub128);									// PADDW		2	2

	// (V - 128)  * 90
	__m128i	v16i_ =  {90, 90, 90, 90, 90, 90, 90, 90};
	vRVect = _m128i _mm_mullo_epi16(vUnpacked16, (__m128i) vect90); 			// PMULLW		9 8 2 2

	//
	//R = [ 64	0	90 ] [ Y	U	V] = 64 * Y + 90 * V
	rVect = _mm_adds_epi16(yRUnpacked16Multiplied, vRVect);			// PADDW	2	2



	// (U- 128)  * -22
	vectM22
	{-22, ...};
	uGVect = _m128i _mm_mullo_epi16(uUnpacked16, (__m128i) vectM22);	// PMULLW	9 8 2 2

	// (V - 128)  * -45
	vectM45
	{-45,	...};
	vGVect = _m128i _mm_mullo_epi16(vUnpacked16, (__m128i) vectM45);	// PMULLW	9 8 2 2

	//
	//G = [ 64	-22	-45 ][ Y	U	V] = 64 * Y - 22 * U - 45 * V
	gVect = _mm_adds_epi16(yRUnpacked16Multiplied, _mm_adds_epi16(uGVect, vGVect));		// PADDW	2	2



	// (U- 128)  * 113
	vect113
	{113, ...};
	uBVect = _m128i _mm_mullo_epi16(uUnpacked16, (__m128i) vect113);	// PMULLW	9 8 2 2

	//
	//B = [ 64	113	0][ Y		U	V] = 64 * Y + 113 * U
	bVect = _mm_adds_epi16(yRUnpacked16Multiplied, uBVect);			// PADDW	2	2
#endif
}



static void unpack_yuyv_to_y_uv_vectors(__m128i* in8Pixels, __m128i* yVect, __m128i* uvVect)
{
	__v16qi 	v16i_mask_off_chromas = {0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0};

	// TOTAL LATENCY:			4 clock cycles
	// Num of pixel handled:	8
	//
	// INPUT:
	//
	// 1 vector of 16 char (in8Pixels)
	// Y1 U12	Y2 V12	Y3 U34	Y4 V34	Y5 U56	Y6 V56	Y7 U78	Y8 V78
	//
	// OUTPUT:
	//
	// 2 vectors of 8 short
	// yVect
	// Y1  0	Y2  0	Y3  0	Y4  0	Y5  0	Y6  0	Y7  0	Y8  0
	//
	// uVect
	// U12 0	V12 0	U34 0	V34 0	U56 0	V56 0	U78 0	V78 0

	// Y unpacking
	*yVect = _mm_and_si128(*in8Pixels, (__m128i) v16i_mask_off_chromas);	// PAND				2	2
	// Y1 0		Y2 0	Y3 0	Y4 0	Y5 0	Y6 0	Y7 0	Y8 0

	//
	// CbCr unpacking
	*uvVect = _mm_srli_epi16(*in8Pixels,  8);								// PSRLW			2	2
	// U12 0	V12 0	U34 0	V34 0	U56 0	V56 0	U78 0	V78 0
}


static void convert_y_uv_vectors_to_rgb_vectors(__m128i* v8i_yVect, __m128i* v8i_uvVect,
		__m128i* v8i_rVect, __m128i* v8i_gVect, __m128i* v8i_bVect)
{
	// Conversion
	//
	// Total latency: 			53 cycles
	// Num of pixel handled:	8
	//
	//	R = 	[ 1		0		1.4		]	( Y )
	//	G = 	[ 1		-0.343	-0.711	]	( U - 128 )
	//	B = 	[ 1		1.765	0		]	( V - 128 )
	//
	//			[ 256	0		358		]	left shift 8
	//			[ 256	-88		-182	]
	//			[ 256	452		0		]
	//
	//
	// INPUT:
	//
	// 2 vectors of 8 short:
	// yVect
	// Y1 0		Y2 0	Y3 0	Y4 0	Y5 0	Y6 0	Y7 0	Y8 0
	//
	// uvVect
	// U12 0	V12 0	U34 0	V34 0	U56 0	V56 0	U78 0	V78 0
	//
	// OUTPUT:
	//
	// 3 vectors of 8 short:
	// rVect
	// R1 0		R2 0	R3 0	R4 0	R5 0	R6 0	R7 0	R8 0
	//
	// gVect
	// G1 0		G2 0	G3 0	G4 0	G5 0	G6 0	G7 0	G8 0
	//
	// bVect
	// B1 0		B2 0	B3 0	B4 0	B5 0	B6 0	B7 0	B8 0


	// U - 128
	__v8hi	v8i_sub128 = {-128, -128, -128, -128, -128, -128, -128, -128};
	*v8i_uvVect = _mm_add_epi16(*v8i_uvVect,  (__m128i)v8i_sub128);	// PADDW		2	2


	//
	// R
	// U and V coefficients
	// 0, 358, 0, 358, 0, 358, 0, 358
	__m128i	v8i_uvRCoeffs = {0x0166000001660000LL, 0x0166000001660000LL};
	v8i_uvRCoeffs = _mm_madd_epi16(*v8i_uvVect, v8i_uvRCoeffs);		// PMADDWD		9 8 2 2
	// U12*0 + V12*358	U34*0 + V34*358	U56*0 + V56*358	U78*0 + V78*358
	// C12		C34		C56		C78			(4 * 32-bits values)
	// Sb Sb A B 		upper 16 bits are always the sign bit due to the coeffs and pixels values
	//					max value with 8bit left shift of coeffs:
	//					452*127 = 57404 (16 bit wide)
	//					min value:
	//					452*-128= 2's(57856) (16 bit wide)

	// shift right by 8 to account for left shift by 8 of coefficients
	__m128i rightShift = _mm_srai_epi32(v8i_uvRCoeffs, 8);			// PSRAD		2	2
	// C12 0 0 0		C34 0 0 0		C56 0 0 0		C78 0 0 0

	rightShift = _mm_shufflehi_epi16(rightShift, 0xA0);				// PSHUFHW		2	2
	// C12 0 0 0		C34 0 0 0		C56 0 C56 0		C78 0 C78 0

	rightShift = _mm_shufflelo_epi16(rightShift, 0xA0);				// PSHUFLW		2	2
	// C12 0 C12 0		C34 0 C12 0		C56 0 C56 0		C78 0 C78 0

	*v8i_rVect = _mm_add_epi16(*v8i_yVect,  rightShift);			// PADDW		2	2
	//


	//
	// G
	// U and V coeffs ()
	// -88, -182, -88, -182, -88, -182, -88, -182
	__m128i v8i_uvGCoeffs =  {0xFF4AFFA8FF4AFF4ALL, 0xFF4AFFA8FF4AFFA8LL};
	v8i_uvGCoeffs = _mm_madd_epi16(*v8i_uvVect, v8i_uvGCoeffs);		// PMADDWD		9 8 2 2
	// U12*-88 + V12*-182	U34*-88 + V34*-182	U56*-88 + V56*-182	U78*-88 + V78*-182
	// 0 C12		0 C34		0 C56		0 C78

	// shift right by 8
	rightShift = _mm_srai_epi32(v8i_uvGCoeffs, 8);					// PSRAD		2	2
	// C12	0		C34	 0		C56 0		C78	0

	rightShift = _mm_shufflehi_epi16(rightShift, 0xA0);				// PSHUFHW		2	2
	// C12 0 0 0		C34 0 0 0		C56 0 C56 0		C78 0 C78 0

	rightShift = _mm_shufflelo_epi16(rightShift, 0xA0);				// PSHUFLW		2	2
	// C12 0 C12 0		C34 0 C12 0		C56 0 C56 0		C78 0 C78 0

	*v8i_gVect = _mm_add_epi16(*v8i_yVect,  rightShift);			// PADDW		2	2
	//

	//
	// B
	// U and V coeffs ()
	// 0, 452, 0, 452, 0, 452, 0, 452
	__m128i v8i_uvBCoeffs =  {0x000001C4000001C4LL, 0x000001C4000001C4LL};
	v8i_uvBCoeffs = _mm_madd_epi16(*v8i_uvVect, v8i_uvBCoeffs);		// PMADDWD		9 8 2 2
	// U12*0 + V12*452	U34*0 + V34*452	U56*0 + V56*452	U78*0 + V78*452
	// 0 C12		0 C34		0 C56		0 C78

	// shift right by 8
	rightShift = _mm_srai_epi32(v8i_uvBCoeffs, 8);					// PSRAD		2	2
	// C12	0		C34	 0		C56	 0		C78	 0

	rightShift = _mm_shufflehi_epi16(rightShift, 0xA0);				// PSHUFHW		2	2
	// C12 0 0 0		C34 0 0 0		C56 0 C56 0		C78 0 C78 0

	rightShift = _mm_shufflelo_epi16(rightShift, 0xA0);				// PSHUFLW		2	2
	// C12 0 C12 0		C34 0 C12 0		C56 0 C56 0		C78 0 C78 0

	*v8i_bVect = _mm_add_epi16(*v8i_yVect,  rightShift);			// PADDW		2	2
	//
}

static void pack_6_rgb_vectors_in_4_bgra_vectors(__m128i* r1, __m128i* g1, __m128i* b1,
		__m128i* r2, __m128i* g2, __m128i* b2,
		__m128i* rgba1, __m128i* rgba2,
		__m128i* rgba3, __m128i* rgba4) {

	// Pack R1, G1, B1 and R2, G2, B2 vectors of signed short
	// in 4 BGRA vectors
	//
	// Total latency:			34 cycles
	// Num of pixel handled:	16
	//
	// INPUT:
	//
	// 6 vectors of 8 short:
	// rVect
	// R1 0		R2 0	R3 0	R4 0	R5 0	R6 0	R7 0	R8 0
	// R9 0		R10 0	R11 0	R12 0	R13 0	R14 0	R15 0	R16 0
	//
	// gVect
	// G1 0		G2 0	G3 0	G4 0	G5 0	G6 0	G7 0	G8 0
	// G9 0		G10 0	G11 0	G12 0	G13 0	G14 0	G15 0	G16 0
	//
	// bVect
	// B1 0		B2 0	B3 0	B4 0	B5 0	B6 0	B7 0	B8 0
	// B9 0		B10 0	B11 0	B12 0	B13 0	B14 0	B15 0	B16 0
	//
	//
	// OUTPUT
	//
	// B1 G1	R1 A1	B2 G2	R2 A2	B3 G3	R3 A3	B4 G4	R4 A4
	// B5 G5	R5 A5	B6 G6	R6 A6	B7 G7	R7 A7	B8 G8	R8 A8
	// B9 G9	R9 A9	B10 G10	R10 A10	B11 G11	R11 A11	B12 G12	R12 A12
	// B13 G13	R13 A13	B14 G14	R14 A14	B15 G15	R15 A15	B16 G16	R16 A16
	//

	__m128i zero = _mm_setzero_si128();

	__m128i rPacked = _mm_packus_epi16(*r1, *r2);						// PACKUSWB		4 4 2 2
	// R1 R2	R3 R4	R5 R6	R7 R8	R9 R10	R11 R12	R13 R14	R15 R16

	__m128i gPacked = _mm_packus_epi16(*g1, *g2);						// PACKUSWB		4 4 2 2
	// G1 G2	G3 G4	G5 G6	G7 G8	G9 G10	G11 G12	G13 G14	G15 G16

	__m128i bPacked = _mm_packus_epi16(*b1, *b2);						// PACKUSWB		4 4 2 2
	// B1 B2	B3 B4	B5 B6	B7 B8	B9 B10	B11 B12	B13 B14	B15 B16


	// unpacking lows
	__m128i rg = _mm_unpacklo_epi8(bPacked, gPacked);					// PUNPCKLBW	2	2
	// B1 G1	B2 G2	B3 G3	B4 G4	B5 G5	B6 G6	B7 G7	B8 G8

	__m128i ba = _mm_unpacklo_epi8(rPacked, zero);						// PUNPCKLBW	2	2
	// R1 A1	R2 A2	R3 A3	R4 A4	R5 A5	R6 A6	R7 A7	R8 A8

	*rgba1 = _mm_unpacklo_epi16(rg, ba);								// PUNPCKLWD	2	2
	// B1 G1	R1 A1	B2 G2	R2 A2	B3 G3	R3 A3	B4 G4	R4 A4

	*rgba2 = _mm_unpackhi_epi16(rg, ba);								// PUNPCKHWD	4	2
	// B5 G5	R5 A5	B6 G6	R6 A6	B7 G7	R7 A7	B8 G8	R8 A8

	// unpacking highs
	rg = _mm_unpackhi_epi8(bPacked, gPacked);							// PUNPCKHBW	4	2
	// B9 G9	B10 G10	B11 G11	B12 G12 B13 G13	B14 G14	B15 G15	B16 G16

	ba = _mm_unpacklo_epi8(rPacked, zero);								// PUNPCKLBW	2	2
	// R9 A9	R10 A10	R11 A11	R12 A12	R13 A13	R14 A14	R15 A15	R16 A16

	*rgba3 = _mm_unpacklo_epi16(rg, ba);								// PUNPCKLWD	2	2
	// B9 G9	R9 A9	B10 G10	R10 A10	B11 G11	R11 A11	B12 G12	R12 A12

	*rgba4 = _mm_unpackhi_epi16(rg, ba);								// PUNPCKHWD	4	2
	// B13 G13	R13 A13	B14 G14	R14 A14	B15 G15	R15 A15	B16 G16	R16 A16
}

static void pack_6_rgb_vectors_in_4_argb_vectors(__m128i* r1, __m128i* g1, __m128i* b1,
		__m128i* r2, __m128i* g2, __m128i* b2,
		__m128i* rgba1, __m128i* rgba2,
		__m128i* rgba3, __m128i* rgba4) {

	// Pack R1, G1, B1 and R2, G2, B2 vectors of signed short
	// in 4 ARGB vectors
	//
	// Total latency:			34 cycles
	// Num of pixel handled:	16
	//
	// INPUT:
	//
	// 6 vectors of 8 short:
	// rVect
	// R1 0		R2 0	R3 0	R4 0	R5 0	R6 0	R7 0	R8 0
	// R9 0		R10 0	R11 0	R12 0	R13 0	R14 0	R15 0	R16 0
	//
	// gVect
	// G1 0		G2 0	G3 0	G4 0	G5 0	G6 0	G7 0	G8 0
	// G9 0		G10 0	G11 0	G12 0	G13 0	G14 0	G15 0	G16 0
	//
	// bVect
	// B1 0		B2 0	B3 0	B4 0	B5 0	B6 0	B7 0	B8 0
	// B9 0		B10 0	B11 0	B12 0	B13 0	B14 0	B15 0	B16 0
	//
	//
	// OUTPUT
	//
	// A1 R1	G1 B1	A2 R2	G2 B2	A3 R3	G3 B3	A4 R4	G4 B4
	// A5 R5	G5 B5	A6 R6	G6 B6	A7 R7	G7 B7	A8 R8	G8 B8
	// A9 R9	G9 B9	A10 R10	G10 B10	A11 R11	G11 B11	A12 R12	G12 B12
	// A13 R13	G13 B13	A14 R14	G14 B14	A15 R15	G15 B15	A16 R16	G16 B16
	//

	__m128i zero = _mm_setzero_si128();

	__m128i rPacked = _mm_packus_epi16(*r1, *r2);						// PACKUSWB		4 4 2 2
	// R1 R2	R3 R4	R5 R6	R7 R8	R9 R10	R11 R12	R13 R14	R15 R16

	__m128i gPacked = _mm_packus_epi16(*g1, *g2);						// PACKUSWB		4 4 2 2
	// G1 G2	G3 G4	G5 G6	G7 G8	G9 G10	G11 G12	G13 G14	G15 G16

	__m128i bPacked = _mm_packus_epi16(*b1, *b2);						// PACKUSWB		4 4 2 2
	// B1 B2	B3 B4	B5 B6	B7 B8	B9 B10	B11 B12	B13 B14	B15 B16


	// unpacking lows
	__m128i rg = _mm_unpacklo_epi8(zero, rPacked);						// PUNPCKLBW	2	2
	// A1 R1	A2 R2	A3 R3	A4 R4	A5 R5	A6 R6	A7 R7	A8 R8

	__m128i ba = _mm_unpacklo_epi8(gPacked, bPacked);					// PUNPCKLBW	2	2
	// G1 B1	G2 B2	G3 B3	G4 B4	G5 B5	G6 B6	G7 B7	G8 B8

	*rgba1 = _mm_unpacklo_epi16(rg, ba);								// PUNPCKLWD	2	2
	// A1 R1	G1 B1	A2 R2	G2 B2	A3 R3	G3 B3	A4 R4	G4 B4

	*rgba2 = _mm_unpackhi_epi16(rg, ba);								// PUNPCKHWD	4	2
	// A5 R5	G5 B5	A6 R6	G6 B6	A7 R7	G7 B7	A8 R8	G8 B8

	// unpacking highs
	rg = _mm_unpackhi_epi8(zero, rPacked);								// PUNPCKHBW	4	2
	// A9 R9	A10 R10	A11 R11	A12 R12	A13 R13	A14 R14	A15 R15	A16 R16

	ba = _mm_unpacklo_epi8(gPacked, bPacked);							// PUNPCKLBW	2	2
	// G9 B9	G10 B10	G11 B11	G12 B12	G13 B13	G14 B14	G15 B15	G16 B16

	*rgba3 = _mm_unpacklo_epi16(rg, ba);								// PUNPCKLWD	2	2
	// A9 R9	G9 B9	A10 R10	G10 B10	A11 R11	G11 B11	A12 R12 G12 B12

	*rgba4 = _mm_unpackhi_epi16(rg, ba);								// PUNPCKHWD	4	2
	// A13 R13	G13 B13	A14 R14 G14 B14	A15 R15	G15 B15	A16 R16	G16 B16
}


static void convert_16pixels_yuyv(__m128i* yuyv_16pixels, __m128i*  rgba_16pixels, int dest_fmt) {

	__m128i*		yuyv_8pixels = yuyv_16pixels;
	__m128i* 		rgba1 = rgba_16pixels++;
	__m128i* 		rgba2 = rgba_16pixels++;
	__m128i* 		rgba3 = rgba_16pixels++;
	__m128i* 		rgba4 = rgba_16pixels;

	__m128i			v16i_yVect, v16i_uvVect;
	__m128i			v8i_rVect1, v8i_gVect1, v8i_bVect1;
	__m128i			v8i_rVect2, v8i_gVect2, v8i_bVect2;

	// unpack first 8 pixels
	unpack_yuyv_to_y_uv_vectors(yuyv_8pixels, &v16i_yVect, &v16i_uvVect);

	// convert first 8 pixels
	convert_y_uv_vectors_to_rgb_vectors(&v16i_yVect, &v16i_uvVect,
			&v8i_rVect1, &v8i_gVect1, &v8i_bVect1);

	// unpack next 8 pixels
	yuyv_8pixels++;
	unpack_yuyv_to_y_uv_vectors(yuyv_8pixels, &v16i_yVect, &v16i_uvVect);

	// convert next 8 pixels
	convert_y_uv_vectors_to_rgb_vectors(&v16i_yVect, &v16i_uvVect,
			&v8i_rVect2, &v8i_gVect2, &v8i_bVect2);

	// pack both sets of 8 pixels
	if (dest_fmt == RGB32)
		pack_6_rgb_vectors_in_4_argb_vectors(&v8i_rVect1, &v8i_gVect1, &v8i_bVect1,
			&v8i_rVect2, &v8i_gVect2, &v8i_bVect2,
			rgba1, rgba2, rgba3, rgba4);
	else if (dest_fmt = BGR32)
		pack_6_rgb_vectors_in_4_bgra_vectors(&v8i_rVect1, &v8i_gVect1, &v8i_bVect1,
					&v8i_rVect2, &v8i_gVect2, &v8i_bVect2,
					rgba1, rgba2, rgba3, rgba4);
	else
		dprint(LIBVIDEO_SOURCE_PIXCONV, LIBVIDEO_LOG_ERROR,
							"[PXC] unknown dest format: %d\n", dest_fmt);
}

static inline int convert_yuyv_to_rgb32_sse2(struct pixconv* conv, unsigned char *src, unsigned char *dst){
	int		 		pixel_count = conv->pixel_count;
	__m128i*		yuyv_16pixels = (__m128i *)src;
	__m128i*		rgba_16pixels = (__m128i *)dst;

	while(pixel_count > 0) {
		convert_16pixels_yuyv(yuyv_16pixels, rgba_16pixels, conv->dest_fmt);

		// each iteration converts 16 pixels
		yuyv_16pixels += 2;	// 8 pixels per line
		rgba_16pixels += 4; // 4 pixels per line
		pixel_count -= 16;
	}

	return 0;
}


static inline int convert_yuyv_to_rgb24_sse2(struct pixconv* conv, unsigned char *src, unsigned char *dst){
	int		 		pixel_count = conv->pixel_count;
	__m128i*		yuyv_16pixels = (__m128i *)src;
	__m128i			rgba_16pixels[4];
	__m128i 		mask = {0x0080808000808080LL, 0x0080808000808080LL};

	while(pixel_count > 0) {
		convert_16pixels_yuyv(yuyv_16pixels, rgba_16pixels, RGB32);

		// copy only RGB bytes to destination buffer
		_mm_maskmoveu_si128(rgba_16pixels[0], mask, (char *) dst);
		_mm_maskmoveu_si128(rgba_16pixels[1], mask, (char *) (dst + 12));
		_mm_maskmoveu_si128(rgba_16pixels[2], mask, (char *) (dst + 24));
		_mm_maskmoveu_si128(rgba_16pixels[3], mask, (char *) (dst + 36));


		// each iteration converts 16 pixels
		yuyv_16pixels += 2;	// 8 pixels per line
		dst += 48;			// 16 pixels = 48 bytes;
		pixel_count -= 16;
	}

	return 0;
}


static int convert_yuyv_to_rgb32_nonsse(struct pixconv* conv, unsigned char *src, unsigned char *dst){
	int a=0, i=0, size = conv->pixel_count;
	int r, g, b;
	int y, u, v;

	while(i++<size){
		if (!a)
			y = src[0] << 8;
		else
			y = src[2] << 8;
		u = src[1] - 128;
		v = src[3] - 128;

		r = (y + (359 * v)) >> 8;
		g = (y - (88 * u) - (183 * v)) >> 8;
		b = (y + (454 * u)) >> 8;

		*(dst++) = CLIP_PIXEL(r);
		*(dst++) = CLIP_PIXEL(g);
		*(dst++) = CLIP_PIXEL(b);
		*(dst++) = 0;

		if (a++) {
			a = 0;
			src += 4;
		}
	}

	return 0;
}


static int convert_yuyv_to_rgb24_nonsse(struct pixconv* conv, unsigned char *src, unsigned char *dst){
	int a=0, i=0, size = conv->pixel_count;
	int r, g, b;
	int y, u, v;

	while(i++<size){
		if (!a)
			y = src[0] << 8;
		else
			y = src[2] << 8;
		u = src[1] - 128;
		v = src[3] - 128;

		r = (y + (359 * v)) >> 8;
		g = (y - (88 * u) - (183 * v)) >> 8;
		b = (y + (454 * u)) >> 8;

		*(dst++) = CLIP_PIXEL(r);
		*(dst++) = CLIP_PIXEL(g);
		*(dst++) = CLIP_PIXEL(b);

		if (a++) {
			a = 0;
			src += 4;
		}
	}

	return 0;
}


int check_conversion_yuyv_to_rgb32(struct pixconv* conv) {
	int result = 1;
	support_sse2(conv->use_sse2);	// check if sse2 is available

	if ((conv->source_fmt == YUYV) && (conv->dest_fmt == RGB32)) {
		dprint(LIBVIDEO_SOURCE_PIXCONV, LIBVIDEO_LOG_DEBUG,
							"[PXC] Found supported YUYV-RGB32 conversion\n");

		// if SSE2 is available and the number of pixels is multiple of 16,
		// use the sse routine. This routine requires the number of pixels
		// to be multiple of 16
		if ((conv->use_sse2 != 0) &&  (((conv->w * conv->h) & 0x0F) == 0)) {
			dprint(LIBVIDEO_SOURCE_PIXCONV, LIBVIDEO_LOG_DEBUG,
											"[PXC] using SSE\n");
			conv->convert = convert_yuyv_to_rgb32_sse2;
		} else {
			dprint(LIBVIDEO_SOURCE_PIXCONV, LIBVIDEO_LOG_DEBUG,
								"[PXC] NOT using SSE (features: %d - align: %d)\n",
								conv->use_sse2, ((conv->w*conv->h) & 0x0F));
			conv->convert = convert_yuyv_to_rgb32_nonsse;
		}
	} else if ((conv->source_fmt == YUYV) && (conv->dest_fmt == RGB24)) {
		dprint(LIBVIDEO_SOURCE_PIXCONV, LIBVIDEO_LOG_DEBUG,
							"[PXC] Found supported YUYV-RGB24 conversion\n");

		// if SSE2 is available and the number of pixels is multiple of 16,
		// use the sse routine. This routine requires the number of pixels
		// to be multiple of 16
		if ((conv->use_sse2 != 0) &&  (((conv->w * conv->h) & 0x0F) == 0)) {
			dprint(LIBVIDEO_SOURCE_PIXCONV, LIBVIDEO_LOG_DEBUG,
											"[PXC] using SSE\n");
			conv->convert = convert_yuyv_to_rgb24_sse2;
		} else {
			dprint(LIBVIDEO_SOURCE_PIXCONV, LIBVIDEO_LOG_DEBUG,
								"[PXC] NOT using SSE (features: %d - align: %d)\n",
								conv->use_sse2, ((conv->w*conv->h) & 0x0F));
			conv->convert = convert_yuyv_to_rgb24_nonsse;
		}

		return 1;
	} else
		result = 0;	// not supported

	return result;
}

