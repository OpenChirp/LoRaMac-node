/*
 * cmsis_replacement.c
 *
 *  Created on: Jan 8, 2017
 *      Author: craig
 */

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/hal/Hwi.h>
#include <stdbool.h>
//#include <std.h>

#define DELAY_MS(i)    Task_sleep(((i) * 1000) / Clock_tickPeriod)
#define DELAY_US(i)    Task_sleep(((i) * 1) / Clock_tickPeriod)

static bool hwi_disabled = 0;
static xdc_UInt hwi_restore_key;

void __enable_irq() {
	/// @todo Analyze code to figure out how to protect disable_irq parts
	if (hwi_disabled) {
	    Hwi_restore(hwi_restore_key);
	    hwi_disabled = false;
	}
}
void __disable_irq() {
	/// @todo Analyze code to figure out how to protect disable_irq parts
    if (!hwi_disabled) {
        hwi_restore_key = Hwi_disable();
        hwi_disabled = true;
    }
}

/**
 *
 * @param Delay Milliseconds
 */
//void HAL_Delay(uint32_t Delay) {
//	DELAY_MS(Delay);
//}
