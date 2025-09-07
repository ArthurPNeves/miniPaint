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
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Ilibs
LDFLAGS :=
LDLIBS :=

TARGET := $(BIN_DIR)/$(PROJECT)

.PHONY: all clean debug release run print-vars windows windows-simple linux

# Linux build (default)
all: linux

linux: $(TARGET)

# Cross-compilation para Windows (versão completa com servidor)
windows: CXX := x86_64-w64-mingw32-g++
windows: CC := x86_64-w64-mingw32-gcc
windows: TARGET := $(BIN_DIR)/$(PROJECT).exe
windows: LDFLAGS += -static-libgcc -static-libstdc++ -static
windows: CXXFLAGS += -D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00 -pthread
windows: LDLIBS += -mconsole -lws2_32 -lwsock32 -lpthread -lmingwex -lmsvcrt -lgcc_eh -lkernel32 -luser32 -lgdi32
windows: $(BIN_DIR)/$(PROJECT).exe

# Cross-compilation para Windows (versão simples sem servidor)
windows-simple: CXX := x86_64-w64-mingw32-g++
windows-simple: CC := x86_64-w64-mingw32-gcc
windows-simple: TARGET := $(BIN_DIR)/$(PROJECT)-simple.exe
windows-simple: LDFLAGS += -static-libgcc -static-libstdc++ -Wl,--subsystem,console
windows-simple: LDLIBS += 
windows-simple: SIMPLE_SRCS := algorithms.cpp transformations.cpp main_simple.cpp
windows-simple: SIMPLE_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SIMPLE_SRCS))
windows-simple: $(BIN_DIR)/$(PROJECT)-simple.exe

# link
$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_DIR)/$(PROJECT).exe: $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_DIR)/$(PROJECT)-simple.exe: $(BUILD_DIR)/algorithms.o $(BUILD_DIR)/transformations.o $(BUILD_DIR)/main_simple.o
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