MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/, %, $(dir $(MAKEFILE_PATH)))
INCLUDES += -I$(CURRENT_DIR)
vpath %.c $(CURRENT_DIR)
SRCS += 

include $(CURRENT_DIR)/fatfs/source/Makefile \
	$(CURRENT_DIR)/letter_shell/src/Makefile \
	$(CURRENT_DIR)/littlefs/Makefile \
	$(CURRENT_DIR)/utilities/Makefile