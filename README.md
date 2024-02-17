# linux-ch32v003
Linux on a $0.15 RISC-V microcontroller

This project enables the CH32V003 microcontroller to run Linux. It achieves this by using an 8 megabyte SPI PSRAM chip and a RISC-V emulator (the very nice [mini-rv32ima by cnlohr](https://github.com/cnlohr/mini-rv32ima)). The emulation is needed because the PSRAM cannot be mapped into the address space of the microcontroler. The Linux kernel and rootfs is loaded into PSRAM at boot from an SD card. FAT filesystem access is provided by the [Petit FatFs](http://elm-chan.org/fsw/ff/00index_p.html) library.

## How to use
This project uses the [ch32v003fun](https://github.com/cnlohr/ch32v003fun) SDK, which must reside in the same folder where this repository was cloned.

The PSRAM is connected to the hardware SPI interface of the CH32V003. The chip select pin of the PSRAM, as well as the pins for the SD card can be configured in the [thing_config.h](thing_config.h) file. The console can be accessed over the UART pins. The SD card containing the [Linux image file](Image) must be formatted as FAT32 or FAT16, and the file must be placed in the root. 

Boot time is around 5 minutes. The Linux image includes the coremark benchmark in the `/root/` folder.
