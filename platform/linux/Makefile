BUILD_DIR := build
OBJ_DIR	:= $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
ELF_FILE := $(BIN_DIR)/main.elf
MAP_FILE := $(BIN_DIR)/main.map
DUMP_FILE := $(BIN_DIR)/main.dump

vpath %.o $(OBJ_DIR)

CC := gcc
CXX := g++
AR := ar
LD := ld
DUMP := objdump

INCLUDES := -I. \
	-I../../third/lvgl

LIBS_DIR := -L.

LIBS := -lpthread -lrt -lSDL2 -llvgl

CFLAGS := -Wall -Werror -g -Wl,-Map=$(MAP_FILE)

SRCS := diskio.c \
	fatfsadapter.c \
	ffsystem.c \
	lfsadapter.c \
	lvglio.c \
	main.c \
	osport.c \
	ramio.c \
	shellio.c 

include ../../apps/Makefile
include ../../core/Makefile
include ../../third/Makefile

BASE_OBJS := $(patsubst %.c, %.o, $(SRCS))
OBJS := $(addprefix $(OBJ_DIR)/,$(BASE_OBJS))

all : $(BUILD_DIR) $(OBJ_DIR) $(BIN_DIR) $(ELF_FILE)

$(DUMP_FILE) : $(ELF_FILE)
	@echo DUMP $(DUMP_FILE)
	@$(DUMP) -d -S $(ELF_FILE) >$(DUMP_FILE)

$(ELF_FILE) : $(BASE_OBJS)
	@echo LD $(ELF_FILE)
	@$(CC) $(CFLAGS) $(OBJS) -o $(ELF_FILE) $(LIBS_DIR) $(LIBS)
	@ls -l $(BIN_DIR)

$(BASE_OBJS) : %.o : %.c
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

	
