/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
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
#include "board.h"


/* LoRa Task Declarations */
#define LORATASKSTACKSIZE   2048
static Task_Struct loraTaskStruct;
static Char loraTaskStack[LORATASKSTACKSIZE];
static void loraTaskFxn(UArg arg0, UArg arg1);
static void StartLoraTask();

#define ISR_WORKER_QUEUE_SIZE 10
static Mailbox_Handle clbkkMbox;

//#include "adc-board.h"

///*!
// * Unique Devices IDs register set ( STM32L1xxx )
// */
//#define         ID1                                 ( 0x1FF80050 )
//#define         ID2                                 ( 0x1FF80054 )
//#define         ID3                                 ( 0x1FF80064 )

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
//    GpioInit( &DcDcEnable, DC_DC_EN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
//
//    /* Init the GPIO extender pins */
//    GpioInit( &IrqMpl3115, IRQ_MPL3115, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
//    GpioInit( &IrqMag3110, IRQ_MAG3110, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
//    GpioInit( &GpsPowerEn, GPS_POWER_ON, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
//    GpioInit( &RadioPushButton, RADIO_PUSH_BUTTON, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
//    GpioInit( &BoardPowerDown, BOARD_POWER_DOWN, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
//    GpioInit( &NcIoe5, SPARE_IO_EXT_5, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
//    GpioInit( &NcIoe6, SPARE_IO_EXT_6, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
//    GpioInit( &NcIoe7, SPARE_IO_EXT_7, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
//    GpioInit( &NIrqSx9500, N_IRQ_SX9500, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
//    GpioInit( &Irq1Mma8451, IRQ_1_MMA8451, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
//    GpioInit( &Irq2Mma8451, IRQ_2_MMA8451, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
//    GpioInit( &TxEnSx9500, TX_EN_SX9500, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
//    GpioInit( &Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//    GpioInit( &Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//    GpioInit( &Led3, LED_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//    GpioInit( &Led4, LED_4, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    // Init temperature, pressure and altitude sensor
//    MPL3115Init( );

    // Init accelerometer
//    MMA8451Init( );

    // Init magnetometer
//    MAG3110Init( );

    // Init SAR
//    SX9500Init( );

    // Init GPS
//    GpsInit( );

    // Switch LED 1, 2, 3, 4 OFF
//    GpioWrite( &Led1, 1 );
//    GpioWrite( &Led2, 1 );
//    GpioWrite( &Led3, 1 );
//    GpioWrite( &Led4, 1 );
}

void BoardInitMcu(void)
{
    clbkkMbox = Mailbox_create(sizeof(isr_worker_t), ISR_WORKER_QUEUE_SIZE, NULL, NULL);
    if (clbkkMbox == NULL)
    {
        System_abort("Failed to create lora callback mailbox");
    }

    GpioMcuInitInterrupt();
    SpiInit( &SX1276.Spi, (PinNames)Board_SX_MOSI, (PinNames)Board_SX_MISO, (PinNames)Board_SX_SCK, (PinNames)NC );
    SX1276IoInit( );

    StartLoraTask();

    if( McuInitialized == false )
    {
        McuInitialized = true;
//        if( GetBoardPowerSource( ) == BATTERY_POWER )
//        {
//            CalibrateSystemWakeupTime( );
//        }
    }

}

void BoardDeInitMcu( void )
{
//    Gpio_t ioPin;

    SpiDeInit( &SX1276.Spi );
    SX1276IoDeInit( );

//    GpioInit( &ioPin, OSC_HSE_IN, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//    GpioInit( &ioPin, OSC_HSE_OUT, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//
//    GpioInit( &ioPin, OSC_LSE_IN, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//    GpioInit( &ioPin, OSC_LSE_OUT, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//
//    GpioInit( &UsbDetect, USB_ON, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

uint32_t BoardGetRandomSeed( void )
{
	/// @todo Fetch some random seed, maybe based on unique ID or random hardware.
//    return ( ( *( uint32_t* )ID1 ) ^ ( *( uint32_t* )ID2 ) ^ ( *( uint32_t* )ID3 ) );
    return 0x34233566;
}

void BoardGetUniqueId( uint8_t *id )
{
	/// @todo Fetch CC2650 board unique ID
    id[7] = 0x01;
    id[6] = 0x02;
    id[5] = 0x03;
    id[4] = 0x04;
    id[3] = 0x05;
    id[2] = 0x06;
    id[1] = 0x07;
    id[0] = 0x08;
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
//#define BATTERY_MAX_LEVEL                           4150 // mV
//#define BATTERY_MIN_LEVEL                           3200 // mV
//#define BATTERY_SHUTDOWN_LEVEL                      3100 // mV

//uint16_t BoardGetPowerSupply( void )
//{
//    float vref = 0;
//    float vdiv = 0;
//    float batteryVoltage = 0;
//
//    AdcInit( &Adc, BAT_LEVEL );
//
//    vref = AdcMcuRead( &Adc, ADC_CHANNEL_17 );
//    vdiv = AdcMcuRead( &Adc, ADC_CHANNEL_8 );
//
//    batteryVoltage = ( FACTORY_POWER_SUPPLY * VREFINT_CAL * vdiv ) / ( vref * ADC_MAX_VALUE );
//
//    //                                vDiv
//    // Divider bridge  VBAT <-> 1M -<--|-->- 1M <-> GND => vBat = 2 * vDiv
//    batteryVoltage = 2 * batteryVoltage;
//
//    return ( uint16_t )( batteryVoltage * 1000 );
//}

uint8_t BoardGetBatteryLevel( void )
{
    return 254;
//    volatile uint8_t batteryLevel = 0;
//    uint16_t batteryVoltage = 0;
//
//    if( GpioRead( &UsbDetect ) == 1 )
//    {
//        batteryLevel = 0;
//    }
//    else
//    {
//        batteryVoltage = BoardGetPowerSupply( );
//
//        if( batteryVoltage >= BATTERY_MAX_LEVEL )
//        {
//            batteryLevel = 254;
//        }
//        else if( ( batteryVoltage > BATTERY_MIN_LEVEL ) && ( batteryVoltage < BATTERY_MAX_LEVEL ) )
//        {
//            batteryLevel = ( ( 253 * ( batteryVoltage - BATTERY_MIN_LEVEL ) ) / ( BATTERY_MAX_LEVEL - BATTERY_MIN_LEVEL ) ) + 1;
//        }
//        else if( batteryVoltage <= BATTERY_SHUTDOWN_LEVEL )
//        {
//            batteryLevel = 255;
//            //GpioInit( &DcDcEnable, DC_DC_EN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//            //GpioInit( &BoardPowerDown, BOARD_POWER_DOWN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
//        }
//        else // BATTERY_MIN_LEVEL
//        {
//            batteryLevel = 1;
//        }
//    }
//    return batteryLevel;
}

//static void BoardUnusedIoInit( void )
//{
//    Gpio_t ioPin;
//
//    if( GetBoardPowerSource( ) == BATTERY_POWER )
//    {
//        GpioInit( &ioPin, USB_DM, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//        GpioInit( &ioPin, USB_DP, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//    }
//
//    GpioInit( &ioPin, TEST_POINT1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//    GpioInit( &ioPin, TEST_POINT2, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//    GpioInit( &ioPin, TEST_POINT3, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//    GpioInit( &ioPin, TEST_POINT4, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//
//    GpioInit( &ioPin, PIN_NC, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//    GpioInit( &ioPin, BOOT_1, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
//
//    GpioInit( &ioPin, RF_RXTX, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//    GpioInit( &ioPin, WKUP1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//
//#if defined( USE_DEBUGGER )
//    HAL_DBGMCU_EnableDBGStopMode( );
//    HAL_DBGMCU_EnableDBGSleepMode( );
//    HAL_DBGMCU_EnableDBGStandbyMode( );
//#else
//    HAL_DBGMCU_DisableDBGSleepMode( );
//    HAL_DBGMCU_DisableDBGStopMode( );
//    HAL_DBGMCU_DisableDBGStandbyMode( );
//
//    GpioInit( &ioPin, SWDIO, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//    GpioInit( &ioPin, SWCLK, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//#endif
//}

//void SystemClockConfig( void )
//{
//    RCC_OscInitTypeDef RCC_OscInitStruct;
//    RCC_ClkInitTypeDef RCC_ClkInitStruct;
//    RCC_PeriphCLKInitTypeDef PeriphClkInit;
//
//    __HAL_RCC_PWR_CLK_ENABLE( );
//
//    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );
//
//    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
//    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
//    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
//    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
//    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
//    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
//    RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
//    HAL_RCC_OscConfig( &RCC_OscInitStruct );
//
//    RCC_ClkInitStruct.ClockType = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 );
//    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
//    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
//    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
//    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
//    HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_1 );
//
//    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
//    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
//    HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit );
//
//    HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq( ) / 1000 );
//
//    HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );
//
//    /*    HAL_NVIC_GetPriorityGrouping*/
//    HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_4 );
//
//    /* SysTick_IRQn interrupt configuration */
//    HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 );
//}

//void CalibrateSystemWakeupTime( void )
//{
//    if( SystemWakeupTimeCalibrated == false )
//    {
//        TimerInit( &CalibrateSystemWakeupTimeTimer, OnCalibrateSystemWakeupTimeTimerEvent );
//        TimerSetValue( &CalibrateSystemWakeupTimeTimer, 1000 );
//        TimerStart( &CalibrateSystemWakeupTimeTimer );
//        while( SystemWakeupTimeCalibrated == false )
//        {
//            TimerLowPowerHandler( );
//        }
//    }
//}

//void SystemClockReConfig( void )
//{
//    __HAL_RCC_PWR_CLK_ENABLE( );
//    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );
//
//    /* Enable HSE */
//    __HAL_RCC_HSE_CONFIG( RCC_HSE_ON );
//
//    /* Wait till HSE is ready */
//    while( __HAL_RCC_GET_FLAG( RCC_FLAG_HSERDY ) == RESET )
//    {
//    }
//
//    /* Enable PLL */
//    __HAL_RCC_PLL_ENABLE( );
//
//    /* Wait till PLL is ready */
//    while( __HAL_RCC_GET_FLAG( RCC_FLAG_PLLRDY ) == RESET )
//    {
//    }
//
//    /* Select PLL as system clock source */
//    __HAL_RCC_SYSCLK_CONFIG ( RCC_SYSCLKSOURCE_PLLCLK );
//
//    /* Wait till PLL is used as system clock source */
//    while( __HAL_RCC_GET_SYSCLK_SOURCE( ) != RCC_SYSCLKSOURCE_STATUS_PLLCLK )
//    {
//    }
//}

//void SysTick_Handler( void )
//{
//    HAL_IncTick( );
//    HAL_SYSTICK_IRQHandler( );
//}

uint8_t GetBoardPowerSource( void )
{
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

void ScheduleISRCallback(isr_worker_t callback) {
    assert(callback);
    if(!Mailbox_post(clbkkMbox, (Ptr)(&callback), BIOS_NO_WAIT)) {
        System_printf("Failed to post a LoRa ISR callback\n");
    }
}

static void loraTaskFxn(UArg arg0, UArg arg1)
{
    while (1)
    {
        isr_worker_t callback;
        if(!Mailbox_pend(clbkkMbox, (Ptr)(&callback), BIOS_WAIT_FOREVER)) {
            System_abort("Failed to pend on LoRa ISR callback mailbox\n");
        }
        assert(callback);
        callback();
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
