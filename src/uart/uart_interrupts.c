#include "commons/defines.h"
#include "drivers/pins.h"
#include "drivers/mcu_init.h"
#include <msp430.h>

#define TX_BUFFER_SIZE 24
const char message[] = "hello world\r\n";
volatile char tx_buffer[TX_BUFFER_SIZE];
volatile unsigned int tx_head = 0;
volatile unsigned int tx_tail = 0;
volatile int tx_active = 0;

void uart_puts(const char *s) {
  while (*s) {
    unsigned int next_head = (tx_head + 1) % TX_BUFFER_SIZE;
    // Wait if buffer is full
    while (next_head == tx_tail);
    tx_buffer[tx_head] = *s++;
    tx_head = next_head;
  }
  // Start TX if not already active
  if (!tx_active) {
    tx_active = 1;
    IE2 |= UCA0TXIE; // Enable TX interrupt
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

  UCA0CTL1 |= UCSWRST;    // ⬅️ Put UART into reset to config 
  UCA0CTL1 |= UCSSEL_2;    // Use SMCLK (1 MHz)
  UCA0BR0 = 104;           // Set baud rate to 9600 1,000,000 / 9600 = 104
  UCA0BR1 = 0;
  UCA0MCTL = UCBRS_1;      // Fine-tuning for baud rate
  UCA0CTL1 &= ~UCSWRST;    // Enable UART

  __enable_interrupt();

  // ------- RUN ------- //
  while(1) {
    uart_puts(message);
    DELAY_MS(1000);
  }
      
}

INTERRUPT(USCIAB0TX_VECTOR)
void USCI0TX_ISR(void) 
{
  if (tx_tail != tx_head) {
    UCA0TXBUF = tx_buffer[tx_tail];
    tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;
  } else {
    IE2 &= ~UCA0TXIE; // Disable TX interrupt when done
    tx_active = 0;
  }
}

