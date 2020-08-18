/**
 ******************************************************************************
 * @file    mass_mal.c
 * @author  MCD Application Team
 * @version V4.0.0
 * @date    21-January-2013
 * @brief   Medium Access Layer interface
 ******************************************************************************
 */

#include "mass_mal.h"

u32 MassMemorySize[1];
u32 MassBlockSize[1];
u32 MassBlockCount[1];

u16 MAL_Init(u8 lun)
{
	switch (lun)
	{
		case 0:
			FLASH_Unlock();
			break;

		default:
			return MAL_FAIL;
	}

	return MAL_OK;
}

u16 MAL_Read(u8 lun, u32 memOffset, u32 *readBuff)
{
	u32 i;

	switch (lun)
	{
		case 0:
			LED_RGB_EnableOne(LED_GREEN);

			for(i = 0; i < MassBlockSize[0]; i += SIZE_OF_U32)
			{
				readBuff[i / SIZE_OF_U32] = *((volatile u32*)(FLASH_MSD_START_ADDR + memOffset + i));
			}

			LED_RGB_DisableOne(LED_GREEN);
			break;

		default:
			return MAL_FAIL;
	}

	return MAL_OK;
}

u16 MAL_Write(u8 lun, u32 memOffset, u32 *writeBuff)
{
	u32 i;

	switch (lun)
	{
		case 0:
			LED_RGB_EnableOne(LED_RED);

			while(FLASH_GetStatus() != FLASH_COMPLETE);
			FLASH_ErasePage(FLASH_MSD_START_ADDR + memOffset);

			for(i = 0; i < MassBlockSize[0]; i += SIZE_OF_U32)
			{
				while(FLASH_GetStatus() != FLASH_COMPLETE);
				FLASH_ProgramWord(FLASH_MSD_START_ADDR + memOffset + i, writeBuff[i / SIZE_OF_U32]);
			}

			LED_RGB_DisableOne(LED_RED);
			break;

		default:
			return MAL_FAIL;
	}

	return MAL_OK;
}

u16 MAL_GetStatus(u8 lun) //Get status
{
	switch(lun)
	{
		case 0:
			MassBlockCount[0] = MSD_FULL_SIZE / MSD_BLOCK_SIZE;
			MassBlockSize[0] =  MSD_BLOCK_SIZE;
			MassMemorySize[0] = MSD_FULL_SIZE;
			break;

		default:
			return MAL_FAIL;
	}

	return MAL_OK;
}
