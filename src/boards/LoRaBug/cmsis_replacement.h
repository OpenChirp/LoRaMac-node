/*
 * cmsis_replacement.h
 *
 *  Created on: Jan 8, 2017
 *      Author: Craig Hesling <craig@hesling.com>
 */

#ifndef LORABUG_CMSIS_REPLACEMENT_H_
#define LORABUG_CMSIS_REPLACEMENT_H_

#include <assert.h>

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed.
  *         If expr is true, it returns no value.
  * @retval None
  */
/* Exported functions ------------------------------------------------------- */
#   define assert_param(expr) assert(expr)
#else
#  define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */


#endif /* LORABUG_CMSIS_REPLACEMENT_H_ */
