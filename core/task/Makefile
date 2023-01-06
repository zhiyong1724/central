MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/, %, $(dir $(MAKEFILE_PATH)))
INCLUDES += -I$(CURRENT_DIR)
vpath %.c $(CURRENT_DIR)
SRCS += osdtscheduler.c \
	osidlescheduler.c \
	osmsgqueue.c \
	osmutex.c \
	osqueuemanager.c \
	osrtscheduler.c \
	ossemaphore.c \
	ossemaphoremanager.c \
	ostask.c \
	ostaskmanager.c \
	ostidmanager.c \
	osvscheduler.c 