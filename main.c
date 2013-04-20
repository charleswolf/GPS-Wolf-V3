/*
    GPS-Wolf
    Copyright (C) 2011  Charles Wolf

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

	A good reference:
	http://www.nongnu.org/avr-libc/user-manual/FAQ.html#faq_use_bv



*/

#define F_CPU 8000000UL //8 MHz Internal Oscillator 


//Modes of operation
#define NORMAL_MODE 0	//standard operating mode
#define DEBUG_MODE	1	//write debug modes 
#define GPGGA_MODE	2	//write each GPGGA message to a file


#define RUN_MODE NORMAL_MODE 


#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lib/ff.c"
#include "lib/diskio.c"

//Global variables
FATFS FileSystemObject;
FIL logFile;
unsigned int bytesWritten;
int i = 0;

char MSG[8];
char tmp[8];
uint8_t HR = 0;

#include "lib/sdcard.c"
#include "lib/uart.c"
#include "lib/nRF24AP1.c"


//Main
int main(void)
{   
	
	//reference un-used pins 
	DDRB 	|= (1<<PB0) | (1<<PB1) | (1<<PB6) | (1<<PB7);
	PORTB 	&= !((1<<PB0) | (1<<PB1) | (1<<PB6) | (1<<PB7));
	DDRC 	|= 0xFF;	//all output
	PORTC 	&= 0x00;	//all low

	//allow time for power to stabilize
	_delay_ms(1000);
	
	init_sdcard(0); 
	softuart_init();	
	nRF24AP1_init();
	ant_hr_config();

	
	sdcard_open ( "debug.txt" ); // open debug file
	f_lseek ( &logFile, f_size(&logFile));//move to last line
	f_write(&logFile, "configured", 10, & bytesWritten);
	f_write(&logFile, "\n", 1, &bytesWritten);//next line
	f_close(&logFile);//close file
	
	softuart_turn_rx_on();	
	softuart_flush_input_buffer();
	while(1)
	{
		while(softuart_kbhit())
		{
			/*******************************************
			 * Message structure of page 0
			 * *****************************************
			 * Byte 	Description
			 * 0		Page #
			 * 1		HW version
			 * 2		SW version
			 * 3		Model Number
			 * 4		Heart Beat Event Time (lsb)
			 * 5		Heart Beat Event Time (msb)
			 * 6		computed heart rate
			********************************************/

			HR = softuart_getchar();
			sdcard_open ( "debug.txt" ); // open debug file
			f_lseek ( &logFile, f_size(&logFile));//move to last line
			utoa(HR, &tmp[0], 16);
			f_write(&logFile, &tmp[0], strlen(tmp), &bytesWritten);
			f_write(&logFile, " ", 1, &bytesWritten);
			f_close(&logFile);//close file
		}
	}

return 1;
}

