/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Andreas Pella (IMST GmbH), Miguel Luis and Gregory Cristian
*/
#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "utilities.h"
#include "timer.h"
#include "delay.h"
#include "gpio.h"
#include "spi.h"
#include "radio.h"

#include "sx1276/sx1276.h"
#include "sx1276-board.h"

#include "cmsis_replacement.h"
#include <LORABUG_V3.1.h>

//////// DEBUG STUFF ////////////
void setPin(PIN_Id pin, uint_t value);
void togglePin(PIN_Id pin);
/////////////////////////////////


#define USE_BAND_915_HYBRID

/*!
 * Define indicating if an external IO expander is to be used
 */
//#define BOARD_IOE_EXT

/*!
 * Generic definition
 */
#ifndef SUCCESS
#define SUCCESS                                     1
#endif

#ifndef FAIL
#define FAIL                                        0
#endif

/*!
 * Board IO Extender pins definitions
 */
//#define IRQ_MPL3115                                 IOE_0
//#define IRQ_MAG3110                                 IOE_1
//#define GPS_POWER_ON                                IOE_2
//#define RADIO_PUSH_BUTTON                           IOE_3
//#define BOARD_POWER_DOWN                            IOE_4
//#define SPARE_IO_EXT_5                              IOE_5
//#define SPARE_IO_EXT_6                              IOE_6
//#define SPARE_IO_EXT_7                              IOE_7
//#define N_IRQ_SX9500                                IOE_8
//#define IRQ_1_MMA8451                               IOE_9
//#define IRQ_2_MMA8451                               IOE_10
//#define TX_EN_SX9500                                IOE_11
//#define LED_1                                       IOE_12
//#define LED_2                                       IOE_13
//#define LED_3                                       IOE_14
//#define LED_4                                       IOE_15

/*!
 * Board MCU pins definitions
 */

#define RADIO_RESET                                 Board_SX_RESET
//
//#define RADIO_MOSI                                  Board_SX_MOSI
//#define RADIO_MISO                                  Board_SX_MISO
//#define RADIO_SCLK                                  Board_SX_SCK
//#define RADIO_NSS                                   Board_SX_NSS
//
//#define RADIO_DIO_0                                 Board_SX_DIO0
//#define RADIO_DIO_1                                 Board_SX_DIO1
//#define RADIO_DIO_2                                 Board_SX_DIO2
//#define RADIO_DIO_3                                 Board_SX_DIO3
//#define RADIO_DIO_4                                 Board_SX_DIO4
//#define RADIO_DIO_5                                 Board_SX_DIO5
//#define RADIO_ANT_SWITCH_DP                         Board_SX_RF_CTRL1
//#define RADIO_ANT_SWITCH_DM                         Board_SX_RF_CTRL2

//#define RADIO_ANT_SWITCH_HF                         PIN_UNASSIGNED
//#define RADIO_ANT_SWITCH_LF                         PIN_UNASSIGNED
//
//#define OSC_LSE_IN                                  PIN_UNASSIGNED
//#define OSC_LSE_OUT                                 PIN_UNASSIGNED
//
//#define OSC_HSE_IN                                  PIN_UNASSIGNED
//#define OSC_HSE_OUT                                 PIN_UNASSIGNED
//
//#define USB_DM                                      PIN_UNASSIGNED
//#define USB_DP                                      PIN_UNASSIGNED
//
//#define I2C_SCL                                     PIN_UNASSIGNED
//#define I2C_SDA                                     PIN_UNASSIGNED
//
//#define BOOT_1                                      PIN_UNASSIGNED
//
//#define GPS_PPS                                     PIN_UNASSIGNED
//#define UART_TX                                     PIN_UNASSIGNED
//#define UART_RX                                     PIN_UNASSIGNED
//
//#define DC_DC_EN                                    PIN_UNASSIGNED
//#define BAT_LEVEL                                   PIN_UNASSIGNED
//#define WKUP1                                       PIN_UNASSIGNED
//#define USB_ON                                      PIN_UNASSIGNED
//
//#define RF_RXTX                                     PIN_UNASSIGNED
//
//#define SWDIO                                       PIN_UNASSIGNED
//#define SWCLK                                       PIN_UNASSIGNED
//
//#define TEST_POINT1                                 PIN_UNASSIGNED
//#define TEST_POINT2                                 PIN_UNASSIGNED
//#define TEST_POINT3                                 PIN_UNASSIGNED
//#define TEST_POINT4                                 PIN_UNASSIGNED
//
//#define PIN_NC                                      PIN_UNASSIGNED

/*!
 * LED GPIO pins objects
 */
//extern Gpio_t IrqMpl3115;
//extern Gpio_t IrqMag3110;
//extern Gpio_t GpsPowerEn;
//extern Gpio_t RadioPushButton;
//extern Gpio_t BoardPowerUp;
//extern Gpio_t NcIoe5;
//extern Gpio_t NcIoe6;
//extern Gpio_t NcIoe7;
//extern Gpio_t NIrqSX9500;
//extern Gpio_t Irq1Mma8451;
//extern Gpio_t Irq2Mma8451;
//extern Gpio_t TxEnSX9500;
//extern Gpio_t Led1;
//extern Gpio_t Led2;
//extern Gpio_t Led3;
//extern Gpio_t Led4;

/*!
 * MCU objects
 */
//extern Adc_t Adc;
//extern I2c_t I2c;
//extern Uart_t Uart1;
//#if defined( USE_USB_CDC )
//extern Uart_t UartUsb;
//#endif
//
//extern Gpio_t GpsPps;
//extern Gpio_t GpsRx;
//extern Gpio_t GpsTx;
//extern Gpio_t UsbDetect;
//extern Gpio_t Wkup1;
//extern Gpio_t DcDcEnable;
//extern Gpio_t BatVal;

enum BoardPowerSource
{
    USB_POWER = 0,//!< USB_POWER
    BATTERY_POWER //!< BATTERY_POWER
};

/*!
 * \brief Disable interrupts
 *
 * \remark IRQ nesting is managed
 */
void BoardDisableIrq( void );

/*!
 * \brief Enable interrupts
 *
 * \remark IRQ nesting is managed
 */
void BoardEnableIrq( void );

/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInitMcu( void );

/*!
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Measure the Potentiometer level
 *
 * \retval value  Potentiometer level ( value in percent )
 */
uint8_t BoardMeasurePotiLevel( void );

/*!
 * \brief Measure the Battery voltage
 *
 * \retval value  battery voltage in volts
 */
uint32_t BoardGetBatteryVoltage( void );

/*!
 * \brief Get the current battery level
 *
 * \retval value  battery level [  0: USB,
 *                                 1: Min level,
 *                                 x: level
 *                               254: fully charged,
 *                               255: Error]
 */
uint8_t BoardGetBatteryLevel( void );

/*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * \retval seed Generated pseudo random seed
 */
uint32_t BoardGetRandomSeed( void );

/*!
 * \brief Gets the board 64 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

/*!
 * \brief Get the board power source
 *
 * \retval value  power source [0: USB_POWER, 1: BATTERY_POWER]
 */
uint8_t GetBoardPowerSource( void );


typedef void (*isr_worker_t)(void);
void ScheduleISRCallback(isr_worker_t callback);

#endif // __BOARD_H__
