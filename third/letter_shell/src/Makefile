MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/, %, $(dir $(MAKEFILE_PATH)))
INCLUDES += -I$(CURRENT_DIR)
vpath %.c $(CURRENT_DIR)
SRCS += shell.c \
	shell_companion.c \
	shell_ext.c