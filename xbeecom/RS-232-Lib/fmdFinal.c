/*
 * fmdFinal.c
 *
 * Created: 10/31/2014 7:54:09 AM
 *  Author: Joseph Hall
 *  
 */ 

#ifndef F_CPU
#define F_CPU 19200000UL // 19.2MHz clock
#endif

#define IDLE 0
#define GO 1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <stdlib.h>

// global variables
unsigned long ovfCount = 0; // counts overflow occurrences of TIMER1_OVF_vect
unsigned int tUP, tDN = 0; // temporarily holds value of TIMER1 between INT1_vect (rising) and main loop
unsigned long cUP, cDN = 0; //temporarily holds value of vfCount between INT1_vect (rising) and main loop
unsigned char rose, fell = 0; // state variable (1 if entered ISR(INT1_vect))
unsigned long tmp = 0;
unsigned char Nave = 120; // number of periods over which to average for frequency estimate
unsigned long sendRate_ms = 50; // default # of ms between data transmissions
unsigned long ovf0 = 0; // counter for timer0 for transmission rate
unsigned char rxd = 0;



void initialize_USART0_for_Xbee(void)
{
	//asynchronous double speed mode, BAUD~115200, BAUD=fosc/(8*(UBRR+1), UBRR=8 =>BAUD=114285, error = [(BAUD_nom)/BAUD - 1]*100% =0.8%
	// for RX: enabled by writing the Receive Enable (RXENn) bit in the
	//	            UCSRnB Register to one
	// read with: while( !(UCSRnA & (1<<RXCn)) )  ;  returnUDRn;
	// For ISR:  Receive Complete Interrupt Enable (RXCIEn) in UCSRnB is set, the USART Receive
	//                Complete interrupt will be executed as long as the RXCn Flag is set 
 	//				  When interrupt-driven data reception is used, the receive complete routine
	//					must read the received data from UDRn in order to clear the RXCn Flag
	UBRR0H = 0;
	UBRR0L = 20; //UBRR = 20 for BAUD of 114285
	UCSR0A |= (1<<U2X0);					// Baud rate set for double speed TX
	UCSR0B |= (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0);		// Enable Usart TX/ RX & RX ISR
	UCSR0C |= (0<<USBS0);					// 1 stop bits
	UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00);	// 8 data bits per frame
}

void initialize_Timer0() // used to time the frequency of transmission output
{
	TCCR0B = (1<<CS00); // clock source no prescale
	TIMSK0 = (1<<TOIE0); // Timer0 Overflow Interrupt Enable
}

/*
	Analog Comparator Propagation Delay: 500ns (p314)
	INTx propagation delay: 3clk cycles
*/
void initialize_Timer1(void) // Timer 1 fires interrupt to increment ovfCounter to keep track of time
{	
	//TCCR1A already zeros for normal mode
	TCCR1B = (1<<ICNC1)|(1<<ICES1)|(1<<CS10); // Input Capture on Rising Edge | clk(i/o)/1 no prescaling
	TIMSK1 = (1<<ICIE1)|(1<<TOIE1); // Input Capture Interrupt Enable | Timer1 Overflow Interrupt Enable
}

unsigned long calc_numClks (unsigned long ovf, unsigned int tcnt1, unsigned long *t1)
{
	unsigned long t2, numClks;
	unsigned long t_1 = *t1;

	t2=ovf*65536+tcnt1; // calculate new time stamp (t2=timeSinceStart*19.2MHz)
	if ((t2>t_1)&&(tcnt1<65500)) //make sure ovfCounter reset and extra ovfCounter increment get omitted
	{
		numClks=t2-t_1; // calculate period*19.2MHz
	}
	else
	{
		numClks = 0;
	}	
	*t1 = t2; // save new time stamp for next time
	
	return numClks;
}

unsigned long calcAveFreq2 (unsigned long *numClksArray)
{
	unsigned long numerator, Hz, mHz, uHz, f, ave;
	unsigned char n;
	unsigned long aveNumClks=0;
	
	for (n=0; n<Nave; n++)
	{
		aveNumClks = aveNumClks+numClksArray[n];
	}

	aveNumClks=aveNumClks/Nave;
	ave = aveNumClks;
	Hz = 192000000000/ave; //calculate tens and ones place of frequency
	
	//round off least significant ones place
	f=Hz/10;
	f*=10;
	f=Hz-f;
	Hz/=10; // cut off ones place
	if (f>=5) // round up if it was greater than 5
	{
		Hz+=1;
	}	
	
	return Hz;
} 

void turnONLEDforDebug(void)
{
	// turns on LED for debugging purposes
	tmp = DDRC;
	tmp = tmp | 0x20;
    DDRC = tmp; // PORTC,5 is now output
	tmp = PORTC;
	tmp = tmp & ~(0x20);
	PORTC = tmp;// PORTC is HIGH to light bulb
}

void toggleLED(void)
{
	PINC = PINC | (1<<PINC5); //toggle LED
}

//Sends one character through UART to Zigbee
void sendUART(unsigned char data)
{
	//Send to PC
	// wait for buffers to be ready
	while ( !( UCSR0A & (1<<UDRE0)) ){
		;
	}
	// Put  byte into buffer, it automatically sends the data 
	UDR0 = data;
}

//Sends long through UART to Zigbee
void sendUARTulong(unsigned long data)
{
	char word[33];
	char i;
	
	ultoa(data,word,10); //convert to string
	
	for (i=0;word[i]!='\0'; i++)
	{
		sendUART(word[i]);	
	}	
	//sendUART(13); //send carriage return (for hyperterminal)
	sendUART(10);	// send new line
}

ISR(TIMER0_OVF_vect)  // increment counter used to time the frequency of transmission output
{
	ovf0++; //increment ovf0 counter
}

ISR(TIMER1_OVF_vect)  // increment overflow counter for timestamping zero-crossings (each overflow = 2^16 clk cycles)
{
	ovfCount = ovfCount + 1;
	
}

ISR(TIMER1_CAPT_vect) // Input Capture Interrupt fires on rising edge with stored having stored TCNT1 in ICR1 at the time of event
{
	tUP = ICR1; // get stored timer count
	cUP = ovfCount; // get overflow count
	rose = 1; // change state in main loop
}

ISR(USART_RX_vect) // when receive byte from zigbee save and change rxd state
{
	tmp = UDR0;
	rxd = 1;
}

int main(void)
{
	unsigned long lastTup, freq, numClks = 0;
	unsigned long numClksArray[Nave];
	unsigned char i=0;	
	// # of counter overflows necessary for a sendRate_ms period between transmissions
	unsigned long sendCount=sendRate_ms*75; // fclk/counterBits/10^3 = 19.2MHz/(2^8)/1000 = 75
	unsigned char state = IDLE;
	unsigned char whichByte = 0;
	
	//initialize UART, and timers
	initialize_USART0_for_Xbee();
	initialize_Timer0();
	initialize_Timer1();
	sei(); // enable interrupts

	while (1) //main loop runs forever
	{
		while(state==IDLE)  // when initially powered, uC waits for commands from PC
		{
			if (rxd==1)
			{
			    // ensure proper sequence of commands and echo back to ensure proper communication
				if (tmp == 'R') //R for Rate
				{
					whichByte = 1;
					rxd = 0;
					tmp = 0;
					sendUARTulong('R'); //ACK
				}
				else if (whichByte == 1) // get high byte of rate
				{
					sendRate_ms = (tmp << 8);
					rxd = 0;
					whichByte = 2;
					sendUARTulong(tmp); // ACK
					tmp = 0;
				}				
				else if (whichByte == 2) // get low byte of rate
				{
					sendRate_ms = (sendRate_ms | tmp);
					rxd = 0;
					whichByte = 0;
					sendCount=sendRate_ms*75;  
					sendUARTulong(tmp); //ACK
					tmp = 0;
				}
				else if (tmp == 'B') // B for Begin
				{
					state = GO;
					rxd = 0;
					sendUARTulong('B'); // ACK
					tmp = 0;
				}
				else if (tmp == 'S')  // S for Stop  
				//(This allows the uC to ACK an errant Stop command even when already IDLE)
				{
					rxd = 0;
					sendUARTulong('S'); // ACK
				}				
				else // If received corrupted byte, send E for Error
				{
					rxd = 0;
					tmp = 0;
					sendUARTulong('E');
				}							
			}	
			else 
				;	// do nothing if not receiving from PC
		}
		
		// Once received and acknowledged the BEGIN command:
		while(state==GO)  
		{
			if (rxd==1) // if receive command it is either STOP or ERROR
			{

				if (tmp=='S') // S for STOP
				{
					state = IDLE; // Will go back to IDLE state
					tmp = 0;
					sendUARTulong('S'); // ACK
				}				
				else
				{
					sendUARTulong('E'); //
				}				
				rxd = 0;
			}
	
		    else if (rose==1) // If detected zero-crossing from Input Capture Interrupt:
			{
				numClks = calc_numClks(cUP,tUP,&lastTup); // calculate number of clock cycles since last 
				if ((numClks>295000)&&(numClks<350000)) // if between 55Hz and 65Hz (otherwise ignore)
				{
					numClksArray[i]=numClks; // add to ring buffer
					i++; // increment buffer index
					if (i>=Nave) // resent buffer index if at the end
					{
						i=0;
					}	
				}
			
			
				if (cUP>=1000)  // if overflow counter is over 1000, reset to zero
				{
					ovfCount=0;
				}					
				rose = 0; // reset state
			}
			else if (ovf0>=sendCount)  // calculate and sends current frequency estimate at the rate specified by PC "R" command
			{
					ovf0=0;
					freq=calcAveFreq2(numClksArray); //calculate inverse of averaged periods
					sendUARTulong(freq); // send
			}		
		
			else ; 
		
		}		
    }
}