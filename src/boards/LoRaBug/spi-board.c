/**
 * This is the HAL implementation of SPI.
 *
 * From lots of experimentation, we can say that the CC2650 has
 * the following idle SPI states during active and sleep modes:
 * MISO - Pulled low (would think this is the internal pulldown)
 * MOSI - HIgh-Z (this is a major problem for the SX1276)
 * CLK - Pulled low (I would guess this is being driven low)
 *
 * @author Craig Hesling
 * @date Jan 7, 2017
 */

#include "board.h"
#include "spi-board.h"

#include <xdc/runtime/System.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/SPI.h>
#include <ti/sysbios/gates/GateMutexPri.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/spi/SPICC26XXDMA.h>

/*
 *  ======== spiMosiCorrect ========
 *  This functions is called to notify the us of an imminent transition
 *  in to sleep mode or from sleep mode.
 *
 *  @pre    Function assumes that clientArg is a pointer to an Spi_t that has already been opened.
 */
static int spiMosiCorrect(unsigned int eventType, uintptr_t eventArg, uintptr_t clientArg)
{
    Spi_t *spi;
    SPICC26XXDMA_Object *spiObject;
    SPICC26XXDMA_HWAttrsV1 const *spiHwAttrs;

    spi = (Spi_t *)clientArg;
    spiObject = ((SPI_Handle) spi->Spi)->object;
    spiHwAttrs = ((SPI_Handle) spi->Spi)->hwAttrs;

    /**
     * What we want to do here is configure the IO mux to connect MOSI to the GPIO
     * peripheral when we go to sleep and connect it back to SSI_TX when we wake up.
     * The SPI driver set the GPIO module for MOSI to the correct output settings
     * before it connected it to SSI_TX.
     */
	switch (eventType) {
		case PowerCC26XX_ENTERING_STANDBY:
		case PowerCC26XX_ENTERING_SHUTDOWN:
		    // Save MOSI's old pin MUX config
		    spi->mosimuxold = PINCC26XX_getMux(spiHwAttrs->mosiPin);
		    // Assign MOSI to the default GPIO peripheral
		    if (PINCC26XX_setMux(spiObject->pinHandle, spiHwAttrs->mosiPin, -1) != PIN_SUCCESS) {
                System_abort("Failed to reassign MOSI to GPIO peripheral\n");
            }
		    break;
		case PowerCC26XX_AWAKE_STANDBY:
		    // Assign MOSI back to the SSI peripheral
            if (PINCC26XX_setMux(spiObject->pinHandle, spiHwAttrs->mosiPin, spi->mosimuxold) != PIN_SUCCESS) {
                System_abort("Failed to reassign MOSI back to the SSI peripheral\n");
            }
		    break;
	}

    return Power_NOTIFYDONE;
}

/**
 * @note We currently ignore all given pins for SPI
 */
void SpiInit( Spi_t *obj, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss )
{
    // SPI Init
    SPI_Params_init(&obj->Params);
    obj->Params.transferMode = SPI_MODE_BLOCKING;

    GateMutexPri_construct(&obj->gmutex, NULL);

    // this doesn't feel right, but they did it this way above
    SpiFormat( obj, 8, 0, 0, 0 ); // want 8bit, SPI_POL0_PHA0, SPI_MASTER
    SpiFrequency( obj, 10000000 );

    IArg key = GateMutexPri_enter(GateMutexPri_handle(&obj->gmutex));
	// Open the SPI peripheral
	obj->Spi = SPI_open(Board_SPI0, &obj->Params);
	// Setup callback for setting MOSI to low on sleep
	Power_registerNotify(&obj->pwrnotifobj,
	                     PowerCC26XX_ENTERING_STANDBY|PowerCC26XX_AWAKE_STANDBY|PowerCC26XX_ENTERING_SHUTDOWN,
	                     (Fxn)spiMosiCorrect,
	                     (UInt32)obj);
	GateMutexPri_leave(GateMutexPri_handle(&obj->gmutex), key);
	if (!obj->Spi){
	    System_abort("Failed to open SPI for SX1276\n");
	}
}

void SpiDeInit( Spi_t *obj )
{
    IArg key = GateMutexPri_enter(GateMutexPri_handle(&obj->gmutex));
    // Unregister the callback for setting MOSI to low on sleep
    Power_unregisterNotify(&obj->pwrnotifobj);
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

