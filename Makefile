all : flash

TARGET:=femto-rv32ima

ADDITIONAL_C_FILES:= psram/psram.c psram/cache32x2x16.c emulator/emulator.c pff/pff.c pff/mmcbbp.c 
EXTRA_CFLAGS:= -Iemulator -Ipsram

CH32V003FUN:=../ch32v003fun/ch32v003fun
MINICHLINK?=$(CH32V003FUN)/../minichlink

include $(CH32V003FUN)/ch32v003fun.mk

flash : cv_flash
clean : cv_clean

linux : 
	make -C linux


.PHONY: linux