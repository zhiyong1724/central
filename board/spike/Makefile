BUILD_DIR := build
OBJ_DIR	:= $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
ELF_FILE := $(BIN_DIR)/central.elf
BIN_FILE := $(BIN_DIR)/central.bin
MAP_FILE := $(BIN_DIR)/central.map
DUMP_FILE := $(BIN_DIR)/central.dump

RISCV32=y
SPIKE=y

vpath %.o $(OBJ_DIR)

CC := riscv32-unknown-elf-gcc
CXX := riscv32-unknown-elf-g++
AR := riscv32-unknown-elf-ar
LD := riscv32-unknown-elf-ld
OBJDUMP := riscv32-unknown-elf-objdump
OBJCOPY := riscv32-unknown-elf-objcopy

INCLUDES := -I.

LIBS_DIR := -L.

LIBS := -lc

CFLAGS := -Wall -Werror -g -O3 -DSYS_TIMER_CYCLE_NS=10000000 -DMTIMECMP=20000

ASM_SRCS-y := 

C_SRCS-y := trap_func.c \
	newlibio.c \
	ram_block.c \
	ram_block1.c \
	main.c

include ../../apps/Makefile
include ../../user/Makefile
include ../../core/Makefile
include ../../third/Makefile
include ../../arch/Makefile
include ../../soc/Makefile

ASM_OBJS := $(patsubst %.S, %.o, $(ASM_SRCS-y))
C_OBJS := $(patsubst %.c, %.o, $(C_SRCS-y))
OBJS := $(addprefix $(OBJ_DIR)/,$(ASM_OBJS))
OBJS += $(addprefix $(OBJ_DIR)/,$(C_OBJS))

all : $(BUILD_DIR) $(OBJ_DIR) $(BIN_DIR) $(ELF_FILE) $(DUMP_FILE) $(BIN_FILE)

$(DUMP_FILE) : $(ELF_FILE)
	@echo OBJDUMP $(DUMP_FILE)
	@$(OBJDUMP) -D -S $(ELF_FILE) >$(DUMP_FILE)
	@ls -l $(BIN_DIR)

$(BIN_FILE) : $(ELF_FILE)
	@echo OBJCOPY $(BIN_FILE)
	@$(OBJCOPY) -O binary $(ELF_FILE) $(BIN_FILE)
	@ls -l $(BIN_DIR)

$(ELF_FILE) : $(ASM_OBJS) $(C_OBJS)
	@echo LD $(ELF_FILE)
	@$(CC) -Wl,-Tcentral.lds -Wl,-Map=$(MAP_FILE) $(OBJS) -o $(ELF_FILE) $(LIBS_DIR) $(LIBS)
	@ls -l $(BIN_DIR)

$(ASM_OBJS) : %.o : %.S
	@echo CC $<
	@$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$@ $(INCLUDES)

$(C_OBJS) : %.o : %.c
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

	
