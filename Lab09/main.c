/*
***********************************************************************
 ECE 362 - Experiment 9 - Fall 2016
***********************************************************************
	 	   			 		  			 		  		
 Completed by: < Chia-Hua Peng >
               < 2220-P >
               < Chia-Hua Peng >
               < 11/9/2016 >


 Academic Honesty Statement:  In entering my name above, I hereby certify
 that I am the individual who created this HC(S)12 source file and that I 
 have not copied the work of any other student (past or present) while 
 completing it. I understand that if I fail to honor this agreement, I will 
 receive a grade of ZERO and be subject to possible disciplinary action.
***********************************************************************

 The objective of this experiment is to implement an analog signal sampling
 and reconstruction application that allows the user to efficiently cycle
 through different input and output sampling frequencies.

 The following design kit resources will be used:

 - left pushbutton (PAD7): cycles through input sampling frequency choices
                           (5000 Hz, 10,000 Hz, and 20,000 Hz)

 - right pushbutton (PAD6): cycles through output sampling frequency choices
                           (23,529 Hz, 47,059 Hz, and 94,118 Hz)

 - LCD: displays current values of input and output sampling frequencies
 - Shift Register: performs SPI -> parallel conversion for LCD interface

***********************************************************************
*/
 
#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

/* All funtions after main should be initialized here */
char inchar(void);
void outchar(char);
void fdisp(void);
void shiftout(char);
void lcdwait(void);
void send_byte(char);
void send_i(char);
void chgline(char);
void print_c(char);
void pmsglcd(char[]);

/*  Variable declarations */ 	   			 		  			 		       
char leftpb	= 0;  // left pushbutton flag
char rghtpb	= 0;  // right pushbutton flag
char prevpbl = 0;  // previous pushbutton state
char prevpbr = 0;

/* LCD COMMUNICATION BIT MASKS */
#define RS 0x04		// RS pin mask (PTT[2])
#define RW 0x08		// R/W pin mask (PTT[3])
#define LCDCLK 0x10	// LCD EN/CLK pin mask (PTT[4])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F	// LCD initialization command
#define LCDCLR 0x01	// LCD clear display command
#define TWOLINE 0x38	// LCD 2-line enable command
#define CURMOV 0xFE	// LCD cursor move instruction
#define LINE1 0x80	// LCD line 1 cursor position
#define LINE2 0xC0	// LCD line 2 cursor position


// my own declarations


int incycle = 0;
int outcycle = 0;
int tmp = 0;
int ct = 0;
int ctout = 0;

/*	 	   		
***********************************************************************
 Initializations
***********************************************************************
*/

void  initializations(void) {

/* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; //; disengage PLL from system
  PLLCTL = PLLCTL | 0x40; //; turn on PLL
  SYNR = 0x02;            //; set PLL multiplier
  REFDV = 0;              //; set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; //; engage PLL


/* Disable watchdog timer (COPCTL register) */
  COPCTL = 0x40   ; //COP off; RTI and COP stopped in BDM-mode

/* Initialize asynchronous serial port (SCI) for 9600 baud, no interrupts */
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port

         
/* Add additional port pin initializations here */
  DDRM = 0xFF;
  DDRT = 0xFF;

/* Initialize the SPI to 6 Mbs */
  SPIBR = 0x10;
  SPICR1 = 0x51;//0x51;
  SPICR2 = 0x00;
	 	   			 		  			 		  		
/* Initialize digital I/O port pins */
  DDRAD = 0; 		//program port AD for input mode
  ATDDIEN = 0xC0;

/* Initialize the LCD
     - pull LCDCLK high (idle)
     - pull R/W' low (write state)
     - turn on LCD (LCDON instruction)
     - enable two-line mode (TWOLINE instruction)
     - clear LCD (LCDCLR instruction)
     - wait for 2ms so that the LCD can wake up     
*/ 
   PTT_PTT4 = 1; // pull LCDCLK high
   PTT_PTT3 = 0; // pull R/W' low
   send_i(LCDON);
   send_i(TWOLINE);
   send_i(LCDCLR);
   
   lcdwait();

/* Initialize RTI for 2.048 ms interrupt rate */	
   RTICTL = 0x1F;          // 2.048 ms
   CRGINT = CRGINT | 0x80; // enable RTI interrupts	

/* Initialize TIM Ch 7 (TC7) for periodic interrupts every 1.000 ms
     - enable timer subsystem
     - set channel 7 for output compare
     - set appropriate pre-scale factor and enable counter reset after OC7
     - set up channel 7 to generate 1 ms interrupt rate
     - initially disable TIM Ch 7 interrupts      
*/
     TSCR1 = 0x80; // enable timer subsystem
     TSCR2 = 0x0C; // set pre-scale factor to 16 and enable counter reset after OC7  24M / 16 = 1.5M
     TIOS = 0x80;  // set channel 7 for output compare
     TC7 = 1500;   // set up ch 7 to generate 1 ms interrupt rate  1500/1.5M = 1ms
     TIE = 0x80;   // initially disable TIM ch 7 interrupts
     
     
// ATD conversion initializations
   ATDCTL2 = 0x80;
   ATDCTL3 = 0x10;
   ATDCTL4 = 0x85;
   
// PWM initialization
   PWME = 0x01;    // enable channel 1
   PWMPOL = 0x01;  // active high
   PWMCLK = 0x00;  // use clock A
   PWMPER0 = 0xFF;  // 255
   PWMDTY0 = 0x7F;  // 50% duty cycle
   
   PWMPRCLK = 0x00; // prescale clock select     clock A = (24M / 255)
                    // clock A / 2^PWMPRCLK = output sampling freq
   DDRP = 0x01;
   MODRR = 0x01;
   PWMCTL = 0x00;
   PWMCAE = 0x00;
   
   
        
	      
}

/*	 		  			 		  		
***********************************************************************
 Main
***********************************************************************
*/

void main(void) {
  DisableInterrupts;
	initializations(); 		  			 		  		
	EnableInterrupts;



  for(;;){
  
    if (leftpb){
      leftpb = 0;
      
      if (incycle == 0){
        incycle = 1;
      } else if (incycle == 1){
        incycle = 2;
      } else if (incycle == 2){
        incycle = 0;
      }
      
      if (incycle == 0){
        TC7 = 1500 / 5; // 5000 Hz
      } else if (incycle == 1){
        TC7 = 1500 / 10; // 10000 Hz
      } else if (incycle == 2){
        TC7 = 1500 / 20; // 20000 Hz
      }
      fdisp();
      
    }
  
  
    if (rghtpb){
      rghtpb = 0;

      if (outcycle == 0){
         outcycle = 1;
      }else if (outcycle == 1){
         outcycle = 2;
      }else if (outcycle == 2){
         outcycle = 0;
      }
      
      if (outcycle == 0){
         PWMPRCLK = 0x00;
      }else if (outcycle == 1){
         PWMPRCLK = 0x01;
      }else if (outcycle == 2){
         PWMPRCLK = 0x02;
      }
      fdisp(); 
           
    } 
  

    
  } /* loop forever */
   
}  /* do not leave main */




/*
***********************************************************************
 RTI interrupt service routine: RTI_ISR

  Initialized for 2.048 ms interrupt rate

  Samples state of pushbuttons (PAD7 = left, PAD6 = right)

  If change in state from "high" to "low" detected, set pushbutton flag
     leftpb (for PAD7 H -> L), rghtpb (for PAD6 H -> L)
     Recall that pushbuttons are momentary contact closures to ground
***********************************************************************
*/

interrupt 7 void RTI_ISR(void)
{
  	// clear RTI interrupt flag
  	CRGFLG = CRGFLG | 0x80; ; 
  	
  	
  	
  	if(PORTAD0_PTAD7 == 0) { // check left pushbutton
      if(prevpbl == 1) {
        leftpb = 1;
      }
    }
    prevpbl = PORTAD0_PTAD7;
    if(PORTAD0_PTAD6 == 0) { // check right pushbutton
      if(prevpbr == 1) {
        rghtpb = 1;
      }
     }
    prevpbr = PORTAD0_PTAD6;
}

/*
***********************************************************************
  TIM interrupt service routine
  used to initiate ATD samples (on Ch 0 and Ch 1)	 		  			 		  		
***********************************************************************
*/

interrupt 15 void TIM_ISR(void)
{
  int bonus = 1;
  
        // clear TIM CH 7 interrupt flag         
 	TFLG1 = TFLG1 | 0x80; 
 	
 	ATDCTL5 = 0x10;
 	
 	while (ATDSTAT1_CCF0 != 1){
 	}
 	
 	
 	bonus = (ATDDR0>>8) / 25 + 1;
 	if (bonus < 1){
 	  bonus = 1;
 	}
 	if (bonus > 25){
 	  bonus = 25; 
 	}
 	
 	ct = ct + bonus;
 	
 	if (ct >= 255){
 	  ct = 0;
 	}
 	
 	tmp = ct * (ATDDR1>>8);
 	PWMDTY0 = tmp >> 8;

}

/*
***********************************************************************
  fdisp: Display "ISF = NNNNN Hz" on the first line of the LCD and display 
         and "OSF = MMMMM Hz" on the second line of the LCD (where NNNNN and
         MMMMM are the input and output sampling frequencies, respectively         
***********************************************************************
*/

void fdisp()
{
  
  
  send_i(LCDCLR);
  chgline(LINE1);
  
  /*
  print_c(((ATDDR0/100000) %10) + 48);
  print_c(((ATDDR0/100000) %10) + 48);
  print_c(((ATDDR0/10000) %10) + 48);
  print_c(((ATDDR0/1000) %10) + 48);
  print_c(((ATDDR0/100) %10) + 48);
  print_c(((ATDDR0/10) %10) + 48);
  print_c(((ATDDR0/1) %10) + 48);
  */
  
  pmsglcd("ISF: ");
  if (incycle % 3 == 0){
    pmsglcd("05000");
  } else if (incycle % 3 == 1){
    pmsglcd("10000");
  } else if (incycle % 3 == 2){
    pmsglcd("20000");
  }
  pmsglcd(" Hz");
  chgline(LINE2);
  pmsglcd("OSF: ");
  if (outcycle % 3 == 0){
    pmsglcd("94118");
  }else if (outcycle % 3 == 1){
    pmsglcd("47059");
  }else{
    pmsglcd("23529");
  }
  pmsglcd(" Hz");
  
  
}

/*
***********************************************************************
  shiftout: Transmits the character x to external shift 
            register using the SPI.  It should shift MSB first.  
             
            MISO = PM[4]
            SCK  = PM[5]
***********************************************************************
*/
 
void shiftout(char x)

{
 
  // read the SPTEF bit, continue if bit is 1
  // write data to SPI data register
  // wait for 30 cycles for SPI data to shift out 
  int j;
  // read the SPTEF bit, continue if bit is 1
  // write data to SPI data register
  // wait for 30 cycles for SPI data to shift out
  while(SPISR_SPTEF==0){
  }  
  SPIDR = x;
  //while(SPISR_SPTEF==0){
  //}
  //lcdwait();
  
  for (j = 0; j < 30; j++){
  }
}

/*
***********************************************************************
  lcdwait: Delay for approx 2 ms
***********************************************************************
*/

void lcdwait()
{
  asm{
      PSHX
      PSHC
      LDX #16000
LP:   
      DBNE X,LP
      PULC
      PULX
  }
}

/*
*********************************************************************** 
  send_byte: writes character x to the LCD
***********************************************************************
*/

void send_byte(char x)
{
     // shift out character
     // pulse LCD clock line low->high->low
     // wait 2 ms for LCD to process data
     shiftout(x);
     PTT_PTT4 = 0;     
     PTT_PTT4 = 1;       
     PTT_PTT4 = 0;
     lcdwait();
}

/*
***********************************************************************
  send_i: Sends instruction byte x to LCD  
***********************************************************************
*/

void send_i(char x)
{
        // set the register select line low (instruction data)
        // send byte
    PTT_PTT2 = 0; // register select -> low    
    send_byte(x);
}

/*
***********************************************************************
  chgline: Move LCD cursor to position x
  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables
***********************************************************************
*/

void chgline(char x)
{
    send_i(CURMOV);
    send_i(x);
}

/*
***********************************************************************
  print_c: Print (single) character x on LCD            
***********************************************************************
*/
 
void print_c(char x)
{
    PTT_PTT2 = 1;
    send_byte(x);
}

/*
***********************************************************************
  pmsglcd: print character string str[] on LCD
***********************************************************************
*/

void pmsglcd(char str[])
{
    int i = 0;
    while(str[i] != 0){
      PTT_PTT2 = 1;
      print_c(str[i]);
      i++;
    }
}


/*
***********************************************************************
 Character I/O Library Routines for 9S12C32 (for debugging only)
***********************************************************************
 Name:         inchar
 Description:  inputs ASCII character from SCI serial port and returns it
 Example:      char ch1 = inchar();
***********************************************************************
*/

char inchar(void) {
  /* receives character from the terminal channel */
        while (!(SCISR1 & 0x20)); /* wait for input */
    return SCIDRL;
}

/*
***********************************************************************
 Name:         outchar
 Description:  outputs ASCII character x to SCI serial port
 Example:      outchar('x');
***********************************************************************
*/

void outchar(char x) {
  /* sends a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
    SCIDRL = x;
}