#include "stm32f10x.h"
#include "stm32f10x_rcc.h"

static void SystemClock_Config(void);

void SystemInit (void)
{
	/* Reset the RCC clock configuration to the default reset state (for debug purpose) */
	/* Set HSION bit */
	RCC->CR |= (u32)0x00000001;

	/* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
	RCC->CFGR &= (u32)0xF8FF0000;

	/* Reset HSEON, CSSON and PLLON bits */
	RCC->CR &= (u32)0xFEF6FFFF;

	/* Reset HSEBYP bit */
	RCC->CR &= (u32)0xFFFBFFFF;

	/* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
	RCC->CFGR &= (u32)0xFF80FFFF;

	/* Disable all interrupts and clear pending bits  */
	RCC->CIR = 0x009F0000;

	/* Configure the System clock frequency, HCLK, PCLK2 and PCLK1 prescalers */
	/* Configure the Flash Latency cycles and enable prefetch buffer */
	SystemClock_Config();

	SCB->VTOR = FLASH_BASE; //Vector Table Relocation in Internal FLASH.
}

static void SystemClock_Config(void)
{
	RCC->CR |= RCC_CR_HSEON; //HSE on
	while ((RCC->CR & RCC_CR_HSERDY) == RESET);

	FLASH->ACR |= (u32)FLASH_ACR_PRFTBE; //Prefetch buffer enable
	FLASH->ACR |= (u32)FLASH_ACR_LATENCY_2; //2 wait state

	RCC->CFGR |= RCC_CFGR_HPRE_DIV1; //AHB prescaler 1 (72 MHz)
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; //APB1 prescaler 2 (36 MHz)
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; //APB2 prescaler 1 (72 MHz)
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6; //ADC prescaler 8 (9 MHz)
	RCC->CFGR |= RCC_PLLSource_HSE_Div2; //HSE clock (16MHz) divided by 2 (8MHz) and selected as PLL input
	RCC->CFGR |= RCC_PLLMul_9; //PLL multiplication factor = 9

	RCC->CR |= RCC_CR_PLLON; //PLL used as system clock
	while((RCC->CR & RCC_CR_PLLRDY) == RESET);

	RCC->CFGR |= RCC_SYSCLKSource_PLLCLK; //PLL selected as system clock
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}
