MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/, %, $(dir $(MAKEFILE_PATH)))
vpath %.c $(CURRENT_DIR)
SRCS += syscommand.c \
	shell_cmd_list.c