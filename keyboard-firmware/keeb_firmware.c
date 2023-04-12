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

//
volatile uint32_t clk_term_t = 0;
volatile bool clk_en = false;

volatile uint32_t tx_term_t = 0;
volatile bool tx_en = false;

volatile char led_bits[LEDNUM];

volatile uint32_t led_duty[LEDNUM];


volatile int32_t arrI;
volatile int32_t txLen = 0;

// PWM
const int pwm_freq = 20;
const int pwm_T_us = (int)(1000000.0 / (float)pwm_freq);
const uint32_t pwm_duties[] = {
	(uint32_t)((float)pwm_T_us*0.25),
	(uint32_t)((float)pwm_T_us*0.5), 
	(uint32_t)((float)pwm_T_us*0.75)
};
static int pwm_wrap;
static int pwm_level[4];

// LED
//define led clock pin
const char led_clk = 27;

//define array of LED pins
const char led[LEDNUM] = {
	7,	//LED_R
	6,	//LED_G
	28	//LED_B
};

// SWITCH ARRAY
//define pins used to receive (switch array -> pico)
const char sw_in[SW_L] = {
	8,	//Y0
	9,	//Y1
	10,	//Y2
	11,	//Y3
	20,	//Y4
	21,	//Y5
	22,	//Y6
	26	//Y7
};

//define pins used transmit (pico -> switch array)
const char sw_out[SW_L] = {
	15,	//X0
	14,	//X1
	13,	//X2
	12,	//X3
	16,	//X4
	17,	//X5
	18,	//X6
	19	//X7
};

void init_pins() {
	//Init switch pins
	for (char i = 0; i < SW_L; i++) {
		gpio_init(sw_out[i]);
		gpio_init(sw_in[i]);
		gpio_set_dir(sw_out[i], GPIO_OUT);
		gpio_set_dir(sw_in[i], GPIO_IN);
		gpio_put(sw_out[i], 0);
	}

	//clk
	gpio_init(led_clk);
	gpio_set_dir(led_clk, GPIO_OUT);
	gpio_put(led_clk, 1);

	//leds
	for (char i = 0; i < LEDNUM; i++) {
		gpio_init(led[i]);
		gpio_set_dir(led[i], GPIO_OUT);
		gpio_put(led[i], 1);
	}
}

char *poll_sw(char *pinArr) {
	char pIndex = 0;
	for (char i = 0; i < SW_L; i++) {
		gpio_put(sw_out[i], 1);
		//mysterious 1us delay
		busy_wait_us_32(1);
		for (char j = 0; j < SW_L; j++) {
			if ( gpio_get(sw_in[j]) ) {
				//debounce
				busy_wait_us_32(20000);
				if ( gpio_get(sw_in[j]) ) {
					pinArr[txLen++] = pIndex;
					//printf("0x%x:\t%d, %d\n", pIndex, j, i);
				}
			}
			++pIndex;
		}
		gpio_put(sw_out[i], 0);
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
	//printf("arrI: %d\ttxLen: %d\n", arrI, txLen);
	if (!arrI) setClk();
	arrI--;
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
			led_duty[i] = pwm_duties[led_bits[i]-1];
			led_alarm(i);
		}
	led_T_alarm();
}

void clk_irq () {
	//clear interrupt bits for alarm 0
	hw_clear_bits(&timer_hw->intr, 1 << 0);
	gpio_put(led_clk, 1);
}

volatile int tStart;

void clk_T_irq () {
	hw_clear_bits(&timer_hw->intr, 1 << 1);
	//printf("%d\n", time_us_32()-tStart);
	if (tx_en) setLEDs();
	clk_en = false;
}


void clk_alarm () {
	irq_remove_handler(0, irq_get_exclusive_handler(0));
	//enable interrupt for alarm 0 (disable led)
	hw_set_bits(&timer_hw->inte, 1 << 0);
	irq_set_exclusive_handler(0, clk_irq);
	irq_set_enabled(0, true);
	//clock has 50% duty
	uint32_t ledTarget = timer_hw->timerawl + pwm_duties[1];

	irq_remove_handler(1, irq_get_exclusive_handler(1));
	
	hw_set_bits(&timer_hw->inte, 1 << 1);
	irq_set_exclusive_handler(TIMER_IRQ_1, clk_T_irq);
	irq_set_enabled(TIMER_IRQ_1, true);
	uint32_t endTarget = timer_hw->timerawl + pwm_T_us;

	timer_hw->alarm[0] = ledTarget;
	timer_hw->alarm[1] = endTarget;
	tStart = time_us_32();
}

void setClk() {
	clk_en = true;
	gpio_put(led_clk, 0);
	clk_alarm();
}

void writePWM (char writeVal) {
	tx_en = true;
	for (char i = 0; i < LEDNUM; i++) 
		led_bits[i] = writeVal >> (2*i) & 3;

	//Change clock behavior based on array size/location
	if (arrI == txLen - 1) setClk();
	else setLEDs();
}

char *emptyArr (char *pinArr) {
	for (char i = 0; pinArr[i] != 255; i++) pinArr[i] = 255;
	return pinArr;
}

void main(void) {
	stdio_init_all();
	init_pins();
	//declare array & allocate space
	char *pinArr = malloc(SW_L * 2 * sizeof(char));
	//Set all indices to 'invalid'
	pinArr = emptyArr(pinArr);

	//printf("\nRESET!\n");

	while (true) {
		//remove contents of array
		if (arrI == -1) {
			pinArr = poll_sw(pinArr);
			arrI = txLen - 1;
		}

		if (arrI > -1 && !tx_en && !clk_en) {
			writePWM(pinArr[arrI]);
			if (!arrI) {
				txLen = 0;
				pinArr = emptyArr(pinArr);
			}
		}
	}
}


