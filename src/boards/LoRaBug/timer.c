/**
 * Holds timer objects and scheduling primitives
 *
 * @author Craig Hesling <craig@hesling.com>
 * @date April 26, 2017
 */

#include <ti/sysbios/knl/Clock.h>
//#include <xdc/runtime/Timestamp.h> // Currently using Clock_getTicks() instead
#include <xdc/std.h>
#include <driverlib/debug.h>

#include <assert.h>

#include "board.h"
#include "timer.h"


//#include "rtc-board.h"

/*!
 * This flag is used to make sure we have looped through the main several time to avoid race issues
 */
//volatile uint8_t HasLoopedThroughMain = 0;

///*!
// * Timers list head pointer
// */
//static TimerEvent_t *TimerListHead = NULL;
//
///*!
// * \brief Adds or replace the head timer of the list.
// *
// * \remark The list is automatically sorted. The list head always contains the
// *         next timer to expire.
// *
// * \param [IN]  obj Timer object to be become the new head
// * \param [IN]  remainingTime Remaining time of the previous head to be replaced
// */
//static void TimerInsertNewHeadTimer( TimerEvent_t *obj, uint32_t remainingTime );
//
///*!
// * \brief Adds a timer to the list.
// *
// * \remark The list is automatically sorted. The list head always contains the
// *         next timer to expire.
// *
// * \param [IN]  obj Timer object to be added to the list
// * \param [IN]  remainingTime Remaining time of the running head after which the object may be added
// */
//static void TimerInsertTimer( TimerEvent_t *obj, uint32_t remainingTime );
//
///*!
// * \brief Sets a timeout with the duration "timestamp"
// *
// * \param [IN] timestamp Delay duration
// */
//static void TimerSetTimeout( TimerEvent_t *obj );
//
///*!
// * \brief Check if the Object to be added is not already in the list
// *
// * \param [IN] timestamp Delay duration
// * \retval true (the object is already in the list) or false
// */
//static bool TimerExists( TimerEvent_t *obj );

/*!
 * \brief Read the timer value of the currently running timer
 *
 * \retval value current timer value
 */
//TimerTime_t TimerGetValue( void );

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

//static void TimerInsertTimer( TimerEvent_t *obj, uint32_t remainingTime )
//{
//    uint32_t aggregatedTimestamp = 0;      // hold the sum of timestamps
//    uint32_t aggregatedTimestampNext = 0;  // hold the sum of timestamps up to the next event
//
//    TimerEvent_t* prev = TimerListHead;
//    TimerEvent_t* cur = TimerListHead->Next;
//
//    if( cur == NULL )
//    { // obj comes just after the head
//        obj->Timestamp -= remainingTime;
//        prev->Next = obj;
//        obj->Next = NULL;
//    }
//    else
//    {
//        aggregatedTimestamp = remainingTime;
//        aggregatedTimestampNext = remainingTime + cur->Timestamp;
//
//        while( prev != NULL )
//        {
//            if( aggregatedTimestampNext > obj->Timestamp )
//            {
//                obj->Timestamp -= aggregatedTimestamp;
//                if( cur != NULL )
//                {
//                    cur->Timestamp -= obj->Timestamp;
//                }
//                prev->Next = obj;
//                obj->Next = cur;
//                break;
//            }
//            else
//            {
//                prev = cur;
//                cur = cur->Next;
//                if( cur == NULL )
//                { // obj comes at the end of the list
//                    aggregatedTimestamp = aggregatedTimestampNext;
//                    obj->Timestamp -= aggregatedTimestamp;
//                    prev->Next = obj;
//                    obj->Next = NULL;
//                    break;
//                }
//                else
//                {
//                    aggregatedTimestamp = aggregatedTimestampNext;
//                    aggregatedTimestampNext = aggregatedTimestampNext + cur->Timestamp;
//                }
//            }
//        }
//    }
//}
//
//static void TimerInsertNewHeadTimer( TimerEvent_t *obj, uint32_t remainingTime )
//{
//    TimerEvent_t* cur = TimerListHead;
//
//    if( cur != NULL )
//    {
//        cur->Timestamp = remainingTime - obj->Timestamp;
//        cur->IsRunning = false;
//    }
//
//    obj->Next = cur;
//    obj->IsRunning = true;
//    TimerListHead = obj;
//    TimerSetTimeout( TimerListHead );
//}
//
//void TimerIrqHandler( void )
//{
//    uint32_t elapsedTime = 0;
//
//    elapsedTime = TimerGetValue( );
//
//    if( elapsedTime >= TimerListHead->Timestamp )
//    {
//        TimerListHead->Timestamp = 0;
//    }
//    else
//    {
//        TimerListHead->Timestamp -= elapsedTime;
//    }
//
//    TimerListHead->IsRunning = false;
//
//    while( ( TimerListHead != NULL ) && ( TimerListHead->Timestamp == 0 ) )
//    {
//        TimerEvent_t* elapsedTimer = TimerListHead;
//        TimerListHead = TimerListHead->Next;
//
//        if( elapsedTimer->Callback != NULL )
//        {
//            elapsedTimer->Callback( );
//        }
//    }
//
//    // start the next TimerListHead if it exists
//    if( TimerListHead != NULL )
//    {
//        if( TimerListHead->IsRunning != true )
//        {
//            TimerListHead->IsRunning = true;
//            TimerSetTimeout( TimerListHead );
//        }
//    }
//}

void TimerStop( TimerEvent_t *obj )
{
    assert(obj);
    Clock_stop( Clock_handle((Clock_Struct*)obj) );
}

//static bool TimerExists( TimerEvent_t *obj )
//{
//    TimerEvent_t* cur = TimerListHead;
//
//    while( cur != NULL )
//    {
//        if( cur == obj )
//        {
//            return true;
//        }
//        cur = cur->Next;
//    }
//    return false;
//}

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

//TimerTime_t TimerGetValue( void )
//{
//    return RtcGetElapsedAlarmTime( );
//}
//
TimerTime_t TimerGetCurrentTime( void )
{
//    return (TimerTime_t)Timestamp_get32();
    return (TimerTime_t) Clock_getTicks();
//    return 0;
//    return RtcGetTimerValue( );

}

TimerTime_t TimerGetElapsedTime( TimerTime_t savedTime )
{
    TimerTime_t now = TimerGetCurrentTime();
    return now - savedTime;
//    return RtcComputeElapsedTime( savedTime );
}

TimerTime_t TimerGetFutureTime( TimerTime_t eventInFuture )
{
    TimerTime_t now = TimerGetCurrentTime();
    return eventInFuture - now;
//    return RtcComputeFutureEventTime( eventInFuture );
}

//static void TimerSetTimeout( TimerEvent_t *obj )
//{
//    HasLoopedThroughMain = 0;
//    obj->Timestamp = RtcGetAdjustedTimeoutValue( obj->Timestamp );
//    RtcSetTimeout( obj->Timestamp );
//}
//
//void TimerLowPowerHandler( void )
//{
//    if( ( TimerListHead != NULL ) && ( TimerListHead->IsRunning == true ) )
//    {
//        if( HasLoopedThroughMain < 5 )
//        {
//            HasLoopedThroughMain++;
//        }
//        else
//        {
//            HasLoopedThroughMain = 0;
//            if( GetBoardPowerSource( ) == BATTERY_POWER )
//            {
//                RtcEnterLowPowerStopMode( );
//            }
//        }
//    }
//}
