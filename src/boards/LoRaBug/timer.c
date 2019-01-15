/**
 * Holds timer objects and scheduling primitives
 *
 * @author Craig Hesling <craig@hesling.com>
 * @date April 26, 2017
 */

#include <stdint.h>

#include <ti/sysbios/knl/Clock.h>
#include <xdc/std.h>
#include <xdc/runtime/Types.h> // Types_Timestamp64
#include <xdc/runtime/Timestamp.h>
#include <driverlib/debug.h>

#include <assert.h>

#include "board.h"
#include "timer.h"

/**
 * This is the callback proxy for all registered timers
 * @param arg0
 */
Void timerCallback(UArg arg0)
{
    assert(arg0);
    // again we have the swi callback problem
    ScheduleISRCallback((isr_worker_t)arg0);
}

void TimerInit( TimerEvent_t *obj, void ( *callback )( void ) )
{
    Clock_Params params;
    Clock_Params_init(&params);

    params.arg = (UArg)callback;
    params.period = 0;
    params.startFlag = FALSE;

    assert(obj);
    Clock_construct((Clock_Struct *)obj, (Clock_FuncPtr)timerCallback, 0, &params);
}

void TimerStart( TimerEvent_t *obj )
{
    assert(obj);
    Clock_start( Clock_handle((Clock_Struct*)obj) );
}

void TimerStop( TimerEvent_t *obj )
{
    assert(obj);
    Clock_stop( Clock_handle((Clock_Struct*)obj) );
}

void TimerReset( TimerEvent_t *obj )
{
    assert(obj);
    TimerStop( obj );
    TimerStart( obj );
}

/**
 * Sets the timer one shot time value in ms
 * @param obj The timer object
 * @param value The time in ms
 */
void TimerSetValue( TimerEvent_t *obj, uint32_t value )
{
    assert(obj);
    Clock_setTimeout(Clock_handle((Clock_Struct*)obj), (UInt32)(value * TIME_MS));
}

#define TIMESTAMP64_TO_UINT64(timestamp) (((uint64_t)((timestamp).hi))<<32 | ((uint64_t)((timestamp).lo)))

TimerTime_t TimerGetCurrentTime( void )
{
    Types_Timestamp64 now;
    Types_FreqHz freq;
    Timestamp_get64(&now);
    Timestamp_getFreq(&freq);

    uint64_t now64 = TIMESTAMP64_TO_UINT64(now);
    uint64_t freq64 = TIMESTAMP64_TO_UINT64(freq);

    return (TimerTime_t) (now64 * 1e3) / freq64;
}

TimerTime_t TimerGetElapsedTime( TimerTime_t savedTime )
{
    TimerTime_t now = TimerGetCurrentTime();
    if (now < savedTime) {
        return (UINT64_MAX - savedTime) + (now + 1);
    }
    return now - savedTime;
}

TimerTime_t TimerGetFutureTime( TimerTime_t eventInFuture )
{
    TimerTime_t now = TimerGetCurrentTime();
    if (eventInFuture < now) {
        return (UINT64_MAX - now) + (eventInFuture + 1);
    }
    return eventInFuture - now;
}
