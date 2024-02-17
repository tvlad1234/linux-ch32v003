#include "ch32v003fun.h"

#include <stdio.h>
#include <stdlib.h>

#include "emulator.h"
#include "psram.h"
#include "thing_config.h"

#include "default64mbdtc.h"

int time_divisor = 256;
uint8_t fast_mode = 0;

static uint32_t HandleException( uint32_t ir, uint32_t retval );
static uint32_t HandleControlStore( uint32_t addy, uint32_t val );
static uint32_t HandleControlLoad( uint32_t addy );
static void HandleOtherCSRWrite( uint8_t *image, uint16_t csrno, uint32_t value );
static uint32_t HandleOtherCSRRead( uint8_t *image, uint16_t csrno );

#define INSTRS_PER_FLIP 1024

#define MICROSECOND_TICKS ( SysTick->CNT / DELAY_US_TIME )
#define KEYPRESS_AVAILABLE ( USART1->STATR & USART_FLAG_RXNE )
#define LAST_KEYPRESS ( USART1->DATAR )

#define MINIRV32WARN( x... ) printf( x );
#define MINIRV32_DECORATE static
#define MINI_RV32_RAM_SIZE (EMULATOR_RAM_MB * 1024 * 1024)
#define MINIRV32_IMPLEMENTATION
#define MINIRV32_POSTEXEC( pc, ir, retval )             \
	{                                                   \
		if ( retval > 0 )                               \
				retval = HandleException( ir, retval ); \
	}

#define MINIRV32_HANDLE_MEM_STORE_CONTROL( addy, val ) if ( HandleControlStore( addy, val ) ) return val;
#define MINIRV32_HANDLE_MEM_LOAD_CONTROL( addy, rval ) rval = HandleControlLoad( addy );
#define MINIRV32_OTHERCSR_WRITE( csrno, value ) HandleOtherCSRWrite( image, csrno, value );
#define MINIRV32_OTHERCSR_READ( csrno, rval )  { rval = HandleOtherCSRRead( image, csrno ); }

#define MINIRV32_CUSTOM_MEMORY_BUS

uint32_t store32;
uint16_t store16;
uint8_t store8;

#define MINIRV32_STORE4( ofs, val ) {store32 = val; cache_write( ofs, &store32, 4 ); }
#define MINIRV32_STORE2( ofs, val ) {store16 = val; cache_write( ofs, &store16, 2 ); }
#define MINIRV32_STORE1( ofs, val ) {store8 = val; cache_write( ofs, &store8, 1 ); }

static inline uint32_t MINIRV32_LOAD4( uint32_t ofs )
{
	uint32_t val;
	cache_read( ofs, &val, 4 );
	return val;
}

static inline uint16_t MINIRV32_LOAD2( uint32_t ofs )
{
	uint16_t val;
	cache_read( ofs, &val, 2 );
	return val;
}

static inline uint8_t MINIRV32_LOAD1( uint32_t ofs )
{
	uint8_t val;
	cache_read( ofs, &val, 1 );
	return val;
}

static inline int8_t MINIRV32_LOAD1_SIGNED( uint32_t ofs )
{
	int8_t val;
	cache_read( ofs, &val, 1 );
	return val;
}

static inline int16_t MINIRV32_LOAD2_SIGNED( uint32_t ofs )
{
	int16_t val;
	cache_read( ofs, &val, 2 );
	return val;
}

#include "mini-rv32ima.h"

struct MiniRV32IMAState core;

int riscv_emu()
{
	uint32_t dtb_ptr = MINI_RV32_RAM_SIZE - sizeof( default64mbdtb );
	uint32_t validram = dtb_ptr;

	psram_load_data( default64mbdtb, dtb_ptr, sizeof( default64mbdtb ) );

	uint32_t dtbRamValue = ( validram >> 24 ) | ( ( ( validram >> 16 ) & 0xff ) << 8 ) |
	                       ( ( ( validram >> 8 ) & 0xff ) << 16 ) | ( ( validram & 0xff ) << 24 );
	MINIRV32_STORE4( dtb_ptr + 0x13c, dtbRamValue );

	core.regs[10] = 0x00; // hart ID
	core.regs[11] = dtb_ptr + MINIRV32_RAM_IMAGE_OFFSET;
	core.extraflags |= 3; // Machine-mode.

	core.pc = MINIRV32_RAM_IMAGE_OFFSET;
	long long instct = -1;

	uint64_t rt;
	uint64_t lastTime = MICROSECOND_TICKS / time_divisor;
	
	for ( rt = 0; rt < instct + 1 || instct < 0; rt += INSTRS_PER_FLIP )
	{
		if(fast_mode ==  1)
		{
			time_divisor = 8;
			fast_mode = 2;
			puts("\n\rEMU: Fast mode engaged!\n\r");
		}
			
		uint64_t *this_ccount = ( (uint64_t *)&core.cyclel );

		uint32_t elapsedUs = MICROSECOND_TICKS / time_divisor - lastTime;
		lastTime += elapsedUs;

		int ret = MiniRV32IMAStep( &core, NULL, 0, elapsedUs, INSTRS_PER_FLIP ); // Execute upto 1024 cycles before breaking out.
		switch ( ret )
		{
			case 0: break;
			case 1:
				Delay_Ms( 1 );
				*this_ccount += INSTRS_PER_FLIP;
				break;
			case 3: instct = 0; break;
			case 0x7777:
				printf( "\n\rREBOOT@0x%08x%08x\n\r", (unsigned int)core.cycleh,  (unsigned int)core.cyclel );
				time_divisor = 256;
				fast_mode = 0;
				cache_reset();
				return EMU_REBOOT; // syscon code for reboot
			case 0x5555:
				printf( "\n\rPOWEROFF@0x%08x%08x\n\r",  (unsigned int)core.cycleh,  (unsigned int)core.cyclel );
				return EMU_POWEROFF; // syscon code for power-off
			default:
				printf( "\n\rUnknown failure\n" );
				return EMU_UNKNOWN;
				break;
		}
	}

	return EMU_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////////
// Functions for the emulator
//////////////////////////////////////////////////////////////////////////

// Exceptions handling

static uint32_t HandleException( uint32_t ir, uint32_t code )
{
	// Weird opcode emitted by duktape on exit.
	if ( code == 3 )
	{
		// Could handle other opcodes here.
	}
	return code;
}

// CSR handling (Linux HVC console)

static inline void HandleOtherCSRWrite( uint8_t *image, uint16_t csrno, uint32_t value )
{
	if ( csrno == 0x139 ) putchar( value );
}

static inline uint32_t HandleOtherCSRRead( uint8_t *image, uint16_t csrno )
{
	if ( csrno == 0x140 )
	{
		if( KEYPRESS_AVAILABLE )
		{
			fast_mode++;
			return LAST_KEYPRESS;
		}
		else return -1;
	}

	return 0;
}

// MMIO handling (8250 UART)

static uint32_t HandleControlStore( uint32_t addy, uint32_t val )
{
	if ( addy == 0x10000000 ) // UART 8250 / 16550 Data Buffer
		putchar( val );

	return 0;
}

static uint32_t HandleControlLoad( uint32_t addy )
{
	return 0;
}
