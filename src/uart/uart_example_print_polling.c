#include "commons/defines.h"
#include "drivers/pins.h"
#include "drivers/mcu_init.h"
#include <msp430.h>

void print_str(char *str)
{
	unsigned int i = 0;
	while(str[i] != '\0')
	{
		while (!(IFG2&UCA0TXIFG));	//  waits until UCA0TXBUF is empty and ready to accept a new byte	
		UCA0TXBUF = str[i];			    //  Writes the next character into the TX buffer
		i++;
	}
}

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
  

  if (CALBC1_1MHZ==0xFF) { while(1); }

	DCOCTL = 0;							// Select lowest DCO settings
	BCSCTL1 = CALBC1_1MHZ;				// Set DCO to 1 MHz "general range"
	DCOCTL = CALDCO_1MHZ;         // Set DCO to 1MHz "fine tune fq"

  P1SEL = BIT1 + BIT2;
  P1SEL2 = BIT1 + BIT2;
  // Select UART RX/TX function on P1.1,P1.2

  UCA0CTL1 |= UCSWRST;    // ⬅️ Put UART into reset *first*
  UCA0CTL1 |= UCSSEL_2;    // Use SMCLK (1 MHz)
  UCA0BR0 = 104;           // Set baud rate to 9600 1,000,000 / 9600 = 104
  UCA0BR1 = 0;
  UCA0MCTL = UCBRS_1;      // Fine-tuning for baud rate
  UCA0CTL1 &= ~UCSWRST;    // Enable UART
  
  IE2 |= UCA0RXIE; 
  // enable uart to receive interrupt, data from computer to MCUtrigger
  // trigger interrupt when a byte has been received over UART and is ready in recevie buffer 

  __enable_interrupt();

  // ------- RUN ------- //
  while(1) {
    print_str("hello world\r\n");
    __delay_cycles(100000);
   }
      
}

/*
INTERRUPT(USCIAB0RX_VECTOR)
void USCI0RX_ISR(void)
{
	while (!(IFG2&UCA0TXIFG));			// Check if TX is ongoing
	UCA0TXBUF = UCA0RXBUF;    			// TX -> Received Char + 1
}
*
