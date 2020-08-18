#include "main.h"

/*
 * Variables for all file systems and file objects
 */
FRESULT	FATFS_Status = FR_NOT_READY, FILE_Status = FR_NO_FILE;
FATFS	FATFS_Obj;
UINT	readBytes;

/*
 * Variables for application file
 */
#define APP_BLOCK_TRANSFER_SIZE	512
FIL		appFile;
FSIZE_t appSize;
u32		appBodySize, appTailSize, appAddrPointer;
u8		appBuffer[APP_BLOCK_TRANSFER_SIZE];

/*
 * Variables for debug_enable file
 */
FIL	debugFile;
u8	debugBuffer[1];
FlagStatus debugOut = SET;

/*
 * Mass storage disk name: constant drive label, drive offset and buffer for flash page with label string
 */
#define DRIVE_LABEL_OFFSET (0x2000)
u8 driveName[11] = "BOOT_LOADER";
u32 pageBuf[512];

/*
 * Copy APP.BIN from USB MASS STORAGE to USER_MEM
 */
u32 compareStatus = FALSE;
void CopyAppToUserMemory(void);

/*
 * Jump to user application
 */
void GoToUserApp(void);

/*
 * PeriphDeInit all
 */
void PeriphDeInit(void);

/*
 * Application hardware CRC compute
 */
u32 AppCRC;

/*
 * Temp vars
 */
u32 i, j, k;

int main(void)
{
	Button_Config();
	USART1_Config();
	CRC_Config();

/*#if DEBUG
	if(f_mount(&FATFS_Obj, "0", 1) == FR_OK)
	{
		if(f_open(&debugFile, "/DEBUG.TXT", FA_READ) == FR_OK)
		{
			f_read(&debugFile, &debugBuffer, 1, &readBytes);
			f_close(&debugFile);
			if(debugBuffer[0] == '1')
			{
				debugOut = SET;
			}
			else
			{
				debugOut = RESET;
			}
		}
		f_mount(NULL, "0", 1);
	}
#endif*/ //-> feature for future

#if DEBUG
	if(debugOut == SET)
	{
		//USART1_Config();
		printf("\r\n\r\n---------------START LOG---------------\r\n\r\n");
		printf("BOOT_MEM start addr:  0x%08X\r\n", FLASH_BOOT_START_ADDR);
		printf("BOOT_MEM size:        %dK\r\n", FLASH_BOOT_SIZE / 1024);
		printf("USER_MEM start addr:  0x%08X\r\n", FLASH_USER_START_ADDR);
		printf("USER_MEM size:        %dK\r\n", FLASH_USER_SIZE / 1024);
		printf("MSD_MEM start addr:   0x%08X\r\n", FLASH_MSD_START_ADDR);
		printf("MSD_MEM size:         %dK\r\n", FLASH_MSD_SIZE / 1024);
		printf("OTHER_MEM start addr: 0x%08X\r\n", FLASH_OTHER_START_ADDR);
		printf("OTHER_MEM size:       %dK\r\n", FLASH_OTHER_SIZE / 1024);

		printf("\r\nTotal memory size:    %dK\r\n\r\n", FLASH_MAX_SIZE / 1024);
	}
#endif

	if(GPIO_ReadInputDataBit(BUTTON_PORT, BUTTON_PIN) == SET) //Bootloader or Mass Storage Device? TRUE == MSD mode
	{
#if DEBUG
		if(debugOut == SET)
		{
			printf("USB MSD Mode...\r\n");
		}
#endif
        LED_RGB_Config();
		USB_Config();
		Interrupts_Config();
		USB_Init();

		while(TRUE); //USB Mass Storage Device
	}

	/*
	 * Bootloader start routine:
	 */

#if DEBUG
	if(debugOut == SET)
	{
		printf("BOOTLOADER Mode...\r\n");
	}
#endif
	FATFS_Status = f_mount(&FATFS_Obj, "0", 1);
#if DEBUG
	if(debugOut == SET)
	{
		printf("FAT FS mount status = %d\r\n", FATFS_Status);
	}
#endif
	if(FATFS_Status == FR_OK)
	{
		FILE_Status = f_open(&appFile, "/APP.BIN", FA_READ);
#if DEBUG
		if(debugOut == SET)
		{
			printf("Application file open status = %d\r\n", FILE_Status);
		}
#endif
		if(FILE_Status == FR_OK)
		{
			appSize = f_size(&appFile);

			for(i = 0; i < appSize; i++) //Byte-to-byte compare files in MSD_MEM and USER_MEM
			{
				f_read(&appFile, &appBuffer, 1, &readBytes);

				if(*((volatile u8*)(FLASH_USER_START_ADDR + i)) != appBuffer[0]) //if byte of USER_MEM != byte of MSD_MEM
				{
					break;
				}
			}

			if(i != appSize) //=> was done "break" instruction in for(;;) cycle => new firmware in MSD_FLASH
			{
#if DEBUG
				if(debugOut == SET)
				{
					printf("Difference between MSD_MEM and USER_MEM: %d byte from %d byte\r\n", i, appSize);
					printf("Start copy MSD_MEM to USER_MEM:\r\n\r\n");
				}
#endif
				CopyAppToUserMemory();
			}
			else
			{
#if DEBUG
				if(debugOut == SET)
				{
					printf("No difference between MSD_MEM and USER_MEM!\r\n");
				}
#endif
			}

			FILE_Status = f_close(&appFile);
#if DEBUG
			if(debugOut == SET)
			{
				printf("File close status = %d\r\n", FILE_Status);
			}
#endif
			FATFS_Status = f_mount(NULL, "0", 1);
#if DEBUG
			if(debugOut == SET)
			{
				printf("FAT FS unmount status = %d\r\n", FATFS_Status);
			}
#endif
			/*
			 * At last, check CRC of firmware, which is located in USER_MEM
			 */

			CRC_ResetDR();
			for(i = 0; i < appSize; i += SIZE_OF_U32)
			{
				AppCRC = CRC_CalcCRC(*((volatile u32*)(FLASH_USER_START_ADDR + i)));
			}
#if DEBUG
			if(debugOut == SET)
			{
				printf("CRC of application in USER_FLASH = 0x%08X\r\n", AppCRC);
				printf("PeriphDeInit peripheral and jump to 0x%08X...\r\n", *((volatile u32*)(FLASH_USER_START_ADDR + 4)));
			}
#endif
			PeriphDeInit();
			GoToUserApp();
		}
		else //if FILE_Status != FR_OK
		{
			if(FILE_Status == FR_NO_FILE)
			{
#if DEBUG
				if(debugOut == SET)
				{
					printf("ERROR: File not found in MSD_MEM\r\n");
				}
#endif
			}
			else //if FILE_Status != FR_NO_FILE
			{
#if DEBUG
				if(debugOut == SET)
				{
					printf("ERROR: Other error of file opening\r\n");
				}
#endif
			}
			FATFS_Status = f_mount(NULL, "0", 1);
#if DEBUG
			if(debugOut == SET)
			{
				printf("FAT FS unmount status = %d\r\n", FATFS_Status);
			}
#endif
			while(TRUE);
		}
	}
	else //FATFS_Status != FR_OK
	{
#if DEBUG
		if(debugOut == SET)
		{
			printf("ERROR: FAT FS not mounted!\r\n");
		}
#endif
		while(TRUE);
	}
}

void GoToUserApp(void)
{
	u32 appJumpAddress;
	void (*GoToApp)(void);

	appJumpAddress = *((volatile u32*)(FLASH_USER_START_ADDR + 4));
	GoToApp = (void (*)(void))appJumpAddress;
	SCB->VTOR = FLASH_USER_START_ADDR;
	__set_MSP(*((volatile u32*) FLASH_USER_START_ADDR)); //stack pointer (to RAM) for USER app in this address
	GoToApp();
}

void CopyAppToUserMemory(void)
{
	f_lseek(&appFile, 0); //Go to the fist position of file

	appTailSize = appSize % APP_BLOCK_TRANSFER_SIZE;
	appBodySize = appSize - appTailSize;
	appAddrPointer = 0;
#if DEBUG
	if(debugOut == SET)
	{
		printf("File size = %d byte\r\nBody size = %d byte\r\nTail size = %d byte\r\n\r\n", appSize, appBodySize, appTailSize);
	}
#endif
	for(i = 0; i < ((appSize / FLASH_PAGE_SIZE) + 1); i++) //Erase n + 1 pages for new application
	{
		while(FLASH_GetStatus() != FLASH_COMPLETE);
		FLASH_ErasePage(FLASH_USER_START_ADDR + i * FLASH_PAGE_SIZE);
#if DEBUG
		if(debugOut == SET)
		{
			printf("Sector %3d (0x%08X - 0x%08X) erased\r\n", 	i,
																FLASH_USER_START_ADDR + i * FLASH_PAGE_SIZE,
																FLASH_USER_START_ADDR + (i + 1) * FLASH_PAGE_SIZE);
		}
#endif
	}
#if DEBUG
	if(debugOut == SET)
	{
		printf("\r\n");
	}
#endif
	for(i = 0; i < appBodySize; i += APP_BLOCK_TRANSFER_SIZE)
	{
		/*
		 * For example, size of File1 = 1030 bytes
		 * File1 = 2 * 512 bytes + 6 bytes
		 * "body" = 2 * 512, "tail" = 6
		 * Let's write "body" and "tail" to MCU FLASH byte after byte with 512-byte blocks
		 */
		FILE_Status = f_read(&appFile, appBuffer, APP_BLOCK_TRANSFER_SIZE, &readBytes); //Read 512 byte from file
#if DEBUG
		if(debugOut == SET)
		{
			printf("%d cycle, read status = %d, %d byte read\r\n", i / APP_BLOCK_TRANSFER_SIZE, FILE_Status, readBytes);
		}
#endif
		for(j = 0; j < APP_BLOCK_TRANSFER_SIZE; j += SIZE_OF_U32)
		{
			while(FLASH_GetStatus() != FLASH_COMPLETE);
			FLASH_ProgramWord(FLASH_USER_START_ADDR + i + j, *((volatile u32*)(appBuffer + j))); //write 512 byte to FLASH
		}
#if DEBUG
		if(debugOut == SET)
		{
			printf("%d byte programmed: ", j);
			printf("0x%08X - 0x%08X\r\n",	FLASH_USER_START_ADDR + appAddrPointer,
											FLASH_USER_START_ADDR + appAddrPointer + APP_BLOCK_TRANSFER_SIZE);
		}
#endif
		appAddrPointer += APP_BLOCK_TRANSFER_SIZE; //pointer to current position in FLASH for write
	}

	FILE_Status = f_read(&appFile, appBuffer, appTailSize, &readBytes); //Read "tail" that < 512 bytes from file
#if DEBUG
	if(debugOut == SET)
	{
		printf("Tail read: read status = %d, %d byte read, size of tail = %d\r\n", FILE_Status, readBytes, appTailSize);
	}
#endif
	while((appTailSize % SIZE_OF_U32) != 0)		//if appTailSize MOD 4 != 0 (seems not possible, but still...)
	{
		appTailSize++;						//increase the tail to a multiple of 4
		appBuffer[appTailSize - 1] = 0xFF;	//and put 0xFF in this tail place
	}
#if DEBUG
	if(debugOut == SET)
	{
		printf("New size of tail = %d\r\n", appTailSize);
	}
#endif
	for(i = 0; i < appTailSize; i += SIZE_OF_U32)
	{
		while(FLASH_GetStatus() != FLASH_COMPLETE);
		FLASH_ProgramWord(FLASH_USER_START_ADDR + appAddrPointer + i, *((volatile u32*)(appBuffer + i))); //write "tail" to FLASH
	}
#if DEBUG
	if(debugOut == SET)
	{
		printf("%d byte programmed: ", appTailSize);
		printf("0x%08X - 0x%08X\r\n", FLASH_USER_START_ADDR + appAddrPointer, FLASH_USER_START_ADDR + appAddrPointer + i);
		printf("\r\n");
	}
#endif
}

void PeriphDeInit(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, DISABLE);

#if DEBUG
	if(debugOut == SET)
	{
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}
		USART_Cmd(USART1, DISABLE);
		RCC_APB2PeriphResetCmd(USART1_CLK | USART1_CLK_PINS, DISABLE);
	}
#endif

	RCC_APB2PeriphClockCmd(BUTTON_CLK, DISABLE);
}

void SysTick_Handler(void)
{

}

int __io_putchar(int ch)
{
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}
	USART_SendData(USART1, (u8)ch);

	return 0;
}

/*
 *  Check CRC of firmware, which is located in MSD_MEM
 */

/*
CRC_ResetDR();
for(i = 0; i < appSize; i += SIZE_OF_U32)
{
	AppCRC = CRC_CalcCRC(*((volatile u32*)(FLASH_USER_START_ADDR + i)));
}
*/
