MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/, %, $(dir $(MAKEFILE_PATH)))
INCLUDES += -I$(CURRENT_DIR) \
	-Iquickjs/include
LIBS_DIR += -Lquickjs/lib
LIBS += -lquickjs.lto -lm
vpath %.c $(CURRENT_DIR)
SRCS += jsinterpreter.c \
	jslibc.c