/*
 * nRF24AP1.c
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
#include "nRF24AP1.h"
#include "soft_uart.c"
#include <util/delay.h>

/**CHANGE LOG
 * 
 * 4-14-2013 - Added initalization 
 * -----------------------  
 * 4-13-2013 - File Creation
 * -----------------------
*/


	
/**
* Name : nRF24AP1_init()
*
* Description: Configure output pins for the nRF24AP1 and bring it out
* of reset.  
*
* Author(s): Charles Wolf
*
* @param: none
*
* @return: none
**/	

void nRF24AP1_init(void)
{
	//configure pins as outputs
	nRF24AP1_DIRECTION	|= ((1<<nRF24AP1_RESET_PIN) 
						| (1<<nRF24AP1_RTS_PIN)
						| (1<<nRF24AP1_SLEEP_PIN)
						| (1<<nRF24AP1_SUSPEND_PIN)
						| (1<<nRF24AP1_TX_PIN));
	nRF24AP1_DIRECTION &= ~(1<<nRF24AP1_RX_PIN);					
						
	//Configure Outputs
	nRF24AP1_PORT |= (1<<nRF24AP1_TX_PIN); //not in use, idle high
	nRF24AP1_PORT &= ~(1<<nRF24AP1_SLEEP_PIN); //disable sleep
	nRF24AP1_PORT |= (1<<nRF24AP1_SUSPEND_PIN); //enable communications
	
	//ensure part sees the reset then bring it out
	nRF24AP1_PORT &= ~(1<<nRF24AP1_RESET_PIN);
	_delay_ms(1000);
	nRF24AP1_PORT |= (1<<nRF24AP1_RESET_PIN);
}



UCHAR checkSum(UCHAR *data, int length)
{
	int i;
	UCHAR chksum = data[0];

	for (i = 1; i < length; i++)
	{
		chksum ^= data[i];  // +1 since skip prefix sync code, we already counted it
	}
	
	return chksum;
}

void send_ant_packet( UCHAR msgID, UCHAR argCnt, ...) 
{
	va_list arg;
	va_start (arg, argCnt);
	uint8_t i;
	uint8_t buf[MESG_MAX_SIZE];
	buf[0] = MESG_TX_SYNC;
	buf[1] = argCnt;
	buf[2] = msgID;
	
	for(i = 0; i < argCnt; i++) 
	{
		buf[i+MESG_HEADER_SIZE] = va_arg(arg, int);
	}
	buf[argCnt+MESG_HEADER_SIZE] = checkSum(buf,argCnt+MESG_HEADER_SIZE);
	
	for(i = 0; i < argCnt+MESG_FRAME_SIZE; i++)
	{
		softuart_putchar(buf[i]);
	}
}

void ant_hr_config(void)
{
	send_ant_packet(MESG_SYSTEM_RESET_ID, 1, 0);
	_delay_ms(600);
	send_ant_packet(MESG_ASSIGN_CHANNEL_ID, 3, CHAN0, 0, NET0);	
	_delay_ms(1000);
	send_ant_packet(MESG_CHANNEL_ID_ID, 5, CHAN0, 0, 0, DEVICETYPE, 0);
	_delay_ms(100);
	send_ant_packet(MESG_NETWORK_KEY_ID, 9, NET0, 0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45);
	_delay_ms(100);
	send_ant_packet(MESG_CHANNEL_SEARCH_TIMEOUT_ID, 2, CHAN0, TIMEOUT);
	_delay_ms(100);	
	send_ant_packet(MESG_CHANNEL_RADIO_FREQ_ID, 2, CHAN0, FREQ);
	_delay_ms(20);
	send_ant_packet(MESG_CHANNEL_MESG_PERIOD_ID, 3, CHAN0, (PERIOD & 0x00FF), ((PERIOD & 0xFF00) >> 8));
	_delay_ms(20);
	send_ant_packet(MESG_OPEN_CHANNEL_ID, 1, CHAN0);
	_delay_ms(20);
}

/***********************************************************************
 * Name: get_ant_msg
 * 
 * Description: wait and recieve a message from the nRF24AP2
 * 
 * Author(s): Charles Wolf
 * 
 * @param: 
 * 		int	max_wait -> timeout in 1mSec increments
 * 		UCHAR *MSG 	-> pointer to the message array
 * @return:
 * 		int -> 	0 = message timeout
 * 				1 = message recieved
 * Comments:
 * 		ANT Message Structure
 * 			SYNC (0xA4)		1 Byte
 * 			length			1 Byte
 * 			message ID		1 Byte
 * 			Data 			length
 * 			CheckSum		1 Byte
 * 			
 * ********************************************************************/
int get_ant_msg(int max_wait, UCHAR *MSG)
{	
	int count = 0;
	int i = 0;
	int outcome = 0;
	int length = 0;
	int checksum = 0;
	
	softuart_turn_rx_on();	
	softuart_flush_input_buffer();
	
	while((count < max_wait) && (outcome != 1))
	{
		//wait for data
		while((softuart_kbhit() < 1) && (count < max_wait))
		{
			_delay_us(100); // 1bit = 208.333uSec
			count++;
		}
		if ( softuart_kbhit() > 0 )
		{
			outcome = 2; //no sync
			//check for sync
			MSG[0] = softuart_getchar();
			//softuart_getchar() is a possible place to get stuck waiting
			if(MSG[0] == MESG_TX_SYNC) 
			{
				MSG[1] = softuart_getchar(); //length
				length = MSG[1]+2;
				if(length > 17) length = 17; //protect length
				for (i = 0; i < length; i++)
				{
					MSG[i+2] = softuart_getchar();
				}
				checksum = checkSum(&MSG[0], length+3);
				if (checksum != MSG[length+4])
				{
					outcome = 3; //bad checksum
				}
				else
				{
					outcome = 1; //message recieved	
				}
			}
		}
	}
	softuart_turn_rx_off();
	return outcome;
}



