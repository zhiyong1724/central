MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/, %, $(dir $(MAKEFILE_PATH)))
INCLUDES += -I$(CURRENT_DIR)
vpath %.c $(CURRENT_DIR)
C_SRCS-y += sys_idle_scheduler.c \
	sys_dt_scheduler.c \
	sys_rt_scheduler.c \
	sys_vscheduler.c \
	sys_cpu_thread.c \
	sys_task_scheduler.c \
	sys_task.c \
	sys_spin_lock.c \
	sys_semaphore.c \
	sys_msg_queue.c \
	sys_mutex.c \
	sys_recursive_mutex.c \
	sys_rwlock.c \
	sys_fifo_scheduler.c \
	sys_tasklet.c \
	sys_timer.c