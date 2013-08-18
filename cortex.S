.section .text, "ax"
.thumb

SendVect1700:	@ RdCode[1][0]
	LDR	R5, =0x400FC040	@ Select Flash memory (no remap)
	MOV	R0, #1
	STR	R0, [R5]

	LDR	R5, =0x4000C000	@ UART0 base address
	LDR	R2, =1024	@ Number of bytes to send
	MOV	R3, #0		@ Start address
	MOV	R4, #0		@ Sum
1:
	LDRB	R0, [R3]	@ Get a byte
	ADD	R3, #1		@ Next address
	ADD	R4, R0		@ Add sum
	BL	2f		@ Send byte
	SUB	R2, #1		@ Repeat for the vector area
	BNE	1b

	MOV	R0, R4		@ Send sum
	BL	2f
	B	.		@ Halt

2:	@ Send a byte 
	LDR	R1, [R5, #20]	@ Read LSR
	MOV	R6, #0x20
	TST	R1, R6		@ Test THRE bit
	BEQ	2b
	STR	R0, [R5, #0]	@ Write to THR
	BX	LR
.pool


SendVect1300:	@ RdCode[2][0]
	LDR	R5, =0x40048000	@ Select Flash memory (no remap)
	MOV	R0, #2
	STR	R0, [R5]

	LDR	R5, =0x40008000	@ UART0 base address
	LDR	R2, =512	@ Number of bytes to send
	MOV	R3, #0		@ Start address
	MOV	R4, #0		@ Sum
1:
	LDRB	R0, [R3]	@ Get a byte
	ADD	R3, #1		@ Next address
	ADD	R4, R0		@ Add sum
	BL	2f		@ Send byte
	SUB	R2, #1		@ Repeat for the vector area
	BNE	1b

	MOV	R0, R4		@ Send sum
	BL	2f
	B	.		@ Halt

2:	@ Send a byte 
	LDR	R1, [R5, #20]	@ Read LSR
	MOV	R6, #0x20
	TST	R1, R6		@ Test THRE bit
	BEQ	2b
	STR	R0, [R5, #0]	@ Write to THR
	BX	LR
.pool


.end
