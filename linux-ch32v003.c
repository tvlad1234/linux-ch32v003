/* port of mini-rv32ima to ch32v003 because why not */

#include "ch32fun.h"
#include <stdio.h>

#include "tiny-rv32ima.h"
#include "pins.h"
#include "vm_config.h"

#define SYSTICK_100_MICROSECOND ((uint32_t)FUNCONF_SYSTEM_CORE_CLOCK / 10000)

void systick_init(void);
void gpio_init(void);

// Global microsecond counter
volatile uint64_t systick_micros;

// UART queue
volatile char key_queue[KEY_QUEUE_LEN + 1];
volatile uint8_t keys_num;

int main()
{
	SystemInit();

	// Enable GPIOs
	gpio_init();

	// Enable UART receive and receive interrupt
	USART1->CTLR1 |= USART_CTLR1_RE | USART_CTLR1_RXNEIE;
	NVIC_EnableIRQ(USART1_IRQn);

	// Configure and enable SPI
	SPI_init();
	SPI_begin_8();

	// Configure and enable systick
	systick_init();

	Delay_Ms(1000);

	// Start RISC-V VM
	vm_init_hw();
	int vm_state = EMU_GET_SD;

	while (1)
	{
		vm_state = start_vm(vm_state);
	}
}

void gpio_init(void)
{
	// Enable GPIO clocks
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;

	// PSRAM CS Push-Pull
	PSRAM_GPIO->CFGLR &= ~(0xf << (4 * PSRAM_CS_PIN));
	PSRAM_GPIO->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP) << (4 * PSRAM_CS_PIN);
	PSRAM_GPIO->BSHR = (1 << PSRAM_CS_PIN);

	// SD Card CS Push-Pull
	SD_CS_GPIO->CFGLR &= ~(0xf << (4 * SD_CS_PIN));
	SD_CS_GPIO->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP) << (4 * SD_CS_PIN);
	SD_CS_GPIO->BSHR = (1 << SD_CS_PIN);

	// SCK on PC5, 10MHz Output, alt func, push-pull
	GPIOC->CFGLR &= ~(0xf << (4 * 5));
	GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP_AF) << (4 * 5);

	// MOSI on PC6, 10MHz Output, alt func, push-pull
	GPIOC->CFGLR &= ~(0xf << (4 * 6));
	GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP_AF) << (4 * 6);

	// MISO on PC7, 10MHz input, floating
	GPIOC->CFGLR &= ~(0xf << (4 * 7));
	GPIOC->CFGLR |= GPIO_CNF_IN_FLOATING << (4 * 7);

#if defined(BOARD_CHINOOK)
	// set HCEN high (HC-05 data mode)
	HCEN_GPIO->CFGLR &= ~(0xf << (4 * HCEN_PIN));
	HCEN_GPIO->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP) << (4 * HCEN_PIN);
	HCEN_GPIO->BSHR = (1 << HCEN_PIN);
#endif

}

void systick_init(void)
{
	// Reset any pre-existing configuration
	SysTick->CTLR = 0x0000;

	// Set the compare register to trigger once per millisecond
	SysTick->CMP = SYSTICK_100_MICROSECOND - 1;

	// Reset the Count Register, and the global counter
	SysTick->CNT = 0x00000000;
	systick_micros = 0x00000000;

	// Set the SysTick Configuration
	SysTick->CTLR |= SYSTICK_CTLR_STE |	 // Enable Counter
					 SYSTICK_CTLR_STIE | // Enable Interrupts
					 SYSTICK_CTLR_STCLK; // Set Clock Source to HCLK/1

	// Enable the SysTick IRQ
	NVIC_EnableIRQ(SysTicK_IRQn);
}

void SysTick_Handler(void) __attribute__((interrupt));
void SysTick_Handler(void)
{
	// Increment the Compare Register for the next trigger
	SysTick->CMP += SYSTICK_100_MICROSECOND;

	// Clear the trigger state for the next IRQ
	SysTick->SR = 0x00000000;

	// Increment the count
	systick_micros += 100;
}

// UART receive interrupt handler
void USART1_IRQHandler(void) __attribute__((interrupt));
void USART1_IRQHandler(void)
{
	if (keys_num <= KEY_QUEUE_LEN)
		key_queue[keys_num++] = USART1->DATAR;
}
