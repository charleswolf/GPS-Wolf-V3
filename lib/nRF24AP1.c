#include "nRF24AP1.h"
#include "soft_uart.c"
#include <util/delay.h>

/**CHANGE LOG
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



void reset_msg(void)
{
   uint8_t i;
	uint8_t buf[5];

	buf[0] = MESG_TX_SYNC; // SYNC Byte
	buf[1] = 0x01; // LENGTH Byte
	buf[2] = MESG_SYSTEM_RESET_ID; // ID Byte
	buf[3] = 0x00; // Data Byte N (N=LENGTH)
	buf[4] = checkSum(buf,4);
	for(i = 0 ; i < 5 ; i++)
	{
		softuart_putchar(buf[i]);
	}

} 


void assignch(void) 
{
	uint8_t i;
	uint8_t buf[6];

	buf[0] = MESG_TX_SYNC;
	buf[1] = 0x02; 	// length
	buf[2] = MESG_REQUEST_ID;
	buf[3] = CHAN0; 
	buf[4] = MESG_CAPABILITIES_ID; //network number?  0??
	buf[5] = checkSum(buf,5);
	for(i = 0 ; i < 6 ; i++)
	{
		softuart_putchar(buf[i]);
	}
}

void assignch1(void) 
{
	uint8_t i;
	uint8_t buf[6];

	buf[0] = MESG_TX_SYNC;
	buf[1] = 0x02; // LENGTH 
	buf[2] = MESG_REQUEST_ID;
	buf[3] = CHAN0; 
	buf[4] = ((UCHAR)0x3D); //???
	buf[5] = checkSum(buf,5);
	for(i = 0 ; i < 6 ; i++)
	{
		softuart_putchar(buf[i]);
	}
}

void assignch2(void)
{
	uint8_t i;
	uint8_t buf[7];

	buf[0] = MESG_TX_SYNC;
	buf[1] = 0x03; // LENGTH 
	buf[2] = MESG_ASSIGN_CHANNEL_ID; 
	buf[3] = CHAN0;
	buf[4] = ((UCHAR)0x00); 
	buf[5] = NET0; 
	buf[6] = checkSum(buf,6);
	for(i = 0 ; i < 7 ; i++)
	{
		softuart_putchar(buf[i]);
	}
}


void assignch3(void)
{
	uint8_t i;
	uint8_t buf[9];

	buf[0] = MESG_TX_SYNC; // SYNC Byte
	buf[1] = 0x05; // LENGTH Byte
	buf[2] = MESG_CHANNEL_ID_ID; // ID Byte
	buf[3] = CHAN0;
	//device number (little-endian)
	buf[4] = ((UCHAR)0x00); //0 = wildcard
	buf[5] = ((UCHAR)0x00);	
	//device type
	buf[6] = ((UCHAR)0x00); //0 = wildcard
	//ID
	buf[7] = ((UCHAR)0x00); //0 = wildcard
	buf[8] = checkSum(buf,8);
	for(i = 0 ; i < 9 ; i++)
	{
		softuart_putchar(buf[i]);
	}
}

void assignch4(void)
{
	uint8_t i;
	uint8_t buf[13];

	buf[0] = MESG_TX_SYNC; // SYNC Byte
	buf[1] = 0x09; // LENGTH Byte
	buf[2] = MESG_NETWORK_KEY_ID; // ID Byte
	buf[3] = NET0;
	//hstr2hex(&buf[4], NETWORK_KEY, 16);  // dest, orig, size
	buf[4] = 0xB9;
	buf[5] = 0xA5;
	buf[6] = 0x21;
	buf[7] = 0xFB;
	buf[8] = 0xBD;
	buf[9] = 0x72;
	buf[10] = 0xC3;
	buf[11] = 0x45;
	buf[12] = checkSum(buf,12);
	for(i = 0 ; i < 13 ; i++)
	{
		softuart_putchar(buf[i]);
	}
}


void timeout(void)
{
	uint8_t i;
	uint8_t buf[6];

	buf[0] = MESG_TX_SYNC; // SYNC Byte
	buf[1] = 0x02; // LENGTH Byte
	buf[2] = MESG_CHANNEL_SEARCH_TIMEOUT_ID; // ID Byte
	buf[3] = CHAN0;
	buf[4] = TIMEOUT; 
	buf[5] = checkSum(buf,5);
	for(i = 0 ; i < 6 ; i++)
	{
		softuart_putchar(buf[i]);
	}
}


void frequency(void)
{
	uint8_t i;
	uint8_t buf[6];

	buf[0] = MESG_TX_SYNC; // SYNC Byte
	buf[1] = 0x02; // LENGTH Byte
	buf[2] = MESG_CHANNEL_RADIO_FREQ_ID; // ID Byte
	buf[3] = CHAN0;
	buf[4] = FREQ; 
	buf[5] = checkSum(buf,5);
	for(i = 0 ; i < 6 ; i++)
	{
		softuart_putchar(buf[i]);
	}
}

void channel_period(void)
{
	uint8_t i;
	uint8_t buf[7];

	buf[0] = MESG_TX_SYNC; // SYNC Byte
	buf[1] = 0x03; // LENGTH Byte
	buf[2] = MESG_CHANNEL_MESG_PERIOD_ID; // ID Byte
	buf[3] = CHAN0;
	//Period (little-endian)
	buf[4] = 0x1f; //mikec   buf[4] = 0x9A;  
	buf[5] = 0x86; //mikec   buf[5] = 0x19;  //   6554 = 25*256 + 154; 25=0x19; 154 = 0x9A
	buf[6] = checkSum(buf,6);
	for(i = 0 ; i < 7 ; i++)
	{
		softuart_putchar(buf[i]);
	}
}

void open_channel(void)
{
	uint8_t i;
	uint8_t buf[5];

	buf[0] = MESG_TX_SYNC; // SYNC Byte
	buf[1] = 0x01; // LENGTH Byte
	buf[2] = MESG_OPEN_CHANNEL_ID; // ID Byte
	buf[3] = CHAN0;
	buf[4] = checkSum(buf,4);
	for(i = 0 ; i < 5 ; i++)
	{
		softuart_putchar(buf[i]);
	}
}



void send_ant_packet( UCHAR msgID, UCHAR argCnt, ...) 
{
	va_list arg;
	va_start (arg, argCnt);
	uint8_t i;
	uint8_t buf[12];
	buf[0] = MESG_TX_SYNC;
	buf[1] = argCnt;
	buf[2] = msgID;
	
	for(i = 0; i < argCnt; i++) 
	{
		buf[i+3] = va_arg(arg, int);
	}
	buf[argCnt+3] = checkSum(buf,argCnt+3);
	
	for(i = 0; i < argCnt; i++)
	{
		softuart_putchar(buf[i]);
	}
}

void ant_hr_config(void)
{
/*	
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
*/
	reset_msg();
	_delay_ms(100);
	assignch();
	_delay_ms(1000);
	assignch1();
	_delay_ms(100);
	assignch2();
	_delay_ms(100);
	assignch3();
	_delay_ms(100);
	assignch4();
	_delay_ms(20);
	timeout();
	_delay_ms(20);
	frequency();
	_delay_ms(20);
	channel_period();
	_delay_ms(20);
	open_channel();
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
	softuart_turn_rx_on();	
	softuart_flush_input_buffer();
	
	int count = 0;
	int i = 0;
	while(count < max_wait)
	{
		//wait for data
		while((!softuart_kbhit()) && (count < max_wait))
		{
			_delay_ms(1); // 1mSec = 4.8bits
			count++;
		}
		//check for sync
		if(softuart_getchar() == MESG_TX_SYNC)
		{
			MSG[0] = 0xA4;
			MSG[1] = softuart_getchar(); //length
			for (i = 0; i < MSG[0]; i++)
			{
				MSG[i+2] = softuart_getchar();
			}
			softuart_turn_rx_off();
			return 1; //message recieved	
		}
	}
	softuart_turn_rx_off();
	return 0;
}



