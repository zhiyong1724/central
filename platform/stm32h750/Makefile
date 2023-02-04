BUILD_DIR := build
OBJ_DIR	:= $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
ELF_FILE := $(BIN_DIR)/main.elf
HEX_FILE := $(BIN_DIR)/main.hex
BIN_FILE := $(BIN_DIR)/main.bin
MAP_FILE := $(BIN_DIR)/main.map
DUMP_FILE := $(BIN_DIR)/main.dump

vpath %.c ./hal/Src
vpath %.c ./drivers
vpath %.o $(OBJ_DIR)

CC := arm-none-eabi-gcc
CXX := arm-none-eabi-g++
AR := arm-none-eabi-ar
LD := arm-none-eabi-ld
CP := arm-none-eabi-objcopy
DUMP := arm-none-eabi-objdump

INCLUDES := -I. \
	-Ihal/Inc \
	-Ihal/Inc/Legacy \
	-Icmsis/Device/ST/STM32H7xx/Include \
	-Icmsis/Include \
	-Idrivers \
	-Iffmpeg/include \
	-I../../third/lvgl

LIBS_DIR := -L. -L./ffmpeg/lib

LIBS := -llvgl -lc -lm

CFLAGS := -Wall -Werror -g -Og -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -DUSE_HAL_DRIVER -DSTM32H750xx -Wl,-Map=$(MAP_FILE) -ffunction-sections -fdata-sections -Wl,--gc-sections

SRCS := stm32h7xx_it.c \
	stm32h7xx_hal_msp.c \
	stm32h7xx_hal_cortex.c \
	stm32h7xx_hal_tim.c \
	stm32h7xx_hal_tim_ex.c \
	stm32h7xx_hal_rcc.c \
	stm32h7xx_hal_rcc_ex.c \
	stm32h7xx_hal_flash.c \
	stm32h7xx_hal_flash_ex.c \
	stm32h7xx_hal_gpio.c \
	stm32h7xx_hal_hsem.c \
	stm32h7xx_hal_dma.c \
	stm32h7xx_hal_dma_ex.c \
	stm32h7xx_hal_mdma.c \
	stm32h7xx_hal_pwr.c \
	stm32h7xx_hal_pwr_ex.c \
	stm32h7xx_hal.c \
	stm32h7xx_hal_i2c.c \
	stm32h7xx_hal_i2c_ex.c \
	stm32h7xx_hal_exti.c \
	stm32h7xx_hal_sai.c \
	stm32h7xx_hal_sai_ex.c \
	stm32h7xx_hal_uart.c \
	stm32h7xx_hal_uart_ex.c \
	stm32h7xx_hal_usart.c \
	stm32h7xx_hal_usart_ex.c \
	stm32h7xx_hal_sdram.c \
	stm32h7xx_hal_qspi.c \
	stm32h7xx_hal_nand.c \
	stm32h7xx_hal_sd.c \
	stm32h7xx_hal_ltdc.c \
	stm32h7xx_ll_fmc.c \
	stm32h7xx_ll_sdmmc.c \
	system_stm32h7xx.c \
	diskio.c \
	downloadmode.c \
	fatfsadapter.c \
	ffsystem.c \
	lfsadapter.c \
	lfsio.c \
	lvglio.c \
	main.c \
	newlibio.c \
	normalmode.c \
	osport.c \
	shellio.c \
	xmodem.c 

ASM_SRCS := startup_stm32h750xx.s

include ./drivers/Makefile
include ../../apps/Makefile
include ../../core/Makefile
include ../../third/Makefile

BASE_OBJS := $(patsubst %.c, %.o, $(SRCS))
OBJS := $(addprefix $(OBJ_DIR)/,$(BASE_OBJS))
BASE_ASM_OBJS := $(patsubst %.s, %.o, $(ASM_SRCS))
OBJS += $(addprefix $(OBJ_DIR)/,$(BASE_ASM_OBJS))

all : $(BUILD_DIR) $(OBJ_DIR) $(BIN_DIR) $(ELF_FILE) $(HEX_FILE) $(BIN_FILE) $(DUMP_FILE)

$(DUMP_FILE) : $(ELF_FILE)
	@echo DUMP $(DUMP_FILE)
	@$(DUMP) -d -S $(ELF_FILE) >$(DUMP_FILE)
	@ls -l $(BIN_DIR)

$(BIN_FILE) : $(ELF_FILE)
	@echo CP $(BIN_FILE)
	@$(CP)  -O binary -S $(ELF_FILE) $(BIN_FILE)

$(HEX_FILE) : $(ELF_FILE)
	@echo CP $(HEX_FILE)
	@$(CP)  -O ihex -S $(ELF_FILE) $(HEX_FILE)

$(ELF_FILE) : $(BASE_OBJS) $(BASE_ASM_OBJS)
	@echo LD $(ELF_FILE)
	@$(CC) $(CFLAGS) -Wl,-Tstm32h750.ld $(OBJS) -o $(ELF_FILE) $(LIBS_DIR) $(LIBS)

$(BASE_OBJS) : %.o : %.c
	@echo CC $<
	@$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$@ $(INCLUDES)

$(BASE_ASM_OBJS) : %.o : %.s
	@echo CC $<
	@$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$@ $(INCLUDES)

$(BUILD_DIR) :
	mkdir $(BUILD_DIR)
	
$(OBJ_DIR) :
	mkdir $(OBJ_DIR)
	
$(BIN_DIR) :
	mkdir $(BIN_DIR)
	
.PHONY : clean

clean :
	rm -rf $(BUILD_DIR)