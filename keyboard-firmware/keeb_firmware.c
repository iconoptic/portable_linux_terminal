/*
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LEDNUM 4
#define SW_L 8

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

const int led[LEDNUM] = {
	27,	//LED_CLK
	7,	//LED_R
	6,	//LED_G
	28	//LED_B
};

const int sw_out[SW_L] = {
	8,	//Y0
	9,	//Y1
	10,	//Y2
	11,	//Y3
	26,	//Y4
	22,	//Y5
	21,	//Y6
	20	//Y7
};

const int sw_in[SW_L] = {
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
	for (int i = 0; i < LEDNUM; i++) {
		gpio_init(led[i]);
		gpio_set_dir(led[i], GPIO_OUT);
	}

	//Init switch pins
	for (int i = 0; i < SW_L; i++) {
		gpio_init(sw_out[i]);
		gpio_init(sw_in[i]);
		gpio_set_dir(sw_out[i], GPIO_OUT);
		gpio_set_dir(sw_in[i], GPIO_IN);
	}
}

int *poll_sw() {
	int *coords = malloc(SW_L * 2 * sizeof(int));
	int pIndex = 0;
	for (int i = 0; i < SW_L; i++) {
		gpio_put(sw_out[i], 1);
		for (int j = 0; j < SW_L; j++) {
			if ( gpio_get(sw_in[j]) ) {
				coords[pIndex]= j;	//X
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
	int *coords;
	while (true) {
//		struct keeb_coord aPin = poll_sw();
		coords = poll_sw();
	//	printf("%d,%d\n", coords[0], coords[1]);
		for (int i = 0; i < SW_L; i++) {
			printf("X%d: %d\n", i, coords[i]);
			printf("Y%d: %d\n\n", i, coords[i+SW_L]);
		}
		/*for (int i = 0; i < aPin.x[0]; i++) {
			gpio_put(led[0], 1);
			sleep_ms(100);
			gpio_put(led[0], 0);
			sleep_ms(100);
		}*/
		gpio_put(LED_PIN, 1);
		//for (int i = 0; i < LEDNUM; i++) gpio_put(led[i], 1);
		sleep_ms(250);
		//for (int i = 0; i < LEDNUM; i++) gpio_put(led[i], 0);
		gpio_put(LED_PIN, 0);
		sleep_ms(250);
		free(coords);
	}
}
