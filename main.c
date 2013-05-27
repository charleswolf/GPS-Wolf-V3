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
#include "lib/lcd.c"


#include "lib/uart.c"
#include "lib/nRF24AP1.c"

//Global variables
FATFS FileSystemObject;
FIL logFile;
unsigned int bytesWritten;
int i = 0;
int j = 0;

unsigned char MSG[21];
char tmp[8];
uint8_t HR = 0;
uint8_t disp_buffer[512];

#include "lib/sdcard.c"


//Main
int main(void)
{
int msg_outcome;

	//reference un-used pins
	DDRB 	|= (1<<PB0) | (1<<PB1) | (1<<PB6) | (1<<PB7);
	PORTB 	&= !((1<<PB0) | (1<<PB1) | (1<<PB6) | (1<<PB7));
	DDRC 	|= 0xFF;	//all output
	PORTC 	&= 0x00;	//all low

	//allow time for power to stabilize
	_delay_ms(1000);
	
	st7565_init();
	st7565_command(CMD_DISPLAY_ON);
	st7565_command(CMD_SET_ALLPTS_NORMAL);
	clear_screen();
	clear_buffer(disp_buffer);
	
	
	drawNum(disp_buffer, 0, 0x35);
	write_buffer(disp_buffer);
	clear_buffer(disp_buffer);
	while(1)
	{
		drawNum(disp_buffer, 0, 0x30);
		write_buffer(disp_buffer);
		clear_buffer(disp_buffer);
		_delay_ms(1000);
		
		drawNum(disp_buffer, 0, 0x31);
		write_buffer(disp_buffer);
		clear_buffer(disp_buffer);
		_delay_ms(1000);
		
		drawNum(disp_buffer, 0, 0x32);
		write_buffer(disp_buffer);
		clear_buffer(disp_buffer);
		_delay_ms(1000);		
		
		drawNum(disp_buffer, 0, 0x33);
		write_buffer(disp_buffer);
		clear_buffer(disp_buffer);
		_delay_ms(1000);
		
		drawNum(disp_buffer, 0, 0x34);
		write_buffer(disp_buffer);
		clear_buffer(disp_buffer);
		_delay_ms(1000);
		
		drawNum(disp_buffer, 0, 0x35);
		write_buffer(disp_buffer);
		clear_buffer(disp_buffer);
		_delay_ms(1000);
		
		drawNum(disp_buffer, 0, 0x36);
		write_buffer(disp_buffer);
		clear_buffer(disp_buffer);
		_delay_ms(1000);
		
		drawNum(disp_buffer, 0, 0x37);
		write_buffer(disp_buffer);
		clear_buffer(disp_buffer);
		_delay_ms(1000);
		
		drawNum(disp_buffer, 0, 0x38);
		write_buffer(disp_buffer);
		clear_buffer(disp_buffer);
		_delay_ms(1000);
		
		drawNum(disp_buffer, 0, 0x39);
		write_buffer(disp_buffer);
		clear_buffer(disp_buffer);
		_delay_ms(1000);
	}	
	
	
	
	
	drawstring(disp_buffer, 0, 3, "Fitness Monitor");
	write_buffer(disp_buffer);
	clear_buffer(disp_buffer);

	init_sdcard(0); 
	softuart_init();
	nRF24AP1_init();
	_delay_ms(1000);
	ant_hr_config();
	


	sdcard_open ( "debug.txt" ); // open debug file
	f_lseek ( &logFile, f_size(&logFile));//move to last line
	f_write(&logFile, "configured", 10, & bytesWritten);
	f_write(&logFile, "\n", 1, &bytesWritten);//next line
	f_close(&logFile);//close file
	


	_delay_ms(1000);
	while(1)
	{
		msg_outcome = get_ant_msg(3000, &MSG[0]);
		j++;
		if (j > 3)
		{
			j = 0;
			if(msg_outcome == 1)
			{
				utoa(MSG[11], &tmp[0], 10);
				sdcard_open ( "debug.txt" );
				f_lseek ( &logFile, f_size(&logFile));//move to last line
				f_write(&logFile, &tmp[0], strlen(tmp), &bytesWritten);
				f_write(&logFile, "\n", 1, &bytesWritten);
				f_close(&logFile);//close file
			
				//clear_screen();
				clear_buffer(disp_buffer);
				drawstring(disp_buffer, 0, 3, &tmp[0]);
				write_buffer(disp_buffer);
				
				
			}
			else
			{
				sdcard_open ( "debug.txt" );
				f_lseek ( &logFile, f_size(&logFile));//move to last line
				f_write(&logFile, "Error\n", 6, &bytesWritten);
				f_close(&logFile);//close file
				
				clear_screen();
				clear_buffer(disp_buffer);
				drawstring(disp_buffer, 0, 3, "error");
				write_buffer(disp_buffer);
			}
		}
	}
	return 1;
}

