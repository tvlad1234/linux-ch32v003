#include "psram.h"

#define ADDR_BITS 24
#define OFFSET_BITS 4
#define CACHE_LINE_SIZE 16
#define INDEX_BITS 5

#define OFFSET(addr) (addr & 0b1111)
#define INDEX(addr) ((addr >> OFFSET_BITS) & 0b11111)
#define TAG(addr) (addr >> 9)
#define BASE(addr) (addr & (~(uint32_t)(0b1111)))

#define LINE_TAG(line) (line->tag)

#define IS_VALID(line) (line->status & 0b01)
#define IS_DIRTY(line) (line->status & 0b10)
#define IS_LRU(line) (line->status & 0b100)

#define SET_VALID(line) line->status = 1
#define SET_DIRTY(line) line->status |= 0b10;
#define SET_LRU(line) line->status |= 0b100;
#define CLEAR_LRU(line) line->status &= ~0b100;

struct Cacheline
{
    uint16_t tag;
    uint8_t data[CACHE_LINE_SIZE];
    uint8_t status;
};
typedef struct Cacheline cacheline_t;

cacheline_t cache[32][2];

uint64_t hits, misses;

void cache_read(uint32_t addr, void *ptr, uint8_t size)
{
    uint8_t index = INDEX(addr);
    uint16_t tag = TAG(addr);
    uint8_t offset = OFFSET(addr);

    cacheline_t *line;

    if (tag == LINE_TAG((&cache[index][0])) && IS_VALID((&cache[index][0])))
    {
        line = &(cache[index][0]);
        CLEAR_LRU((&cache[index][0]));
        SET_LRU((&cache[index][1]));
        hits++;
    }
    else if (tag == LINE_TAG((&cache[index][1])) && IS_VALID((&cache[index][1])))
    {
        line = &(cache[index][1]);
        CLEAR_LRU((&cache[index][1]));
        SET_LRU((&cache[index][0]));
        hits++;
    }

    else // miss
    {
        misses++;

        if (IS_LRU((&cache[index][0])))
        {
            line = &(cache[index][0]);
            CLEAR_LRU((&cache[index][0]));
            SET_LRU((&cache[index][1]));
        }

        else
        {
            line = &(cache[index][1]);
            CLEAR_LRU((&cache[index][1]));
            SET_LRU((&cache[index][0]));
        }

        if (IS_VALID(line) && IS_DIRTY(line)) // if line is valid and dirty, flush it to RAM
        {
            // flush line to RAM
            uint32_t flush_base = (index << OFFSET_BITS) | ((uint32_t)(LINE_TAG(line)) << 9);
            psram_write(flush_base, line->data, CACHE_LINE_SIZE);
        }

        // get line from RAM
        uint32_t base = BASE(addr);
        psram_read(base, line->data, CACHE_LINE_SIZE);

        line->tag = tag; // set the tag of the line
        SET_VALID(line); // mark the line as valid
    }

/*
    if (offset + size > CACHE_LINE_SIZE)
    {
        // printf("cross boundary read!\n");
        size = CACHE_LINE_SIZE - offset;
    }
*/

    for (int i = 0; i < size; i++)
        ((uint8_t *)(ptr))[i] = line->data[offset + i];
}

void cache_write(uint32_t addr, void *ptr, uint8_t size)
{
    uint8_t index = INDEX(addr);
    uint32_t tag = TAG(addr);
    uint8_t offset = OFFSET(addr);

      cacheline_t *line;

    if (tag == LINE_TAG((&cache[index][0])) && IS_VALID((&cache[index][0])))
    {
        line = &(cache[index][0]);
        CLEAR_LRU((&cache[index][0]));
        SET_LRU((&cache[index][1]));
        hits++;
    }
    else if (tag == LINE_TAG((&cache[index][1])) && IS_VALID((&cache[index][1])))
    {
        line = &(cache[index][1]);
        CLEAR_LRU((&cache[index][1]));
        SET_LRU((&cache[index][0]));
        hits++;
    }

    else // miss
    {
        misses++;

        if (IS_LRU((&cache[index][0])))
        {
            line = &(cache[index][0]);
            CLEAR_LRU((&cache[index][0]));
            SET_LRU((&cache[index][1]));
        }

        else
        {
            line = &(cache[index][1]);
            CLEAR_LRU((&cache[index][1]));
            SET_LRU((&cache[index][0]));
        }

        if (IS_VALID(line) && IS_DIRTY(line)) // if line is valid and dirty, flush it to RAM
        {
            // flush line to RAM
            uint32_t flush_base = (index << OFFSET_BITS) | ((uint32_t)(LINE_TAG(line)) << 9);
            psram_write(flush_base, line->data, CACHE_LINE_SIZE);
        }

        // get line from RAM
        uint32_t base = BASE(addr);
        psram_read(base, line->data, CACHE_LINE_SIZE);

        line->tag = tag; // set the tag of the line
        SET_VALID(line); // mark the line as valid
    }
/*
    if (offset + size > CACHE_LINE_SIZE)
    {
        // printf("cross boundary write!\n");
        size = CACHE_LINE_SIZE - offset;
    }
*/
    for (int i = 0; i < size; i++)
        line->data[offset + i] = ((uint8_t *)(ptr))[i];
    SET_DIRTY(line); // mark the line as dirty
}

void cache_reset()
{
    for(int i=0; i < 32; i++)
    {
        cache[i][0].status = 0;
        cache[i][1].status = 0;
    }
}
