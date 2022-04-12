BUILD_DIR := build
OBJ_DIR	:= $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
LIB_FILE := $(BIN_DIR)/liblvgl.a

LVGL_DIR_NAME ?= .
LVGL_DIR ?= ${shell pwd}
include $(LVGL_DIR)/$(LVGL_DIR_NAME)/lvgl.mk

vpath %.o $(OBJ_DIR)

CC := arm-none-eabi-gcc
CXX := arm-none-eabi-g++
AR := arm-none-eabi-ar
LD := arm-none-eabi-ld

INCLUDES := -I. -I../../platform/stm32h750/cmsis/Device/ST/STM32H7xx/Include -I../../platform/stm32h750/cmsis/Include

LIBS_DIR := -L.

LIBS := -lpthread

CFLAGS := $(CFLAGS) -O3 -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -DUSE_HAL_DRIVER -DSTM32H750xx -DSTM32H7

CSRCS := $(CSRCS)

BASE_OBJS := $(patsubst %.c, %.o, $(CSRCS))

OBJS := $(addprefix $(OBJ_DIR)/, $(notdir $(BASE_OBJS)))

all : $(BUILD_DIR) $(OBJ_DIR) $(BIN_DIR) $(LIB_FILE)

$(LIB_FILE) : $(BASE_OBJS)
	@echo AR $(LIB_FILE)
	@$(AR) -rv $(LIB_FILE) $(OBJS)
	@ls -l $(BIN_DIR)

$(BASE_OBJS) : %.o : %.c
	@echo CC $<
	@$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(notdir $@) $(INCLUDES)

$(BUILD_DIR) :
	mkdir $(BUILD_DIR)
	
$(OBJ_DIR) :
	mkdir $(OBJ_DIR)
	
$(BIN_DIR) :
	mkdir $(BIN_DIR)
	
.PHONY : clean

clean :
	rm -rf $(BUILD_DIR)

	
