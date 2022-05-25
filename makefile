
CC = gcc
RM = rm

SRC_DIRS := ./src
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')

BUILD_DIR = ./build
LIB = ./lib
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
TARGET_EXEC = Main

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

CXXFLAGS := $(INC_FLAGS)
# hello.o: hello.c
# 	cc -c ./lib/src/hello.c -o hello.o

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	mkdir -p $(dir $@)
	$(CC) $(INC_FLAGS) $(OBJS) -o $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@ 

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)/*
