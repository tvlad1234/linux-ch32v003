#ifndef _THING_CONFIG_H
#define _THIGN_CONFIG_H

#include "ch32v003fun.h"

// Image filename
#define IMAGE_FILENAME "IMAGE"

// PSRAM pins 
#define PSRAM_GPIO GPIOC
#define PSRAM_CS_PIN 4

// SD card pins
#define SD_CLK_GPIO GPIOD
#define SD_CLK_PIN 3

#define SD_TX_GPIO GPIOD
#define SD_TX_PIN 2

#define SD_RX_GPIO GPIOC
#define SD_RX_PIN 1

#define SD_CS_GPIO GPIOD
#define SD_CS_PIN 4

// RAM size in megabytes
#define EMULATOR_RAM_MB 8

#endif