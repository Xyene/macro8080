.equ TIMER0_BASE, 0xFF202000
.equ TIMER0_STATUS, 0
.equ TIMER0_CONTROL, 4
.equ TIMER0_PERIODL, 8
.equ TIMER0_PERIODH, 12
.equ TICKS_PER_SEC, 100000000

.text
.global initialize_interrupts
initialize_interrupts:
	subi sp, sp, 4
	stw ra, 0(sp)

	# Initialize the timer's period
	movia r8, TIMER0_BASE
	# Lower 16 bitis
	addi r9, r0, %lo(TICKS_PER_SEC)
	stwio r9, TIMER0_PERIODL(r8)
	# Upper 16 bits
	addi r9, r0, %hi(TICKS_PER_SEC)
	stwio r9, TIMER0_PERIODH(r8)

	# Stop the timer
	stwio r0, TIMER0_CONTROL(r8);

	# Make timer generate interrupts, and start the timer
	addi r9, r0, 0x5 # start and interrupt bits are high
	stwio r9, TIMER0_CONTROL(r8);

	# Enable interrupts on the CPU
	addi r9, r0, 0x1
	wrctl status, r9

	# Accept interrupts from timer1
	addi r9, r0, 0x1
	wrctl ienable, r9

	ldw ra, 0(sp)
	addi sp, sp, 4

	ret

.section .exceptions, "ax"
interrupt_handler:
  # Prologue
  subi sp, sp, 32
  stw ea, 0(sp)
  stw ra, 4(sp)
  stw r2, 8(sp)
  stw r4, 12(sp)
  stw r5, 16(sp)
  stw r6, 20(sp)
  stw r9, 24(sp)
  stw r10, 28(sp)

  # Check if interrupt was caused by a device
  rdctl et, ipending
  beq et, r0, interrupt_epilogue

  # Interrupt was caused by a device, make sure we
  # re-execute the interrupted instruction
  subi ea, ea, 4
  stw ea, 0(sp)

  # Check if interrupt was caused by the timer
  andi r9, et, 0x1
  beq r9, r0, interrupt_epilogue

timer_handler:
 	# Don't need to save registers. Are just restoring them from the stack right away
 	call toggle_underscore

	movia r10, TIMER0_BASE
	# clearing status register acknowledges interrupt
  stwio r0, TIMER0_STATUS(r10)

	addi r9, r0, 0x5 # start and interrupt bits are high
	stwio r9, TIMER0_CONTROL(r10);

interrupt_epilogue:
  ldw r10, 28(sp)
  ldw r9, 24(sp)
  ldw r6, 20(sp)
  ldw r5, 16(sp)
  ldw r4, 12(sp)
  ldw r2, 8(sp)
	ldw ra, 4(sp)
	ldw ea, 0(sp)
  addi sp, sp, 32

  eret
