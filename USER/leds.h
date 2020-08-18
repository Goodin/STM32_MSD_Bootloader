#ifndef __LEDS_H
#define __LEDS_H

#include "main.h"

#define LED_RED		0
#define LED_GREEN	1
#define LED_BLUE	2

void LED_RGB_SetBrightness(u32 led, u32 brightness);
void LED_RGB_EnableAll(void);
void LED_RGB_DisableAll(void);
void LED_RGB_EnableOne(u32 led);
void LED_RGB_DisableOne(u32 led);
void LED_RGB_ReverceOne(u32 led);

#endif /*__LEDS_H */
