/*
 */

#include <stdio.h>
//#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LEDNUM 3
#define SW_L 8

// LED
const char LED_PIN = PICO_DEFAULT_LED_PIN;

//define led clock pin
const char led_clk = 27;

//define array of LED pins
const char led[LEDNUM] = {
	7,	//LED_R
	6,	//LED_G
	5	//LED_B
};
//arrays to store led slices/channels
static char led_ch[LEDNUM];
static char led_sl[LEDNUM];

volatile char timerLen[LEDNUM];

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
		gpio_put(led[i], 1);
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
	char pIndex = 0;
	char arrIndex = 0;
	for (char i = 0; i < SW_L; i++) {
		gpio_put(sw_out[i], 0);
		for (char j = 0; j < SW_L; j++) {
			if ( !gpio_get(sw_in[j]) ) 
				pinArr[arrIndex++] = pIndex;
			++pIndex;
		}
		gpio_put(sw_out[i], 1);
	}
	return pinArr;
}

void blinkClk () {
	gpio_put(led_clk, 1);
	sleep_ms(200);
	gpio_put(led_clk, 0);
	sleep_ms(200);
}

int64_t led_alarm(alarm_id_t id, void *user_data) {
	if (gpio_get(led_clk)) gpio_put(led_clk, 0);
}

void writePWM (char writeVal) {
	if (gpio_get(led_clk)) {
		gpio_put(led_clk, 1);
		add_alarm_in_ms(100, led_alarm, NULL, false);
	} else {
		char halfNyb;
		for (char i = 0; i < LEDNUM; i++) {
			halfNyb = writeVal >> (2*i)&3;
			if (halfNyb) {
				gpio_put(led[i], 1);
			}
			printf("%x\t", halfNyb);
		}
		printf("\n");
	}
}

void main(void) {
	stdio_init_all();
	init_pins();
	//declare array & allocate space
	volatile char *pinArr = malloc(SW_L * 2 * sizeof(char));
	volatile char *pwmQueue = malloc(SW_L * 2 * sizeof(char));
	volatile char queueI = 255;
	while (true) {
		//remove contents of array
		for (char i = 0; pinArr[i] != 255; i++) pinArr[i] = 255;
		pinArr = poll_sw(pinArr);
		for (char i = 0; pinArr[i] != 255; i++) {
		       	printf("%d: %d\n", i, pinArr[i]);
			pwmQueue[++queueI] = 
		}
		if 
	}
	free(pinArr);
	free(pwmQueue);
}

