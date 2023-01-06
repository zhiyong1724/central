MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/, %, $(dir $(MAKEFILE_PATH)))
INCLUDES += -I$(CURRENT_DIR)
vpath %.c $(CURRENT_DIR)
SRCS += 

ifeq ($(PLATFORM),H750)
include $(CURRENT_DIR)/audioplayer/Makefile \
	$(CURRENT_DIR)/test/Makefile 
else
include $(CURRENT_DIR)/test/Makefile
endif