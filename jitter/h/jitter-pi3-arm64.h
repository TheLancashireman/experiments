/* jitter-pi3-arm64.h - configuration to allow the jitter experiment to run on a raspberry pi3 (64-bit)
 *
 * (c) 2019 David Haworth
*/
#ifndef JITTER_PI3_ARM64_H
#define JITTER_PI3_ARM64_H	1

#include <dv-arm-bcm2835-gpio.h>
#include <dv-arm-bcm2835-uart.h>
#include <dv-arm-bcm2835-aux.h>
#include <dv-arm-bcm2835-interruptcontroller.h>
#if 0	/* FIXME */
#include <dv-armv6-mmu.h>
#include <dv-arm-cp15.h>
#endif
#include <dv-arm-bcm2835-systimer.h>
#include <dv-arm-bcm2835-armtimer.h>

#define hw_UartInterruptId		dv_iid_aux
#define hw_TimerInterruptId		dv_iid_timer

static inline void hw_ClearTimer(void)
{
	dv_arm_bcm2835_armtimer_clr_irq();		/* Clear the interrupt */
}

static inline void hw_SetLed(int i, dv_boolean_t state)
{
	if ( (i < 0) || (i >= 4) )	return;

	static const dv_u32_t led_map[4] = {17, 18, 27, 22};
	if ( state )
		dv_arm_bcm2835_gpio_pin_clear(led_map[i]);
	else
		dv_arm_bcm2835_gpio_pin_set(led_map[i]);
}

static inline void hw_EnableUartRxInterrupt(void)
{
	dv_arm_bcm2835_uart.ier |= DV_IER_RxInt;
}

static inline void hw_InitialiseMillisecondTicker(int millis)
{
	dv_arm_bcm2835_armtimer_init(1);			/* Use a prescaler of 1 for high resolution */
	dv_arm_bcm2835_armtimer_set_load(250000 * millis);
}

#endif
