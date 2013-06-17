#ifndef _USART_H_
#define _USART_H_

#define FOSC 8000000	//clock speed
#define BAUD 9600
//#define MYUBRR FOSC/16/BAUD-1
#define MYUBRR 51
void USARTInit(int ubrr_value);
char USARTReadChar(void);
void USARTWriteChar(char data);
void uart_puts(char *s);
void uart_puts_p( const char *prg_s );

#define uart_puts_P(s___) uart_puts_p(PSTR(s___))

#endif //_USART_H_

// eof

