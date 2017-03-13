 ;***********************************************************************
; ECE 362 - Experiment 6 - Fall 2016
;***********************************************************************
;
; Completed by: < Chia-Hua Peng >
;               < 2220-P >
;               < 5 >
;
;
; Academic Honesty Statement:  In signing this statement, I hereby certify
; that I am the individual who created this HC(S)12 source file and that I 
; have not copied the work of any other student (past or present) while 
; completing it. I understand that if I fail to honor this agreement, I will 
; receive a grade ofZERO and be subject to possible disciplinary action.
;
;
; Signature: ______Chia-Hua Peng_______________________________   Date: _____10/2/2016_______
;
;       
;***********************************************************************
;
; The objective of this experiment is to implement a Big More-Than-Ten
; 19-second play clock on the 9S12C32 development kit.  A waveform
; generator will be used to provide a 100 Hz periodic interrupt. A
; GAL22V10 will be used to provide the requisite interfacing logic,
; which includes an interrupt device flag and a BCD-to-display-code
; decoder for driving a pair of common-anode 7-segment LEDs (see
; lab description document for details). Note that this application
; is intended to run in a "turn-key" fashion. 
;
; The following docking board resources will be used:
;
; - left pushbutton (PAD7): shot clock reset
; - right pushbutton (PAD6): shot clock start/stop
; - left LED (PT1): shot clock run/stop state
; - right LED (PT0): shot clock expired (00) state
;
; CAUTION: Be sure to set the DSO waveform generator output to produce a 
;          5 V peak-to-peak square wave with +2.5 V offset
;
;========================================================================
;
; MC68HC9S12C32 REGISTER MAP

INITRM	EQU	$0010	; INITRM - INTERNAL RAM POSITION REGISTER
INITRG	EQU	$0011	; INITRG - INTERNAL REGISTER POSITION REGISTER

; ==== CRG - Clock and Reset Generator

SYNR	EQU	$0034	; CRG synthesizer register
REFDV 	EQU	$0035	; CRG reference divider register
CRGFLG	EQU	$0037	; CRG flags register
CRGINT	EQU	$0038
CLKSEL	EQU	$0039	; CRG clock select register
PLLCTL	EQU	$003A	; CRG PLL control register
RTICTL	EQU	$003B
COPCTL	EQU	$003C

; ==== PORT M

PTM	EQU	$0250	; Port M data register (bit 3 will be used to clear device flag)
DDRM  	EQU	$0252	; Port M data direction register

; ==== Port AD - Digital Input (Pushbuttons)

PTAD    EQU     $0270   ; Port AD data register
DDRAD   EQU     $0272   ; Port AD data direction register
ATDDIEN	EQU	$008D	; Port AD digital input enable
			; (programs Port AD bit positions as digital inputs)

; ==== Port T - docking module LEDs and external PLD interface

PTT   	EQU	$0240	; Port T data register
DDRT	EQU	$0242	; Port T data direction register

; ==== IRQ control register

IRQCR	EQU	$001E	; (will use default state out of reset) 

; ==== SCI Register Definitions

SCIBDH	EQU	$00C8	; SCI0BDH - SCI BAUD RATE CONTROL REGISTER
SCIBDL	EQU	$00C9	; SCI0BDL - SCI BAUD RATE CONTROL REGISTER
SCICR1	EQU	$00CA	; SCI0CR1 - SCI CONTROL REGISTER
SCICR2	EQU	$00CB	; SCI0CR2 - SCI CONTROL REGISTER
SCISR1	EQU	$00CC	; SCI0SR1 - SCI STATUS REGISTER
SCISR2	EQU	$00CD	; SCI0SR2 - SCI STATUS REGISTER
SCIDRH	EQU	$00CE	; SCI0DRH - SCI DATA REGISTER
SCIDRL	EQU	$00CF	; SCI0DRL - SCI DATA REGISTER

;***********************************************************************
;
; ASCII character definitions
;

CR   equ  $0d ; RETURN
LF   equ  $0a ; LINE FEED

; ======================================================================
;
;  Variable declarations (SRAM)
;  Others may be added if deemed necessary


intcnt	equ	$3800	; number of IRQ interrupts accumulated
			          	; (set tenths flag when count reaches 10)
tenths	equ	$3801	; one-tenth second flag
leftpb	equ	$3802	; left pushbutton flag
rghtpb	equ	$3803	; right pushbutton flag
runstp	equ	$3804	; run/stop flag


mask    equ $3805
clkval  equ $3806
prevl   equ $3808
prevr   equ $3809
currl   equ $380A
currr   equ $380B


;***********************************************************************
;  BOOT-UP ENTRY POINT
;***********************************************************************

	org	$8000
startup	sei			; Disable interrupts
	movb	#$00,INITRG	; set registers to $0000
	movb	#$39,INITRM	; map RAM ($3800 - $3FFF)
	lds	#$3FCE		; initialize stack pointer

;
; Set the PLL speed (bus clock = 24 MHz)
;

	bclr	CLKSEL,$80	; disengage PLL from system
	bset	PLLCTL,$40	; turn on PLL
	movb	#$2,SYNR	; set PLL multiplier
	movb	#$0,REFDV	; set PLL divider
	nop
	nop
plp	brclr	CRGFLG,$08,plp	; while (!(crg.crgflg.bit.lock==1))
	bset	CLKSEL,$80	; engage PLL 

;
; Disable watchdog timer (COPCTL register)
;
	movb	#$40,COPCTL	; COP off; RTI and COP stopped in BDM-mode

;
; Initialize asynchronous serial port (SCI) for 9600 baud, no interrupts
; Note - for debugging use only
;
	movb	#$00,SCIBDH	; set baud rate to 9600
	movb	#$9C,SCIBDL	; 24,000,000 / 16 / 156 = 9600 (approx)
	movb	#$00,SCICR1	; $9C = 156
	movb	#$0C,SCICR2	; initialize SCI for program-driven operation


;***********************************************************************
;  START OF CODE FOR EXPERIMENT 6
;***********************************************************************
;
;  Flag and variable initialization (others may be added)
;
	clr	intcnt
	clr	tenths
	clr	leftpb
	clr	rghtpb
	clr	runstp
 
 
;
;  Initialize Port AD for use as a digital input port (for pushbuttons)
;
	clr	DDRAD		; program port AD for input mode
	movb	#$C0,ATDDIEN	; program PTAD7 and PTAD6 pins as digital inputs

;
;  Initialize Port M to provide asynchronous reset to external device flag on bit 3
;
	bset	DDRM,$08 ; configure pin 3 of Port M (PM3) as output signal
	bset	DDRT,$FF ; configure pin 3 of Port M (PM3) as output signal
	bset	PTM,$08	 ; send active high clear pulse to external device flag
	bclr	PTM,$08	 ; return asynchronous clear signal to low state

;
;  < add any additional initialization code here >
;


       jsr reset



	cli		; enable IRQ interrupts - last step before starting main loop
;
;  Start of main program-driven polling loop
;
main

;  If the "tenth second" flag is set, then
;    - clear the "tenth second" flag
;    - If the "run/stop" flag is set, then
;      + decrement the play clock value (stored as packed 4-digit BCD number)
;      + update the 7-segment display
;        o if the time remaining is greater than or equal to 10 seconds,
;          display the tens and ones digits
;        o if the time remaining is less than 10 seconds but
;          greater than or equal to 1 second, blank the tens digit
;          and only display the ones digit
;        o if the time remaining is less than 1.0 second,
;          turn on the middle decimal point and display
;          the tenths digit instead of the ones digit
;          (starting with .9 and ending with .0)
;    - Endif
;  Endif
         

         ldaa tenths
oiff     cmpa #0
         beq oiffend
         clr tenths
         ldaa runstp
iiff     cmpa #0
         beq iiffend
         
         ldaa clkval+1
         adda #$99
         daa
         staa clkval+1
         ldaa clkval
         adca #$99
         daa
         staa clkval
         
         ldd clkval
         cpd #$100
         blo skip1
         ; display tens and ones digits
         ;jsr distens
         ;jsr disones
         stab PTT
         bset PTT,%0110
         bclr PTT,%1001
         
         
         
         
         bra iiffend
skip1    ldd clkval
         cpd #$10
         blo skip2
         ; display ones digit
         ;jsr disones
         stab PTT
         bset PTT,%0010
         bclr PTT,%1101         
         
         
         bra iiffend
skip2    ; display "." and tenths digit
         ;jsr distenths
         lslb
         lslb
         lslb
         lslb
         stab PTT
         bset PTT,%1010
         bclr PTT,%0101

iiffend
oiffend


;  If the left pushbutton ("reset play clock") flag is set, then:
;    - clear the left pushbutton flag
;    - clear the "run/stop" flag
;    - turn off both the "run/stop" and "time expired" LEDs
;    - reset the display to "19" 
;  Endif

         ldaa leftpb
         cmpa #0
         beq lpbend
         clr leftpb
         clr runstp
         
         ;clear both "run/stop(left)" and "time expired (right)" LEDs 
         ldd #$0190
         std clkval
         bset PTT,%10010100
         bclr PTT,%01101011
         
         
lpbend



;  If the right pushbutton ("start/stop") flag is set, then
;    - clear the right pushbutton flag
;    - toggle the "run/stop" flag
;    - toggle the "run/stop" LED
;  Endif

         ldaa rghtpb
         cmpa #0
         beq rpbend
         clr rghtpb         
         ldab runstp ; toggle flag
         comb
         andb #1
         stab runstp
         
         
         ; toggle the LED
         ldab PTT
         comb
         andb #$02
         cmpb #$02
         beq seton
setoff   bclr PTT,$02
         bra setend                
seton    bset PTT,$02
setend         
         
         
rpbend



;  If the play clock has reached ".0", then:
;    - clear the "run/stop" flag
;    - turn on the "time expired" LED
;    - turn off the "run/stop" LED
;  Endif

         ldx clkval
         cpx #0
         bne nonzero
         clr runstp
         
         ;LEDs
         bset PTT,$01
         bclr PTT,$02
         
nonzero



	jmp	main	;  main is a while( ) loop with no terminating condition



;***********************************************************************
;
; IRQ interrupt service routine
;
;  External waveform generator produces 100 Hz IRQ interrupt rate
;
;  Use INTCNT to determine when 10 IRQ interrupts have accumulated 
;    -> set tenths flag and clear INTCNT to 0
;
;  Sample state of pushbuttons (PAD7 = left, PAD6 = right)
;  If change in state from "high" to "low" is detected, set pushbutton flag
;     leftpb (for PAD7 H -> L), rghtpb (for PAD6 H -> L)
;     (pushbuttons are momentary contact closures to ground)
;

irq_isr
        

; < place your code for the IRQ interrupt service routine here >

; < be sure to clear external device flag by asserting bit 3 of Port M - see above >
        
        
        
        bset	PTM,$08	 ; send active high clear pulse to external device flag
        bclr  PTM,$08
	      pshd
        pshc
        
        ; 100 Hz 
        ldaa intcnt       
        inca
        staa intcnt
        cmpa #10
        bne nothing
        bset tenths,$01
        ldaa #0
        staa intcnt
nothing        
                
        ; pushbutton
        ldaa PTAD
        anda #$80       ;PAD7
        bmi if1end      ; current 1 -> nothing
        ldab prevl      ; current 0, prev 1 -> set flag
        cmpb #1         ; current 0, prev 0 -> nothing
        bne if1end
        movb #$01,leftpb
if1end
        
        ldaa PTAD
        anda #$40
        lsla
        bmi if2end
        ldab prevr
        cmpb #1
        bne if2end
        movb #$01,rghtpb
if2end
        
        ldaa PTAD
        anda #$80
        bmi currl1
currl0  movb #$00,prevl 
        bra currle
currl1  movb #$01,prevl
currle  

        ldaa PTAD
        anda #$40
        lsla
        bmi currr1
currr0  movb #$00,prevr
        bra currre
currr1  movb #$01,prevr
currre      
        
        
        bset  PTM,$08
        bclr	PTM,$08	 ; return asynchronous clear signal to low state
                
        pulc
        puld
        rti
        
       




;***********************************************************************
;
; Add any additional subroutines needed here 
;
;  < place additional routines here >

reset  
       ldy #$0190
       sty clkval
       bclr PTT,$FC
       bset PTT,$94
       
       
       
       ldaa #$01
       staa prevl
       staa prevr
       
       
       
       rts

distens 
        pshd
        
        ldd clkval
        anda #$0F
        lsla
        lsla
        ldab PTT
        stab mask
        oraa mask
        staa PTT        
                 
        puld
        rts

disones
        pshd
                
        ldd clkval        
        andb #$F0     ; ones digit in the upper nibble of the B register
        ldaa PTT
        staa mask
        orab mask 
        stab PTT
                
        puld
        rts
       
distenths
        pshd
        
        
        ldd clkval
        andb #$0F   ; tenths digit in lower nibble of B register
        lslb
        lslb
        lslb
        lslb
        orab #$08
        ldaa PTT
        staa mask
        orab mask
        stab PTT
                
        puld
        rts



;***********************************************************************
; Character I/O Library Routines for 9S12C32
;
; (these maay prove useful for testing and debugging but are otherwise
; not used for this application)
;***********************************************************************

rxdrf    equ   $20    ; receive buffer full mask pattern
txdre    equ   $80    ; transmit buffer empty mask pattern

;***********************************************************************
; Name:         inchar
; Description:  inputs ASCII character from SCI serial port
;                  and returns it in the A register
; Returns:      ASCII character in A register
; Modifies:     A register
;***********************************************************************

inchar  brclr  SCISR1,rxdrf,inchar
        ldaa   SCIDRL ; return ASCII character in A register
        rts

;***********************************************************************
; Name:         outchar
; Description:  outputs ASCII character passed in the A register
;                  to the SCI serial port
;***********************************************************************

outchar brclr  SCISR1,txdre,outchar
        staa   SCIDRL ; output ASCII character to SCI
        rts

;***********************************************************************
;
; Define 'where you want to go today' (reset and interrupt vectors) 
;
; If get a "bad" interrupt, just return from it

BadInt	rti

;
; ------------------ VECTOR TABLE --------------------
;

	org	$FF8A
	fdb	BadInt	;$FF8A: VREG LVI
	fdb	BadInt	;$FF8C: PWM emergency shutdown
	fdb	BadInt	;$FF8E: PortP
	fdb	BadInt	;$FF90: Reserved
	fdb	BadInt	;$FF92: Reserved
	fdb	BadInt	;$FF94: Reserved
	fdb	BadInt	;$FF96: Reserved
	fdb	BadInt	;$FF98: Reserved
	fdb	BadInt	;$FF9A: Reserved
	fdb	BadInt	;$FF9C: Reserved
	fdb	BadInt	;$FF9E: Reserved
	fdb	BadInt	;$FFA0: Reserved
	fdb	BadInt	;$FFA2: Reserved
	fdb	BadInt	;$FFA4: Reserved
	fdb	BadInt	;$FFA6: Reserved
	fdb	BadInt	;$FFA8: Reserved
	fdb	BadInt	;$FFAA: Reserved
	fdb	BadInt	;$FFAC: Reserved
	fdb	BadInt	;$FFAE: Reserved
	fdb	BadInt	;$FFB0: CAN transmit
	fdb	BadInt	;$FFB2: CAN receive
	fdb	BadInt	;$FFB4: CAN errors
	fdb	BadInt	;$FFB6: CAN wake-up
	fdb	BadInt	;$FFB8: FLASH
	fdb	BadInt	;$FFBA: Reserved
	fdb	BadInt	;$FFBC: Reserved
	fdb	BadInt	;$FFBE: Reserved
	fdb	BadInt	;$FFC0: Reserved
	fdb	BadInt	;$FFC2: Reserved
	fdb	BadInt	;$FFC4: CRG self-clock-mode
	fdb	BadInt	;$FFC6: CRG PLL Lock
	fdb	BadInt	;$FFC8: Reserved
	fdb	BadInt	;$FFCA: Reserved
	fdb	BadInt	;$FFCC: Reserved
	fdb	BadInt	;$FFCE: PORTJ
	fdb	BadInt	;$FFD0: Reserved
	fdb	BadInt	;$FFD2: ATD
	fdb	BadInt	;$FFD4: Reserved
	fdb	BadInt	;$FFD6: SCI Serial System
	fdb	BadInt	;$FFD8: SPI Serial Transfer Complete
	fdb	BadInt	;$FFDA: Pulse Accumulator Input Edge
	fdb	BadInt	;$FFDC: Pulse Accumulator Overflow
	fdb	BadInt	;$FFDE: Timer Overflow
	fdb	BadInt	;$FFE0: Standard Timer Channel 7
	fdb	BadInt	;$FFE2: Standard Timer Channel 6
	fdb	BadInt	;$FFE4: Standard Timer Channel 5
	fdb	BadInt	;$FFE6: Standard Timer Channel 4
	fdb	BadInt	;$FFE8: Standard Timer Channel 3
	fdb	BadInt	;$FFEA: Standard Timer Channel 2
	fdb	BadInt	;$FFEC: Standard Timer Channel 1
	fdb	BadInt	;$FFEE: Standard Timer Channel 0
	fdb	BadInt	;$FFF0: Real Time Interrupt (RTI)
	fdb	irq_isr	;$FFF2: IRQ (External Pin or Parallel I/O) (IRQ)
	fdb	BadInt	;$FFF4: XIRQ (Pseudo Non-Maskable Interrupt) (XIRQ)
	fdb	BadInt	;$FFF6: Software Interrupt (SWI)
	fdb	BadInt	;$FFF8: Illegal Opcode Trap ()
	fdb	startup	;$FFFA: COP Failure (Reset) ()
	fdb	BadInt	;$FFFC: Clock Monitor Fail (Reset) ()
	fdb	startup	;$FFFE: /RESET
	end

