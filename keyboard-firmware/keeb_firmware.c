/*
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#define LEDNUM 3
#define SW_L 8

// PWM
const float pwm_freq = 10.0;
const int pwm_T_us = (int)((1.0 / pwm_freq) * 1000000.0); 
static int pwm_wrap;
static int pwm_level[4];

// LED
const char LED_PIN = PICO_DEFAULT_LED_PIN;

//define led clock pin
const char led_clk = 27;
static char clk_sl, clk_ch;

//define array of LED pins
const char led[LEDNUM] = {
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
	//Init switch pins
	for (char i = 0; i < SW_L; i++) {
		gpio_init(sw_out[i]);
		gpio_init(sw_in[i]);
		gpio_set_dir(sw_out[i], GPIO_OUT);
		gpio_set_dir(sw_in[i], GPIO_IN);
		gpio_put(sw_out[i], 1);
	}
}

void configPWM () {
	//calculate divder and wrap
	float clkFreq = 125000000.0;
	double divider = (clkFreq / (4096.0 * pwm_freq))/16.0;
	pwm_wrap = (int)(clkFreq / divider / pwm_freq - 1.0);
	float duty = 0.0;

	//Calculate level for each duty
	for (char i = 0; i < sizeof pwm_level / sizeof pwm_level[0]; i++) {
		pwm_level[i] = (int)((float)pwm_wrap*duty);
		duty += 0.25;
	}

	//clk
	gpio_set_function(led_clk, GPIO_FUNC_PWM);
	clk_sl = pwm_gpio_to_slice_num(led_clk);
	clk_ch = pwm_gpio_to_channel(led_clk);

	//tx LEDs
	for (char i = 0; i < LEDNUM; i++) {
		gpio_set_function(led[i], GPIO_FUNC_PWM);
		led_sl[i] = pwm_gpio_to_slice_num(led[i]);
		led_ch[i] = pwm_gpio_to_channel(led[i]);

		//set pwm clock divider and wrap value
		pwm_set_clkdiv(led_sl[i], divider);
		pwm_set_wrap(led_sl[i], pwm_wrap);
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

void writeLED (char ledIndex, char writeVal) {
	pwm_set_chan_level(led_sl[ledIndex], led_ch[ledIndex], pwm_level[writeVal]);
	printf("%d\n", pwm_level[writeVal]);
}

void blinkClk() {
	pwm_set_chan_level(clk_sl, clk_ch, pwm_level[2]);
	add_alarm_in_us(pwm_T_us, 
}

void writePWM (char writeVal) {
	blinkClk();
	char halfNyb;
	for (char i = 0; i < LEDNUM; i++) {
		halfNyb = writeVal >> (2*i)&3;
		if (halfNyb) {
			writeLED(i, halfNyb);
			pwm_set_enabled(led_sl[i], true);
		}
		printf("%x\t", halfNyb);
	}
	printf("\n");
	sleep_ms(400);
	for (char i = 0; i < LEDNUM; i++) pwm_set_enabled(led_sl[i], false);
	blinkClk();
}


void main(void) {
	stdio_init_all();
	init_pins();
	configPWM();
	//declare array & allocate space
	char *pinArr = malloc(SW_L * 2 * sizeof(char));

	printf("%d\n", pwm_T_us);

	for (char i = 0; i < LEDNUM; i++) {
		writeLED(i, i+1);
		pwm_set_enabled(led_sl[i], true);
	}
	//writePWM(0x1b);
	/*while (true) {
		//remove contents of array
		for (char i = 0; pinArr[i] != 255; i++) pinArr[i] = 255;
		pinArr = poll_sw(pinArr);
		for (char i = 0; pinArr[i] != 255; i++) {
			//if ( pinArr[i] != 255) {
			printf("%d: %d\n", i, pinArr[i]);
			//writePWM(pinArr[i]);
			//}
		}
		//for (unsigned char i = 0; i < LEDNUM; i++) pwm_set_enabled(led_sl[0], false);
	}*/
}

