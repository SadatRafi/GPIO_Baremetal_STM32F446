# 🔌 STM32F446 GPIO Tutorial – Digital Input & Output (Bare-Metal, Keil)

This is a beginner-friendly tutorial for controlling GPIO pins on the STM32F446 Nucleo board using **bare-metal programming** in **Keil µVision**.  
We demonstrate how to toggle an onboard LED using the onboard push-button, with simple loop-based delay for debouncing — no HAL, no external libraries.

---

## 🧱 Hardware Setup

- **Board:** STM32F446 Nucleo
- **LED:** LD2 (connected to **GPIOA Pin 5**)
- **Push-button:** B1 (connected to **GPIOC Pin 13**, active-low)

---

## 📘 Description

- 🟢 **Objective:** Toggle the onboard LED (PA5) each time the push-button (PC13) is pressed.
- 🛠️ **Method:** Direct register access (bare-metal).
- ⏱️ **Debouncing:** Handled via a simple blocking loop — no dedicated delay function.
- 🧰 **Toolchain:** Keil µVision IDE (Arm MDK), CMSIS headers only.

---

## 🔧 How It Works

### 🧰 Step 1. Project Setup in Keil µVision

1. Open **Keil µVision** and create a new project.
2. Select the device:  
   `STM32F446RETx` (or the appropriate variant on your Nucleo board).
3. Open **Manage Run-Time Environment**:  
   <img width="1920" height="1080" alt="Capture" src="https://github.com/user-attachments/assets/b5b7f209-20ac-404f-88ba-c6100512dadd" />
   - Under **Device**, check ✅ `Startup`.
   - Under **CMSIS**, select `CORE`.

### 📄 Step 2. Add Source File

1. In the **Project window**, right-click on **Source Group 1** →  
<img width="1920" height="1037" alt="Capture1" src="https://github.com/user-attachments/assets/a2475d25-7eed-4bad-90fd-dc078e4b1bc0" />
2. Choose **C File (.c)** and name it `main.c`.

## 🧾 Step 3: Include Device-Specific Header

At the very beginning of `main.c`, include:

```c
#include "stm32f4xx.h"  // Device-specific header
```

### 🔍 Why it's needed

This header is provided by **CMSIS (Cortex Microcontroller Software Interface Standard)**.

It includes:

- The memory addresses and definitions of all STM32F4 peripheral registers.
- The register structures (e.g. `GPIOA`, `RCC`, `NVIC`) mapped to their correct base addresses.
- Helpful macros and bit definitions for clean and readable bare-metal code.

Without this header, you wouldn't be able to use register names directly, like:

```c
RCC->AHB1ENR |= (1 << 0);       // Enable GPIOA clock  
GPIOA->MODER |= (1 << (5 * 2)); // Set PA5 as output  
```

### 🧾 Step 4: Enable the clock of Ithe O peripherals

Before using **GPIOA** and **GPIOC** on the STM32F446 Nucleo board, you must enable their clocks through the RCC (Reset and Clock Control) peripheral.

In this code:

```c
RCC->AHB1ENR |= (1 << 0);  // Enable clock for GPIOA
RCC->AHB1ENR |= (1 << 2);  // Enable clock for GPIOC
```
### 🔍 What's Happening?

- `RCC->AHB1ENR` is the **AHB1 peripheral clock enable register**.
- Each bit in this register corresponds to a peripheral on the AHB1 bus:
  - Bit 0 → GPIOA  
  - Bit 2 → GPIOC

By setting these bits:

- **GPIOA gets clock** so that we can control PA5 (the onboard LED).
- **GPIOC gets clock** so we can read PC13 (the user button).

### 🧠 Without This?

If you skip these lines:

- Any access to `GPIOA->MODER`, `GPIOA->ODR`, or `GPIOC->IDR` will **fail or do nothing**, because the peripherals aren't powered.

## 🧾 Step 5: Set PC13 as Input (Using Bitmask Macro)

To configure **PC13** (User Button) as an input pin, we use the following line:

```c
GPIOC->MODER &= ~GPIO_MODER_MODER13; // Clear mode bits for PC13 (input mode)
```
### 🔍 Why This Works

- Each GPIO pin on STM32 has **2 mode bits** in the `MODER` register.
- For pin **13**, those bits are located at positions **[27:26]**.
- To configure PC13 as an **input**, those bits must be set to `00`.

The macro `GPIO_MODER_MODER13` is defined in the STM32 device header as:

```c
#define GPIO_MODER_MODER13_Pos  (13U * 2U)                         // Bit position = 26
#define GPIO_MODER_MODER13_Msk  (0x3U << GPIO_MODER_MODER13_Pos)  // Mask for bits 27:26
#define GPIO_MODER_MODER13      GPIO_MODER_MODER13_Msk
```
So when we write:

```c
GPIOC->MODER &= ~GPIO_MODER_MODER13;
```
We're doing:

```c 
GPIOC->MODER &= 0xF3FFFFFF;  // Clear only bits 27 and 26
```

## 🧾 Step 6: Set PA5 (LED) as Output (Using Bitmask Macros)

To configure **PA5** (the onboard LED on STM32F446 Nucleo) as an **output pin**, use:

```c
GPIOA->MODER &= ~GPIO_MODER_MODER5;         // Clear PA5 mode bits
GPIOA->MODER |=  GPIO_MODER_MODER5_0;       // Set PA5 as output (01)
```
### 🔍 Why This Works

Each pin in the `MODER` register uses **2 bits** to define its mode:

- `00` → Input  
- `01` → Output  
- `10` → Alternate Function  
- `11` → Analog

For **PA5**, its 2 mode bits are at positions **[11:10]**.

- `GPIO_MODER_MODER5` is a mask that targets both bits (clears them).
- `GPIO_MODER_MODER5_0` sets the **lower bit** of that pair (bit 10), which corresponds to mode `01` — **output**.

## 🧾 Step 7: Toggle LED on Button Press (with Debounce)

### 🔁 What We're Doing

- We're **polling** the button (PC13) in a loop.
- If the button is **pressed** (logic low), we:
  - Wait a short time (debounce).
  - Wait until the button is released.
  - Wait again (debounce after release).
  - Then **toggle the LED** (PA5).

### 💡 Key Code

```c
if (!(GPIOC->IDR & GPIO_IDR_ID13)) {
    for (volatile int i = 0; i < 1000; i++);    // Debounce delay

    while (!(GPIOC->IDR & GPIO_IDR_ID13));     // Wait for release

    for (volatile int i = 0; i < 1000; i++);    // Debounce delay

    GPIOA->ODR ^= GPIO_ODR_OD5;                // Toggle LED
}
```
### 🔍 Explanation

- GPIOC->IDR & GPIO_IDR_ID13 checks input state of PC13.

   - It’s 0 when button is pressed, due to pull-up.

- GPIOA->ODR ^= GPIO_ODR_OD5 toggles the output state of PA5.

- The simple for loops add a blocking delay to debounce the input.

## Now check the final code below

```c
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
```
