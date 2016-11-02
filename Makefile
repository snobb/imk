TARGET          := imk
CC              ?= cc
BUILD_HOST      := build_host.h
SRC             := $(wildcard *.c)

OS              := $(shell uname -s)

ifeq ($(OS), Linux)
    GRP = root
    SRC += compat/poll_linux.c
else ifeq ($(OS),OpenBSD)
    GRP = wheel
    SRC += compat/poll_bsd.c
else
    $(error Unrecognized OS)
endif

INSTALL         := install
INSTALL_ARGS    := -o root -g $(GRP) -m 755
INSTALL_DIR     := /usr/local/bin/

OBJ             := $(SRC:.c=.o)

INCLUDES        :=
LIBS            :=

CFLAGS          := -Wall $(INCLUDES)
LFLAGS          := $(LIBS)

ifeq ($(CC), $(filter $(CC), clang gcc cc))
    CFLAGS := -std=c99 -pedantic
endif

all: debug

debug: CFLAGS += -g -ggdb -DDEBUG -fdiagnostics-color=auto
debug: LFLAGS += -g
debug: build

release: CFLAGS += -O3
release: clean build
	strip $(TARGET)

build: $(BUILD_HOST) $(TARGET)

$(BUILD_HOST):
	@echo "#define BUILD_HOST \"`hostname`\""      > $(BUILD_HOST)
	@echo "#define BUILD_OS \"`uname`\""          >> $(BUILD_HOST)
	@echo "#define BUILD_PLATFORM \"`uname -m`\"" >> $(BUILD_HOST)
	@echo "#define BUILD_KERNEL \"`uname -r`\""   >> $(BUILD_HOST)

$(TARGET): $(BUILD_HOST) $(OBJ)
	$(CC) $(LFLAGS) -o $@ $(OBJ)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $?

install: release
	$(INSTALL) $(INSTALL_ARGS) $(TARGET) $(INSTALL_DIR)
	@echo "DONE"

clean:
	-rm -f *.core
	-rm -f $(BUILD_HOST)
	-rm -f $(TARGET)
	-rm -f *.o compat/*.o

.PHONY : all debug release build run clean
