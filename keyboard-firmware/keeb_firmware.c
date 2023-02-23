/*
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

#define LEDNUM 3
#define SW_L 8

#define ALRM_ID 0

//
volatile uint32_t clk_term_t = 0;
volatile bool clk_en = false;

volatile uint32_t tx_term_t = 0;
volatile bool tx_en = false;

volatile char led_bits[LEDNUM];

volatile uint32_t led_duty[LEDNUM];

// PWM
const int pwm_freq = 1;
const int pwm_T_us = (int)(1000000.0 / (float)pwm_freq);
const uint32_t pwm_duties[] = {
	0,
	(uint32_t)((float)pwm_T_us*0.25),
	(uint32_t)((float)pwm_T_us*0.5), 
	(uint32_t)((float)pwm_T_us*0.75)
};
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

void init_pins() {
	//Init switch pins
	for (char i = 0; i < SW_L; i++) {
		gpio_init(sw_out[i]);
		gpio_init(sw_in[i]);
		gpio_set_dir(sw_out[i], GPIO_OUT);
		gpio_set_dir(sw_in[i], GPIO_IN);
		gpio_put(sw_out[i], 1);
	}

	//clk
	gpio_init(led_clk);
	gpio_set_dir(led_clk, GPIO_OUT);

	//leds
	for (char i = 0; i < LEDNUM; i++) {
		gpio_init(led[i]);
		gpio_set_dir(led[i], GPIO_OUT);
		gpio_put(led[i], 1);
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

volatile char armed_mask;
void setClk();

void led_irq () {
	for (char i = 0; i < LEDNUM; i++) {
		if ( !((timer_hw->armed >> i) & 1) && ((armed_mask >> i) & 1)) {
			hw_clear_bits(&timer_hw->intr, 1 << i);
			gpio_put(led[i], 1);
		}
	}
}

void led_T_irq () {
	hw_clear_bits(&timer_hw->intr, 1 << 3);
	tx_en = false;
	setClk();
	//printf("%d", timer_hw->armed);
}

void led_alarm (char i) {
	uint32_t delay, ledTarget;

	irq_remove_handler(i, irq_get_exclusive_handler(i));

	hw_set_bits(&timer_hw->inte, 1 << i);
	irq_set_exclusive_handler(i, led_irq);
	irq_set_enabled(i, true);
	delay = (uint32_t)(led_duty[i]);
	ledTarget = timer_hw->timerawl + delay;
	timer_hw->alarm[i] = ledTarget;

}

void led_T_alarm () {
	irq_remove_handler(3, irq_get_exclusive_handler(3));
	
	hw_set_bits(&timer_hw->inte, 1 << 3);
	irq_set_exclusive_handler(TIMER_IRQ_3, led_T_irq);
	irq_set_enabled(3, true);
	uint32_t pwm_T_Target = timer_hw->timerawl + pwm_T_us;
	timer_hw->alarm[3] = pwm_T_Target;
	armed_mask = timer_hw->armed;
}

void setLEDs () {
	for (char i = 0; i < LEDNUM; i++)
		if (led_bits[i]) {
			gpio_put(led[i], 0);
			led_duty[i] = pwm_duties[led_bits[i]];
			led_alarm(i);
		}
	led_T_alarm();
}

void clk_T_irq () {
	//clear interrupt bits for alarm 0
	hw_clear_bits(&timer_hw->intr, 1 << 0);
	gpio_put(led_clk, 0);
}

volatile int tStart;

void clk_irq () {
	hw_clear_bits(&timer_hw->intr, 1 << 1);
	printf("%d\n", time_us_32()-tStart);
	if (tx_en) setLEDs();
}


void clk_alarm () {
	irq_remove_handler(0, irq_get_exclusive_handler(0));
	//enable interrupt for alarm 0 (disable led)
	hw_set_bits(&timer_hw->inte, 1 << 0);
	irq_set_exclusive_handler(0, clk_T_irq);
	irq_set_enabled(0, true);
	uint32_t delay = (uint32_t)((float)pwm_T_us*0.5);
	uint32_t ledTarget = timer_hw->timerawl + delay;

	irq_remove_handler(1, irq_get_exclusive_handler(1));
	
	hw_set_bits(&timer_hw->inte, 1 << 1);
	irq_set_exclusive_handler(TIMER_IRQ_1, clk_irq);
	irq_set_enabled(TIMER_IRQ_1, true);
	uint32_t endTarget = timer_hw->timerawl + pwm_T_us;

	timer_hw->alarm[0] = ledTarget;
	timer_hw->alarm[1] = endTarget;
	tStart = time_us_32();
}

void setClk() {
	gpio_put(led_clk, 1);
	clk_alarm();
}

void writePWM (char writeVal) {
	tx_en = true;
	for (char i = 0; i < LEDNUM; i++) 
		led_bits[i] = writeVal >> (2*i)&3;
	setClk();
}

void main(void) {
	stdio_init_all();
	init_pins();
	//declare array & allocate space
	char *pinArr = malloc(SW_L * 2 * sizeof(char));
	//initial delay to prevent timing errors
	sleep_ms(500);

	printf("\nRESET!\n");

	while (true) {
		//remove contents of array
		for (char i = 0; pinArr[i] != 255; i++) pinArr[i] = 255;
		pinArr = poll_sw(pinArr);

		for (char i = 0; pinArr[i] != 255; i++) {
			writePWM(pinArr[i]);
			break;
		}

	}
}

