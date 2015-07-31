README.txt | 328p_quadSevenSegDisplay

	This is a firmware for the ATmega 328p that displays ASCII strings on a quad seven-
segment display, originally a "bubble-type", common-cathode one. It receives these
strings over I2C and can accept any length of string, but will only display the first 50 characters. 
	
NOTES
	. Slave address: see 328_quadSevenSegDisplay.c, "#define SL_ADDR" value. 

	. It does not function as a typical i2c slave, which might expose registers to
	  an inquiring master. Instead it assumes that the data being passed to it is
	  meant to be displayed. There is no "read" functionality yet. 

	. Some "read" functionality is implemented, but doing so changes the displayed
	  characters and resets the "display_length" variable used to determine the 
	  correct scrolling amount. The first read attempt (eg by Linux/GNU's i2cdump 
	  utility) will be correct, but will break the display until written to again.

	. The files were made in Atmel Studio, and may need other #include directives 
	  to work on other IDEs. 

WIRING
	 
  Seven Seg: 						328p Pins:
	 ____      a 						B0 : a   	D7 : b
	|	 |  f     b 					B2 : c 		D5 : d
	|	 |     g 						B2 : e		D6 : f
	 ---- 								B7 : g
	|	 |  e     c 					
	|____|     d 						B4 : Cat1	B6: Cat2
										B5 : Cat3	B1: Cat4


  Bubble Display: https://www.sparkfun.com/products/12710



