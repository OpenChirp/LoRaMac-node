/**
 * Since the library likes to do lots of long blocking operations, we need callbacks to run in a complete task.
 *
 * @author Craig Hesling <craig@hesling.com>
 * @date April 26, 2017
 */

#include <assert.h>
#include "board.h"
#include "gpio-board.h"

#include <LORABUG.h>

/* XDCtools Header files */
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h> // Error_Block

/* BIOS Header files */
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/gates/GateMutexPri.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>

/**
 * Pin state used to open pin handle the first time
 */
static PIN_State pinState;
/**
 * The pin handle that holds all open pins
 */
static PIN_Handle pinHandle = NULL;
/**
 * The mutex for all pin operations.
 *
 * Since we use one pin handle for all pins controlled here,
 * we need to make sure pin functions are only called one at a time.
 */
static GateMutexPri_Struct pinMutex;

/**
 * This is the local registry for interrupt handlers
 * We only allow registering interrupt for the 6 SX127x DIO lines
 */
static GpioIrqHandler *GpioIrq[6] = { 0 };

static int8_t irqPinId2Index(uint8_t pinId);
static void pinIntCallback(PIN_Handle handle, PIN_Id pinId);

/**
 * @note We assume this will be the first function called on a new pin
 *       and that the first invocation will not race with another instance.
 *       This is because we initialize runtime globals upon first invocation.
 */
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
            break;
        case PIN_PULL_UP:
            pconfig |= PIN_PULLUP;
            break;
        case PIN_PULL_DOWN:
            pconfig |= PIN_PULLDOWN;
            break;
        }
    }

    // Set initial value
    if (mode == PIN_OUTPUT)
    {
        pconfig |= value ? PIN_GPIO_HIGH : PIN_GPIO_LOW;
    }

    /* Check if this is the first pin ever used */
    if (pinHandle == NULL)
    {
        IArg key;
        PIN_Status status;
        // Open new handle
        PIN_Config pconfigs[] = { pconfig, PIN_TERMINATE };

        GateMutexPri_construct(&pinMutex, NULL);

        key = GateMutexPri_enter(GateMutexPri_handle(&pinMutex));
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
        GateMutexPri_leave(GateMutexPri_handle(&pinMutex), key);
    }
    else
    {
        IArg key;
        PIN_Status status;

        key = GateMutexPri_enter(GateMutexPri_handle(&pinMutex));
        /// @note Since callers like to use this to reconfigure
        /// and I don't see a clear way to query the handle about
        /// which pins are added, we simply try to add the pin
        /// (which may fail) and then set config
        // Add pin to open handle
        status = PIN_add(pinHandle, pconfig);
        // We ignore the status, since we know that it may fail
        // when the pin is already in the pinHandle.
        //if(status != PIN_SUCCESS) {
        //	// Error adding pin
        //	System_abort("Failed to add pin to handle\n");
        //}
        // now ensure the config is set if caller is
        // just reconfiguring pin - do not change interrupt config
        status = PIN_setConfig(pinHandle, PIN_BM_ALL & (~PIN_BM_IRQ), pconfig);
        if (status != PIN_SUCCESS)
        {
            System_abort("Failed to set pin's new config\n");
        }
        GateMutexPri_leave(GateMutexPri_handle(&pinMutex), key);
    }
}

void GpioMcuSetInterrupt(Gpio_t *obj, IrqModes irqMode,
                         IrqPriorities irqPriority, GpioIrqHandler *irqHandler)
{
    IArg key;
    PIN_Config config = PIN_ID(obj->pinIndex);
    int8_t index;

    switch (irqMode)
    {
    case NO_IRQ:
        config |= PIN_IRQ_DIS;
        break;
    case IRQ_RISING_EDGE:
        config |= PIN_IRQ_POSEDGE;
        break;
    case IRQ_FALLING_EDGE:
        config |= PIN_IRQ_NEGEDGE;
        break;
    case IRQ_RISING_FALLING_EDGE:
        config |= PIN_IRQ_BOTHEDGES;
        break;
    }
    index = irqPinId2Index(obj->pinIndex);
    if (index < 0)
    {
        System_abort("Failed to set interrupt for pin not for SX1276\n");
    }
    GpioIrq[index] = irqHandler;

    key = GateMutexPri_enter(GateMutexPri_handle(&pinMutex));
    if (PIN_setInterrupt(pinHandle, config) != PIN_SUCCESS)
    {
        System_abort("Failed to set interrupt for pin\n");
    }
    GateMutexPri_leave(GateMutexPri_handle(&pinMutex), key);
}

void GpioMcuRemoveInterrupt(Gpio_t *obj)
{
    IArg key;
    int8_t index = irqPinId2Index(obj->pinIndex);
    if (index < 0)
    {
        System_abort("Failed to clear interrupt for pin not for SX1276\n");
    }
    GpioIrq[index] = NULL;
    key = GateMutexPri_enter(GateMutexPri_handle(&pinMutex));
    PIN_setInterrupt(pinHandle, PIN_ID(obj->pinIndex) | PIN_IRQ_DIS);
    GateMutexPri_leave(GateMutexPri_handle(&pinMutex), key);
}

void GpioMcuWrite(Gpio_t *obj, uint32_t value)
{
    IArg key;
    assert(obj);
    key = GateMutexPri_enter(GateMutexPri_handle(&pinMutex));
    PIN_setOutputValue(pinHandle, PIN_ID(obj->pinIndex), value);
    GateMutexPri_leave(GateMutexPri_handle(&pinMutex), key);
}

void GpioMcuToggle(Gpio_t *obj)
{
    IArg key;
    assert(obj);
    key = GateMutexPri_enter(GateMutexPri_handle(&pinMutex));
    PIN_setOutputValue(pinHandle, PIN_ID(obj->pinIndex),
                       !PIN_getInputValue(PIN_ID(obj->pinIndex)));
    GateMutexPri_leave(GateMutexPri_handle(&pinMutex), key);
}

uint32_t GpioMcuRead(Gpio_t *obj)
{
    assert(obj);
    return PIN_getInputValue(PIN_ID(obj->pinIndex));
}

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

/**
 * Since the interrupt routines like to do a lot of work and
 * call blocking functions, we need to simply offload to our callback task.
 *
 * @param handle The pin handle associated with the interrupting pin
 * @param pinId The pin that is interrupting
 */
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
