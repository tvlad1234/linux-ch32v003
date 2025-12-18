#ifndef _PINS_H
#define _PINS_H

#include "ch32fun.h"

#if defined(BOARD_CHINOOK)

// PSRAM CS
#define PSRAM_GPIO GPIOD
#define PSRAM_CS_PIN 0

// SD CS
#define SD_CS_GPIO GPIOC
#define SD_CS_PIN 0

// HC-05
#define HCEN_GPIO GPIOC
#define HCEN_PIN 4

#else

// PSRAM CS
#define PSRAM_GPIO GPIOD
#define PSRAM_CS_PIN 3

// SD CS
#define SD_CS_GPIO GPIOC
#define SD_CS_PIN 0

#endif

#endif
