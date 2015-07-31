/* quadSevenSegDisplay
 *
 * This firmware lets an ATmega328p display a string on 
 * a quad 7-seg "bubble" display. It acts as a dumb i2c
 * slave to receive any ASCII value that can be displayed
 * on it. It scrolls to show up to 50 characters.
 *
 * Any string terminated with a NULL (0x00) or an ACII
 * value less than 0x20 can be sent to it as an I2C 
 * slave, but only the first 50 characters will be 
 * displayed. 
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#define Disp1 4
#define Disp2 6
#define Disp3 5
#define Disp4 1
#define SL_ADDR 0x12

void initI2C(void);
void initSevenSeg(void);
void setDisplay(uint8_t);

typedef enum {Digit1, Digit2, Digit3, Digit4} sevenSegState;
sevenSegState stage = Digit1;

uint8_t display [50] = {'h', 'e', 'l', 'l', 'o'};
uint8_t display_length = 5;
uint8_t i2c_count = 0;
uint8_t display_pos = 0;
uint8_t i2c_reg;

ISR(TIMER1_COMPA_vect)
{	//RUNS EVERY ~0.8 SECONDS

	if (display_length < 4) {			// If the current string is
		switch(display_length) {		// less than 4 characters,
			case 1:						// we should pad it with 
			display[1] = ' ';			// spaces.
			case 2:
			display[2] = ' ';
			case 3:
			display[3] = ' ';
			break;
		}
	}
										//if display_length=4, no shifting occurs
	else if (display_length > 4) {
		display_pos++;
		if ( (display_pos+3) >= display_length )		//shift up to string end
			display_pos = 0;
	}
}

ISR(TIMER0_COMPA_vect)
{	//RUNS EVERY 5MS
	
	switch(stage) {
		case Digit1:
			PORTB ^= ((1 << Disp1) | (1 << Disp4));		//turn off Digit4 and turn on Digit1
			setDisplay(display[display_pos]);
			stage = Digit2;
			break;
		case Digit2:
			PORTB ^= ((1 << Disp2) | (1 << Disp1));		//turn off Digit1 and turn on Digit2
			setDisplay(display[display_pos+1]);
			stage = Digit3;
			break;
		case Digit3:
			PORTB ^= ((1 << Disp3) | (1 << Disp2));		//turn off Digit2 and turn on Digit3
			setDisplay(display[display_pos+2]);
			stage = Digit4;
			break;
		case Digit4:
			PORTB ^= ((1 << Disp4) | (1 << Disp3));		//turn off Digit3 and turn on Digit4
			setDisplay(display[display_pos+3]);
			stage = Digit1;
			break;
	}
}

ISR(TWI_vect)
{
	cli();

	switch(TWSR) {
		
		//SLAVE RECEIVER
		case 0x60:					//SLA+W
			i2c_count = 0;
			display_pos = 0;
			break;
		case 0x80:					//data byte received, ack'ed
			i2c_reg = TWDR;
			if (TWDR < 0x20) {			//if a NULL or similar character is received
				display_length = i2c_count;
				i2c_count = 0;
			}
			else if (i2c_count < 50) {
				display[i2c_count] = TWDR;
				i2c_count++;
			}
			break;
			
		//SLAVE TRANSMITTER
		case 0xA8:				//SLA+R
			TWDR = display[i2c_reg];
			break;
		case 0xB8:				//data byte transferred, ack
			TWDR = display[++i2c_reg];
			break;
		case 0xC0:				//data byte transferred, no ack
			break;
	}
	
	TWCR |= (1 << TWINT);		//clear the interrupt flag
	sei();
}

int main(void)
{
	initI2C();
	initSevenSeg();
	sei();

	while(1);
}

void initI2C(void)
{
	TWAR = (SL_ADDR << 1);									//slave address
	TWCR |= ((1 << TWEN) | (1 << TWEA) | (1 << TWIE));		//TWI enable; enable ack
	TWCR |= (1 << TWINT);									//clear the interrupt flag
}

void initSevenSeg(void)
{
	DDRB |= 0xFF;							//Port B Outputs
	DDRD |= 0xE0;							//Port D Outputs
	
	PORTB &= 0xFD;							//initial GND pins
	PORTB |= 0x70;							//initial GND pins

	//Timer 1: Scrolling at ~0.8 sec
	TCCR1B |= (1 << WGM12);					//Configure timer 1 for CTC mode
	TIMSK1 |= (1 << OCIE1A);				//Enable CTC T1 interrupt
	OCR1A = 12000;							//overflow and match at 
	TCCR1B |= ((1 << CS10) | (1 << CS11));	//prescale: none

	//Timer 0: Time-multiplexed display interrupt at 5ms
	TCCR0A |= (1 << WGM01);					//CTC mode
	TIMSK0 |= (1 << OCIE1A);				//interrupt on match A
	OCR0A = 78;								//output compare reg A
	TCCR0B |= ((1 << CS01) | (1 << CS00));	//prescale: 64

}

void setDisplay(uint8_t dig)
{
	switch(dig) {
		case '0':
		case 'O':
		case 'o':
			PORTB &= 0x7F;
			PORTB |= 0x0D;
			PORTD |= 0xEF;
			break;
		case '1':
		case 'I':
		case 'i':
			PORTB &= 0x76;
			PORTB |= 0x04;
			PORTD &= 0x9F;
			PORTD |= 0x80;
			break;
		case '2':
			PORTB &= 0xFB;
			PORTB |= 0x89;
			PORTD &= 0xBF;
			PORTD |= 0xA0;
			break;
		case '3':
			PORTB &= 0xF7;
			PORTB |= 0x85;
			PORTD &= 0xBF;
			PORTD |= 0xA0;
			break;
		case '4':
			PORTB &= 0xF6;
			PORTB |= 0x84;
			PORTD &= 0xDF;
			PORTD |= 0xC0;
			break;
		case '5':
		case 'S':
		case 's':
			PORTB &= 0xF7;
			PORTB |= 0x85;
			PORTD &= 0x7F;
			PORTD |= 0x60;
			break;
		case '6':
			PORTB |= 0x8D;
			PORTD &= 0x7F;
			PORTD |= 0x60;
			break;
		case '7':
			PORTB &= 0x77;
			PORTB |= 0x05;
			PORTD &= 0x9F;
			PORTD |= 0x80;
			break;
		case '8':
			PORTB |= 0x8D;
			PORTD |= 0xE0;
			break;
		case '9':
			PORTB &= 0xF7;
			PORTB |= 0x85;
			PORTD &= 0xDF;
			PORTD |= 0xC0;
			break;
		case 'A': //A
		case 'a':
			PORTB |= 0x8D;
			PORTD &= 0xDF;
			PORTD |= 0xC0;
			break;
		case 'B': //b
		case 'b':
			PORTB &= 0xFE;
			PORTB |= 0x8C;
			PORTD &= 0x7F;
			PORTD |= 0x60;
			break;
		case 'C': //c
		case 'c':
			PORTB &= 0xFA;
			PORTB |= 0x88;
			PORTD &= 0x3F;
			PORTD |= 0x20;
			break;
		case 'D': //d
		case 'd':
			PORTB &= 0xFE;
			PORTB |= 0x8C;
			PORTD &= 0xBF;
			PORTD |= 0xA0;
			break;
		case 'E': //E
		case 'e':
			PORTB &= 0xFB;
			PORTB |= 0x89;
			PORTD &= 0x7F;
			PORTD |= 0x60;
			break;
		case 'F': //F
		case 'f':
			PORTB &= 0xFB;
			PORTB |= 0x89;
			PORTD &= 0x5F;
			PORTD |= 0x40;
			break;
		case 'G': //G
		case 'g':
			PORTB &= 0xF7; 
			PORTB |= 0x85; 
			PORTD |= 0xE0;
			break;
		case 'H':	//upper-case H
		case 'h':
			PORTB &= 0xFE;
			PORTB |= 0x8C;
			PORTD &= 0xDF;
			PORTD |= 0xC0;
			break;
		case 'L':	//upper-case L
		case 'l':
			PORTB &= 0x7A;
			PORTB |= 0x08;
			PORTD &= 0x7F;
			PORTD |= 0x60;
			break;
		case 'P':	//upper-case p
		case 'p':
			PORTB &= 0xFB;
			PORTB |= 0x89;
			PORTD &= 0xDF;
			PORTD |= 0xC0;
			break;
		case 'R':
		case 'r':
			PORTB &= 0xFA;
			PORTB |= 0x88;
			PORTD &= 0x1F;
			break;
		case 'T':	//lower-case t
		case 't':
			PORTB &= 0xFA;
			PORTB |= 0x88;
			PORTD &= 0x7F;
			PORTD |= 0x60;
			break;
		case 'U':	//upper-case U
			PORTB &= 0x7E;
			PORTB |= 0x0C;
			PORTD |= 0xE0;
			break;
		case 'u':	//lower-case u
			PORTB &= 0x7E;
			PORTB |= 0x0C;
			PORTD &= 0x3F;
			PORTD |= 0x20;
			break;
		case 'Y':
		case 'y':
			PORTB &= 0xF6;
			PORTB |= 0x84;
			PORTD |= 0xE0;
			break;
		case '=':	//equals sign
			PORTB &= 0xF2;
			PORTB |= 0x80;
			PORTD &= 0x3F;
			PORTD |= 0x20;
			break;
		case ' ':	//space
			PORTB &= 0x72;
			PORTD &= 0x1F;
			break;	
		case '_':	//underline
			PORTB &= 0x72;
			PORTD &= 0x3F;
			PORTD |= 0x20;
			break;
		default:	//space
			PORTB &= 0x72;
			PORTD &= 0x1F;
			break;
	}
}
