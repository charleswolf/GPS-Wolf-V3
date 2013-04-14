/*
    07-07-08
    Copyright Spark Fun Electronics© 2008
    Aaron Weiss
	aaron at sparkfun.com
    
    Master AP1 Intitialization @ 4800bps
*/

#include <stdlib.h>

#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#define FOSC 8000000 // 8MHz
#define BAUD 4800
#define MYUBRR 103 // Calculated from http://www.wormfood.net/avrbaudcalc.php

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

#define STATUS_LED 5 // Pin 28 (PC5)

// Define functions
//=======================
void ioinit(void);      // initializes IO
static int uart_putchar(char c, FILE *stream); // sends char out of UART
uint8_t uart_getchar(void); // receives char from UART

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void delay_ms(uint16_t x); // general purpose delay

void config(void); // runs config functions
//=======================

// Configuration functions
//=======================
void reset(void); 
void assignch(void); 
void setchid(void); 
void opench(void);
void send(void);
//=======================

int main (void)
{
    ioinit();
	
	config();
	
	while(1)
    {
	sbi(PORTC, STATUS_LED);
	delay_ms(500);
	cbi(PORTC, STATUS_LED);
	delay_ms(500);
	}
    return(0);
}


void config (void)
{
	reset();
	delay_ms(1000);
	assignch();
	delay_ms(1000);
	setchid();
	delay_ms(1000);
	opench();
	delay_ms(1000);
	send();
}

// Resets module
void reset (void) 
{
	uint8_t i;
	uint8_t setup[4];
	
	setup[0] = 0xa4; // SYNC Byte
	setup[1] = 0x01; // LENGTH Byte
	setup[2] = 0x4a; // ID Byte
	setup[3] = 0x00; // Data Byte N (N=LENGTH)
	setup[4] = 0xef; // Checksum
	
	for(i = 0 ; i < 5 ; i++)
    {
	putchar(setup[i]);
    }
}

// Assigns CH=0, CH Type=10(TX), Net#=0
void assignch (void) 
{
	uint8_t i;
	uint8_t setup[6];
	
	setup[0] = 0xa4;
	setup[1] = 0x03;
	setup[2] = 0x42;
	setup[3] = 0x00;
	setup[4] = 0x10;
	setup[5] = 0x00;
	setup[6] = 0xf5;
	
	for(i = 0 ; i < 7 ; i++)
    {
	putchar(setup[i]);
    }
}

// Assigns Device#=3100, Device Type ID=01, Trans Type=01
void setchid (void) 
{
	uint8_t i;
	uint8_t setup[8];
	
	setup[0] = 0xa4;
	setup[1] = 0x05;
	setup[2] = 0x51;
	setup[3] = 0x00;
	setup[4] = 0x31;
	setup[5] = 0x00;
	setup[6] = 0x01;
	setup[7] = 0x05;
	setup[8] = 0xc5;
	
	for(i = 0 ; i < 9 ; i++)
    { 
	putchar(setup[i]);
    }
}

// Opens CH 0
void opench (void) 
{
	uint8_t i;
	uint8_t setup[4];
	
	setup[0] = 0xa4;
	setup[1] = 0x01;
	setup[2] = 0x4b;
	setup[3] = 0x00;
	setup[4] = 0xee;
	
	for(i = 0 ; i < 5 ; i++)
    { 
	putchar(setup[i]);
    }
}

// Sends Data=AAAAAAAAAAAAAAAA
void send (void) 
{
	uint8_t i;
	uint8_t setup[12];
	
	setup[0] = 0xa4;
	setup[1] = 0x09;
	setup[2] = 0x4e;
	setup[3] = 0x00;
	setup[4] = 0xaa;
	setup[5] = 0xaa;
	setup[6] = 0xaa;
	setup[7] = 0xaa;
	setup[8] = 0xaa;
	setup[9] = 0xaa;
	setup[10] = 0xaa;
	setup[11] = 0xaa;
	setup[12] = 0xe3;
	
	for(i = 0 ; i < 13 ; i++)
    { 
	putchar(setup[i]);
    }
}

void ioinit (void)
{
    //1 = output, 0 = input
    DDRB = 0b11101111; //PB4 = MISO 
    DDRC = 0b11111111; //all outputs
    DDRD = 0b11111110; //PORTD (RX on PD0)

    //USART Baud rate: 4800
    UBRR0H = (MYUBRR >> 8);
    UBRR0L = MYUBRR;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    
    stdout = &mystdout; //Required for printf init
}

static int uart_putchar(char c, FILE *stream)
{
    if (c == '\n') uart_putchar('\r', stream);
  
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    
    return 0;
}

uint8_t uart_getchar(void)
{
    while( !(UCSR0A & (1<<RXC0)) );
    return(UDR0);
}

//General short delays
void delay_ms(uint16_t x)
{
  uint8_t y, z;
  for ( ; x > 0 ; x--){
    for ( y = 0 ; y < 90 ; y++){
      for ( z = 0 ; z < 6 ; z++){
        asm volatile ("nop");
      }
    }
  }
}
