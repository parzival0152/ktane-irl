#include "hardware/gpio.h"

#define TIMESTEP 459
#define DOT 1*TIMESTEP
#define DASH 3*TIMESTEP
#define WORD 5*TIMESTEP

void dot(uint8_t gpio){ 
	gpio_put(gpio, 1);
	sleep_ms(DOT);
	gpio_put(gpio, 0);
	sleep_ms(DOT);
}

void dash(uint8_t gpio){ 
	gpio_put(gpio, 1);
	sleep_ms(DASH);
	gpio_put(gpio, 0);
	sleep_ms(DOT);
}

void blink_letter(uint8_t gpio, char letter) {
	switch(letter){
		case 'a':
			dot(gpio);
			dash(gpio);
			break;
		case 'b':
			dash(gpio);
			dot(gpio);
			dot(gpio);
			dot(gpio);
			break;
		case 'c':
			dash(gpio);
			dot(gpio);
			dash(gpio);
			dot(gpio);
			break;
		case 'd':
			dash(gpio);
			dot(gpio);
			dot(gpio);
			break;
		case 'e':
			dot(gpio);
			break;
		case 'f':
			dot(gpio);
			dot(gpio);
			dash(gpio);
			dot(gpio);
			break;
		case 'g':
			dash(gpio);
			dash(gpio);
			dot(gpio);
			break;
		case 'h':
			dot(gpio);
			dot(gpio);
			dot(gpio);
			dot(gpio);
			break;
		case 'i':
			dot(gpio);
			dot(gpio);
			break;
		case 'j':
			dot(gpio);
			dash(gpio);
			dash(gpio);
			dash(gpio);
			break;
		case 'k':
			dash(gpio);
			dot(gpio);
			dash(gpio);
			break;
		case 'l':
			dot(gpio);
			dash(gpio);
			dot(gpio);
			dot(gpio);
			break;
		case 'm':
			dash(gpio);
			dash(gpio);
			break;
		case 'n':
			dash(gpio);
			dot(gpio);
			break;
		case 'o':
			dash(gpio);
			dash(gpio);
			dash(gpio);
			break;
		case 'p':
			dot(gpio);
			dash(gpio);
			dash(gpio);
			dot(gpio);
			break;
		case 'q':
			dash(gpio);
			dash(gpio);
			dot(gpio);
			dash(gpio);
			break;
		case 'r':
			dot(gpio);
			dash(gpio);
			dot(gpio);
			break;
		case 's':
			dot(gpio);
			dot(gpio);
			dot(gpio);
			break;
		case 't':
			dash(gpio);
			break;
		case 'u':
			dot(gpio);
			dot(gpio);
			dash(gpio);
			break;
		case 'v':
			dot(gpio);
			dot(gpio);
			dot(gpio);
			dash(gpio);
			break;
		case 'w':
			dot(gpio);
			dash(gpio);
			dash(gpio);
			break;
		case 'x':
			dash(gpio);
			dot(gpio);
			dot(gpio);
			dash(gpio);
			break;
		case 'y':
			dash(gpio);
			dot(gpio);
			dash(gpio);
			dash(gpio);
			break;
		case 'z':
			dash(gpio);
			dash(gpio);
			dot(gpio);
			dot(gpio);
			break;
		default:
			break;
	}
}

void blink_word(uint8_t gpio, const char* word, uint8_t length){
	for(uint i = 0; i < length; i++){
		blink_letter(gpio, word[i]);
		sleep_ms(DASH);
	}
	sleep_ms(WORD);
}
