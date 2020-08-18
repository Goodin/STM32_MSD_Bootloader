#include "leds.h"

void LED_RGB_SetBrightness(u32 led, u32 brightness)
{
	if(brightness >= 100)
	{
		brightness = 100;
	}

	brightness = 100 - brightness;

	switch(led)
	{
		case LED_RED:
			TIM3->CCR4 = brightness;
			break;

		case LED_GREEN:
			TIM3->CCR3 = brightness;
			break;

		case LED_BLUE:
			TIM3->CCR2 = brightness;
			break;

		default:
			break;
	}
}

void LED_RGB_EnableAll(void)
{
	TIM3 -> CCR2 = 0;
	TIM3 -> CCR3 = 0;
	TIM3 -> CCR4 = 0;
}

void LED_RGB_DisableAll(void)
{
	TIM3 -> CCR2 = 100;
	TIM3 -> CCR3 = 100;
	TIM3 -> CCR4 = 100;
}

void LED_RGB_EnableOne(u32 led)
{
	LED_RGB_SetBrightness(led, 100);
}

void LED_RGB_DisableOne(u32 led)
{
	LED_RGB_SetBrightness(led, 0);
}

void LED_RGB_ReverceOne(u32 led)
{
	u32 currentBrightness = 0, newBrightness;

	switch(led)
	{
		case LED_RED:
			currentBrightness = TIM3->CCR4;
			break;

		case LED_GREEN:
			currentBrightness = TIM3->CCR3;
			break;

		case LED_BLUE:
			currentBrightness = TIM3->CCR2;
			break;

		default:
			break;
	}

	if(currentBrightness >= 50)
	{
		newBrightness = 0;
	}
	else
	{
		newBrightness = 100;
	}

	switch(led)
	{
		case LED_RED:
			TIM3->CCR4 = newBrightness;
			break;

		case LED_GREEN:
			TIM3->CCR3 = newBrightness;
			break;

		case LED_BLUE:
			TIM3->CCR2 = newBrightness;
			break;

		default:
			break;
	}
}
