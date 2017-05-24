/*
  Events.h - interrupt handlers

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

#ifndef __Events_H
#define __Events_H
/* MODULE Events */

#include "fsl_device_registers.h"
#include "clockMan1.h"
#include "pin_mux.h"
#include "osa1.h"
#include "gpio1.h"
#include "i2cCom1.h"
#include "flash1.h"
#include "spiComEZPort.h"
#include "lpuartUblox.h"

#ifdef __cplusplus
extern "C" {
#endif


/*! spiCom1 IRQ handler */
void SPI1_IRQHandler(void);

/*! i2cCom1 IRQ handler */
void i2cCom1_IRQHandler(void);

/*! i2cCom1 slave callback */
void i2cCom1_Callback0(uint8_t instance,i2c_slave_event_t slaveEvent,void *userData);

void flash1_Callback(void);
/*! spiComEZPort IRQ handler */
void SPI0_IRQHandler(void);

void lpuartUblox_RxCallback(uint32_t instance, void * lpuartState);

/* END Events */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif
/* ifndef __Events_H*/
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
