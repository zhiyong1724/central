MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/, %, $(dir $(MAKEFILE_PATH)))
INCLUDES += -I$(CURRENT_DIR)
vpath %.c $(CURRENT_DIR)
SRCS += sys_bitmap_index.c \
	sys_list.c \
	sys_queue.c \
	sys_string.c \
	sys_tree.c \
	sys_vector.c \
	sys_stack.c \
	sys_kmp.c \
	sys_id_manager.c \
	cregex.c