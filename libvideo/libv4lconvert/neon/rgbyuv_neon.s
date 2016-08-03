@ RGB <-> YUV conversion routines, optimized for ARM NEON
@ (C) 2016 mailmindlin
@
@ This program is distributed in the hope that it will be useful,
@ but WITHOUT ANY WARRANTY; without even the implied warranty of
@ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@ GNU Lesser General Public License for more details.

.text
.align 4

.globl v4lconvert_neon_load_yuv_values
.globl v4lconvert_neon_yuyv_to_rgb24
@ r0: *src
@ r1: *dest
@ r2: width (in macropixels [2 horizontal pixels])
@ r3: height (in pixels)
v4lconvert_neon_yuyv_to_rgb24:
	umull			r2, r3, r2, r3		@[r3:r2] = width * height
	push {r4-r6, lr}						@Push lr
	
	ands			r4, r2, #7			@Basically length % 8 (TODO maybe optimize for out-of-order exec.)
	beq			.convert_yuyv_to_rgb24_x1e		@If r4 == 0, skip the whole thing
	push			{r2, r3}			@Store r2 and r3 (the length)
	mov				r2, r4				@Move r4 to r6, so {r2 - r5} can be used for processing
	
	.convert_yuyv_to_rgb24_x1:			@Loop to convert 1 macropixel at a time (slow)
		ldrd r2, [r0], #4				@Load 1 macropixel (4 bytes)
		@and r4, r2, #0xFFFF				@Get r2 & (2^16 - 1), so it contains Y2 and V
		lsr r2, r2, #16					@Shift r2>>16, so it contains Y1 and U
		and r5, r4, #255				@Get U
		and r3, r2, #255				@Get V
		
		mov r4, #128
		qsub r5, r5, r4
		qsub r3, r3, r4
		
		and r4, r4, #8					@Get Y2
		lsr r2, r2, #8					@Get Y1
		
		@TODO finish
		
		add			r0, r0, #4
		add			r1, r1, #6
		
		subs		r6, r6, #1
		bne		.convert_yuyv_to_rgb24_x1
		pop {r2, r3}
	.convert_yuyv_to_rgb24_x1e:
	
	@Emulate 'lsr [r3:r2], [r3:r2], #2'
	and r4, r3, #7						@Move lower three bits from r3 to r4 [r4 = r3 & 0b111]
	lsr r2, r2, #3						@Shift r2 to the right by 3 [r2 = r2 >>> 3]
	ror r4, r4, #3						@Rotate r4 to the right by 3 (equivalent to [r4 = r4 << 29])
	lsr r3, r3, #3						@Shift r3 to the right by 3 [r3 = r3 >>> 3]
	orr r2, r2, r4						@Set high 3 bits in r2 to the original 3 low bits of r3
	
	@ Initialize the q11-q15 with constants
	mov r4, 		#128
	vdup.16			q15, r4					@Fill q15 (d30, d31) with 128 (uint16)
	
	vld4.8		{d0 - d3}, [r0]!			@Preload 8 pixels (q0, q1) for first loop
	
	@ See http://www.fourcc.org/yuv2ppm.c for the coefficients
	movw			r5, #29507				@Load 1.37075 (Vr high 16 bits) into r5 for staging
	movw			r4, #45106				@Load 0.698001 (Vg high 16 bits) into r4 for staging
	movt			r5, #16303				@Load 1.37075 (Vr low 16 bits) into r5 for staging
	movt			r4, #16178				@Load 0.698001 (Vg low 16 bits) into r4 for staging
	vdup.32			q11, r5					@Fill q11 with 1.370705 ('v' coefficient for red, or Vr)
	movw			r6, #56892				@Load 0.337633 (Ug high 16 bits) into r6 for staging
	movw			r5, #49354				@Load 1.732446 (Ub high 16 bits) into r5 for staging
	movt			r6, #16044				@Load 0.337633 (Ug low 16 bits) into r6 for staging
	movt			r5, #16349				@Load 1.732446 (Ub low 16 bits) into r5 for staging
	vdup.32			q12, r4					@Fill q12 with 0.698001 ('v' coefficient for green, or Vg)
	vdup.32			q13, r6					@Fill q13 with 0.337633 ('u' coefficient for green, or Ug)
	vdup.32			q14, r5					@Fill q14 with 1.732446 ('u' coefficient for blue, or Ub)
	
	
	@ Convert 8 macropixels at a time, or 16 real pixels
	.convert_yuyv_to_rgb24_x8:
		vmovl.u8	q2, d1					@Convert uint8 to int16 ('u' bytes)
		vmovl.u8	q4, d3					@Convert uint8 to int16 ('v' bytes) (q3 will be filled with u 5-8 later)
		vmovl.u8	q9, d0					@Convert uint8 to int16 ('y1' bytes)
		
		vqsub.s16	q2, q2, q15				@Subtract 128 from the 'u' bytes
		vqsub.s16	q4, q4, q15				@Subtract 128 from the 'v' bytes
		
		vmovl.u8	q10, d2					@Convert uint8 to int16 ('y2' bytes)
		
		vld4.8		{d0 - d3}, [r0]!		@Preload 8 pixels (q0, q1) for next loop
		
		vmovl.s16	q3, d5					@Convert the int16 to int32 (the 'u' 5 - 8)
		vmovl.s16	q5, d9					@Convert the int16 to int32 (the 'v' 5 - 8)
		vmovl.s16	q2, d4					@Convert the int16 to int32 (the 'u' 1 - 4)
		vmovl.s16	q4, d8					@Convert the int16 to int32 (the 'v' 1 - 4)
		
		vcvt.f32.s32 q2, q2					@Convert the int32 to float32 (the 'u' 1 - 4)
		vcvt.f32.s32 q3, q3					@Convert the int32 to float32 (the 'u' 5 - 8)
		vcvt.f32.s32 q4, q4					@Convert the int32 to float32 (the 'v' 1 - 4)
		vcvt.f32.s32 q5, q5					@Convert the int32 to float32 (the 'v' 5 - 8)
		
		
		@ Calculate green (it takes both u and v, so calculating it first will make the others easier,
		@ Because of less register pressure).
		vmul.f32	q6, q2, q13				@Calculate the 'u' part of green (1 - 4)
		vmul.f32	q7, q4, q12				@Calculate the 'v' part of green (1 - 4)
		vmul.f32	q8, q3, q13				@Calculate the 'u' part of green (5 - 8)
		vadd.f32	q6, q7, q6				@Add the 'u' and 'v' part of green together (1 - 4)
		vmul.f32	q7, q5, q12				@Calculate the 'v' part of green (5 - 8)
		
		vcvt.s32.f32 q6, q6					@Convert the float32 to int32 (1 - 4)
		
		vadd.f32	q7, q8, q7				@Add the 'u' and 'v' part of green together (5 - 8)
		vqmovn.s32	d12, q6					@Convert the int32 to int16 (1 - 4) (d12 is q6 low)
		vcvt.s32.f32 q7, q7					@Convert the float32 to int32 (5 - 8)
		vqmovn.s32	d13, q7					@Convert the int32 to int16 (5 - 8) (d13 is q6 high)
		@ NOTE: q6 is the green offset from y (1 - 8)
		
		
		@ Calculate red (it just takes v, so it should not take up any more registers)
		vmul.f32	q4, q4, q11				@Multiply 'v' by its green weight (1 - 4)
		vmul.f32	q5, q5, q11				@Multiply 'v' by its green weight (5 - 8)
		vcvt.s32.f32 q4, q4					@Convert float32 to int32 (5 - 8)
		vcvt.s32.f32 q5, q5					@Convert float32 to int32 (1 - 4)
		vqmovn.s32	d8, q4					@Convert int32 to int16 (1 - 4) (d8 is q4 low)
		vqmovn.s32	d9, q5					@Convert int32 to int16 (5 - 8) (d9 is q4 high)
		@NOTE: q4 is the red offset from y (1 - 8)
		
		
		@ Calculate blue (it just takes u)
		vmul.f32	q2, q2, q14				@Multiply 'u' by its blue weight (1 - 4)
		vmul.f32	q3, q3, q14				@Multiply 'u' by its blue weight (5 - 8)
		vcvt.s32.f32 q2, q2					@Convert float32 to int32 (1 - 4)
		vcvt.s32.f32 q3, q3					@Convert float32 to int32 (5 - 8)
		vqmovn.s32	d4, q2					@Convert int32 to int16 (1 - 4) (d4 is q2 low)
		vqmovn.s32	d5, q3					@Convert int32 to int16 (5 - 8) (d5 is q2 high)
		@NOTE: q2 is the blue offset from y (1 - 8)
		
		@The NEON queue starts to fill up here, so let's give the CPU something to do, by calculating whether it should loop around again.'
		subs		r2, r2, #1				@Subtract 1 from the length
		subcss		r3, r3, #1				@r2 underflowed, so subtract 1 from r3
		
		@ Calculate RGB values
		vqadd.s16	q5, q9, q4				@Red 1	= 'y1' + 'r'
		vqsub.s16	q7, q9, q6				@Green 1= 'y1' + 'g'
		vqsub.s16	q8, q10, q6				@Green 2= 'y2' - 'g'
		vqadd.s16	q6, q10, q4				@Red 2	= 'y2' + 'r'
		vqadd.s16	q3, q9, q2				@Blue 1	= 'y1' + 'b'
		vqadd.s16	q4, q10, q2				@Blue 2	= 'y2' + 'b'
		
		@ Convert RGB to u8 (and move the registers to the right places)
		vqmovun.s16	d4, q5					@Convert the int16 to uint8 (the 'r1' bytes)
		vqmovun.s16	d7, q6					@Convert the int16 to uint8 (the 'r2' bytes)
		vqmovun.s16	d5, q7					@Convert the int16 to uint8 (the 'g1' bytes)
		vqmovun.s16	d8, q8					@Convert the int16 to uint8 (the 'g2' bytes)
		vqmovun.s16	d6, q3					@Convert the int16 to uint8 (the 'b1' bytes)
		vqmovun.s16	d9, q4					@Convert the int16 to uint8 (the 'b2' bytes)
		
		@ Interleave RGB (TODO make sure that this is right, I think that the operands might need to be switched for vzip)
		vzip.8		d4, d7					@Interleave 'r1' & 'r2'
		vzip.8		d5, d8					@Interleave 'g1' & 'g2'
		vzip.8		d6, d9					@Interleave 'b1' & 'b2'
		
		vst3.8		{d4 - d6}, [r1]!		@Store RGB (1 - 6)
		vst3.8		{d7 - d9}, [r1]!		@Store RGB (6 - 12)
		
		blo 	.convert_yuyv_to_rgb24_x8	@Loop around again for another batch of pixels (C flag was set already)
	
	pop {r4-r6, pc}							@Return
