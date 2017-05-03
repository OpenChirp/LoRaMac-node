/**
 * @brief System accessors for the CC26xx
 *
 * @author Craig Hesling <craig@hesling.com>
 * @date Apr 28, 2017
 */

#ifndef LORAMAC_NODE_SRC_BOARDS_LORABUG_CC26XX_SYSTEM_H_
#define LORAMAC_NODE_SRC_BOARDS_LORABUG_CC26XX_SYSTEM_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Get CCFG Setting */
inline uint64_t CCFG_IEEE_MAC_Addr();
inline uint64_t CCFG_IEEE_BLE_Addr();

/* Get FCFG Setting */
inline uint64_t FCFG1_MAC_15_4_Addr();
inline uint64_t FCFG1_MAC_BLE_Addr();

/* TRNG */

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
                       uint32_t ui32ClocksPerSample);


#ifdef __cplusplus
}
#endif

#endif /* LORAMAC_NODE_SRC_BOARDS_LORABUG_CC26XX_SYSTEM_H_ */
