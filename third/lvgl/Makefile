BUILD_DIR := build
OBJ_DIR	:= $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
LIB_FILE := $(BIN_DIR)/liblvgl.a

LVGL_DIR_NAME ?= .
LVGL_DIR ?= ${shell pwd}
include $(LVGL_DIR)/$(LVGL_DIR_NAME)/lvgl.mk

vpath %.o $(OBJ_DIR)

CC := gcc
CXX := g++
AR := ar
LD := ld

INCLUDES := -I. 

LIBS_DIR := -L.

LIBS := -lpthread

CFLAGS := $(CFLAGS) -O3 

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

	
