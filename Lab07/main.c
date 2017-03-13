// ***********************************************************************
//  ECE 362 - Experiment 7 - Fall 2016
//
// Dual-channel LED bar graph display                    
// ***********************************************************************
//	 	   			 		  			 		  		
// Completed by: < Chia-Hua Peng >
//               < 2220-P >
//               < 5 >
//               < date completed >
//
//
// Academic Honesty Statement:  In entering my name above, I hereby certify
// that I am the individual who created this HC(S)12 source file and that I 
// have not copied the work of any other student (past or present) while 
// completing it. I understand that if I fail to honor this agreement, I will 
// receive a grade of ZERO and be subject to possible disciplinary action.
//
// ***********************************************************************

#include <hidef.h>           /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

// All funtions after main should be initialized here
#define RTICTLI 0x70


// Note: inchar and outchar can be used for debugging purposes

char inchar(void);
void outchar(char x);
			 		  		
//  Variable declarations  	   			 		  			 		       
int tenthsec = 0;  // one-tenth second flag
int leftpb = 0;    // left pushbutton flag
int rghtpb = 0;    // right pushbutton flag
int runstp = 0;    // run/stop flag                         
int rticnt = 0;    // RTICNT (variable)
int prevpb = 0;    // previous state of pushbuttons (variable)


/* My Own Variable */
short THRESH1,THRESH2,THRESH3,THRESH4,THRESH5;
int output1 = 0;
int output2 = 0;
int serial_data = 0;
int i = 0;
	 	   		
// Initializations
 
void  initializations(void) {

// Set the PLL speed (bus clock = 24 MHz)

  		CLKSEL = CLKSEL & 0x80; // disengage PLL from system
  		PLLCTL = PLLCTL | 0x40; // turn on PLL
  		SYNR = 0x02;            // set PLL multiplier
  		REFDV = 0;              // set PLL divider
  		while (!(CRGFLG & 0x08)){  }
  		CLKSEL = CLKSEL | 0x80; // engage PLL
  
// Disable watchdog timer (COPCTL register)

      COPCTL = 0x40;    //COP off - RTI and COP stopped in BDM-mode

// Initialize asynchronous serial port (SCI) for 9600 baud, no interrupts

      SCIBDH =  0x00; //set baud rate to 9600
      SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
      SCICR1 =  0x00; //$9C = 156
      SCICR2 =  0x0C; //initialize SCI for program-driven operation
         
//  Initialize Port AD pins 7 and 6 for use as digital inputs

	    DDRAD = 0; 		//program port AD for input mode
      ATDDIEN = 0xC0; //program PAD7 and PAD6 pins as digital inputs
         
         
         
         
//  Add additional port pin initializations here  (e.g., Other DDRs, Ports) 
      //DDRT = 0x18; 
      DDRT = 0xFF;
      PTT_PTT0 = 0;
      PTT_PTT1 = 0;

//  Define bar graph segment thresholds (THRESH1..THRESH5)
//  NOTE: These are binary fractions
      
      THRESH1 = 0x2000; //0.2  0x4000
      THRESH2 = 0x5000; //0.4  0x7000
      THRESH3 = 0x8500; //0.5  0x9000
      THRESH4 = 0xB000; //0.7  0xA000
      THRESH5 = 0xE000; //0.9  0xD000
      /*
      THRESH1 = 0x0100; //0.2  0x4000
      THRESH2 = 0x0300; //0.4  0x7000
      THRESH3 = 0x0600; //0.5  0x9000
      THRESH4 = 0x0800; //0.7  0xA000
      THRESH5 = 0x0A00; //0.9  0xD000
      */

//  Add RTI/interrupt initializations here

      ATDCTL2 = 0xC0;
      ATDCTL3 = 0x10;
      ATDCTL4 = 0x05;
      ATDCTL5 = 0x10;
     
      RTICTL = RTICTLI; // establish RTI interrupt rate  8.
      rticnt = 0;
      CRGINT = CRGINT | 0x80; // enable RTI interrupts

}
	 		  			 		  		
 
// Main (non-terminating loop)
 
void main(void) {
	initializations(); 		  			 		  		
	EnableInterrupts;


  for(;;) {


// Main program loop (state machine)
// Start of main program-driven polling loop

	 	   			 		  			 		  		
//  If the "tenth second" flag is set, then
//    - clear the "tenth second" flag
//    - if "run/stop" flag is set, then
//       - initiate ATD coversion sequence
//       - apply thresholds to converted values
//       - determine 5-bit bar graph bit settings for each input channel
//       - transmit 10-bit data to external shift register
//    - endif
//  Endif
    

    if (tenthsec){
      tenthsec = 0;
      if (runstp){   
        // - initiate ATD coversion sequence
        //ATDCTL2 = 0x80; 
        //ATDCTL3 = 0x10;
        //ATDCTL4 = 0x05; 
        
        //ADC_convert();
        
        ATDCTL5 = 0x10; // sets up ADC to perform a single conversion,
        while(ATDSTAT1_CCF0 != 1){}
        
        //ATDCTL5 = 0x11;
        //while(ATDSTAT1_CCF1 != 1){}
        while(ATDSTAT0_SCF != 1){}
        
        // - apply thresholds to converted values
        
        //ATDCTL5 = 0x10;
        
        
        if (ATDDR0 >= THRESH5){
           output1 = 5;
        }else if (ATDDR0 >= THRESH4){
           output1 = 4;
        }else if (ATDDR0 >= THRESH3){
           output1 = 3;
        }else if (ATDDR0 >= THRESH2){
           output1 = 2;
        }else if (ATDDR0 >= THRESH1){
           output1 = 1;
        }else{
           output1 = 0;
        }
        
        
        if (ATDDR1 >= THRESH5){
           output2 = 5;
        }else if (ATDDR1 >= THRESH4){
           output2 = 4;
        }else if (ATDDR1 >= THRESH3){
           output2 = 3;
        }else if (ATDDR1 >= THRESH2){
           output2 = 2;
        }else if (ATDDR1 >= THRESH1){
           output2 = 1;
        }else{
           output2 = 0;
        }
        //serial_data = (output1 >> 5) | output2;
        
        for (i = 0; i < 5; i++){
          PTT_PTT4 = 0;
          
          if (output1 > 0){
            PTT_PTT3 = 1;
            output1--;  
          }else{
            PTT_PTT3 = 0;
          }
          PTT_PTT4 = 1;
        }
        
        for (i = 0; i < 5; i++){
          PTT_PTT4 = 0;
          
          if (output2 > 0){
            PTT_PTT3 = 1;
            output2--;  
          }else{
            PTT_PTT3 = 0;
          }
          PTT_PTT4 = 1;
        }
        
         
      }
    }

	 	   			 		  			 		  		
//  If the left pushbutton ("stop BGD") flag is set, then:
//    - clear the left pushbutton flag
//    - clear the "run/stop" flag (and "freeze" BGD)
//    - turn on left LED/turn off right LED (on docking module)
//  Endif
   	if (leftpb == 1){
   	  leftpb = 0;
   	  runstp = 0;
   	  PTT_PTT1 = 1;
   	  PTT_PTT0 = 0;
   	}

//  If the right pushbutton ("start BGD") flag is set, then
//    - clear the right pushbutton flag
//    - set the "run/stop" flag (enable BGD updates)
//    - turn off left LED/turn on right LED (on docking module)
//  Endif
	 	if (rghtpb == 1){
	 	  rghtpb = 0;
	 	  runstp = 1;
	 	  PTT_PTT1 = 0;
	 	  PTT_PTT0 = 1;
	 	}
	 	
	 	
	 	

  } /* loop forever */
  
}  /* make sure that you never leave main */




// ***********************************************************************                       
// RTI interrupt service routine: rti_isr
//
//  Initialized for 5-10 ms (approx.) interrupt rate - note: you need to
//    add code above to do this
//
//  Samples state of pushbuttons (PAD7 = left, PAD6 = right)
//
//  If change in state from "high" to "low" detected, set pushbutton flag
//     leftpb (for PAD7 H -> L), rghtpb (for PAD6 H -> L)
//     Recall that pushbuttons are momentary contact closures to ground
//
//  Also, keeps track of when one-tenth of a second's worth of RTI interrupts
//     accumulate, and sets the "tenth second" flag         	   			 		  			 		  		
 
interrupt 7 void RTI_ISR( void)
{
 // set CRGFLG bit to clear RTI device flag
  	CRGFLG = CRGFLG | 0x80; 
	  
	  
	  rticnt++;
	  
	  if (rticnt >= 12){
	     rticnt = 0;
	     tenthsec = 1;
	  }
	  
	  if(PORTAD0_PTAD7 == 0) { // check left pushbutton
      if(prevpb == 1) {
        leftpb = 1;
      }
    }
    prevpb = PORTAD0_PTAD7;
    if(PORTAD0_PTAD6 == 0) { // check right pushbutton
      if(prevpb == 1) {
        rghtpb = 1;
      }
     }
    prevpb = PORTAD0_PTAD6; 
	  

}


// ***********************************************************************
// Character I/O Library Routines for 9S12C32 (for debugging only)
// ***********************************************************************
// Name:         inchar
// Description:  inputs ASCII character from SCI serial port and returns it
// ***********************************************************************
char  inchar(void) {
  /* receives character from the terminal channel */
        while (!(SCISR1 & 0x20)); /* wait for RDR input */
    return SCIDRL;
 
}

// ***********************************************************************
// Name:         outchar
// Description:  outputs ASCII character passed in outchar()
//                  to the SCI serial port
// ***********************************************************************/
void outchar(char ch) {
  /* transmits a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for TDR empty */
    SCIDRL = ch;
}

