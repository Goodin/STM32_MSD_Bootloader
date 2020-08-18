/**
  ******************************************************************************
  * @file    mass_mal.h
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Header for mass_mal.c file.
  ******************************************************************************
  */

#ifndef __MASS_MAL_H
#define __MASS_MAL_H

#include "main.h"

#define MAL_OK		0
#define MAL_FAIL	!MAL_OK
#define MAX_LUN		1

#define MSD_BLOCK_SIZE	FLASH_PAGE_SIZE
#define MSD_FULL_SIZE	FLASH_MSD_SIZE

u16 MAL_Init (u8 lun);
u16 MAL_Read(u8 lun, u32 memOffset, u32 *readBuff);
u16 MAL_Write(u8 lun, u32 memOffset, u32 *writeBuff);
u16 MAL_GetStatus (u8 lun);

#endif /* __MASS_MAL_H */
