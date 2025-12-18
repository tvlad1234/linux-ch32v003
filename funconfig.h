#ifndef _FUNCONFIG_H
#define _FUNCONFIG_H

// Though this should be on by default we can extra force it on.
// #define FUNCONF_USE_DEBUGPRINTF 1
// #define FUNCONF_DEBUGPRINTF_TIMEOUT (1<<31) // Wait for a very very long time.

#define FUNCONF_USE_DEBUGPRINTF 0
#define FUNCONF_USE_UARTPRINTF  1

#if defined(BOARD_CHINOOK)
#define FUNCONF_UART_PRINTF_BAUD 38400
#else
#define FUNCONF_UART_PRINTF_BAUD 115200
#endif

#define CH32V003           1

#endif

