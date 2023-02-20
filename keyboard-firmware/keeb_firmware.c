/*
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#define LEDNUM 3
#define SW_L 8

//
volatile uint32_t clk_term_t = 0;
volatile bool clk_en = false;

volatile uint32_t tx_term_t = 0;
volatile bool tx_en = false;

volatile char led_bits[LEDNUM];

// PWM
const float pwm_freq = 5;
const int pwm_T_us = (int)(1000000.0 / pwm_freq); 
static int pwm_wrap;
static int pwm_level[4];

// LED
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

// SWITCH ARRAY
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

void setClk();
void clrClk();

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
	//clock slice and channel
	clk_sl = pwm_gpio_to_slice_num(led_clk);
	clk_ch = pwm_gpio_to_channel(led_clk);

	//calculate divder and wrap
	float clkFreq = 125000000.0;
	double divider = (clkFreq / (4096.0 * pwm_freq))/16.0;
	pwm_wrap = (int)(clkFreq / divider / pwm_freq - 1.0);

	printf("\nDivider = %f\tpwm_wrap = %d\n", divider, pwm_wrap);

	float duty = 0.0;

	//Calculate level for each duty
	for (char i = 0; i < sizeof pwm_level / sizeof pwm_level[0]; i++) {
		pwm_level[i] = (int)((float)pwm_wrap*duty);
		duty += 0.25;
	}

	//clk
	gpio_set_function(led_clk, GPIO_FUNC_PWM);
	pwm_set_clkdiv(clk_sl, divider);
	pwm_set_wrap(clk_sl, pwm_wrap);
	pwm_set_chan_level(clk_sl, clk_ch, pwm_level[1]);

	//tx LEDs
	for (char i = 0; i < LEDNUM; i++) {
		gpio_set_function(led[i], GPIO_FUNC_PWM);
		led_sl[i] = pwm_gpio_to_slice_num(led[i]);
		led_ch[i] = pwm_gpio_to_channel(led[i]);

		//set pwm clock divider and wrap value
		pwm_set_clkdiv(led_sl[i], divider);
		pwm_set_wrap(led_sl[i], pwm_wrap);
		pwm_set_output_polarity(led_sl[i], true, true);
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

void clrLEDs () {
	for (char i = 0; i < LEDNUM; i++) pwm_set_enabled(led_sl[i], false);
	//setClk();
	tx_en = false;
}

void writeLED (char ledIndex) {
	pwm_set_chan_level(led_sl[ledIndex], led_ch[ledIndex], pwm_level[led_bits[ledIndex]]);
	pwm_set_enabled(led_sl[ledIndex], true);
}

void clrClk () { //(alarm_id_t id, void *user_data) {
	pwm_set_enabled(clk_sl, false);
	clk_en = false; 
	//printf("%d\n", time_us_32());
	clk_term_t = 0;
	if (!tx_en) {
		tx_en = true;
		for (char i = 0; i < LEDNUM; i++) writeLED(i);
		tx_term_t = time_us_32() + pwm_T_us;
	}
}

void setClk() {
	clk_en = true;
	pwm_set_enabled(clk_sl, true);
	clk_term_t = time_us_32() + (pwm_T_us);
	printf("%d\n", clk_term_t);
	//return add_alarm_in_us(pwm_T_us, clrClk, NULL, false);
}

void writePWM (char writeVal) {
	for (char i = 0; i < LEDNUM; i++) 
		led_bits[i] = writeVal >> (2*i)&3;
	setClk();
}

void main(void) {
	stdio_init_all();
	init_pins();
	configPWM();
	//declare array & allocate space
	char *pinArr = malloc(SW_L * 2 * sizeof(char));
	//initial delay to prevent timing errors
	sleep_ms(500);

	while (true) {
		//remove contents of array
		for (char i = 0; pinArr[i] != 255; i++) pinArr[i] = 255;
		pinArr = poll_sw(pinArr);

		for (char i = 0; pinArr[i] != 255; i++) {
			writePWM(pinArr[i]);
			break;
		}
		
		if (clk_en && time_us_32() >= clk_term_t ) clrClk();
		else if (tx_en && time_us_32() >= tx_term_t ) clrLEDs();
	}
}

