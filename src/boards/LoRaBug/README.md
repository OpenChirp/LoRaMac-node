# What is going on in here?
This original library seemed to be capable of using the SX1509 I2C IO Expander,
among other things. Since we do not use that module to communicate with the
SX1276, we have omitted Expanded IO options.
See [pinName-ioe.h](pinName-ioe.h) and [pinName-board.h](pinName-board.h).

Since the LoRaMac-node/src/boards/SensorNode Board used he SX1276, instead of
the typical SX1272, I believe I pulled in that board specific lib as starter
code.
To upgrade in the future, run meld (or some awesome directory diff tool)
against the SensorNode directory tree.

# Terms
* IOE - IO Expander. This refers to the optional SX1509 I2C IO Expander.

# TI-RTOS SYS/BIOS Required Modules
- Clock
- Mailbox
- GateMutexPri
- Event (cc26xx/system.c)

# SPI
From lots of experimentation, we can say that the CC2650 has
the following idle SPI states during active and sleep modes:
 * MISO - Pulled low (would think this is the internal pulldown)
 * MOSI - HIgh-Z (this is a major problem for the SX1276)
 * CLK - Pulled low (I would guess this is being driven low)

It is fine to allow the CC2650 to sleep with the SPI open, but without
adjusting the MOSI line to go low, you cannot get the SX1276 to stop
burning power.
The solution employed here to pull MOSI low before sleep is to unroute the MOSI
pin from SSI_TX and return it to the default GPIO peripheral, where it was
already set to output,push-pull, and low.
This is achieved by registering a power notification callback after the spi is
opened(since notifications are called fifo) that simply adjust the IO mux
setting for MOSI.

When using SPI, the CC2650 seems to enable pulldowns on the SCK and MISO
lines when not being used and sets MOSI to high-z
