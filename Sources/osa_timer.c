/*
  osa_timer.c - Provide timing functions

  https://hologram.io

  Copyright (c) 2016 Konekt, Inc.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Cpu.h"

/* Timer period */
#define OSA1_TIMER_PERIOD_US           1000U
/* Software ISR counter */
static volatile uint32_t SwTimerIsrCounter = 0U;

/*
** ===================================================================
**     Method      :  HWTIMER_SYS_TimerIsr (component fsl_os_abstraction)
**
**     Description :
**         Interrupt service routine.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void SysTick_Handler(void)
{
	SwTimerIsrCounter++;
}

/*
** ===================================================================
**     Method      :  OSA_TimeInit (component fsl_os_abstraction)
**
**     Description :
**         This function initializes the timer used in BM OSA, the
**         functions such as OSA_TimeDelay, OSA_TimeGetMsec, and the
**         timeout are all based on this timer.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void OSA_TimeInit(void)
{
    uint64_t divider;

    /* Disable timer and interrupt */
    SysTick->CTRL = 0U;
    /* A write of any value to current value register clears the field to 0, and also clears the SYST_CSR COUNTFLAG bit to 0. */
    SysTick->VAL = 0U;
    #if FSL_FEATURE_SYSTICK_HAS_EXT_REF
    /* Set the clock source back to core freq */
    CLOCK_SYS_SetSystickSrc(kClockSystickSrcCore);
    #endif
    /* Get SysTick counter input frequency and compute divider value */
    divider = ((((uint64_t)CLOCK_SYS_GetSystickFreq() * OSA1_TIMER_PERIOD_US)) / 1000000U);
    assert(divider != 0U);
    /* Set divide input clock of systick timer */
    SysTick->LOAD = (uint32_t)(divider - 1U);
    /* Set interrupt priority and enable interrupt */
    NVIC_SetPriority(SysTick_IRQn, 1U);
    /* Run timer and enable interrupt */
    SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
}

/*
** ===================================================================
**     Method      :  OSA_TimeDiff (component fsl_os_abstraction)
**
**     Description :
**         This function gets the difference between two time stamp, time
**         overflow is considered.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
uint32_t OSA_TimeDiff(uint32_t time_start, uint32_t time_end)
{
    if (time_end >= time_start) {
        return time_end - time_start;
    } else {
        /* Sw ISR counter is 16 bits. */
        return 0xFFFFFFFFUL - time_start + time_end + 1;
    }
}

/*
** ===================================================================
**     Method      :  OSA_TimeGetMsec (component fsl_os_abstraction)
**
**     Description :
**         This function gets current time in milliseconds.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
uint32_t OSA_TimeGetMsec(void)
{
    return (SwTimerIsrCounter);
}
