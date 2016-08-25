#define F_CPU 16000000UL 
#include "DHT11-22_def.h"
 
#define _MASK_DHT  ( 0b00000001 << _PINNUM_DHT )
#define _PIN_DHT_GET       ( (_PIN_DHT & _MASK_DHT)>>_PINNUM_DHT )
#define _PORT_DHT_SET(x)   ( ((x)==0) ? (_PORT_DHT&= ~_MASK_DHT) : (_PORT_DHT|= _MASK_DHT) )
#define _DDR_DHT_SET(x)    ( ((x)==0) ? ( _DDR_DHT&= ~_MASK_DHT) : ( _DDR_DHT|= _MASK_DHT) )

static unsigned char  checkSum,  packDHT[5]= {0,0,0,0,0},
	dhtHighDuration=0,  minLevel=0,  maxLevel=0,  dhtSplitLevel=0;  
static float    temperature, humidity;

float getTemp() { return temperature; }
float getHum()  { return humidity; }
unsigned char getCheckSum() { return checkSum; }
unsigned char getMinLevel() {return minLevel; }
unsigned char getMaxLevel() {return maxLevel; }
unsigned char getDhtSplitLevel() {return dhtSplitLevel; }

static char initDHT() { // В случае ошибки инициации датчика возвращает этап на котором она произошла, иначе 0.
	char  dhtErr=0; 
	_DDR_DHT_SET(1); 	_PORT_DHT_SET(0);
	_delay_ms(19); 
	asm("cli");
	_DDR_DHT_SET(0);	_PORT_DHT_SET(1);
	_delay_us(10);
	if (!_PIN_DHT_GET)  dhtErr = 1;
	_delay_us(22);
	if ( (_PIN_DHT_GET)&&(!dhtErr) )   dhtErr = 2;
	_delay_us(88);
	if ( (!_PIN_DHT_GET)&&(!dhtErr) )  dhtErr = 3;
	_delay_us(77);
	if ( (_PIN_DHT_GET)&&(!dhtErr) )   dhtErr = 4; 
	return  dhtErr;
}

static void DhtMinMaxCalc()  { // Определяет максимальные и минимальньные длительности высокого уровня от датчика. Они нужны для последующего расчета dhtSplitLevel.
	dhtHighDuration= 0; // Если переменную создавать здесь, то показания неверные и в протеусе значение переменной не дебажится.
	while ( !_PIN_DHT_GET )    _delay_us(1);
	while (  _PIN_DHT_GET )  {
		_delay_us(1);
		dhtHighDuration++;
	}
	if      (minLevel > dhtHighDuration)  minLevel= dhtHighDuration;
	else if (maxLevel < dhtHighDuration)  maxLevel= dhtHighDuration;
}

unsigned char calibrateDHT() { // Для вычисления dhtSplitLevel. При успешной калибровке возвращает 1, иначе 0. 
	if ( initDHT() )  {   // Содержит  asm("cli");
		asm("sei");
		return 0;
	}
	for ( char bit=0;  bit < 40;  bit++)   DhtMinMaxCalc();
	asm("sei");
	
	dhtSplitLevel= (minLevel + maxLevel) / 2; // dhtSplitLevel - количество мкС, при удержании высокого уровня на пине данных датчика больше которых считаем, что датчик передает 1.
	return 1;
}

static char getDhtBit() { // Возвращает бит данных в зависимости от длительности высокого уровня на пине данных датчика.
	dhtHighDuration= 0; // Если переменную создавать здесь, то показания неверные и в протеусе значение переменной не дебажится.
	while ( !_PIN_DHT_GET )    _delay_us(1);
	while (  _PIN_DHT_GET )  {
		_delay_us(1);
		dhtHighDuration++;
	}
	if ( dhtHighDuration < dhtSplitLevel )	return 0;
	return 1;
}
//###############################################
#if defined _DHT11
static void calcResults() { // Получает из прочитанного пакета данных от датчика humidity, temperature, checkSum.
	temperature= packDHT[2];
	humidity= packDHT[0];
	checkSum= packDHT[4];
}
//###############################################
#elif defined _DHT22
static void calcResults() { // Получает из прочитанного пакета данных от датчика humidity, temperature, checkSum.
	temperature= packDHT[3]*0.1 + (packDHT[2] & 0b01111111)*25.6;
	if (packDHT[2] & 0b10000000)  temperature*= -1;
	humidity=  packDHT[1]*0.1 + packDHT[0]*25.6;
	checkSum= packDHT[4];
}
#endif
//###############################################

unsigned char readDHT() { // Возращает 1, если чтение датчика прошло успешно, иначе 0.   
	if ( initDHT() )  {   // Содержит  asm("cli");
		asm("sei");
		return 0;
	}
	for (unsigned char byte=0;  byte < 5;  byte++) {   // Начало считывания пакета данных от датчика.
		packDHT[byte] = 0;
		for ( char bit=0;  bit < 8;  bit++)
			packDHT[byte]= (packDHT[byte] << 1) | getDhtBit();
	}
	asm("sei");
	
	calcResults();
	return 1;
}
