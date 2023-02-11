/*
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#define LEDNUM 3
#define SW_L 8

const unsigned char LED_PIN = PICO_DEFAULT_LED_PIN;

//define led clock pin
const unsigned char led_clk = 27;

//define array of LED pins
const unsigned char led[LEDNUM] = {
	7,	//LED_R
	6,	//LED_G
	5	//LED_B
};
//arrays to store led slices/channels
char led_ch[LEDNUM];
char led_sl[LEDNUM];

//define pins used transmit (pico -> switch array)
const char sw_out[SW_L] = {
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
const char sw_in[SW_L] = {
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
	for (char i = 0; i < LEDNUM; i++) {
		gpio_init(led[i]);
		gpio_set_dir(led[i], GPIO_OUT);
		//set pwm
		gpio_set_function(led[i], GPIO_FUNC_PWM);
		led_sl[i] = pwm_gpio_to_slice_num(led[i]);
		led_ch[i] = pwm_gpio_to_channel(led[i]);
		printf("Slice=%d\tChannel=%d\n", led_sl[i], led_ch[i]);
		//gpio_put(led[i], 1);
	}

	//Init switch pins
	for (char i = 0; i < SW_L; i++) {
		gpio_init(sw_out[i]);
		gpio_init(sw_in[i]);
		gpio_set_dir(sw_out[i], GPIO_OUT);
		gpio_set_dir(sw_in[i], GPIO_IN);
		gpio_put(sw_out[i], 1);
	}
}

char *poll_sw(char *pinArr) {
	unsigned char pIndex = 0;
	unsigned char arrIndex = 0;
	for (char i = 0; i < SW_L; i++) {
		gpio_put(sw_out[i], 0);
		for (char j = 0; j < SW_L-4; j++) {
			pIndex++;
			if ( !gpio_get(sw_in[j]) ) {
				pinArr[arrIndex++] = pIndex;
			}
		}
		gpio_put(sw_out[i], 1);
	}
	return pinArr;
}

void main(void) {
	stdio_init_all();
	init_pins();
	short pwm_wrap = 65465;
	float pwm_duty;
	//declare array & allocate space
	char *pinArr = malloc(SW_L * 2 * sizeof(char));
	pwm_duty = 0.25;
	for (char i = 0; i < LEDNUM; i++) {
		pwm_set_clkdiv(led_sl[i], 38.1875);
		pwm_set_wrap (led_sl[i], pwm_wrap);
		pwm_set_gpio_level(led[i], (int)((float)pwm_wrap*pwm_duty));
		pwm_set_enabled(led_sl[i], true);
		pwm_duty += 0.25;
	}
	while (true) {
		//remove contents of array
		for (char i = 0; i < SW_L*2; i++) pinArr[i] = -1;
		pinArr = poll_sw(pinArr);
		for (char i = 0; i < SW_L; i++) {
			if ( pinArr[i] != 255) {
				printf("%d: %d\n", i, pinArr[i]);
			}
		}
		//for (unsigned char i = 0; i < LEDNUM; i++) pwm_set_enabled(led_sl[0], false);
	}
}
