/*
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#define LEDNUM 3
#define SW_L 8

const short LED_PIN = PICO_DEFAULT_LED_PIN;

//define led clock pin
const short led_clk = 27;

//define array of LED pins
const short led[LEDNUM] = {
	7,	//LED_R
	6,	//LED_G
	28	//LED_B
};
//arrays to store led slices/channels
short led_ch[LEDNUM];
short led_sl[LEDNUM];

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
	gpio_init(led_clk);
	gpio_set_dir(led_clk, GPIO_OUT);
	for (short i = 0; i < LEDNUM; i++) {
		gpio_init(led[i]);
		gpio_set_dir(led[i], GPIO_OUT);
		//set pwm
		gpio_set_function(led[i], GPIO_FUNC_PWM);
		led_sl[i] = pwm_gpio_to_slice_num(led[i]);
		led_ch[i] = pwm_gpio_to_channel(led[i]);
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
	short depCount = 0;
	for (short i = 0; i < SW_L; i++) {
		gpio_put(sw_out[i], 1);
		for (short j = 0; j < SW_L; j++) {
			if ( gpio_get(sw_in[j]) ) {
				depCount++;
				coords[pIndex]= j;		//X
				coords[pIndex+SW_L] = i;	//Y
				pIndex++;
			}
		}
		gpio_put(sw_out[i], 0);
	}
	//printf("%d\n", depCount);
	return coords;
}

void main(void) {
	stdio_init_all();
	init_pins();
	short pwm_wrap = 400;
	float pwm_duty = 0.25;
	//declare array & allocate space
	short *coords = malloc(SW_L * 2 * sizeof(short));
	while (true) {
		//remove contents of array
		for (short i = 0; i < SW_L*2; i++) coords[i] = -1;
		coords = poll_sw(coords);
		/*for (int i = 0; i < SW_L; i++) {
			if ( coords[i] != -1) {
				printf("X%d: %d\t", i, coords[i]);
				printf("Y%d: %d\n", i, coords[i+SW_L]);
			}
		}*/
		printf("\n");
		for (short i = 0; i < LEDNUM; i++) {
			pwm_set_wrap (led_sl[i], pwm_wrap);
			pwm_set_enabled(led_sl[i], true);
			pwm_set_gpio_level(led[i], (int)(pwm_wrap*pwm_duty));
			printf("%f\t", pwm_duty);
			pwm_duty += 0.25;
		}
		printf("\n");
		pwm_duty = 0.25;
		//gpio_put(led_clk, 1);
		gpio_put(LED_PIN, 1);
		//for (int i = 0; i < LEDNUM; i++) gpio_put(led[i], 1);
		sleep_ms(250);
		//for (int i = 0; i < LEDNUM; i++) gpio_put(led[i], 0);
		for (short i = 0; i < LEDNUM; i++) pwm_set_enabled(led_sl[0], false);
		//gpio_put(LED_PIN, 0);
		gpio_put(led_clk, 0);
		sleep_ms(250);
	}
}
