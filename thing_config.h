#ifndef _THING_CONFIG_H
#define _THING_CONFIG_H

#include "ch32v003fun.h"

// Image filename
#define IMAGE_FILENAME "IMAGE"

// PSRAM CS 
#define PSRAM_GPIO GPIOD
#define PSRAM_CS_PIN 3

// SD CS
#define SD_CS_GPIO GPIOC
#define SD_CS_PIN 0

// RAM size in megabytes
#define EMULATOR_RAM_MB 8

#endif