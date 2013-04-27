/*
 * main.c
 *
 * Copyright 2013 Charles Wolf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */


#define F_CPU 8000000UL //8 MHz Internal Oscillator

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

#include "lib/uart.c"
#include "lib/nRF24AP1.c"

//Global variables
FATFS FileSystemObject;
FIL logFile;
unsigned int bytesWritten;
int i = 0;

unsigned char MSG[21];
char tmp[8];
uint8_t HR = 0;

#include "lib/sdcard.c"


//Main
int main(void)
{
int blah;

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

	while(1)
	{
		sdcard_open ( "debug.txt" ); // open debug file
		f_lseek ( &logFile, f_size(&logFile));//move to last line

		blah = get_ant_msg(22001, &MSG[0]);
		if(blah == 1)
		{
			for(i = 0; i< MSG[1] + 4; i++)
			{
				utoa(MSG[i], &tmp[0], 16);
				f_write(&logFile, &tmp[0], strlen(tmp), &bytesWritten);
				f_write(&logFile, " ", 1, &bytesWritten);
			}
		}
		else if (blah == 2)
		{
			f_write(&logFile, "No Sync 0x", 10, &bytesWritten);
			utoa(MSG[0], &tmp[0], 16);
			f_write(&logFile, &tmp[0], strlen(tmp), &bytesWritten);
		}
		else
		{
			f_write(&logFile, "Timeout", 7, &bytesWritten);
		}
		f_write(&logFile, "\n", 1, &bytesWritten);
		f_close(&logFile);//close file
	}

	return 1;
}

