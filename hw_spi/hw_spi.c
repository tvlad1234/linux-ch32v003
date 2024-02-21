#include <stdint.h> 
#include "ch32v003fun.h"

static inline uint8_t SPI_read_8();
static inline void SPI_write_8(uint8_t data);

// SPI peripheral power enable / disable (default off, init() automatically enables)
// send SPI peripheral to sleep
static inline void SPI_poweroff();
// wake SPI peripheral from sleep
static inline void SPI_poweron();

// ######## internal function declarations
static inline void SPI_wait_TX_complete();
static inline uint8_t SPI_is_RX_empty();
static inline void SPI_wait_RX_available();

void SPI_set_prescaler(uint8_t presc)
{
    SPI1->CTLR1 &= ~SPI_CTLR1_BR;
    SPI1->CTLR1 |= SPI_CTLR1_BR & (presc << 3);
}

void SPI_init()
{
    SPI_poweron();


    // reset control register
    SPI1->CTLR1 = 0;

    SPI_set_prescaler(0);

    SPI1->CTLR1 |= (SPI_CPOL_Low | SPI_CPHA_1Edge);

    SPI1->CTLR1 |= SPI_NSS_Soft; // SSM NSS software control mode

    // SCK on PC5, 10MHz Output, alt func, push-pull
    GPIOC->CFGLR &= ~(0xf << (4 * 5));
    GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP_AF) << (4 * 5);

    // CH32V003 is master
    SPI1->CTLR1 |= SPI_Mode_Master;

    // set data direction and configure data pins
    SPI1->CTLR1 |= SPI_Direction_2Lines_FullDuplex;

    // MOSI on PC6, 10MHz Output, alt func, push-pull
    GPIOC->CFGLR &= ~(0xf << (4 * 6));
    GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP_AF) << (4 * 6);

    // MISO on PC7, 10MHz input, floating
    GPIOC->CFGLR &= ~(0xf << (4 * 7));
    GPIOC->CFGLR |= GPIO_CNF_IN_FLOATING << (4 * 7);
}

void SPI_begin_8()
{
    SPI1->CTLR1 &= ~(SPI_CTLR1_DFF); // DFF 16bit data-length enable, writable only when SPE is 0
    SPI1->CTLR1 |= SPI_CTLR1_SPE;
}

void SPI_end()
{
    SPI1->CTLR1 &= ~(SPI_CTLR1_SPE);
}

static inline uint8_t SPI_read_8()
{
    return SPI1->DATAR;
}

static inline void SPI_write_8(uint8_t data)
{
    SPI1->DATAR = data;
}

uint8_t SPI_transfer_8(uint8_t data)
{
    SPI_write_8(data);
    SPI_wait_TX_complete();
    asm volatile("nop");
    SPI_wait_RX_available();
    return SPI_read_8();
}

static inline void SPI_poweroff()
{
    SPI_end();
    RCC->APB2PCENR &= ~RCC_APB2Periph_SPI1;
}
static inline void SPI_poweron()
{
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1;
}

// ########  small internal function definitions, static inline
static inline void SPI_wait_TX_complete()
{
    while (!(SPI1->STATR & SPI_STATR_TXE))
    {
    }
}
static inline uint8_t SPI_is_RX_empty()
{
    return SPI1->STATR & SPI_STATR_RXNE;
}
static inline void SPI_wait_RX_available()
{
    while (!(SPI1->STATR & SPI_STATR_RXNE))
    {
    }
}
static inline void SPI_wait_not_busy()
{
    while ((SPI1->STATR & SPI_STATR_BSY) != 0)
    {
    }
}
static inline void SPI_wait_transmit_finished()
{
    SPI_wait_TX_complete();
    SPI_wait_not_busy();
}

