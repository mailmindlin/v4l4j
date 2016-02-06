.text
.align 4

.globl v4lconvert_neon_yuyv_to_rgb24
@ r0: *src
@ r1: *dest
@ r2: len (in sets of 8 pixels)
v4lconvert_neon_yuyv_to_rgb24:
	push {ip,lr}							@Push lr
	
	@ Initialize the subtraction register with the value 128
	mov r3, 		#128
	vdup.16			q15, r3					@Fill q15 (d30, d31) with 128 (uint16)
	
	@ See http://www.fourcc.org/yuv2ppm.c for the coefficients
	movw	r3, #29507						@Load 1.37075 (high 16 bits) into r3 for staging
	movt	r3, #16303						@Load 1.37075 (low 16 bits) into r3 for staging
	vdup.32			q11, r3					@Fill q11 with 1.370705 ('v' coefficient for red)
	movw	r3, #45106						@Load 0.698001 (high 16 bits) into r3 for staging
	movt	r3, #16178						@Load 0.698001 (low 16 bits) into r3 for staging
	vdup.32			q12, r3					@Fill q12 with 0.698001 ('v' coefficient for green)
	movw	r3, #56892						@Load 0.337633 (high 16 bits) into r3 for staging
	movt	r3, #16044						@Load 0.337633 (low 16 bits) into r3 for staging
	vdup.32			q13, r3					@Fill q13 with 0.337633 ('u' coefficient for green)
	movw	r3, #49354						@Load 1.732446 (high 16 bits) into r3 for staging
	movt	r3, #16349						@Load 1.732446 (low 16 bits) into r3 for staging
	vdup.32			q14, r3					@Fill q14 with 1.732446 ('u' coefficient for blue)
	
	.loop
		vld4.8		{d0 - d3}, [r0]!		@Load 8 pixels (q0, q1)
		
		vmovl.u8	q2, d1					@Convert uint8 to int16 ('u' bytes)
		vmovl.u8	q0, d0					@Convert uint8 to int16 ('y1' bytes)
		vmovl.u8	q4, d3					@Convert uint8 to int16 ('v' bytes) (q3 will be filled with u 5-8 later)
		vmovl.u8	q1, d2					@Convert uint8 to int16 ('y2' bytes)
		
		vqsub.s16	q2, q2, q15				@Subtract 128 from the 'u' bytes
		vqsub.s16	q4, q4, q15				@Subtract 128 from the 'v' bytes
		
		vmovl.s16	q3, d5					@Convert the int16 to int32 (the 'u' 5 - 8)
		vmovl.s16	q2, d4					@Convert the int16 to int32 (the 'u' 1 - 4)
		vmovl.s16	q5, d7					@Convert the int16 to int32 (the 'v' 5 - 8)
		vmovl.s16	q4, d6					@Convert the int16 to int32 (the 'v' 1 - 4)
		
		vcvt.f32.s32 q2, q2					@Convert the int32 to float32 (the 'u' 1 - 4)
		vcvt.f32.s32 q3, q3					@Convert the int32 to float32 (the 'u' 5 - 8)
		vcvt.f32.s32 q4, q4					@Convert the int32 to float32 (the 'v' 1 - 4)
		vcvt.f32.s32 q5, q5					@Convert the int32 to float32 (the 'v' 5 - 8)
		
		
		@ Calculate green (it takes both u and v, so calculating it first will make the others easier,
		@ Because of less register pressure).
		vmul.f32	q6, q2, q13				@Calculate the 'u' part of green (1 - 4)
		vmul.f32	q7, q4, q12				@Calculate the 'v' part of green (1 - 4)
		vadd.f32	q6, q7, q6				@Add the 'u' and 'v' part of green together (1 - 4)
		vcvt.s32.f32 q6, q6					@Convert the float32 to int32 (1 - 4)
		vqmovn.s32	d12, q6					@Convert the int32 to int16 (1 - 4) (d12 is q6 low)
		
		vmul.f32	q7, q3, q13				@Calculate the 'u' part of green (5 - 8)
		vmul.f32	q8, q5, q12				@Calculate the 'v' part of green (5 - 8)
		vadd.f32	q7, q8, q7				@Add the 'u' and 'v' part of green together (5 - 8)
		vcvt.s32.f32 q7, q7					@Convert the float32 to int32 (5 - 8)
		vqmovn.s32	d13, q7					@Convert the int32 to int16 (5 - 8) (d13 is q6 high)
		@ NOTE: q6 is the green offset from y (1 - 8)
		
		
		@ Calculate red (it just takes v, so it should not take up any more registers)
		vmul.f32	q4, q4, q11				@Multiply 'v' by its green weight (1 - 4)
		vmul.f32	q5, q5, q11				@Multiply 'v' by its green weight (5 - 8)
		vcvt.s32.f32 q5, q5					@Convert float32 to int32 (1 - 4)
		vcvt.s32.f32 q4, q4					@Convert float32 to int32 (5 - 8)
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
		
		@ Calculate RGB values
		vqadd.s16	q5, q1, q4				@Red 2	= 'y2' + 'r'
		vqsub.s16	q7, q1, q6				@Green 2= 'y2' - 'g'
		vqadd.s16	q3, q1, q2				@Blue 2	= 'y2' + 'b'
		vqadd.s16	q4, q0, q4				@Red 1	= 'y1' + 'r'
		vqsub.s16	q6, q0, q6				@Green 1= 'y1' + 'g'
		vqadd.s16	q2, q0, q2				@Blue 1	= 'y1' + 'b'
		
		
		@ Convert RGB to u8
		vqmovun.s16	d0, q4					@Convert the int16 to uint8 (the 'r1' bytes)
		vqmovun.s16	d1, q6					@Convert the int16 to uint8 (the 'g1' bytes)
		vqmovun.s16	d2, q2					@Convert the int16 to uint8 (the 'b1' bytes)
		vqmovun.s16	d3, q5					@Convert the int16 to uint8 (the 'r2' bytes)
		vqmovun.s16	d4, q7					@Convert the int16 to uint8 (the 'g2' bytes)
		vqmovun.s16	d5, q3					@Convert the int16 to uint8 (the 'b2' bytes)
		
		@ Interleave RGB (TODO make sure that this is right, I think that the operands might need to be switched for vzip)
		vzip.8		d0, d3					@Interleave 'r1' & 'r2'
		vzip.8		d1, d4					@Interleave 'g1' & 'g2'
		vzip.8		d2, d5					@Interleave 'b1' & 'b2'
		
		vst3.8		{d0 - d2}, [r1]!		@Store RGB (1 - 6)
		vst3.8		{d3 - d5}, [r1]!		@Store RGB (6 - 12)
		
		subs		r2, r2, #1				@Decrement len by 1
		bne			.loop					@IF len>0, GOTO .loop
	
	pop {ip,pc}								@Return
