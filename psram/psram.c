#include <stdlib.h>

#include "ch32v003fun.h"
#include "thing_config.h"

#define CH32V003_SPI_SPEED_HZ 24000000
#define CH32V003_SPI_DIRECTION_2LINE_TXRX
#define CH32V003_SPI_CLK_MODE_POL0_PHA0		//leading = rising		trailing = falling		sample on leading		default if you're unsure
#define CH32V003_SPI_NSS_SOFTWARE_ANY_MANUAL	// toggle manually!
#define CH32V003_SPI_IMPLEMENTATION

#include "../extralibs/ch32v003_SPI.h"

#define PSRAM_CMD_RES_EN 0x66
#define PSRAM_CMD_RESET 0x99
#define PSRAM_CMD_READ_ID 0x9F
#define PSRAM_CMD_READ 0x03
#define PSRAM_CMD_READ_FAST 0x0B
#define PSRAM_CMD_WRITE 0x02
#define PSRAM_KGD 0x5D

#define PSRAM_DESEL PSRAM_GPIO->BSHR = ( 1 << PSRAM_CS_PIN )
#define PSRAM_SEL PSRAM_GPIO->BSHR = ( 1 << ( 16 + PSRAM_CS_PIN ) )

void PSRAM_SPI_WRITE( void const *buf, uint32_t size )
{
	while(size--)
		SPI_transfer_8( *(uint8_t *)(buf++) );
}

void PSRAM_SPI_READ( uint8_t *buf, uint32_t size )
{
	while(size--)
		*(buf++) = SPI_transfer_8(0);

}

uint8_t zeros[3];

void psram_send_cmd( uint8_t cmd )
{
	PSRAM_SPI_WRITE( &cmd, 1 );
}

void psram_reset()
{
	PSRAM_SEL;
	psram_send_cmd( PSRAM_CMD_RES_EN );
	psram_send_cmd( PSRAM_CMD_RESET );
	PSRAM_DESEL;
	Delay_Ms( 10 );
}

void psram_read_id( uint8_t *dst )
{
	PSRAM_SEL;
	psram_send_cmd( PSRAM_CMD_READ_ID );
	PSRAM_SPI_WRITE( zeros, 3 );
	PSRAM_SPI_READ( dst, 6 );
	PSRAM_DESEL;
}

int psram_init()
{
	SPI_init();
	SPI_begin_8();
	
	// PSRAM CS Push-Pull
	PSRAM_GPIO->CFGLR &= ~( 0xf << ( 4 * PSRAM_CS_PIN ) );
	PSRAM_GPIO->CFGLR |= ( GPIO_Speed_50MHz | GPIO_CNF_OUT_PP ) << ( 4 * PSRAM_CS_PIN );
	PSRAM_DESEL;
	Delay_Ms(10);

	psram_reset();

	uint8_t chipId[6];
	psram_read_id( chipId );

	if ( chipId[1] != PSRAM_KGD ) return -1;
	
	return 0;
}

uint8_t cmdAddr[5];

void psram_write( uint32_t addr, void const *ptr, uint32_t size )
{
	cmdAddr[0] = PSRAM_CMD_WRITE;

	cmdAddr[1] = ( addr >> 16 ) & 0xff;
	cmdAddr[2] = ( addr >> 8 ) & 0xff;
	cmdAddr[3] = addr & 0xff;

	PSRAM_SEL;
	PSRAM_SPI_WRITE( cmdAddr, 4 );
	PSRAM_SPI_WRITE( ptr, size );
	PSRAM_DESEL;
}

void psram_read( uint32_t addr, void *ptr, uint32_t size )
{
	cmdAddr[0] = PSRAM_CMD_READ;

	cmdAddr[1] = ( addr >> 16 ) & 0xff;
	cmdAddr[2] = ( addr >> 8 ) & 0xff;
	cmdAddr[3] = addr & 0xff;

	PSRAM_SEL;
	PSRAM_SPI_WRITE( cmdAddr, 4 );
	PSRAM_SPI_READ( ptr, size );
	PSRAM_DESEL;
}

void psram_load_data( void const *buf, uint32_t addr, uint32_t size )
{
	while ( size >= 64 )
	{
		psram_write( addr, buf, 64 );
		addr += 64;
		buf += 64;
		size -= 64;
	}

	if ( size ) psram_write( addr, buf, size );
}

void psram_read_data( void *buf, uint32_t addr, uint32_t size )
{
	while ( size >= 64 )
	{
		psram_read( addr, buf, 64 );
		addr += 64;
		buf += 64;
		size -= 64;
	}

	if ( size ) psram_read( addr, buf, size );
}
