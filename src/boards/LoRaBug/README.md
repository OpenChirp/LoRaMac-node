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
