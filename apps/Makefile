MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/, %, $(dir $(MAKEFILE_PATH)))
INCLUDES += -I$(CURRENT_DIR)
vpath %.c $(CURRENT_DIR)
C_SRCS-y += 
include $(CURRENT_DIR)/resourcemonitor/Makefile \
	$(CURRENT_DIR)/fscommand/Makefile \
	$(CURRENT_DIR)/syscommand/Makefile