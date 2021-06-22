BUILD_DIR := build
OBJ_DIR	:= $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
OUTPUT := $(BIN_DIR)/main.elf

vpath %.c $(FLATFORM_DIR)
vpath %.c ./core
vpath %.c ./core/base
vpath %.c ./core/mem
vpath %.c ./core/task
vpath %.o $(OBJ_DIR)

INCLUDES := -I$(FLATFORM_DIR) \
	-I./core \
	-I./core/base \
	-I./core/mem \
	-I./core/task

CFLAGS := -Wall -Werror $(COMPILE_FLAGS)

SRCS := $(SRCS) \
	ostree.c \
	osstring.c \
	oslist.c \
	osbuddy.c \
	osmempool.c \
	osmemmanager.c \
	osmem.c \
	osvector.c \
	osdtscheduler.c \
	osrtscheduler.c \
	osidlescheduler.c \
	osbitmapindex.c \
	ostidmanager.c \
	osvscheduler.c \
	ostaskmanager.c \
	ostask.c \
	ossemaphoremanager.c \
	ossemaphore.c \
	osmutex.c \
	osqueuemanager.c \
	osqueue.c \
	oscentral.c \
	osport.c \
	main.c

BASE_OBJS := $(patsubst %.c, %.o, $(SRCS))
OBJS := $(addprefix $(OBJ_DIR)/,$(BASE_OBJS))

all : $(BUILD_DIR) $(OBJ_DIR) $(BIN_DIR) $(OUTPUT)

$(OUTPUT) : $(BASE_OBJS)
	@echo LD $(OUTPUT)
	@$(CC) $(CFLAGS) $(OBJS) -o $(OUTPUT) $(LIBS_DIR) $(LIBS)
	@ls -l $(BIN_DIR)

$(BASE_OBJS) : %.o : %.c
	@echo CC $<
	@$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$@ $(INCLUDES)

$(BUILD_DIR) :
	mkdir $(BUILD_DIR)
	
$(OBJ_DIR) :
	mkdir $(OBJ_DIR)
	
$(BIN_DIR) :
	mkdir $(BIN_DIR)
	
.PHONY : clean

clean :
	rm -rf $(BUILD_DIR)

	
