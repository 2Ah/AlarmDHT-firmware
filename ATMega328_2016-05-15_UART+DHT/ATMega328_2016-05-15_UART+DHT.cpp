#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "DHT11-22.h"
#include "UART_ATMEGA328.h"

#define _PORT_LED_POWER(x)  ( ((x)==0) ? (PORTD&= ~0b00010000) : (PORTD|= 0b00010000) ) /* Светодиод питания. При отсутствии DHT датчика - мигаем.*/  

int main() {  
	char st[6];  
	DDRB=  0b00000000;  
	PORTB= 0b11111111;
	DDRC=  0b00000000;  
	PORTC= 0b11111111;
	DDRD=  0b00010000;  //DHT22 + LED_POWER 
	PORTD= 0b11101111;
	_PORT_LED_POWER(1);
	init_UART();
	_delay_ms(999); 
	calibrateDHT();
	/*
	itoa( calibrateDHT(), st, 10); 
	send_Uart_str("\r\ncalibrateDHT=");  send_Uart_str(st); 
	itoa( getMinLevel(), st, 10);
	send_Uart_str(" min=");   send_Uart_str(st);
	itoa( getMaxLevel(), st, 10);
	send_Uart_str(" max=");   send_Uart_str(st);
	itoa( getDhtSplitLevel(), st, 10);
	send_Uart_str(" split=");   send_Uart_str(st); //*/
	while(1)  {
		_delay_ms(999);   
		if ( !readDHT() )  { // При отсутствии DHT датчика - мигаем.
			_PORT_LED_POWER(0);
			_delay_ms(999);
			_PORT_LED_POWER(1); 
			send_Uart_str("\r\nDHT ERROR"); 
			continue;
		}
		itoa( getTemp(), st, 10); // Запись в переменную st целой части значения температуры.
		send_Uart_str("\r\nDHT T=");   send_Uart_str(st);
		 
		itoa( getHum(), st, 10); // Запись в переменную st целой части значения влажности.
		send_Uart_str(" H=");   send_Uart_str(st); 
	}  
}