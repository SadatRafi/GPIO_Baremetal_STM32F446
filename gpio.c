#include "stm32f4xx.h"  // Device-specific header from CMSIS, provides register definitions

int main(void) 
{
	// 1. Enable clocks for GPIOA and GPIOC using RCC (Reset and Clock Control)
	RCC->AHB1ENR |= (1 << 0);  /* Enable clock for GPIOA (bit 0 of AHB1ENR) */
	RCC->AHB1ENR |= (1 << 2);  /* Enable clock for GPIOC (bit 2 of AHB1ENR) */

	// 2. Configure PA5 (onboard LED) as General Purpose Output
	GPIOA->MODER &= ~GPIO_MODER_MODER5;   /* Clear mode bits [11:10] for PA5 */
	GPIOA->MODER |=  GPIO_MODER_MODER5_0; /* Set mode bits to 01: output mode */

	// 3. Configure PC13 (user button) as input
	GPIOC->MODER &= ~GPIO_MODER_MODER13;  /* Clear mode bits [27:26] for PC13 — sets it to input (00) */

	while (1) /* Infinite loop — keeps the MCU running */
	{
		while (1) /* Inner loop: continuously check button state */
		{
			if (!(GPIOC->IDR & GPIO_IDR_ID13)) 
			/* Check if PC13 is LOW (button pressed). Active-low logic is used here. */
			{
				for (volatile int i = 0; i < 1000; i++); 
				/* Simple delay loop to debounce the initial press (approx. 1–5 ms) */

				while (!(GPIOC->IDR & GPIO_IDR_ID13));  
				/* Wait here until button is released (PC13 goes HIGH) */

				for (volatile int i = 0; i < 1000; i++); 
				/* Debounce delay after button release */

				GPIOA->ODR ^= GPIO_ODR_OD5;  
				/* Toggle PA5 (LED) using XOR: if it was ON, turn OFF, and vice versa */
			}
		}
	}
}
