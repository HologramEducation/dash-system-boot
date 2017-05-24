/*
  Events.c - interrupt handlers

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
#include "Events.h"

#ifdef __cplusplus
extern "C" {
#endif


/* User includes (#include below this line is not maintained by Processor Expert) */
#include "ipc_i2c.h"
#include "boot.h"

/*! i2cCom1 IRQ handler */
void i2cCom1_IRQHandler(void)
{
  I2C_DRV_IRQHandler(FSL_I2CCOM1);
}

/*! i2cCom1 slave callback */
void i2cCom1_Callback0(uint8_t instance,i2c_slave_event_t slaveEvent,void *userData)
{
	i2cCom1_Handler(slaveEvent);
}

void flash1_Callback(void)
{
    /* Write your code here ... */
}
/*! spiComEZPort IRQ handler */
void SPI0_IRQHandler(void)
{
#if SPICOMEZPORT_DMA_MODE
  SPI_DRV_DmaIRQHandler(FSL_SPICOMEZPORT);
#else
  SPI_DRV_IRQHandler(FSL_SPICOMEZPORT);
#endif
  /* Write your code here ... */
}

void lpuartUblox_RxCallback(uint32_t instance, void * lpuartState)
{
    lpuart_state_t * ptr =  (lpuart_state_t *)lpuartState;
    RING_push(&ublox_ring, *(ptr->rxBuff));
}

/* END Events */

#ifdef __cplusplus
}  /* extern "C" */
#endif

/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
