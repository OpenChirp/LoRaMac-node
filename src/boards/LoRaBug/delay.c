/**
 * Delay functions implementation
 *
 * @author Craig Hesling <craig@hesling.com>
 * @date April 26, 2017
 */

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

#include "board.h"
#include "delay.h"

void Delay( float s )
{
    DelayMs( s * 1000.0f );
}

void DelayMs( uint32_t ms )
{
    Task_sleep(TIME_MS * ms);
}
