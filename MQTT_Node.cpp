/*
 * MQTT_Node.cpp
 *
 *	Created: 3-5-2015 21:37:15
 *	Author: Arjan Vuik
 *	Description: Handle topics relayed from a MQTT broker, via an ESP8266
 *	Device:ATTiny2313
 */ 


#define F_CPU 8000000UL
#define USART_BAUDRATE 115200
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdbool.h>

char ReceivedBuffer[30];
char TransmitBuffer[30];
char Byte;
char LF = 0x0A;
uint8_t ByteCount;
volatile uint8_t ReceivedByte;
volatile bool Recv = false;

//prototypes
ISR(USART_RXC_vect);//Interrupt vector
void UART_Init(void);//init UART
void USART_SendByte(char Data);//Send routine
void USART_Send(uint8_t size);//send handler
void RST_RCV_BUF(void);//receive buffer routine
char ByteRecv();//receive routine

int main(void)
{
	UART_Init(); //init UART
	sei();
	DDRB |= (1<<PB0)|(1<<PB1);
	PORTB = 0;

	memset(TransmitBuffer, 0, sizeof(TransmitBuffer));
    
	while(1)
    {
		if (Recv)
		{
			ReceivedBuffer[ByteCount]=ReceivedByte;
			Recv=0;
			ByteCount++;
			if(ByteCount > 100){
				memmove(&ReceivedBuffer[1], &ReceivedBuffer[0],sizeof(ReceivedBuffer)-1);
				ByteCount = 100;
			}
		}
		
		if(strstr(ReceivedBuffer, "/1121:1")){
			PORTB |= (1<<PB0);
			RST_RCV_BUF();		
		}
		if(strstr(ReceivedBuffer, "/1121:0")){
			PORTB &= ~((1<<PB0));
			RST_RCV_BUF();
		}
		if(strstr(ReceivedBuffer, "/1122:1")){
			PORTB |= (1<<PB1);
			RST_RCV_BUF();
		}
		if(strstr(ReceivedBuffer, "/1122:0")){
			PORTB &= ~((1<<PB1));
			RST_RCV_BUF();
		}
    }
}

ISR(USART_RX_vect)
{
	ReceivedByte = UDR;             //read UART register into value
	Recv = true;
}

void UART_Init (void)
{
	UCSRB = ((1<<TXEN)|(1<<RXEN) | (1<<RXCIE));   // Turn the transmission and reception circuitry on
	//UCSRC = ((1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1)); // Use 8-bit character sizes
	UBRRH = (BAUD_PRESCALE >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
	UBRRL = BAUD_PRESCALE; // Load lower 8-bits of the baud rate value into the low byte of the UBRR register

}

void USART_SendByte(char Data){

	// Wait until last byte has been transmitted
	while((UCSRA &(1<<UDRE)) == 0);

	// Transmit data
	UDR = Data;
}

void USART_Send(uint8_t size){
	for (int i = 0; size>i; i++)
	{
		USART_SendByte(TransmitBuffer[i]);
	}
	memset(TransmitBuffer, 0, sizeof(TransmitBuffer));
}
void RST_RCV_BUF(void){
	memset(ReceivedBuffer, 0, sizeof(ReceivedBuffer));
	ByteCount = 0;
}
