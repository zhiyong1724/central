MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/, %, $(dir $(MAKEFILE_PATH)))
INCLUDES += -I$(CURRENT_DIR)
vpath %.S $(CURRENT_DIR)
vpath %.c $(CURRENT_DIR)
ASM_SRCS-$(RISCV32) += start.S \
	cpu.S \
	atomic.S
C_SRCS-$(RISCV32) += backtrace.c