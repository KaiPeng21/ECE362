;STEP 1-3

        org $800
        ldaa #$08
        ldab #$02
        ldx #$900
        ldy #$00
        
        ;STEP 1
        movw #$0A00,$0900  ;STEP 1-A: MOVW instruction using immediate/extended addressing modes.
        leay B,x           ;STEP 1-B: LEA instruction using accumulator offset indexed addressing mode.
        psha               ;STEP 1-C: PSHA instruction, followed by PSHB and PULD. 
        pshb
        puld
        
        ;STEP 2
        adcb #$01    ;STEP 2-A: ADCB instruction using immediate addressing mode. 
        addd $902   ;STEP 2-B: ADDD instruction using extended addressing mode. 
        adda #$0A    ;STEP 2-C: ADDA instruction using immediate addressing, followed by DAA instruction.
        daa
        suba #$F7    ;SUBA instruction using immediate addressing, followed by DAA instruction 
        daa
        
        ldy #$100
        emul        ;STEP 2-E: EMUL instruction using inherent addressing mode
        ldd #$200
        ldx #$800
        fdiv        ;STEP 2-F: FDIV instruction using inherent addressing mode.
        ldd #$800
        ldx #$200
        fdiv
        
        ;STEP 3
        ldaa #$65
        ldab #$CA
        ldx  #$900
        lsla        ;STEP 3-A: LSLA instruction using inherent addressing mode. 
        rorb        ;STEP 3-B: RORB instruction using inherent addressing mode. 
        andcc #$0
        cpx  $904   ;STEP 3-C: CPX instruction using extended addressing mode. 
        orcc  #$B5
        andcc #$AA    ;STEP 3-D: ANDCC instruction using immediate addressing mode.
        orcc  #$15    ;STEP 3-E: ORCC instruction using immediate addressing mode. 
        
        stop
        
        
        org $902
        fcb $00
        fcb $FF
        
        org $904
        fcb $FF
        fcb $FF
