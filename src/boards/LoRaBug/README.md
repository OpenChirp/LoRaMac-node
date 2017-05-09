# What is going on in here?
This library seems to be capable of using the SX1509 I2C IO Expander, among other things.
Since we do not use that module to communicate with the SX1276, we have omitted Expanded IO
options. See pinName-ioe.h.

Since the LoRaMac-node/src/boards/SensorNode seemed to be the Board that used
the SX1276, instead of the SX1272, I think I pulled in that board specific lib.
To upgrade in the future, run meld against the SensorNode directory tree.

# Terms
* IOE - IO Expander. This refers to the optional SX1509 I2C IO Expander.

# TI-RTOS SYS/BIOS Required Modules
- Clock
- Mailbox
- GateMutexPri
- Event (cc26xx/system.c)
