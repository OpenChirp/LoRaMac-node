/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Delay functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

#include "delay.h"

#define TIME_MS (1000/Clock_tickPeriod)

void Delay( float s )
{
    DelayMs( s * 1000.0f );
}

void DelayMs( uint32_t ms )
{
    Task_sleep(TIME_MS * ms);
}
