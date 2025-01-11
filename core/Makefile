MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/, %, $(dir $(MAKEFILE_PATH)))
INCLUDES += -I$(CURRENT_DIR)
vpath %.c $(CURRENT_DIR)
SRCS += central.c 

include $(CURRENT_DIR)/base/Makefile \
	$(CURRENT_DIR)/mem/Makefile \
	$(CURRENT_DIR)/task/Makefile \
	$(CURRENT_DIR)/vfs/Makefile 