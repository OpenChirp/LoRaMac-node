/**
 * @brief System accessors for the CC26xx
 *
 * @author Craig Hesling <craig@hesling.com>
 * @date Apr 28, 2017
 */

#include <stddef.h>

#include <ti/sysbios/knl/Event.h>

#include <inc/hw_types.h>  // HWREG() - 32bit access
#include <inc/hw_memmap.h> // CCFG_BASE
#include <inc/hw_ccfg.h>
#include <inc/hw_fcfg1.h>

#include <driverlib/aon_batmon.h>

#include <inc/hw_ints.h> // INT_TRNG_IRQ
#include <inc/hw_trng.h>
#include <driverlib/trng.h>
#include <ti/drivers/PIN.h>

/* Get CCFG Setting */
uint64_t CCFG_IEEE_MAC_Addr() {
    uint32_t lsw = HWREG(CCFG_BASE + CCFG_O_IEEE_MAC_0); // [31:0]
    uint32_t msw = HWREG(CCFG_BASE + CCFG_O_IEEE_MAC_1); // [63:32]
    return (((uint64_t)msw)<<32) | ((uint64_t)lsw);
}

uint64_t CCFG_IEEE_BLE_Addr() {
    uint32_t lsw = HWREG(CCFG_BASE + CCFG_O_IEEE_BLE_0); // [31:0]
    uint32_t msw = HWREG(CCFG_BASE + CCFG_O_IEEE_BLE_1); // [63:32]
    return (((uint64_t)msw)<<32) | ((uint64_t)lsw);
}

/* Get FCFG Setting */
uint64_t FCFG1_MAC_15_4_Addr() {
    uint32_t lsw = HWREG(FCFG1_BASE + FCFG1_O_MAC_15_4_0); // [31:0]
    uint32_t msw = HWREG(FCFG1_BASE + FCFG1_O_MAC_15_4_1); // [63:32]
    return (((uint64_t)msw)<<32) | ((uint64_t)lsw);
}

uint64_t FCFG1_MAC_BLE_Addr() {
    uint32_t lsw = HWREG(FCFG1_BASE + FCFG1_O_MAC_BLE_0); // [31:0]
    uint32_t msw = HWREG(FCFG1_BASE + FCFG1_O_MAC_BLE_1); // [63:32]
    return (((uint64_t)msw)<<32) | ((uint64_t)lsw);
}

/* Temp and Battery Voltage */

/**
 * Enables BatMon, spins until new battery measurement takes place, and disables BatMon.
 *
 * A voltage of 3.3V would be 0b000000000000000000000_011_00000011
 *
 * @return Returns integer and fractional Voltage (bits [10:8] integer, [7:0] fraction)
 * @see AONBatMonBatteryVoltageGet
 */
uint32_t PollBatteryVoltage() {
	uint32_t reading;
	// AON Battery Monitor is enabled at boot and is recommended not to disable
	// See CC2650 Manual 18.1
//	AONBatMonEnable();
	while(!AONBatMonNewBatteryMeasureReady()) ;
	reading = AONBatMonBatteryVoltageGet();
//	AONBatMonDisable();
	return reading;
}

uint32_t PollTemperature() {
	while (!AONBatMonNewTempMeasureReady()) ;
	//TODO: Implement Temperature Fetching
	return 0;
}

/* TRNG */

//static Event_Struct trngEventStruct;
//static Event_Params trngEventParams;

/**
 * This call will block while the TRNG is getting generating the value.
 *
 * The parameters of this function set the minimum and maximum number of
 * samples required in each generation of a new random number.
 *
 * @param ui32MinSamplesPerCycle is the minimum number of samples per each
 * generated random number. Constraints:
 * - Value must be bigger than or equal to 2^6 and less than 2^14.
 * - The 6 LSBs of the argument are truncated.
 * - If the value is zero, the number of samples is fixed to the value determined
 *   by ui32MaxSamplesPerCycle. To ensure same entropy in all generated random
 *   numbers the value 0 should be used.
 * @param ui32MaxSamplesPerCycle is the maximum number of samples per each
 * generated random number. Constraints:
 * - Value must be between 2^8 and 2^24 (both included).
 * - The 8 LSBs of the argument are truncated.
 * - Value 0 and 2^24 both give the highest possible value.
 * @param ui32ClocksPerSample is the number of clock cycles for each time
 * a new sample is generated from the FROs.
 * - 0  : Every sample.
 * - 1  : Every second sample.
 * - ...
 * - 15 : Every 16. sample.
 * @return The resultant random value
 *
 * @see TRNGConfigure
 */
uint64_t TRNG_GetValue(uint32_t ui32MinSamplesPerCycle,
                       uint32_t ui32MaxSamplesPerCycle,
                       uint32_t ui32ClocksPerSample) {
    // TODO: Configure TRNG, start TRNG, register Hwi for INT_TRNG_IRQ, and wait

//    Event_Params_init(&trngEventParams);
//    trngEventParams.instance->name = "TRNGEvents";
//    Event_construct(&trngEventStruct, &trngEventParams);
//    TRNGEnable();
////    TRNGIntRegister(trngIntCallback);
//    TRNGDisable();
//    Event_destruct(&trngEventStruct);
    return 0xFFFFFFFFFFFFFFFF;
}
