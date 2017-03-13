/*
************************************************************************
 ECE 362 - Mini-Project C Source File - Fall 2016
 
 
 Speaker Test
 
***********************************************************************
	 	   			 		  			 		  		
 Team ID: < 15 >

 Project Name: < Mini Music Tap >

 Team Members:

   - Team/Doc Leader: < Suprith Ramanan >      Signature: Suprith Ramanan
   
   - Software Leader: < Hengyi Lin >      Signature: Hengyi Lin

   - Interface Leader: < Paul Swartz >     Signature: Paul Swartz

   - Peripheral Leader: < Chia-Hua Peng >    Signature: Chia-Hua Peng


 Academic Honesty Statement:  In signing above, we hereby certify that we 
 are the individuals who created this HC(S)12 source file and that we have
 not copied the work of any other student (past or present) while completing 
 it. We understand that if we fail to honor this agreement, we will receive 
 a grade of ZERO and be subject to possible disciplinary action.

***********************************************************************

 The objective of this Mini-Project is to design an exciting arcade-style music game, where the goal is to press specific combinations of buttons in sync with the popular theme song, "Super Mario Bros."


***********************************************************************

 List of project-specific success criteria (functionality that will be
 demonstrated):

 1. Use of SPI module to shift out data to an LCD display and to a large number of LEDs.

 2. Use of ATD module to modify the difficulty of the game and the speed of the music.

 3. Use of PWM module to output music on a speaker.

 4. Use of TIM module to show a countdown timer on the LCD display. 

 5. Use of RTI to control the speed of the song and to control the length of each musical note.
 
***********************************************************************

  Date code started: < Nov 16th >

  Update history (add an entry every time a significant change is made):

  Date: < Nov 16th >  Name: < Chia-Hua Peng >   Update: < PWM audio output >

  Date: < Nov 18th >  Name: <  Chia-Hua Peng and Hengyi Lin >   Update: < Coded Super Mario Bros theme music >

  Date: < Nov 19th >  Name: < Chia-Hua Peng and Hengyi Lin >   Update: < Created algorithms for the LED shift patterns >

 Date: < Nov 21st >  Name: < Chia-Hua Peng and Hengyi Lin >   Update: < Pushbuttons, LCD display, and countdown timer >

 Date: < Nov 22nd >  Name: < Chia-Hua Peng and Hengyi Lin >   Update: < Matching Algorithm, Player score display >


***********************************************************************
*/

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

/* All functions after main should be initialized here */
char inchar(void);
void outchar(char x);

/* music functions */
void pulse_wait(void);

/* lcd functions */
void pmsglcd(char str[]);
void print_c(char x);
void diff_disp(void);
void game_disp(void);
void lcd_shiftout(char x);
void send_byte(char x);
void send_i(char x);
void chgline(char x);
void lcdwait(void);

/* led functions */
void led_shiftout(char led);


void debug(void);
void outmsg(char[]);


/* Variable declarations */
int ratecnt = 0;
char shift_flag = 0;
char rtiflag = 0;
char onems = 0;
char onemscnt = 0;
char onesec = 0;
int oneseccnt = 0;
char runstp = 0;

char leftLED = 0;
char midLED = 0;
char rghtLED = 0;


char runstppb = 0;
char leftpb = 0;
char midpb = 0;
char rghtpb = 0;

char prevpbrun = 1;
char prevpbl = 1;
char prevpbr = 1;
char prevpbm = 1;

int gameover = 0;
int difficulty = 600;
int timer = 30;
int score = 0;

int rticnt = 0;

char goodbad = 0;
   	   			 		  			 		       

/* Special ASCII characters */
#define CR 0x0D		// ASCII return 
#define LF 0x0A		// ASCII new line 

/* LCD COMMUNICATION BIT MASKS (note - different than previous labs) */
#define RS 0x10		// RS pin mask (PTT[4])
#define RW 0x20		// R/W pin mask (PTT[5])
#define LCDCLK 0x40	// LCD EN/CLK pin mask (PTT[6])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F	// LCD initialization command
#define LCDCLR 0x01	// LCD clear display command
#define TWOLINE 0x38	// LCD 2-line enable command
#define CURMOV 0xFE	// LCD cursor move instruction
#define LINE1 0x80	// LCD line 1 cursor position
#define LINE2 0xC0	// LCD line 2 cursor position


/*Music Tones -- TIM module interrupt frequency is the frequency of the audio signal*/
// TC7 = 1500 -> 1ms -> 1kHz
// TC7 = 15000 -> 10ms -> 100 Hz  

int TC7_DO_L = 5734/2;	
int TC7_DOA_L = 5412/2;
int TC7_RE_L = 5108/2;
int TC7_REA_L = 4821/2;
int TC7_MI_L = 4551/2;
int TC7_FA_L = 4295/2;
int TC7_FAA_L = 4054/2;
int TC7_SO_L = 3827/2;
int TC7_SOA_L = 3612/2;
int TC7_LA_L = 3409/2;
int TC7_LAA_L = 3218/2;
int TC7_TI_L = 3037/2;

int TC7_DO_M = 2867/2;
int TC7_DOA_M = 2706/2;
int TC7_RE_M = 2554/2;
int TC7_REA_M = 2411/2;
int TC7_MI_M = 2275/2;
int TC7_FA_M = 2148/2;
int TC7_FAA_M = 2027/2;
int TC7_SO_M = 1913/2;
int TC7_SOA_M = 1806/2;
int TC7_LA_M = 1705/2;
int TC7_LAA_M = 1609/2;
int TC7_TI_M = 1519/2;

int TC7_DO_H = 1433/2;
int TC7_DOA_H = 1353/2;
int TC7_RE_H = 1277/2;
int TC7_REA_H = 1205/2;
int TC7_MI_H = 1138/2;
int TC7_FA_H = 1074/2;
int TC7_FAA_H = 1014/2;
int TC7_SO_H = 957/2;
int TC7_SOA_H = 903/2;
int TC7_LA_H = 852/2;
int TC7_LAA_H = 804/2;
int TC7_TI_H = 759/2;

int TC7_PAUSE = 65535;

/*Music Array*/
#define MSIZE 103
char duration[MSIZE];
int tone[MSIZE];
int i_music = 0;

	 	   		
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

/* Initialize asynchronous serial port (SCI) for 9600 baud, interrupts off initially */
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port

/* Initialize peripherals */
  
  // additional port pins
  DDRM = 0xFF;
  DDRT = 0xFF;
  
  DDRAD = 0; //program port AD for input mode
  ATDDIEN = 0xF0;
  
  // PWM initialization
  PWME = 0x00;    // enable channel 0
  PWMPOL = 0x01;  // active high
  PWMCLK = 0x00;  // use clock A
  PWMPER0 = 0xFF;  // 255
  PWMDTY0 = 0;
   
  PWMPRCLK = 0x00; // prescale clock select     clock A = (24M / 255)
                    // clock A / 2^PWMPRCLK = output sampling freq
  DDRP = 0x01;
  MODRR = 0x01;
  PWMCTL = 0x00;
  PWMCAE = 0x00;
  
  
  // TIM initialization
  TSCR1 = 0x80; // enable timer subsystem
  TSCR2 = 0x0C; // set pre-scale factor to 16 and enable counter reset after OC7  24M / 16 = 1.5M
  TIOS = 0x80;  // set channel 7 for output compare
  TC7 = TC7_MI_M;   // set up ch 7 to generate 1 ms interrupt rate  1500/1.5M = 1ms
  TIE = 0x00;   // initially disable TIM ch 7 through 5 interrupts 
  
  
  // ATD initialization
  ATDCTL2 = 0x80;
  ATDCTL3 = 0x08;
  ATDCTL4 = 0x85;
  
  
  
  // SPI initialization
  SPIBR = 0x10;
  SPICR1 = 0x50; // most significant bit first
  SPICR2 = 0x00;
  PTT_PTT5 = 1;  // slave select pin (1-LED, 0-LCD)
  
  
  // LCD initialization
   PTT_PTT4 = 1; // pull LCDCLK high
   PTT_PTT3 = 0; // pull R/W' low
   send_i(LCDON);
   send_i(TWOLINE);
   send_i(LCDCLR);
   
   lcdwait(); 
  
  
          
/* Initialize interrupts */
	RTICTL = 0x10;          // 0.128 ms
  CRGINT = CRGINT | 0x80; // enable RTI interrupts      
	      
	
	// Other variables
	gameover = 0;
	runstp = 0;
	timer = 27500;
	score = 0;
	goodbad = 0;
	      
/* Set up music*/
  //Create array of audio frequencies for hard-coding the music
  tone[0] = TC7_MI_M;
  tone[1] = TC7_MI_M;
  tone[2] = TC7_MI_M;
  tone[3] = TC7_PAUSE; 
  tone[4] = TC7_DO_M;
  tone[5] = TC7_MI_M;
  tone[6] = TC7_SO_M;
  tone[7] = TC7_PAUSE;
  tone[8] = TC7_SO_L;
  tone[9] = TC7_PAUSE;
  tone[10] = TC7_DO_M;
  tone[11] = TC7_PAUSE;
  tone[12] = TC7_SO_L;
  tone[13] = TC7_PAUSE;
  tone[14] = TC7_MI_L;
  tone[15] = TC7_PAUSE;
  tone[16] = TC7_LA_L;
  tone[17] = TC7_TI_L;
  tone[18] = TC7_PAUSE; 
	tone[19] = TC7_LAA_L;
	tone[20] = TC7_LA_L;
	
	tone[21] = TC7_SO_L;
	tone[22] = TC7_MI_M;
	tone[23] = TC7_SO_M;
	tone[24] = TC7_LA_M;
	tone[25] = TC7_FA_M;
	tone[26] = TC7_SO_M;
	tone[27] = TC7_PAUSE;
	tone[28] = TC7_MI_M;
  tone[29] = TC7_DO_M;
  tone[30] = TC7_RE_M;
  tone[31] = TC7_TI_L;
  tone[32] = TC7_PAUSE;
  tone[33] = TC7_DO_M;
  tone[34] = TC7_PAUSE;
  tone[35] = TC7_SO_L;
  tone[36] = TC7_PAUSE;
  tone[37] = TC7_MI_L;
  tone[38] = TC7_PAUSE;
  tone[39] = TC7_LA_L;
  tone[40] = TC7_TI_L;
  tone[41] = TC7_PAUSE;
  tone[42] = TC7_LAA_L;
  tone[43] = TC7_LA_L;
  
  
  tone[44] = TC7_SO_L;
  tone[45] = TC7_MI_M;  
  tone[46] = TC7_SO_M;
  tone[47] = TC7_LA_M;
  tone[48] = TC7_FA_M;
  tone[49] = TC7_SO_M;
  tone[50] = TC7_PAUSE;
  tone[51] = TC7_MI_M;
  tone[52] = TC7_DO_M;
  tone[53] = TC7_RE_M;
  tone[54] = TC7_TI_L;
  tone[55] = TC7_PAUSE;
  tone[56] = TC7_PAUSE;
  tone[57] = TC7_SO_M;
  tone[58] = TC7_FAA_M;
  tone[59] = TC7_FA_M;
  tone[60] = TC7_REA_M;
  tone[61] = TC7_MI_M;
  tone[62] = TC7_PAUSE;
  tone[63] = TC7_SOA_L;
  tone[64] = TC7_LA_L;
  tone[65] = TC7_DO_M;
  tone[66] = TC7_PAUSE;
  tone[67] = TC7_LA_L;
  tone[68] = TC7_DO_M;
  tone[69] = TC7_RE_M; 
  
  tone[70] = TC7_PAUSE;
  tone[71] = TC7_SO_M;
  tone[72] = TC7_FAA_M;
  tone[73] = TC7_FA_M;
  tone[74] = TC7_REA_M;
  tone[75] = TC7_MI_M;
  tone[76] = TC7_PAUSE;
  tone[77] = TC7_DO_H; 
  tone[78] = TC7_DO_H;
  tone[79] = TC7_DO_H;
  tone[80] = TC7_PAUSE;
  tone[81] = TC7_PAUSE;
  tone[82] = TC7_SO_M;
  tone[83] = TC7_FAA_M;
  tone[84] = TC7_FA_M;
  tone[85] = TC7_REA_M; 
  tone[86] = TC7_MI_M;
  tone[87] = TC7_PAUSE;
  tone[88] = TC7_SOA_L;
  tone[89] = TC7_LA_L;
  tone[90] = TC7_DO_M;
  tone[91] = TC7_PAUSE;
  tone[92] = TC7_LA_L;
  tone[93] = TC7_DO_M;
  tone[94] = TC7_RE_M;
  tone[95] = TC7_PAUSE;
  tone[96] = TC7_REA_M;
  tone[97] = TC7_PAUSE;
  tone[98] = TC7_RE_M;
  tone[99] = TC7_PAUSE;
  tone[100] = TC7_DO_M;
  tone[101] = TC7_PAUSE;
  tone[102] = TC7_PAUSE;
  
  //  quaver/eighth note - set to 6
  //  demiquaver/ sixteenth note - set to 3
  //  One of the Triplet - set to 4

	//Hard code an array containing the duration of each note 
	duration[0] = 3;
	duration[1] = 6;
  duration[2] = 3;
  duration[3] = 3;
  duration[4] = 3;
  duration[5] = 6;
  duration[6] = 6;
  duration[7] = 6;
  duration[8] = 6;
  duration[9] = 6;
  duration[10] = 6;
	duration[11] = 3;
  duration[12] = 3;
  duration[13] = 6;
  duration[14] = 6;
  duration[15] = 3;
  duration[16] = 6;
  duration[17] = 3;
  duration[18] = 3;
  duration[19] = 3; 
  duration[20] = 6;
  
  
	duration[21] = 4;
  duration[22] = 4;
  duration[23] = 4;
  duration[24] = 6;
  duration[25] = 3;
  duration[26] = 3;
  duration[27] = 3;
  duration[28] = 6;
  duration[29] = 3; 
  duration[30] = 3;
	duration[31] = 6;
  duration[32] = 3;
  duration[33] = 6;
  duration[34] = 3;
  duration[35] = 3;
  duration[36] = 6;
  duration[37] = 6;
  duration[38] = 3;
  duration[39] = 6; 
  duration[40] = 3;
	duration[41] = 3;
  duration[42] = 3;
  duration[43] = 6;
  
  
  duration[44] = 4;
  duration[45] = 4;
  duration[46] = 4;
  duration[47] = 6;
  duration[48] = 3;
  duration[49] = 3; 
  duration[50] = 3;
	duration[51] = 6;
  duration[52] = 3;
  duration[53] = 3;
  duration[54] = 6;
  duration[55] = 3;
  duration[56] = 6;
  duration[57] = 3;
  duration[58] = 3;
  duration[59] = 3; 
  duration[60] = 6;
	duration[61] = 3;
  duration[62] = 3;
  duration[63] = 3;
  duration[64] = 3;
  duration[65] = 3;
  duration[66] = 3;
  duration[67] = 3;
  duration[68] = 3;
  duration[69] = 3;
   
  duration[70] = 6;
	duration[71] = 3;
  duration[72] = 3;
  duration[73] = 3;
  duration[74] = 6;
  duration[75] = 3;
  duration[76] = 3;
  duration[77] = 6;
  duration[78] = 3;
  duration[79] = 6; 
  duration[80] = 6;
	duration[81] = 6;
  duration[82] = 3;
  duration[83] = 3;
  duration[84] = 3;
  duration[85] = 6;
  duration[86] = 3;
  duration[87] = 3;
  duration[88] = 3;
  duration[89] = 3; 
  duration[90] = 3;
	duration[91] = 3;
  duration[92] = 3;
  duration[93] = 3;
  duration[94] = 3;
  duration[95] = 6;
  duration[96] = 6;
  duration[97] = 3;
  duration[98] = 6;
  duration[99] = 3; 
  duration[100] = 6;
  duration[101] = 6;
  duration[102] = 12;
  
     
}

	 		  			 		  		
/*	 		  			 		  		
***********************************************************************
Main
***********************************************************************
*/
void main(void) {
  int i = 0;
  
  DisableInterrupts
	initializations(); 		  			 		  		
	EnableInterrupts;

 for(;;) {
  
/* < start of your main loop > */

   // toggle run/stop if game is not over and the runstp pb is pressed
   if (runstppb){
      runstppb = 0;
      
      if (!gameover){			//Deactivate PWM audio output when game is stopped
         if (runstp){
            runstp = 0;
        
            PWME = 0x00;		//Set duty of PWM to zero. Disable output
            PWMDTY0 = 0;
            TIE = 0x00;
          }else{			//Reactivate PWM music output when game is running
            runstp = 1;
        
            PWME = 0x01;		//Set duty enable output
            TIE = 0x80;
          }
      }
      
   }
   
   // game NOT over and stop -> do ATD conversion
   if (!runstp && !gameover){
     ATDCTL5 = 0x10;			//Read ATD input
     while(ATDSTAT1_CCF0 != 1){		//wait for ATD read
     }
     difficulty = ATDDR0H*2;		//Set difficulty (speed of music) to ATD result
     if (difficulty < 256){		
       difficulty = 256;
     }
   }
   
   // game is over
   if (gameover){			//Deactivate PWM audio output and set duty cycle to 0 at end of game
     PWMDTY0 = 0;
     PWME = 0x00;
     TIE = 0x00;
   }


	
   if (!gameover && runstp == 1 && shift_flag == 1){		//At the end of the duration of each musical note, shift to the next note in the array
      shift_flag = 0;				
      
      TC7 = tone[i_music];
     
      
      
      if (i == duration[i_music]){
        i = 0;
        i_music = (i_music+1) % MSIZE;				//Change music notes. Upon reaching end of song, loop to the beginning.
        pulse_wait();						//Add a short delay after each note
      }
      
      
      
      if (i == 0){
        
        // check logic
        // shift LED logic
        // next on the Most Significant Bit
        
        
        if (leftpb){						//Increment the score by 5 if the pushbutton is pressed while its corresponding LED is on
          leftpb = 0;
          if (leftLED & 0x01 == 0x01){ // match  pb-1 led-1
            score += 5;
            goodbad = (goodbad & 0b11011111) | 0b00010000;	//Unused (Debugging Purpose)
          }else{ // not match pb-1 led-0			//Decrement the score if the pushbutton is pressed at an incorrect time
            score--;
            goodbad = (goodbad & 0b11101111) | 0b00100000;	//Unused (Debugging Purpose)
          }
          
        }else{
          if (leftLED & 0x01 == 0x01){ // not match  pb-0 led-1		//Decrement the score if the pushbutton is not pressed when the LED is illuminated
            score--;
            goodbad = (goodbad & 0b11101111) | 0b00100000;	//Unused (Debugging Purpose)
          }else{ // match pb-0 led-0					
            goodbad = goodbad & 0b11001111;			//Unused (Debugging Purpose)
          }
        }
        
        if (midpb){						
          midpb = 0;
          if (midLED & 0x01 == 0x01){ // match  pb-1 led-1	//Increment the score by 5 if the pushbutton is pressed while its corresponding LED is on
            score += 5;
            goodbad = (goodbad & 0b11110111) | 0b00000100;	//Unused (Debugging Purpose)
          }else{ // not match pb-1 led-0			//Decrement the score if the pushbutton is pressed at an incorrect time
            score--;
            goodbad = (goodbad & 0b11111011) | 0b00001000;	//Unused (Debugging Purpose)
          }
          
        }else{
          if (midLED & 0x01 == 0x01){ // not match  pb-0 led-1		//Decrement the score if the pushbutton is not pressed when the LED is illuminated
            score--;
            goodbad = (goodbad & 0b11111011) | 0b00001000;	//Unused (Debugging Purpose)
          }else{ // match pb-0 led-0
            // nothing
            goodbad = goodbad & 0b11110011;			//Unused (Debugging Purpose)
          }
        }
        
        
        if (rghtpb){
          rghtpb = 0;
          if (rghtLED & 0x01 == 0x01){ // match  pb-1 led-1		//Increment the score by 5 if the pushbutton is pressed while its corresponding LED is on
            score += 5;
            goodbad = (goodbad & 0b11111101) | 0b00000001; // right good		//Unused (Debugging Purpose)
          }else{ // not match pb-1 led-0				//Decrement the score if the pushbutton is pressed at an incorrect time
            score--;
            goodbad = (goodbad & 0b11111110) | 0b00000010; // right bad		//Unused (Debugging Purpose)
          }
          
        }else{
          if (rghtLED & 0x01 == 0x01){ // not match  pb-0 led-1			//Decrement the score if the pushbutton is not pressed when the LED is illuminated
            score--;
            goodbad = (goodbad & 0b11111110) | 0b00000010; // right bad		//Unused (Debugging Purpose)
          }else{ // match pb-0 led-0
            goodbad = goodbad & 0b11111100; // clear right goodbad		//Unused  (Debugging Purpose)
          }
        }
        
        if (score < 0){			//Score cannot be negative
          score = 0;
        }
        
        //Determine, based on the tone of the musical note being played, whether the left LED should be illuminated
        if ((i_music %3 == 0) &&(tone[i_music] == TC7_DO_L || tone[i_music] == TC7_DOA_L ||tone[i_music] == TC7_REA_L ||tone[i_music] == TC7_MI_L ||tone[i_music] == TC7_SOA_L ||tone[i_music] == TC7_TI_L ||tone[i_music] == TC7_DO_M ||tone[i_music] == TC7_DOA_M ||tone[i_music] == TC7_REA_M ||tone[i_music] == TC7_SOA_M ||tone[i_music] == TC7_LA_M ||tone[i_music] == TC7_TI_M)){
          leftLED = ((leftLED >> 1) & 0x7F) | 0x80;   // NEXT 1
        }else{
          leftLED = ((leftLED >> 1) & 0x7F) | 0x00;   // NEXT 0
        }

        //Determine, based on the tone of the musical note being played, whether the mid LED should be illuminated
        if ((i_music %3 == 0) &&(tone[i_music] == TC7_RE_L || tone[i_music] == TC7_REA_L ||tone[i_music] == TC7_FA_L ||tone[i_music] == TC7_FAA_L ||tone[i_music] == TC7_SO_L ||tone[i_music] == TC7_SOA_L ||tone[i_music] == TC7_LA_L ||tone[i_music] == TC7_LAA_L ||tone[i_music] == TC7_RE_M ||tone[i_music] == TC7_REA_M ||tone[i_music] == TC7_FA_M ||tone[i_music] == TC7_FAA_M ||tone[i_music] == TC7_SOA_M ||tone[i_music] == TC7_LAA_M)){
          midLED = ((midLED >> 1) & 0x7F) | 0x80;   // NEXT 1
          //midLED = ((midLED >> 1) & 0x7F) | 0x00;   // NEXT 0
        }else{
          //midLED = ((midLED >> 1) & 0x7F) | 0x80;   // NEXT 1
          midLED = ((midLED >> 1) & 0x7F) | 0x00;   // NEXT 0
        }

        //Determine, based on the tone of the musical note being played, whether the right LED should be illuminated
        if ((i_music %3 == 0) &&(tone[i_music] == TC7_DOA_L || tone[i_music] == TC7_FAA_L ||tone[i_music] == TC7_LAA_L ||tone[i_music] == TC7_DOA_M ||tone[i_music] == TC7_MI_M ||tone[i_music] == TC7_FAA_M ||tone[i_music] == TC7_SO_M ||tone[i_music] == TC7_LAA_M ||tone[i_music] == TC7_DO_H)){
          rghtLED = ((rghtLED >> 1) & 0x7F) | 0x80;   // NEXT 1
        }else{
          rghtLED = ((rghtLED >> 1) & 0x7F) | 0x00;   // NEXT 0
        }
      }
      
      led_shiftout(goodbad);
      led_shiftout(rghtLED);
      led_shiftout(midLED);
      led_shiftout(leftLED);
      
      //debug();
      
      i++;           
   }
   
   
   
   if (rtiflag){
      rtiflag = 0;
   }
   
   if (onems){
      onems = 0;
      if (runstp && !gameover){        	
         // react++;
      }
      
      if (runstp && !gameover){		//Decrement the timer every one millisecond when the game is running
         timer--;
         if (timer == 0){
           gameover = 1;
         }
      }
   }
   		
   if (onesec){				//Update the LCD display every 1 second
      onesec = 0;
      
      // SPI lcd shift out
      if (runstp){			//Show countdown clock and score when game is running
        game_disp();
      }else{				//Show game difficulty display when game is stopped
        diff_disp();
      }
      
   }
  

  
   } /* loop forever */
   
}   /* do not leave main */




/*
***********************************************************************                       
 RTI interrupt service routine: RTI_ISR
************************************************************************
*/

interrupt 7 void RTI_ISR(void)
{
  	// clear RTI interrupt flagt 
  	CRGFLG = CRGFLG | 0x80; 
  	
  	// pushbuttons
  	
  	if(PORTAD0_PTAD7 == 0) { // check runstp pushbutton
      if(prevpbrun == 1) {
        runstppb = 1;
      }
    }
    prevpbrun = PORTAD0_PTAD7;
    
    
    if(PORTAD0_PTAD6 == 0) { // check right pushbutton
      if(prevpbr == 1) {
        rghtpb = 1;
      }
     }
    prevpbr = PORTAD0_PTAD6;
    
    if(PORTAD0_PTAD5 == 0) { // check mid pushbutton
      if(prevpbm == 1) {
        midpb = 1;
      }
     }
    prevpbm = PORTAD0_PTAD5;    
    
    if(PORTAD0_PTAD4 == 0) { // check left pushbutton
      if(prevpbl == 1) {
        leftpb = 1;
      }
     }
    prevpbl = PORTAD0_PTAD4; 
  	
  	
  	
  	
  	//Increment a counter the is used to determine the tempo of the music and therefore the duration of each note. The next note in the hard-coded array of musical
	//notes will start when the rate counter is equal to difficulty setting.
  	ratecnt++;
  	if (ratecnt >= difficulty){
  	  ratecnt = 0;
  	  shift_flag = 1;
  	}
  	// ratecnt
  	// 600 - easy/ 480 - normal/ 360- Hard / 240- expert 
  	
  	rticnt++;
  	if (rticnt >= 5){
  	  rtiflag = 1;
  	}
  	
  	//Counter that sets a flag every one millisecond
  	onemscnt++;
  	if (onemscnt >= 8){
  	  onemscnt = 0;
  	  onems = 1;
  	}
  	
	//Counter that sets a flag every one second
  	oneseccnt++;
  	if (oneseccnt >= 7813){
  	  oneseccnt = 0;
  	  onesec = 1;
  	}
  	
  	
 

}

/*
***********************************************************************                       
  TIM interrupt service routine	  		
***********************************************************************
*/


interrupt 14 void TIM_ISR_ch6(void)
{
  	// clear TIM CH 6 interrupt flag 
 	TFLG1 = TFLG1 | 0x40; 
 
  

}

interrupt 15 void TIM_ISR(void)
{
  	// clear TIM CH 7 interrupt flag 
 	TFLG1 = TFLG1 | 0x80; 
 
  if (tone[i_music] == TC7_PAUSE){		//If on a pause note, do not output an audio signal on PWM
    PWMDTY0 = 0;
  }else{					//If not on a pause note, toggle the PWM duty cycle from 100 to 0 or 0 to 100
    if (PWMDTY0 == 100){
      PWMDTY0 = 0;
    }else{
      PWMDTY0 = 100;
    } 
  }
 
  

}

/*
***********************************************************************                       
  SCI interrupt service routine		 		  		
***********************************************************************
*/

interrupt 20 void SCI_ISR(void)
{
 


}


//This function is used to help the user audibly differentiate between two notes by putting a small gap between each musical note
void pulse_wait() 
{
  int tmp = TC7;		//Save the value of TC7
  TC7 = TC7_PAUSE;		//Disable audio
  asm{				//Wait for a few milliseconds
      PSHX
      PSHC
      LDX #8000
LP:   
      DBNE X,LP
      PULC
      PULX
  }
  TC7 = tmp;  			//Return TC7 to it's former value 
}
/////////////////////////////////////////////////////////////////////////////
//    LED Functions
/////////////////////////////////////////////////////////////////////////////

void led_shiftout(char led){
  int j;
  PTT_PTT5 = 1;  // slave select (1-LED, 0-LCD)
  
  while(SPISR_SPTEF==0){		//Wait for SPI data register to be empty
  }
  SPIDR = led;				//Put LED data into the SPI data register
  for (j = 0; j < 90; j++){		//Wait for shift out
  }
}


/////////////////////////////////////////////////////////////////////////////
//    LCD Functions
/////////////////////////////////////////////////////////////////////////////
//Print out a string of characters on LCD display
void pmsglcd(char str[]){
    int i = 0;
    while(str[i] != 0){
      PTT_PTT2 = 1;				//Set PT2 to 1 to represent sending a data byte
      print_c(str[i]);				
      i++;
    }  
}

//Send a single character to the LCD
void print_c(char x){
    PTT_PTT2 = 1; 		//Set PT2 to 1 to represent sending a data byte
    send_byte(x);  		
}


//This function displays a bar on the LCD whos length corresponds to the difficulty setting of the game
void diff_disp(){

    
    
    send_i(LCDCLR);				//clear LCD display

    chgline(LINE1);				//Move cursor to line 1 and output the message "SPEED:"
    pmsglcd("SPEED:");				
    
    chgline(LINE2);				//Move cursor to line 2 and output a bar display on the LCD, which has a length corresponding to the difficulty setting
						//The faster the music tempo, the longer the bar
       
    if (difficulty >= 256+16*9){
      print_c(0xFF);    
    }else if (difficulty >= 256+16*8){
      print_c(0xFF);
      print_c(0xFF);;
    }else if (difficulty >= 256+16*7){
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
    }else if (difficulty >= 256+16*6){
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
    }else if (difficulty >= 256+16*5){
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
    }else if (difficulty >= 256+16*4){
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
    }else if (difficulty >= 256+16*3){
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
    }else if (difficulty >= 256+16*2){
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
    }else if (difficulty >= 256+16*1){
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
    }else{
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
      print_c(0xFF);
    }
    
}

//Display the user's score and the amount of time remaining on the LCD
void game_disp(){				
    send_i(LCDCLR);				//clear LCD display
    chgline(LINE1);				//Move cursor to line 1
    pmsglcd("SCORE: ");				//Display the score by converting decimal to ascii
    print_c(score/100%10 + 48);
    print_c(score/10%10 + 48);
    print_c(score/1%10 + 48);
    
    chgline(LINE2);				//Move cursor to line 2
    pmsglcd("TIME REMAIN: ");			//Display time remaining--convert decimal time value to ascii
    print_c(timer/10000 % 10 + 48);
    print_c(timer/1000 % 10 +48);
    pmsglcd("s");
}

//Use SPI to shift out a character to the LCD
void lcd_shiftout(char x){
  int j;
  
  // read the SPTEF bit, continue if bit is 1
  // write data to SPI data register
  // wait for 30 cycles for SPI data to shift out
  
  PTT_PTT5 = 0;			
  while(SPISR_SPTEF==0){
  }  
  SPIDR = x;

  
  for (j = 0; j < 30; j++){
  }
}

void send_byte(char x){
     // shift out character
     // pulse LCD clock line low->high->low
     // wait 2 ms for LCD to process data
     lcd_shiftout(x);
     PTT_PTT4 = 0;     
     PTT_PTT4 = 1;       
     PTT_PTT4 = 0;
     lcdwait();

}

//Send an instruction byte to LCD
void send_i(char x){
    // set the register select line low (instruction data)
    // send byte
    PTT_PTT2 = 0; // register select -> low    
    send_byte(x);
}

//Send an 2 instruction bytes to LCD--one to declare a cursor move, the other containing the new cursor location
void chgline(char x){
	//send cursor move instruction
	//send new cursor location
    send_i(CURMOV);
    send_i(x);
}


/*
***********************************************************************                       
  LCD wait routine
  wait for 2ms		 		  		
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

//Used for debugging
void debug(){
    int i = 0;
    outchar(CR);
    outchar(LF);
    outmsg("leftLED: ");
    for (i = 0; i < 8; i++){
      if (leftLED & 0x80 == 0x80){
        outchar('1');
      }else{
        outchar('0');
      }
      leftLED = leftLED << 1;
    }
    outchar(CR);
    outchar(LF);
    outmsg("midLED: ");
    for (i = 0; i < 8; i++){
      if (midLED & 0x80 == 0x80){
        outchar('1');
      }else{
        outchar('0');
      }
      midLED = midLED << 1;
    }
    outchar(CR);
    outchar(LF);
    outmsg("rghtLED: ");
    for (i = 0; i < 8; i++){
      if (rghtLED & 0x80 == 0x80){
        outchar('1');
      }else{
        outchar('0');
      }
      rghtLED = rghtLED << 1;
    }
    outchar(CR);
    outchar(LF);
}

void outmsg(char str[]){
  int i = 0;
  while(str[i] != 0){
    outchar(str[i]);
    i++;
  }
}

/*
***********************************************************************
 Character I/O Library Routines for 9S12C32 
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
 Name:         outchar    (use only for DEBUGGING purposes)
 Description:  outputs ASCII character x to SCI serial port
 Example:      outchar('x');
***********************************************************************
*/

void outchar(char x) {
  /* sends a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
    SCIDRL = x;
}