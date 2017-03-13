;****************************************************************************
; Step 4-A:
;
; Write a program that loads two 16-bit numbers from the memory location
; "ops", divides the first number by the second, then stores the quotient
; in the memory location "quot" and the remainder in memory location
; "remain". Note that the space for the operands and results has
; already been allocated for you below. Each has a label associated with
; the memory location. You can use the labels "ops", "quot", and "remain"
; when writing your code, and the assembler will convert it to the
; appropriate memory address.
;
; Note that the instructions for the next step begin at memory location
; $0820. Check your assembled source listing (".lst") file to make sure
; that your code does not pass that location. 

step4a  org $0800

; Put your code here

        ;pshd
        ;pshx
        ;pshy
        
        ldy #ops
        ldd ops
        ldx 2,y
        idiv
        stx quot
        std remain
        
        ;puly
        ;pulx
        ;puld

        stop ; use a placeholder for breakpoint
ops     rmb 4      
quot    rmb 2
remain  rmb 2
        rmb 3

;****************************************************************************
; Step 4-B:
;
; Write a program that tests whether the unsigned value contained
; in the A register is higher than value stored at the memory location
; "tval". If it is, the program sets the variable "higher" to $FF,
; and if not, the program sets the variable "higher" to $00.
;
;****************************************************************************
;          org $0820
; Put your code here
          
          
;          cmpa tval
;          bhi  ifs
;          ldaa #$00              
;eif       staa higher
;          stop ; use a placeholder for breakpoint
;          jmp  $840
;
;ifs       ldaa #$FF
;          jmp  eif
;
;
;tval   fcb 100
;higher rmb 1 

           org $0820
if1
           cmpa tval
           bhi  if2
           ldaa #$00
           bra  if_end
if2        
           cmpa tval
           ldaa #$FF
           bra  if_end
if_end
           staa higher
           
           stop

tval fcb 100
higher rmb 1



        
;****************************************************************************
; Step 4-C:
;
; Write a program that performs the addition of the two
; 3-byte numbers located at the memory location "adds" and stores
; the result in the memory location "sum".
;
; Note: You may not change the values of any of the registers.
;
; Therefore, you should push any registers used in the program at the
; beginning of the program, and then pull (pop) them off at the end
; of the program.
;
; NOTE: The operands are stored MSB to LSB.
;
;****************************************************************************

          org $0840

; Put your code here
          pshd
          pshx
          pshy
          
          andcc #$0   ; clear CCR
          ldy   #adds
          ldx   #sum
          ; add the least significant 2 bytes and store
          ldd   1,y
          addd  4,y
          std   1,x
          ; add the most significant byte with carry out and store
          ldab  0,y
          adcb  3,y
          stab  0,x
          
          puly
          pulx
          puld
          stop
          

          org $0870
adds      rmb 6 ; Addends
sum       rmb 3 ; Sum


;****************************************************************************
; Step 4-D:
;
; Write a program that will transfer a specified number of bytes of data
; from one memory location (source) to another memory location (destination).
; Assume that the source address is contained in the Y register, the
; destination address is contained in the X register, and the A register
; contains the number of bytes of data to be transferred. The X, Y and A
; registers should return with their original values, and the other registers
; should be unchanged.
;
; Note: For this program, you should use a FOR loop. The basic
; structure of a FOR loop is:
;
; loop check counter
; branch out of loop if done (here, to label "done")
; perform action
; branch back to "loop"
; done next instruction
;
; NOTE: When testing this program, make sure that you are not transferring
; data to memory locations where your program is located!!! Check your
; assembled listing file to see where your programs are located.
;****************************************************************************
         org $0890
; Put your code here
         pshb
         psha
         pshx
         pshy
loop     tsta
         beq  done
         ldab 1,y+
         stab 1,x+
         deca
         jmp  loop         
done
         puly
         pulx
         pula
         pulb

         stop ; use a placeholder for breakpoint
         
         org $A00
         fcb $01
         fcb $02
         fcb $03
         fcb $04
         fcb $05
         org $A05
         fcb $AA
         fcb $BB
         fcb $CC
         fcb $DD
         fcb $EE
         
 
;***********************************************************************
; Step 4-E:
;
; Write a program that determines how many bits of the number passed
; in the A register are 1's. The A register should return its original
; value, and the number of 1 bits should be returned in the B register.
;
; Note: For this program, you should use a DO loop. The basic
; structure of a DO loop is:
;
; initialize counter
; loop perform action
; update and check counter
; branch back to "loop" if not done
;
; You will need to maintain three pieces of data for this program:
; (a) initial value (in A register)
; (b) number of 1 bits (returned in B register)
; (c) counter for the loop
;
; Since we only have two accumulators available in the HC(S)12, you will
; need to use an index register, a local variable (stored in memory), or
; the stack to implement this. A memory location with the label "count"
; has been reserved below if you would like to use it.
;
;***********************************************************************
;        org $0920
; Put your code here
;        psha
;        pshx
;        
;        ldab  #$0
;does    andcc #$0
;        adda  #$0
;        bmi   adbit
;added   lsla
;        bne   does
;         
;        pulx    
;        pula
;
;        stop ; use a placeholder for breakpoint


;adbit   incb
;        jmp   added

;count   rmb 1
;        end         
        
        
           
        
        org $0920
        
        psha
        pshx
        
        ldab #$0
does    
        andcc #$0

iff1    cmpa  #$0
        bmi   iff2
        bra   iff_end
iff2    
        incb
        bra   iff_end           
iff_end 
        lsla
        bne   does
        
        stab count       
        pulx
        pula
        
        stop
        
        
count   rmb 1
        end 