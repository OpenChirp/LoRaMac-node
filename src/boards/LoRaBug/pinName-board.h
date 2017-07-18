/**@file pinName-board.h
 * This file expresses all possible CC2650 GPIO names.
 * These are used in the definition of the PinName enum in \a board.h
 *
 * @author Craig Hesling <craig@hesling.com>
 * @date Jan 7, 2017
 */

#ifndef __PIN_NAME_MCU_H__
#define __PIN_NAME_MCU_H__

#include <driverlib/ioc.h> // IOID_##

/*!
 * CC2650 Pin Names
 *
 * Since we cannot have macro names in an enumeration, we must have separate names.
 */
#define MCU_PINS               \
	PIN_IOID_0  = IOID_0,  \
	PIN_IOID_1  = IOID_1,  \
	PIN_IOID_2  = IOID_2,  \
	PIN_IOID_3  = IOID_3,  \
	PIN_IOID_4  = IOID_4,  \
	PIN_IOID_5  = IOID_5,  \
	PIN_IOID_6  = IOID_6,  \
	PIN_IOID_7  = IOID_7,  \
	PIN_IOID_8  = IOID_8,  \
	PIN_IOID_9  = IOID_9,  \
	PIN_IOID_10 = IOID_10, \
	PIN_IOID_11 = IOID_11, \
	PIN_IOID_12 = IOID_12, \
	PIN_IOID_13 = IOID_13, \
	PIN_IOID_14 = IOID_14, \
	PIN_IOID_15 = IOID_15, \
	PIN_IOID_16 = IOID_16, \
	PIN_IOID_17 = IOID_17, \
	PIN_IOID_18 = IOID_18, \
	PIN_IOID_19 = IOID_19, \
	PIN_IOID_20 = IOID_20, \
	PIN_IOID_21 = IOID_21, \
	PIN_IOID_22 = IOID_22, \
	PIN_IOID_23 = IOID_23, \
	PIN_IOID_24 = IOID_24, \
	PIN_IOID_25 = IOID_25, \
	PIN_IOID_26 = IOID_26, \
	PIN_IOID_27 = IOID_27, \
	PIN_IOID_28 = IOID_28, \
	PIN_IOID_29 = IOID_29, \
	PIN_IOID_30 = IOID_30, \
	PIN_IOID_31 = IOID_31

#endif // __PIN_NAME_MCU_H__
