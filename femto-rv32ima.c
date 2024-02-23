/* port of mini-rv32ima to ch32v003 because why not */

#include "ch32v003fun.h"
#include <stdio.h>

#include "emulator.h"
#include "hw_spi.h"
#include "pff/pff.h"
#include "psram.h"
#include "thing_config.h"

void load_sd_file( uint32_t addr, const char filename[] );

int main()
{
	SystemInit();
	
	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;
	USART1->CTLR1 |= USART_CTLR1_RE;

	// PSRAM CS Push-Pull
	PSRAM_GPIO->CFGLR &= ~( 0xf << ( 4 * PSRAM_CS_PIN ) );
	PSRAM_GPIO->CFGLR |= ( GPIO_Speed_50MHz | GPIO_CNF_OUT_PP ) << ( 4 * PSRAM_CS_PIN );
	PSRAM_GPIO->BSHR = ( 1 << PSRAM_CS_PIN );

	// SD Card CS Push-Pull
	SD_CS_GPIO->CFGLR &= ~( 0xf << ( 4 * SD_CS_PIN ) );
	SD_CS_GPIO->CFGLR |= ( GPIO_Speed_50MHz | GPIO_CNF_OUT_PP ) << ( 4 * SD_CS_PIN );
	SD_CS_GPIO->BSHR = ( 1 << SD_CS_PIN );

	// Enable SPI
	SPI_init();
	SPI_begin_8();

	Delay_Ms( 2000 );

	printf("\n\rfemto-rv32ima, compiled %s\n\r", __TIMESTAMP__);

	if ( psram_init() )
	{
		printf( "PSRAM init failed.\n\r" );
		while ( 1 )
			;
	}
	else printf( "PSRAM init ok!\n\r" );

	load_sd_file( 0, IMAGE_FILENAME );

	puts( "\nStarting RISC-V VM\n\n\r" );
	int c = riscv_emu();
	while ( c == EMU_REBOOT ) c = riscv_emu();

	for( ;; )
		;
}


void die( FRESULT rc )
{
	printf( "Failed with rc=%u.\n", rc );
	for ( ;; )
		;
}

void load_sd_file( uint32_t addr, const char filename[] )
{
	FATFS fatfs; /* File system object */
	UINT br;
	BYTE buff[64];

	printf( "Mounting volume.\n\r" );
	FRESULT rc = pf_mount( &fatfs );
	if ( rc ) die( rc );

	printf( "Opening file \"%s\"\n\r", filename );
	rc = pf_open( filename );
	if ( rc ) die( rc );

	printf( "Loading image into RAM\n\r" );
	for ( ;; )
	{
		rc = pf_read( buff, sizeof( buff ), &br ); /* Read a chunk of file */
		if ( rc || !br ) break; /* Error or end of file */

		psram_write( addr, buff, br );
		addr += br;
	}
	if ( rc ) die( rc );
}
