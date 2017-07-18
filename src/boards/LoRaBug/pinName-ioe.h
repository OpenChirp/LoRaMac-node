/**@file pinName-ioe.h
 * This file was intended to specify all of the SX1509 (I2C) IO Extension
 * pin names, when you are using the extension chip. Since the LoRaBug
 * does not make use of the SX1509 to communicate with the SX1276 radio,
 * we omit these labels.
 * These are used in the definition of the PinName enum in \a board.h
 *
 * @author Craig Hesling <craig@hesling.com>
 * @date Jan 7, 2017
 */

#ifndef __PIN_NAME_IOE_H__
#define __PIN_NAME_IOE_H__

#include <ti/drivers/PIN.h> // IOID_UNASSIGNED

// SX1509 Pin Names
#define IOE_PINS PIN_IOE_UNASSIGNED = PIN_UNASSIGNED

#endif // __PIN_NAME_IOE_H__
