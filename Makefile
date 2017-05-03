TARGET          := imk
CC              ?= cc
BUILD_HOST      := build_host.h
OS              := $(shell uname -s)
LUA_VERSION     := 5.2

INSTALL         := install
INSTALL_ARGS    := -o root -g $(GRP) -m 755
INSTALL_DIR     := /usr/local/bin/

CFLAGS          := -Wall

ifeq ($(CC), $(filter $(CC), clang gcc cc musl-gcc))
    CFLAGS      += -std=c99 -pedantic
endif

SRC             := $(wildcard *.c)

ifeq ($(OS), Linux)
    GRP         := root
    CFLAGS      += $(shell pkg-config --cflags lua$(LUA_VERSION))
    LDFLAGS     += $(shell pkg-config --libs lua$(LUA_VERSION))
    SRC         += compat/compat_linux.c
else ifeq ($(OS), $(filter $(OS), NetBSD OpenBSD FreeBSD Darwin))
    GRP         := wheel
    CFLAGS      += $(shell pkg-config --cflags lua-$(LUA_VERSION))
    LDFLAGS     += $(shell pkg-config --libs lua-$(LUA_VERSION))
    SRC         += compat/compat_bsd.c
else
    $(error Unrecognized OS)
endif

OBJ             := $(SRC:.c=.o)

all: debug

debug: CFLAGS += -g -ggdb -DDEBUG
debug: LDFLAGS += -g
debug: build

release: CFLAGS += -O3
release: clean build
	strip $(TARGET)

static: CFLAGS += -static
static: LDFLAGS += -static
static: release

build: $(BUILD_HOST) $(TARGET)

$(BUILD_HOST):
	@echo "#define BUILD_HOST \"`hostname`\""      > $(BUILD_HOST)
	@echo "#define BUILD_OS \"`uname`\""          >> $(BUILD_HOST)
	@echo "#define BUILD_PLATFORM \"`uname -m`\"" >> $(BUILD_HOST)
	@echo "#define BUILD_KERNEL \"`uname -r`\""   >> $(BUILD_HOST)

$(TARGET): $(BUILD_HOST) $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)

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
	-rm -f config.log

.PHONY : all debug release static build install clean
