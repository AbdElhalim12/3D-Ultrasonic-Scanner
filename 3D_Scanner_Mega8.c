/*
 * _3D_Scanner_Mega8.c
 *
 * Created: 23.06.2015 18:38:27
 *  Author: Arthur Palmer
 *
 * This is a quick and dirty hack for my projekt "3D Ultrasonic Scanner":
 * http://www.thingiverse.com/thing:892800
 */ 

#define F_CPU 8000000UL

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

char stop = 1; // start/stop => sending 's' in terminal (250kbit, 8bit, no parity)

/*
	PORT C.5 => Ultrasonic Trigger
	PORT C.4 => Ultrasonic Echo
	
	PORT B.0 => ServoMotor Vertikal
	PORT B.1 => ServoMotor Horizontal
*/

void USART_Transmit( unsigned char data );

ISR(USART_RXC_vect)
{
	char c = UDR;
	if (c == 's') stop = (stop + 1) & 0x01;
	USART_Transmit (c);
}

void warte(int x)
{
	x = x / 1.9;
	while (x--)
		_delay_us(1);
}

int US_Messen()
{
	unsigned int max_us_time = 3000;
	unsigned int time = max_us_time;
	PORTC = 0x20;
	_delay_ms(10);
	PORTC = 0x00;
	while (!(PINC & 0x10));
	while ((PINC & 0x10) && time)
	{
		time--;
		//_delay_us(0.1);
	}
	return max_us_time-time;
}

void USART_Transmit( unsigned char data )
{
	while (!(UCSRA & (1<<UDRE)));
	UDR = data;
}

void serial_send_str( const char *s )
{
	while (*s != 0)
	{
		while (!(UCSRA & (1<<UDRE)));
		UDR = *s++;
	}
}

void init_serial( void )
{
	UBRRL = 1;	// 250kbit @ 8Mhz CPU_CLK
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
	UCSRC = (1 << URSEL) | (3 << UCSZ0);
}

void set_position ( int x, int y )
{
	int xdelay = 1500;
	int ydelay = 1500;

	PORTB = 0x01;
	warte(ydelay + y);
	PORTB = 0x00;
	warte(3000 - ydelay + y);

	PORTB = 0x02;
	warte(xdelay + x);
	PORTB = 0x00;
	warte(3000 - xdelay + x);
}

void init_adc()
{
	ADMUX = (1 << REFS0) | (3 << MUX0); // Ref = AVCC; ADC3;
	ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADFR) | (3 << ADPS0);
}

int IR_messen()
{
	return ADC;
}

int main( void )
{
	const char mode = 0;	// mode 0 => Ultraschall; 1 => Infrarot;
	
	int dist = 0;
	char buf[64];

	DDRC = 0b00100000;	// Ultraschall + LED
	DDRB = 0b00000011;	// Servo-Motoren
	
	init_serial();
	init_adc();
	
	sei();
	
	sprintf(buf, "Starting...\n\r");
	serial_send_str(buf);
	
	while(1)
	{
		for (int y = 300; y <= 1100; y += 10)
		{
			for (int x = -1000; x <= 1000; x += 20)
			{
				while (stop) set_position(x, y);
				set_position(x, y);
				if (mode == 0) dist = US_Messen();
				if (mode == 1) dist = IR_messen();
				//sprintf(buf, "X: %d; Y: %d; D: %u;\n\r", x, y, dist);
				sprintf(buf, "0X%04X,", dist);
				serial_send_str(buf);
			}
			for (int z = 0; z < 100; z++)
				set_position(-1000, y);
		}
		stop = 1;
	}
}