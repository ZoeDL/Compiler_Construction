	read => r3
	loadI 0 => r4
	loadI 1 => r0
	cmp_LE r0, r3 => r5
	cbr r5 -> L0, L1
L0: nop
	loadI 1 => r1
	cmp_LE r1, r3 => r6
	cbr r6 -> L2, L3
L2: nop
	loadI 1 => r2
	cmp_LE r2, r3 => r7
	cbr r7 -> L4, L5
L4: nop
	sub r3, r2 => r18
	rshiftI r18, 2 => r18
	lshiftI r18, 2 => r18
	add r2, r18 => r19
	cbr r18 -> L6, L7
L6: nop
	mult r3, r3 => r8
	mult r8, r3 => r9
	mult r0, r9 => r10
	mult r3, r3 => r11
	mult r1, r11 => r12
	add r10, r12 => r13
	add r13, r2 => r14
	i2i r14 => r4
	addI r2, 1 => r2
	mult r3, r3 => r8
	mult r8, r3 => r9
	mult r0, r9 => r10
	mult r3, r3 => r11
	mult r1, r11 => r12
	add r10, r12 => r13
	add r13, r2 => r14
	i2i r14 => r4
	addI r2, 1 => r2
	mult r3, r3 => r8
	mult r8, r3 => r9
	mult r0, r9 => r10
	mult r3, r3 => r11
	mult r1, r11 => r12
	add r10, r12 => r13
	add r13, r2 => r14
	i2i r14 => r4
	addI r2, 1 => r2
	mult r3, r3 => r8
	mult r8, r3 => r9
	mult r0, r9 => r10
	mult r3, r3 => r11
	mult r1, r11 => r12
	add r10, r12 => r13
	add r13, r2 => r14
	i2i r14 => r4
	addI r2, 1 => r2
	cmp_LT r2, r19 => r15
	cbr r15 -> L6, L7
L7: nop
	cmp_LE r2, r3 => r15
	cbr r15 -> L8, L5
L8: nop
	mult r3, r3 => r8
	mult r8, r3 => r9
	mult r0, r9 => r10
	mult r3, r3 => r11
	mult r1, r11 => r12
	add r10, r12 => r13
	add r13, r2 => r14
	i2i r14 => r4
	addI r2, 1 => r2
	cmp_LE r2, r3 => r15
	cbr r15 -> L8, L5
L5: nop
	addI r1, 1 => r1
	cmp_LE r1, r3 => r16
	cbr r16 -> L2, L3
L3: nop
	addI r0, 1 => r0
	cmp_LE r0, r3 => r17
	cbr r17 -> L0, L1
L1: nop
	write r4
	halt
