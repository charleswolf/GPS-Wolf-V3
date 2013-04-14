#include "nRF24AP1.h"

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

void nRF24AP1_init()
{
	//configure pins as outputs
	nRF24AP1_DIRECTION	|= ((1<<nRF24AP1_RESET_PIN) 
						| (1<<nRF24AP1_RTS_PIN)
						| (1<<nRF24AP1_SLEEP_PIN)
						| (1<<nRF24AP1_SUSPEND_PIN)
						| (1<<nRF24AP1_TX_PIN)
						);
	nRF24AP1_DIRECTION &= !(1<<nRF24AP1_RX_PIN);					
						
	//Configure Outputs
	nRF24AP1_PORT &= !(1<<nRF24AP1_TX_PIN); //not in use, idle low
	nRF24AP1_PORT &= !(1<<nRF24AP1_SLEEP_PIN); //disable sleep
	nRF24AP1_PORT |= (1<<nRF24AP1_SUSPEND_PIN); //enable communications
	
	//ensure part sees the reset then bring it out
	nRF24AP1_PORT &= !(1<<nRF24AP1_RESET_PIN);
	_delay_ms(100);
	nRF24AP1_PORT |= (1<<nRF24AP1_RESET_PIN);
	
}



