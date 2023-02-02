MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/, %, $(dir $(MAKEFILE_PATH)))
INCLUDES += -I$(CURRENT_DIR)
vpath %.c $(CURRENT_DIR)
SRCS += dma.c \
	es8388.c \
	fmc.c \
	gpio.c \
	gt9147.c \
	key.c \
	keymanager.c \
	led.c \
	ltdc.c \
	nandflash.c \
	pcf8574.c \
	sai.c \
	sdcard.c \
	sdmmc.c \
	sdram.c \
	softiic.c \
	usart.c \
	volumemanager.c