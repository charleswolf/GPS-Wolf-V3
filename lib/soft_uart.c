#include "soft_uart.h"

/**CHANGE LOG
 * -----------------------  
 * 4-13-2013 - File Creation
 * -----------------------
*/

#define SU_TRUE 1
#define SU_FALSE 0

// startbit and stopbit parsed internaly (see ISR)
#define RX_NUM_OF_BITS (8)
volatile static char              inbuf[SOFTUART_IN_BUF_SIZE];
volatile static unsigned char    qin  = 0;
/*volatile*/ static unsigned char qout = 0;
volatile static unsigned char    flag_rx_off;
volatile static unsigned char    flag_rx_ready;

// 1 Startbit, 8 Databits, 1 Stopbit = 10 Bits/Frame
#define TX_NUM_OF_BITS (10)
volatile static unsigned char  flag_tx_ready;
volatile static unsigned char  timer_tx_ctr;
volatile static unsigned char  bits_left_in_tx;
volatile static unsigned short internal_tx_buffer; /* ! mt: was type uchar - this was wrong */

#define set_tx_pin_high()      ( SOFTUART_TXPORT |=  ( 1<<SOFTUART_TXBIT ) )
#define set_tx_pin_low()       ( SOFTUART_TXPORT &= ~( 1<<SOFTUART_TXBIT ) )
#define get_rx_pin_status()    ( SOFTUART_RXPIN  & ( 1<<SOFTUART_RXBIT ) )
// #define get_rx_pin_status() ( ( SOFTUART_RXPIN & ( 1<<SOFTUART_RXBIT ) ) ? 1 : 0 )

ISR(SOFTUART_T_COMP_LABEL)
{
	static unsigned char flag_rx_waiting_for_stop_bit = SU_FALSE;
	static unsigned char rx_mask;
	
	static char timer_rx_ctr;
	static char bits_left_in_rx;
	static unsigned char internal_rx_buffer;
	
	char start_bit, flag_in;
	char tmp;
	
	// Transmitter Section
	if ( flag_tx_ready ) {
		tmp = timer_tx_ctr;
		if ( --tmp <= 0 ) { // if ( --timer_tx_ctr <= 0 )
			if ( internal_tx_buffer & 0x01 ) {
				set_tx_pin_high();
			}
			else {
				set_tx_pin_low();
			}
			internal_tx_buffer >>= 1;
			tmp = 3; // timer_tx_ctr = 3;
			if ( --bits_left_in_tx <= 0 ) {
				flag_tx_ready = SU_FALSE;
			}
		}
		timer_tx_ctr = tmp;
	}

	// Receiver Section
	if ( flag_rx_off == SU_FALSE ) {
		if ( flag_rx_waiting_for_stop_bit ) {
			if ( --timer_rx_ctr <= 0 ) {
				flag_rx_waiting_for_stop_bit = SU_FALSE;
				flag_rx_ready = SU_FALSE;
				inbuf[qin] = internal_rx_buffer;
				if ( ++qin >= SOFTUART_IN_BUF_SIZE ) {
					// overflow - rst inbuf-index
					qin = 0;
				}
			}
		}
		else {  // rx_test_busy
			if ( flag_rx_ready == SU_FALSE ) {
				start_bit = get_rx_pin_status();
				// test for start bit
				if ( start_bit == 0 ) {
					flag_rx_ready      = SU_TRUE;
					internal_rx_buffer = 0;
					timer_rx_ctr       = 4;
					bits_left_in_rx    = RX_NUM_OF_BITS;
					rx_mask            = 1;
				}
			}
			else {  // rx_busy
				if ( --timer_rx_ctr <= 0 ) {
					// rcv
					timer_rx_ctr = 3;
					flag_in = get_rx_pin_status();
					if ( flag_in ) {
						internal_rx_buffer |= rx_mask;
					}
					rx_mask <<= 1;
					if ( --bits_left_in_rx <= 0 ) {
						flag_rx_waiting_for_stop_bit = SU_TRUE;
					}
				}
			}
		}
	}
}

static void avr_io_init(void)
{
	// TX-Pin as output
	SOFTUART_TXDDR |=  ( 1 << SOFTUART_TXBIT );
	// RX-Pin as input
	SOFTUART_RXDDR &= ~( 1 << SOFTUART_RXBIT );
}

static void avr_timer_init(void)
{
	unsigned char sreg_tmp;
	
	sreg_tmp = SREG;
	cli();
	
	SOFTUART_T_COMP_REG = SOFTUART_TIMERTOP;     /* set top */

	SOFTUART_T_CONTR_REGA = SOFTUART_CTC_MASKA | SOFTUART_PRESC_MASKA;
	SOFTUART_T_CONTR_REGB = SOFTUART_CTC_MASKB | SOFTUART_PRESC_MASKB;

	SOFTUART_T_INTCTL_REG |= SOFTUART_CMPINT_EN_MASK;

	SOFTUART_T_CNT_REG = 0; /* reset counter */
	
	SREG = sreg_tmp;
}

void softuart_init( void )
{
	flag_tx_ready = SU_FALSE;
	flag_rx_ready = SU_FALSE;
	flag_rx_off   = SU_FALSE;
	
	set_tx_pin_high(); /* mt: set to high to avoid garbage on init */
	avr_io_init();

	// timer_set( BAUD_RATE );
	// set_timer_interrupt( timer_isr );
	avr_timer_init(); // replaces the two calls above
	sei();//enable interrupts
}

static void idle(void)
{
	// timeout handling goes here 
	// - but there is a "softuart_kbhit" in this code...
	// add watchdog-reset here if needed
}

void softuart_turn_rx_on( void )
{
	flag_rx_off = SU_FALSE;
}

void softuart_turn_rx_off( void )
{
	flag_rx_off = SU_TRUE;
}

char softuart_getchar( void )
{
	char ch;

	while ( qout == qin ) {
		idle();
	}
	ch = inbuf[qout];
	if ( ++qout >= SOFTUART_IN_BUF_SIZE ) {
		qout = 0;
	}
	
	return( ch );
}

unsigned char softuart_kbhit( void )
{
	return( qin != qout );
}

void softuart_flush_input_buffer( void )
{
	qin  = 0;
	qout = 0;
}
	
unsigned char softuart_can_transmit( void ) 
{
	return ( flag_tx_ready );
}

void softuart_putchar( const char ch )
{
	while ( flag_tx_ready );
		// wait for transmitter ready
		// add watchdog-reset here if needed;

	// invoke_UART_transmit
	timer_tx_ctr       = 3;
	bits_left_in_tx    = TX_NUM_OF_BITS;
	internal_tx_buffer = ( ch<<1 ) | 0x200;
	flag_tx_ready      = SU_TRUE;
}

void softuart_puts( const char *s ){
	while ( *s )
		softuart_putchar( *s++ );
}


