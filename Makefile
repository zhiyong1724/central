BUILD_DIR := build
OBJ_DIR	:= $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
ELF_FILE := $(BIN_DIR)/main.elf
MAP_FILE := $(BIN_DIR)/main.map
DUMP_FILE := $(BIN_DIR)/main.dump

vpath %.c $(FLATFORM_DIR)
vpath %.c ./core
vpath %.c ./core/base
vpath %.c ./core/mem
vpath %.c ./core/task
vpath %.c ./core/vfs
vpath %.c ./third/fatfs
vpath %.c ./third/fatfs/source
vpath %.c ./third/letter_shell/src
vpath %.o $(OBJ_DIR)

INCLUDES := -I$(FLATFORM_DIR) \
	-I./core \
	-I./core/base \
	-I./core/mem \
	-I./core/task \
	-I./core/vfs \
	-I./third/fatfs \
	-I./third/fatfs/source \
	-I./third/letter_shell/src

CFLAGS := -Wall -Werror -Wl,-Map=$(MAP_FILE) $(COMPILE_FLAGS)

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
	osvfs.c \
	osf.c \
	ff.c \
	ffsystem.c \
	ffunicode.c \
	diskio.c \
	ramio.c \
	fatfsadapter.c \
	shell.c \
	shell_cmd_list.c \
	shell_companion.c \
	shell_ext.c \
	shellio.c \
	main.c

BASE_OBJS := $(patsubst %.c, %.o, $(SRCS))
OBJS := $(addprefix $(OBJ_DIR)/,$(BASE_OBJS))

all : $(BUILD_DIR) $(OBJ_DIR) $(BIN_DIR) $(ELF_FILE) $(DUMP_FILE)

$(DUMP_FILE) : $(ELF_FILE)
	@echo DUMP $(DUMP_FILE)
	@$(DUMP) -d -S $(ELF_FILE) >$(DUMP_FILE)

$(ELF_FILE) : $(BASE_OBJS)
	@echo LD $(ELF_FILE)
	@$(CC) $(CFLAGS) $(OBJS) -o $(ELF_FILE) $(LIBS_DIR) $(LIBS)
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

	
