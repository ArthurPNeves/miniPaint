SHELL := /bin/bash
PROJECT := $(notdir $(CURDIR))
BUILD_DIR := build
BIN_DIR := bin

# find sources (exclude build/bin/.git)
SRCS := $(shell find . -type f \( -name '*.cpp' -o -name '*.c' \) -not -path './$(BUILD_DIR)/*' -not -path './$(BIN_DIR)/*' -not -path './.git/*')

# map sources to build/*.o (preserve directories)
OBJS := $(patsubst ./%,$(BUILD_DIR)/%,$(SRCS))
OBJS := $(patsubst %.cpp,%.o,$(OBJS))
OBJS := $(patsubst %.c,%.o,$(OBJS))

CXX := g++
CC := gcc
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Iexternal/json/include
LDFLAGS :=
LDLIBS :=

TARGET := $(BIN_DIR)/$(PROJECT)

.PHONY: all clean debug release run print-vars

all: $(TARGET)

# link
$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# compile C++ source -> build path
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# compile C source -> build path
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CXXFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

run: $(TARGET)
	./$(TARGET)

debug: CXXFLAGS += -g -O0
debug: all

release: CXXFLAGS += -O3
release: all

print-vars:
	@echo "PROJECT=$(PROJECT)"
	@echo "SRCS=$(SRCS)"
	@echo "OBJS=$(OBJS)"
	@echo "TARGET=$(TARGET)"
