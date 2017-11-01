/**
 * @author Craig Hesling <craig@hesling.com>
 */

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/BIOS.h> // BIOS_WAIT_FOREVER

/* XDCtools Header files */
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h> // Error_Block

#include <stdbool.h>
#include <assert.h>
#include "gpio-board.h"
#include "cc26xx/system.h"
#include "board.h"


/* Callback Task Declarations */
#define LORATASKSTACKSIZE   2048
static Task_Struct loraTaskStruct;
static Char loraTaskStack[LORATASKSTACKSIZE];
static void loraTaskFxn(UArg arg0, UArg arg1);
static bool loraTaskEnabled = false;
static void StartLoraTask();
static void EnableLoraTask();
static void DisableLoraTask();

/* Mailbox parameters for callback task */
#define ISR_WORKER_QUEUE_SIZE 10
static Mailbox_Handle clbkkMbox;

/* Pin configuration for spi, when in sleep mode (deinit'ed) */
static PIN_Handle sxSpiSleepPinHandle;
static PIN_State sxSpiSleepPinState;
// Note: Simple PIN_INPUT_EN | PIN_PULL_DOWN does not work
static PIN_Config sxSpiSleepPinTable[] = {
     Board_SX_MOSI | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW,
     Board_SX_MISO | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW,
     Board_SX_SCK  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW,
     PIN_TERMINATE
};

/*!
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

/*!
 * Flag to indicate if the SystemWakeupTime is Calibrated
 */
//static bool SystemWakeupTimeCalibrated = false;

/*!
 * Callback indicating the end of the system wake-up time calibration
 */
//static void OnCalibrateSystemWakeupTimeTimerEvent( void )
//{
//    SystemWakeupTimeCalibrated = true;
//}


static bool hwi_disabled = 0;
static xdc_UInt hwi_restore_key;

/*!
 * Nested interrupt counter.
 *
 * \remark Interrupt should only be fully disabled once the value is 0
 */
static uint8_t IrqNestLevel = 0;

/**
 * TODO: Make these only Enable/Disable the GPIO IRQs
 */
void BoardDisableIrq(void)
{
    if (!hwi_disabled)
    {
        hwi_restore_key = Hwi_disable();
        hwi_disabled = true;
    }
    IrqNestLevel++;
}

/**
 * TODO: Make these only Enable/Disable the GPIO IRQs
 */
void BoardEnableIrq(void)
{
    IrqNestLevel--;
    if (IrqNestLevel == 0)
    {
        if (hwi_disabled)
        {
            Hwi_restore(hwi_restore_key);
            hwi_disabled = false;
        }
    }
}

void BoardInitPeriph( void )
{
}

void BoardInitMcu(void)
{
    if( McuInitialized == false )
    {
        McuInitialized = true;

        clbkkMbox = Mailbox_create(sizeof(isr_worker_t), ISR_WORKER_QUEUE_SIZE, NULL, NULL);
        if (clbkkMbox == NULL)
        {
            System_abort("Failed to create lora callback mailbox");
        }
        StartLoraTask();

//        if( GetBoardPowerSource( ) == BATTERY_POWER )
//        {
//            CalibrateSystemWakeupTime( );
//        }
    } else {
        /* Close SPI sleep profile */
        PIN_close(sxSpiSleepPinHandle);
    }

    SpiInit( &SX1276.Spi, (PinNames)Board_SX_MOSI, (PinNames)Board_SX_MISO, (PinNames)Board_SX_SCK, (PinNames)NC );
    SX1276IoInit( );
    EnableLoraTask();
}

void BoardDeInitMcu( void )
{
    DisableLoraTask();
    SpiDeInit( &SX1276.Spi );
    SX1276IoDeInit( );

    /* Set SPI MOSI, MISO, and SCK lines to output low */
    sxSpiSleepPinHandle = PIN_open(&sxSpiSleepPinState, sxSpiSleepPinTable);
    if (sxSpiSleepPinHandle == NULL)
    {
        System_abort("Failed to open board SX SPI Sleep pins\n");
    }

    /* we assume that elsewhere in the program the nRST, nCS, and both antenna control lines are configured for sleep already */
}

uint32_t BoardGetRandomSeed( void )
{
	/// @todo Fetch some random seed, maybe based on unique ID or random hardware.
//    return ( ( *( uint32_t* )ID1 ) ^ ( *( uint32_t* )ID2 ) ^ ( *( uint32_t* )ID3 ) );
    return (uint32_t)(FCFG1_MAC_15_4_Addr() & 0xFFFFFFFF);
}

/**
 * We use the factory IEEE 15.4 address
 * @param id The 8 byte unique id
 */
void BoardGetUniqueId( uint8_t *id )
{
    // Previously, we set id[7] through id[0] byte by byte
	*((uint64_t *)id) = FCFG1_MAC_15_4_Addr();
}

///*!
// * Factory power supply
// */
//#define FACTORY_POWER_SUPPLY                        3.0L
//
///*!
// * VREF calibration value
// */
//#define VREFINT_CAL                                 ( *( uint16_t* )0x1FF80078 )
//
///*!
// * ADC maximum value
// */
//#define ADC_MAX_VALUE                               4096
//
///*!
// * Battery thresholds
// */
#define BATTERY_MAX_LEVEL                           3140 // mV
#define BATTERY_MIN_LEVEL                           2000 // mV

uint32_t BoardGetBatteryVoltage( void )
{

	// Read the battery voltage (V), only the first 12 bits
	uint16_t voltage = (uint16_t)AONBatMonBatteryVoltageGet();

	// Convert to from V to mV to avoid fractions.
	// Fractional part is in the lower 8 bits thus converting is done as follows:
	// (1/256)/(1/1000) = 1000/256 = 125/32
	// This is done most effectively by multiplying by 125 and then shifting
	// 5 bits to the right.
	voltage = (voltage * 125) >> 5;
	return voltage;

}

/* \brief Get the current battery level
*
* \retval value  battery level [  0: USB,
*                                 1: Min level,
*                                 x: level
*                               254: fully charged,
*                               255: Error]
*/
uint8_t BoardGetBatteryLevel( void )
{
    volatile uint8_t batteryLevel = 0;
    uint16_t batteryVoltage = BoardGetBatteryVoltage();

    // Detect if it is conNected to USB
    if( batteryVoltage > 3310 )
    {
        batteryLevel = 0;
    }
    else if( batteryVoltage >= BATTERY_MAX_LEVEL )
    {
    	batteryLevel = 254;
    }
    else if( ( batteryVoltage > BATTERY_MIN_LEVEL ) && ( batteryVoltage < BATTERY_MAX_LEVEL ) )
	{
    	batteryLevel = ( ( 253 * ( batteryVoltage - BATTERY_MIN_LEVEL ) ) / ( BATTERY_MAX_LEVEL - BATTERY_MIN_LEVEL ) ) + 1;
	}
	else if( batteryVoltage <= BATTERY_MIN_LEVEL )
	{
            batteryLevel = 255;
    }

    return batteryLevel;
}


uint8_t GetBoardPowerSource( void )
{
    //TODO: Implement this by checking the UART lines for the FTDI's signature pullup on the RX line.

//#if defined( USE_USB_CDC )
//    if( GpioRead( &UsbDetect ) == 1 )
//    {
//        return BATTERY_POWER;
//    }
//    else
//    {
//        return USB_POWER;
//    }
//#else
    return USB_POWER;
//#endif
}

static void StartLoraTask() {
    /* Start LoRa Task */
    Task_Params loraTaskParams;
    Task_Params_init(&loraTaskParams);
    loraTaskParams.stackSize = LORATASKSTACKSIZE;
    loraTaskParams.stack = &loraTaskStack;
    loraTaskParams.priority = 3;
    //loraTaskParams.instance->name = "lora_radio_task";
    Task_construct(&loraTaskStruct, (Task_FuncPtr) loraTaskFxn, &loraTaskParams,
                   NULL);
}

static void EnableLoraTask() {
    loraTaskEnabled = true;
}

static void DisableLoraTask() {
    loraTaskEnabled = false;
}

void ScheduleISRCallback(isr_worker_t callback) {
    assert(callback);
    if (loraTaskEnabled) {
        if(!Mailbox_post(clbkkMbox, (Ptr)(&callback), BIOS_NO_WAIT)) {
            System_printf("Failed to post a LoRa ISR callback\n");
        }
    }
}


/**
 * This is the high priority task that runs the pin interrupt and clock
 * callbacks for the LoRaMAC and radio library. A separate task is
 * needed because library likes to do a lot of work and call multiple
 * blocking functions during the callbacks.
 * The down side to this implementation of performing callback is that we
 * cannot guarantee run-to-completion priority.
 *
 * @param arg0 Unused TI-RTOS argument
 * @param arg1 Unused TI-RTOS argument
 */
static void loraTaskFxn(UArg arg0, UArg arg1)
{
    while (1)
    {
        isr_worker_t callback;
        if(!Mailbox_pend(clbkkMbox, (Ptr)(&callback), BIOS_WAIT_FOREVER)) {
            System_abort("Failed to pend on LoRa ISR callback mailbox\n");
        }
        assert(callback);

        if (loraTaskEnabled) {
            callback();
        }
    }
}

#ifdef USE_FULL_ASSERT
/*
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 */
void assert_failed( uint8_t* file, uint32_t line )
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while( 1 )
    {
    }
}
#endif
