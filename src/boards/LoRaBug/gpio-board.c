/**
 * Since the library likes to do lots of long blocking operations, we need callbacks to run in a complete task.
 */

#include "board.h"
#include "gpio-board.h"

#include <LORABUG_V3.1.h>

/* XDCtools Header files */
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h> // Error_Block

/* BIOS Header files */
#include <ti/sysbios/knl/Event.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>

static PIN_State pinState;
static PIN_Handle pinHandle = NULL;

//static Event_Handle sxevents;
//static Error_Block eb;

static GpioIrqHandler *GpioIrq[6] = { 0 };

static int8_t irqPinId2Index(uint8_t pinId);
static void pinIntCallback(PIN_Handle handle, PIN_Id pinId);

void GpioMcuInit(Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config,
                 PinTypes type, uint32_t value)
{
    PIN_Config pconfig = (PIN_Config) (uint32_t) pin;
    obj->pin = pin;
    obj->pinIndex = pin; // happens to be the same value

    // Set Input / Output
    switch (mode)
    {
    case PIN_INPUT:
        pconfig |= PIN_INPUT_EN;
        break;
    case PIN_OUTPUT:
        pconfig |= PIN_GPIO_OUTPUT_EN;
        break;
    case PIN_ALTERNATE_FCT:
    case PIN_ANALOGIC:
        // not supported, so default to input
        pconfig |= PIN_INPUT_EN;
        break;
    }

    // Set drive method
    pconfig |= (config == PIN_PUSH_PULL) ? PIN_PUSHPULL : PIN_OPENDRAIN;

    // Set pull-ups/downs
    if (mode == PIN_INPUT)
    {
        switch (type)
        {
        case PIN_NO_PULL:
            pconfig |= PIN_NOPULL;
        case PIN_PULL_UP:
            pconfig |= PIN_PULLUP;
        case PIN_PULL_DOWN:
            pconfig |= PIN_PULLDOWN;
        }
    }

    // Set initial value
    if (mode == PIN_OUTPUT)
    {
        pconfig |= value ? PIN_GPIO_HIGH : PIN_GPIO_LOW;
    }

    if (pinHandle == NULL)
    {
        PIN_Status status;
        // Open new handle
        PIN_Config pconfigs[] = { pconfig, PIN_TERMINATE };
        pinHandle = PIN_open(&pinState, pconfigs);
        if (pinHandle == NULL)
        {
            // Error opening pin
            System_abort("Failed to open pin\n");
        }
        // register a default callback for all pins - this does not enable the interrupt
        status = PIN_registerIntCb(pinHandle, pinIntCallback);
        if (status != PIN_SUCCESS)
        {
            System_abort("Failed to register interrupt callback for pin\n");
        }
    }
    else
    {
        PIN_Status status;
        /// @note Since callers like to use this to reconfigure
        /// and I don't see a clear way to query the handle about
        /// which pins are added, we simply try to add the pin
        /// (which may fail) and then set config

        // Add pin to open handle
        status = PIN_add(pinHandle, pconfig);
//    	if(status != PIN_SUCCESS) {
//    		// Error adding pin
//    		while(1) ;
//    	}
        // now ensure the config is set if caller is
        // just reconfiguring pin
        status = PIN_setConfig(pinHandle, PIN_BM_ALL, pconfig);
        if (status != PIN_SUCCESS)
        {
            System_abort("Failed to set pin's new config\n");
        }
    }

//	GPIO_InitTypeDef GPIO_InitStructure;

//    if( pin == NC )
//    {
//        return;
//    }
//    obj->pinIndex = ( 0x01 << ( obj->pin & 0x0F ) );

//    if( ( obj->pin & 0xF0 ) == 0x00 )
//    {
//        obj->port = GPIOA;
//        __HAL_RCC_GPIOA_CLK_ENABLE( );
//    }
//    else if( ( obj->pin & 0xF0 ) == 0x10 )
//    {
//        obj->port = GPIOB;
//        __HAL_RCC_GPIOB_CLK_ENABLE( );
//    }
//    else if( ( obj->pin & 0xF0 ) == 0x20 )
//    {
//        obj->port = GPIOC;
//        __HAL_RCC_GPIOC_CLK_ENABLE( );
//    }
//    else if( ( obj->pin & 0xF0 ) == 0x30 )
//    {
//        obj->port = GPIOD;
//        __HAL_RCC_GPIOD_CLK_ENABLE( );
//    }
//    else
//    {
//        obj->port = GPIOH;
//        __HAL_RCC_GPIOH_CLK_ENABLE( );
//    }
//
//    GPIO_InitStructure.Pin =  obj->pinIndex ;
//    GPIO_InitStructure.Pull = type;
//    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
//
//    if( mode == PIN_INPUT )
//    {
//        GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
//    }
//    else if( mode == PIN_ANALOGIC )
//    {
//        GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
//    }
//    else if( mode == PIN_ALTERNATE_FCT )
//    {
//        if( config == PIN_OPEN_DRAIN )
//        {
//            GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
//        }
//        else
//        {
//            GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
//        }
//        GPIO_InitStructure.Alternate = value;
//    }
//    else // mode ouptut
//    {
//        if( config == PIN_OPEN_DRAIN )
//        {
//            GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
//        }
//        else
//        {
//            GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
//        }
//    }
//
//    HAL_GPIO_Init( obj->port, &GPIO_InitStructure );
//
//    // Sets initial output value
//    if( mode == PIN_OUTPUT )
//    {
//        GpioMcuWrite( obj, value );
//    }
}

void GpioMcuSetInterrupt(Gpio_t *obj, IrqModes irqMode,
                         IrqPriorities irqPriority, GpioIrqHandler *irqHandler)
{
    PIN_Config config = PIN_ID(obj->pinIndex);
    int8_t index;
    switch (irqMode)
    {
    case NO_IRQ:
        config |= PIN_IRQ_DIS;
    case IRQ_RISING_EDGE:
        config |= PIN_IRQ_NEGEDGE;
    case IRQ_FALLING_EDGE:
        config |= PIN_IRQ_POSEDGE;
    case IRQ_RISING_FALLING_EDGE:
        config |= PIN_IRQ_BOTHEDGES;
    }
    index = irqPinId2Index(obj->pinIndex);
    if (index < 0)
    {
        System_abort("Failed to set interrupt for pin not for SX1276\n");
    }
    GpioIrq[index] = irqHandler;

    if (PIN_setInterrupt(pinHandle, config) != PIN_SUCCESS)
    {
        System_abort("Failed to set interrupt for pin\n");
    }

//    uint32_t priority = 0;
//
//    IRQn_Type IRQnb = EXTI0_IRQn;
//    GPIO_InitTypeDef   GPIO_InitStructure;
//
//    if( irqHandler == NULL )
//    {
//        return;
//    }
//
//    GPIO_InitStructure.Pin =  obj->pinIndex;
//
//    if( irqMode == IRQ_RISING_EDGE )
//    {
//        GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
//    }
//    else if( irqMode == IRQ_FALLING_EDGE )
//    {
//        GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
//    }
//    else
//    {
//        GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
//    }
//
//    GPIO_InitStructure.Pull = GPIO_NOPULL;
//    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
//
//    HAL_GPIO_Init( obj->port, &GPIO_InitStructure );
//
//    switch( irqPriority )
//    {
//    case IRQ_VERY_LOW_PRIORITY:
//    case IRQ_LOW_PRIORITY:
//        priority = 3;
//        break;
//    case IRQ_MEDIUM_PRIORITY:
//        priority = 2;
//        break;
//    case IRQ_HIGH_PRIORITY:
//        priority = 1;
//        break;
//    case IRQ_VERY_HIGH_PRIORITY:
//    default:
//        priority = 0;
//        break;
//    }
//
//    switch( obj->pinIndex )
//    {
//    case GPIO_PIN_0:
//        IRQnb = EXTI0_IRQn;
//        break;
//    case GPIO_PIN_1:
//        IRQnb = EXTI1_IRQn;
//        break;
//    case GPIO_PIN_2:
//        IRQnb = EXTI2_IRQn;
//        break;
//    case GPIO_PIN_3:
//        IRQnb = EXTI3_IRQn;
//        break;
//    case GPIO_PIN_4:
//        IRQnb = EXTI4_IRQn;
//        break;
//    case GPIO_PIN_5:
//    case GPIO_PIN_6:
//    case GPIO_PIN_7:
//    case GPIO_PIN_8:
//    case GPIO_PIN_9:
//        IRQnb = EXTI9_5_IRQn;
//        break;
//    case GPIO_PIN_10:
//    case GPIO_PIN_11:
//    case GPIO_PIN_12:
//    case GPIO_PIN_13:
//    case GPIO_PIN_14:
//    case GPIO_PIN_15:
//        IRQnb = EXTI15_10_IRQn;
//        break;
//    default:
//        break;
//    }
//
//    GpioIrq[(obj->pin ) & 0x0F] = irqHandler;
//
//    HAL_NVIC_SetPriority( IRQnb , priority, 0 );
//    HAL_NVIC_EnableIRQ( IRQnb );
}

void GpioMcuRemoveInterrupt(Gpio_t *obj)
{
    int8_t index = irqPinId2Index(obj->pinIndex);
    if (index < 0)
    {
        System_abort("Failed to clear interrupt for pin not for SX1276\n");
    }
    GpioIrq[index] = NULL;
    PIN_setInterrupt(pinHandle, PIN_ID(obj->pinIndex) | PIN_IRQ_DIS);

//    GPIO_InitTypeDef   GPIO_InitStructure;
//
//    GPIO_InitStructure.Pin =  obj->pinIndex ;
//    GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
//    HAL_GPIO_Init( obj->port, &GPIO_InitStructure );

}

void GpioMcuWrite(Gpio_t *obj, uint32_t value)
{
    PIN_setOutputValue(pinHandle, PIN_ID(obj->pinIndex), value);
//    if( ( obj == NULL ) || ( obj->port == NULL ) )
//    {
//        assert_param( FAIL );
//    }
//    // Check if pin is not connected
//    if( obj->pin == NC )
//    {
//        return;
//    }
//    HAL_GPIO_WritePin( obj->port, obj->pinIndex , ( GPIO_PinState )value );
}

void GpioMcuToggle(Gpio_t *obj)
{
    PIN_setOutputValue(pinHandle, PIN_ID(obj->pinIndex),
                       !PIN_getInputValue(PIN_ID(obj->pinIndex)));
//    if( ( obj == NULL ) || ( obj->port == NULL ) )
//    {
//        assert_param( FAIL );
//    }
//
//    // Check if pin is not connected
//    if( obj->pin == NC )
//    {
//        return;
//    }
//    HAL_GPIO_TogglePin( obj->port, obj->pinIndex );
}

uint32_t GpioMcuRead(Gpio_t *obj)
{
    return PIN_getInputValue(PIN_ID(obj->pinIndex));
//    if( obj == NULL )
//    {
//        assert_param( FAIL );
//    }
//    // Check if pin is not connected
//    if( obj->pin == NC )
//    {
//        return 0;
//    }
//    return HAL_GPIO_ReadPin( obj->port, obj->pinIndex );
}

//void EXTI0_IRQHandler( void )
//{
//#if !defined( USE_NO_TIMER )
//    RtcRecoverMcuStatus( );
//#endif
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_0 );
//}
//
//void EXTI1_IRQHandler( void )
//{
//#if !defined( USE_NO_TIMER )
//    RtcRecoverMcuStatus( );
//#endif
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_1 );
//}
//
//void EXTI2_IRQHandler( void )
//{
//#if !defined( USE_NO_TIMER )
//    RtcRecoverMcuStatus( );
//#endif
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_2 );
//}
//
//void EXTI3_IRQHandler( void )
//{
//#if !defined( USE_NO_TIMER )
//    RtcRecoverMcuStatus( );
//#endif
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_3 );
//}
//
//void EXTI4_IRQHandler( void )
//{
//#if !defined( USE_NO_TIMER )
//    RtcRecoverMcuStatus( );
//#endif
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_4 );
//}
//
//void EXTI9_5_IRQHandler( void )
//{
//#if !defined( USE_NO_TIMER )
//    RtcRecoverMcuStatus( );
//#endif
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_5 );
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_6 );
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_7 );
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_8 );
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_9 );
//}
//
//void EXTI15_10_IRQHandler( void )
//{
//#if !defined( USE_NO_TIMER )
//    RtcRecoverMcuStatus( );
//#endif
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_10 );
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_11 );
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_12 );
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_13 );
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_14 );
//    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_15 );
//}
//
//void HAL_GPIO_EXTI_Callback( uint16_t gpioPin )
//{
//    uint8_t callbackIndex = 0;
//
//    if( gpioPin > 0 )
//    {
//        while( gpioPin != 0x01 )
//        {
//            gpioPin = gpioPin >> 1;
//            callbackIndex++;
//        }
//    }
//
//    if( GpioIrq[callbackIndex] != NULL )
//    {
//        GpioIrq[callbackIndex]( );
//    }
//}

static int8_t irqPinId2Index(uint8_t pinId)
{
    switch (pinId)
    {
    case Board_SX_DIO0:
        return 0;
    case Board_SX_DIO1:
        return 1;
    case Board_SX_DIO2:
        return 2;
    case Board_SX_DIO3:
        return 3;
    case Board_SX_DIO4:
        return 4;
    case Board_SX_DIO5:
        return 5;
    case Board_SX_RESET:
    case Board_SX_NSS:
    default:
        System_abort("Asked for irqPin of a non-DIO pin\n");
        return -1;
    }
}

//static void pinIntCallback(PIN_Handle handle, PIN_Id pinId)
//{
//    int8_t index = irqPinId2Index(pinId);
//    if (index >= 0)
//    {
//        if (GpioIrq[index] != NULL)
//        {
//            switch (pinId)
//            {
//            case Board_SX_DIO0:
//                Event_post(sxevents, Event_Id_00);
//                break;
//            case Board_SX_DIO1:
//                Event_post(sxevents, Event_Id_01);
//                break;
//            case Board_SX_DIO2:
//                Event_post(sxevents, Event_Id_02);
//                break;
//            case Board_SX_DIO3:
//                Event_post(sxevents, Event_Id_03);
//                break;
//            case Board_SX_DIO4:
//                Event_post(sxevents, Event_Id_04);
//                break;
//            case Board_SX_DIO5:
//                Event_post(sxevents, Event_Id_05);
//                break;
//            }
//        }
//    }
//}

static void pinIntCallback(PIN_Handle handle, PIN_Id pinId)
{
    int8_t index = irqPinId2Index(pinId);
    GpioIrqHandler *handler;
    if (index >= 0)
    {
        handler = GpioIrq[index];
        if (handler)
        {
            ScheduleISRCallback((isr_worker_t) handler);
        }
    }
}

void GpioMcuInitInterrupt()
{
//    /* create an Event object. All events are binary */
//    sxevents = Event_create(NULL, &eb);
//    if (sxevents == NULL)
//    {
//        System_abort("Event sxevents create failed");
//    }
}

///**
// *
// * @param timeout The timeout spec for Event_pend. This can be BIOS_WAIT_FOREVER.
// */
//void GpioMcuHandleInterrupt(UInt32 timeout)
//{
//    UInt events;
//    GpioIrqHandler *handler;
//
//    events = Event_pend(sxevents,
//                        Event_Id_NONE,
//                        Event_Id_00 + Event_Id_01 + Event_Id_02 + Event_Id_03 + Event_Id_04 + Event_Id_05,
//                        timeout);
//
//    if(events)
//        printf("events = 0x%X\n", events);
//
//    /* Process all the events */
//    if (events & Event_Id_00)
//    {
//        handler = GpioIrq[0];
//        if (handler != NULL)
//        {
//            handler();
//        }
//    }
//    if (events & Event_Id_01)
//    {
//        handler = GpioIrq[1];
//        if (handler != NULL)
//        {
//            handler();
//        }
//    }
//    if (events & Event_Id_02)
//    {
//        handler = GpioIrq[2];
//        if (handler != NULL)
//        {
//            handler();
//        }
//    }
//    if (events & Event_Id_03)
//    {
//        handler = GpioIrq[3];
//        if (handler != NULL)
//        {
//            handler();
//        }
//    }
//    if (events & Event_Id_04)
//    {
//        handler = GpioIrq[4];
//        if (handler != NULL)
//        {
//            handler();
//        }
//    }
//    if (events & Event_Id_05)
//    {
//        handler = GpioIrq[5];
//        if (handler != NULL)
//        {
//            handler();
//        }
//    }
//}
