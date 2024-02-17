#ifndef __PSRAM_H
#define __PSRAM_H

#include "ch32v003fun.h"

int psram_init();
void psram_read( uint32_t addr, void *ptr, uint32_t size );
void psram_write( uint32_t addr, void const *ptr, uint32_t size );
void psram_load_data( void const *d, uint32_t addr, uint32_t size );
void psram_read_data( void *buf, uint32_t addr, uint32_t size );

void cache_read(uint32_t addr, void *ptr, uint8_t size);
void cache_write(uint32_t addr, void *ptr, uint8_t size);
void cache_reset();


#endif