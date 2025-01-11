MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/, %, $(dir $(MAKEFILE_PATH)))
INCLUDES += -I$(CURRENT_DIR)
vpath %.c $(CURRENT_DIR)
SRCS += sys_dt_scheduler.c \
	sys_idle_scheduler.c \
	sys_msg_queue.c \
	sys_lock.c \
	sys_queue_manager.c \
	sys_rt_scheduler.c \
	sys_semaphore.c \
	sys_semaphore_manager.c \
	sys_task.c \
	sys_task_manager.c \
	sys_vscheduler.c 