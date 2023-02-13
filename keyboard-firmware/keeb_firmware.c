/*
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#define LEDNUM 3
#define SW_L 8

<<<<<<< HEAD
// PWM
const int pwm_wrap = 65465;
const float pwm_duty[3] = {0.25, 0.5, 0.75};


// LED
const char LED_PIN = PICO_DEFAULT_LED_PIN;

//define led clock pin
const char led_clk = 27;

//define array of LED pins
const char led[LEDNUM] = {
=======
const unsigned char LED_PIN = PICO_DEFAULT_LED_PIN;

//define led clock pin
const unsigned char led_clk = 27;

//define array of LED pins
const unsigned char led[LEDNUM] = {
>>>>>>> 5b4c2ca8e709423e5e20248f608a8be3a366f61d
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
<<<<<<< HEAD
		//gpio_init(led[i]);
		//gpio_set_dir(led[i], GPIO_OUT);
=======
		gpio_init(led[i]);
		gpio_set_dir(led[i], GPIO_OUT);
>>>>>>> 5b4c2ca8e709423e5e20248f608a8be3a366f61d
		//set pwm
		gpio_set_function(led[i], GPIO_FUNC_PWM);
		led_sl[i] = pwm_gpio_to_slice_num(led[i]);
		led_ch[i] = pwm_gpio_to_channel(led[i]);
		printf("Slice=%d\tChannel=%d\n", led_sl[i], led_ch[i]);
<<<<<<< HEAD
=======
		//gpio_put(led[i], 1);
>>>>>>> 5b4c2ca8e709423e5e20248f608a8be3a366f61d
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
<<<<<<< HEAD
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
	pwm_set_clkdiv_int_frac (led_sl[ledIndex], 38, 3);
	pwm_set_wrap (led_sl[ledIndex], pwm_wrap);
	pwm_set_chan_level(led_sl[ledIndex], led_ch[ledIndex], (int)((float)pwm_wrap*pwm_duty[writeVal-1]));
}

void blinkClk () {
	gpio_put(led_clk, 1);
	sleep_ms(200);
	gpio_put(led_clk, 0);
	sleep_ms(200);
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
=======
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
>>>>>>> 5b4c2ca8e709423e5e20248f608a8be3a366f61d
}

void main(void) {
	stdio_init_all();
	init_pins();
<<<<<<< HEAD
	//declare array & allocate space
	char *pinArr = malloc(SW_L * 2 * sizeof(char));
	/*while (true) { 
		writeLED (0, 1);
		writeLED (1, 2);
		writeLED (2, 3);
	}*/
	writePWM(0x1b);
	while (true) {
		//remove contents of array
		for (char i = 0; pinArr[i] != 255; i++) pinArr[i] = 255;
		pinArr = poll_sw(pinArr);
		for (char i = 0; pinArr[i] != 255; i++) {
			//if ( pinArr[i] != 255) {
			printf("%d: %d\n", i, pinArr[i]);
			writePWM(pinArr[i]);
			//}
=======
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
>>>>>>> 5b4c2ca8e709423e5e20248f608a8be3a366f61d
		}
		//for (unsigned char i = 0; i < LEDNUM; i++) pwm_set_enabled(led_sl[0], false);
	}
}
