/**
 * This is the HAL implementation of SPI.
 *
 * @author Craig Hesling
 * @date Jan 7, 2017
 */

#include "board.h"
#include "spi-board.h"
//#include "stm32l1xx_hal_spi.h"

#include <xdc/runtime/System.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/SPI.h>
#include <ti/sysbios/gates/GateMutexPri.h>

/**
 * @note We currently ignore all given pins for SPI
 */
void SpiInit( Spi_t *obj, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss )
{
    // SPI Init
    SPI_Params_init(&obj->Params);
    obj->Params.transferMode = SPI_MODE_BLOCKING;

    GateMutexPri_Params_init(&obj->gmutexParams);
    GateMutexPri_construct(&obj->gmutex, &obj->gmutexParams);

    // this doesn't feel right, but they did it this way above
    SpiFormat( obj, 8, 0, 0, 0 ); // want 8bit, SPI_POL0_PHA0, SPI_MASTER
    SpiFrequency( obj, 10000000 );

    IArg key = GateMutexPri_enter(GateMutexPri_handle(&obj->gmutex));
	// Open the SPI and perform the transfer
	obj->Spi = SPI_open(Board_SPI0, &obj->Params);
	GateMutexPri_leave(GateMutexPri_handle(&obj->gmutex), key);
	if (!obj->Spi){
	    System_abort("Failed to open SPI for SX1276\n");
	}
}

void SpiDeInit( Spi_t *obj )
{
    IArg key = GateMutexPri_enter(GateMutexPri_handle(&obj->gmutex));
    SPI_close(obj->Spi);
    // We do not want to close NSS pin, since SX1276 seems to go active if the line goes low
    GateMutexPri_leave(GateMutexPri_handle(&obj->gmutex), key);
}

void SpiFormat( Spi_t *obj, int8_t bits, int8_t cpol, int8_t cpha, int8_t slave )
{
	// Set data frame size in bits
	obj->Params.dataSize = bits;

	// Set frame format
	switch (((cpol & 1) << 1) | (cpha & 1)) {
	default:
	case SPI_POL0_PHA0:
		obj->Params.frameFormat = SPI_POL0_PHA0;
		break;
	case SPI_POL0_PHA1:
		obj->Params.frameFormat = SPI_POL0_PHA1;
		break;
	case SPI_POL1_PHA0:
		obj->Params.frameFormat = SPI_POL1_PHA0;
		break;
	case SPI_POL1_PHA1:
		obj->Params.frameFormat = SPI_POL1_PHA1;
		break;
	}

	// Set mode
	obj->Params.mode = slave ? SPI_SLAVE : SPI_MASTER;
}

void SpiFrequency( Spi_t *obj, uint32_t hz )
{
	obj->Params.bitRate = hz;
}

uint16_t SpiInOut( Spi_t *obj, uint16_t outData )
{
    bool status;
    uint8_t rxBuf;
    SPI_Transaction trans = {
    		.count = 1,
			.txBuf = &outData,
			.rxBuf = &rxBuf
    };
    IArg key = GateMutexPri_enter(GateMutexPri_handle(&obj->gmutex));
    status = SPI_transfer(obj->Spi, &trans);
    GateMutexPri_leave(GateMutexPri_handle(&obj->gmutex), key);
    if (!status) {
        System_abort("Failed to SPI transact byte with SX1276\n");
    }
    return rxBuf;
}

