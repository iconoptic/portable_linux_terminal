/*
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LEDNUM 4
#define SW_L 8

const uint LED_PIN = PICO_DEFAULT_LED_PIN;


//define array of LED pins
const short led[LEDNUM] = {
	27,	//LED_CLK
	7,	//LED_R
	6,	//LED_G
	28	//LED_B
};

//define pins used transmit (pico -> switch array)
const short sw_out[SW_L] = {
	8,	//Y0
	9,	//Y1
	10,	//Y2
	11,	//Y3
	26,	//Y4
	22,	//Y5
	21,	//Y6
	20	//Y7
};

//define pins used to receive (switch array -> pico)
const short sw_in[SW_L] = {
	12,	//X0
	13,	//X1
	14,	//X2
	15,	//X3
	19,	//X4
	18,	//X5
	17,	//X6
	16	//X7
};
	
void init_pins() {
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);

	//Init LED pins
	for (short i = 0; i < LEDNUM; i++) {
		gpio_init(led[i]);
		gpio_set_dir(led[i], GPIO_OUT);
	}

	//Init switch pins
	for (short i = 0; i < SW_L; i++) {
		gpio_init(sw_out[i]);
		gpio_init(sw_in[i]);
		gpio_set_dir(sw_out[i], GPIO_OUT);
		gpio_set_dir(sw_in[i], GPIO_IN);
	}
}

short *poll_sw(short *coords) {
	short pIndex = 0;
	for (short i = 0; i < SW_L; i++) {
		gpio_put(sw_out[i], 1);
		for (short j = 0; j < SW_L; j++) {
			if ( gpio_get(sw_in[j]) ) {
				coords[pIndex]= j;		//X
				coords[pIndex+SW_L] = i;	//Y
				pIndex++;
			}
		}
		gpio_put(sw_out[i], 0);
	}
	return coords;
}

void main(void) {
	stdio_init_all();
	init_pins();
	//declare array & allocate space
	short *coords = malloc(SW_L * 2 * sizeof(short));
	while (true) {
		//remove contents of array
		for (short i = 0; i < SW_L*2; i++) coords[i] = -1;
		coords = poll_sw(coords);
		for (int i = 0; i < SW_L; i++) {
			if ( coords[i] != -1) {
				printf("X%d: %d\n", i, coords[i]);
				printf("Y%d: %d\n\n", i, coords[i+SW_L]);
			}
		}
		gpio_put(LED_PIN, 1);
		//for (int i = 0; i < LEDNUM; i++) gpio_put(led[i], 1);
		sleep_ms(250);
		//for (int i = 0; i < LEDNUM; i++) gpio_put(led[i], 0);
		gpio_put(LED_PIN, 0);
		sleep_ms(250);
	}
}
