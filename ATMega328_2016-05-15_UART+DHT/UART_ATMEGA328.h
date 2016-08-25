void init_UART(void)  
{   
	// USART Communication Parameters: 8 Data, 1 Stop, No Parity
	// USART Receiver: On
	// USART Transmitter: On
	// USART0 Mode: Asynchronous
	UCSR0A=(0<<RXC0) | (0<<TXC0) | (0<<UDRE0) | (0<<FE0) | (0<<DOR0) | (0<<UPE0) | (0<<U2X0) | (0<<MPCM0);
	UCSR0B=(0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (1<<RXEN0) | (1<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80);
	UCSR0C=(0<<UMSEL01) | (0<<UMSEL00) | (0<<UPM01) | (0<<UPM00) | (0<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00) | (0<<UCPOL0);
	//UBRR0H=0x00;	UBRR0L=0x33; // При 8Mhz USART Baud Rate: 9600
	UBRR0H=0x00;	UBRR0L=0x19; // При 16Mhz USART Baud Rate: 38400	
}

unsigned char getch_Uart(void)//	Получение байта
{
	while( !(UCSR0A&(1<<RXC0)) )	{} //	Устанавливается, когда регистр свободен
	return UDR0;
}

inline void send_Uart(unsigned char c)//	Отправка байта
{
	while( !(UCSR0A&(1<<UDRE0)) )	 {} //	Устанавливается, когда регистр свободен
	UDR0 = c;
}

inline void send_Uart_str( char* s)//	Отправка строки
{
	while (*s != 0) send_Uart(*s++);
}

uint_fast16_t pow10(uint_fast8_t ex) // Ф-я возведения 10 в степень.
{
	uint_fast16_t  ret=1;
	while  (ex--)  ret*= 10;
	return  ret;
}

void send_int_Uart(int i)//	Отправка целого числа
{
	unsigned char maxDigit=5,  dig=maxDigit;
	char *iStrCur;
	char *iStr="-32000"; //Выделение памяти для строки под максимальное возможное кол-во знаков.
	iStrCur= iStr; //Указатели теперь указывают на одинаковый адрес.
	if (i == 0)  iStr="0";
	else
	{
		if (i < 0)
		{
			i*= -1;
			*iStrCur++= '-';
		}
		
		while ( !( i / pow10(dig-1) ) )
		dig--;  // Отсекаем незначащие нули в старших разрядах.
		while (dig)
		{
			dig--;
			*iStrCur++= '0' + i / pow10( dig);
			//i= i % (int) round( pow(10, dig) );
			i= i % pow10( dig) ;
		}
		*iStrCur=0;
	}
	send_Uart_str(iStr);
} 
