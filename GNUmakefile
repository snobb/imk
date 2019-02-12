TARGET          := imk
CC              ?= cc
BUILD_HOST      := build_host.h
SRC             := $(wildcard *.c)
OS              := $(shell uname -s)

ifeq ($(OS), Linux)
    GRP    := root
    SRC    += compat/poll_linux.c
    CFLAGS += -D_DEFAULT_SOURCE
else ifeq ($(OS), $(filter $(OS), NetBSD OpenBSD FreeBSD Darwin))
    GRP    := wheel
    SRC    += compat/poll_bsd.c
else
    $(error Unrecognized OS)
endif

INSTALL         := install
INSTALL_ARGS    := -o root -g $(GRP) -m 755
INSTALL_DIR     := /usr/local/bin/

OBJ             := $(SRC:.c=.o)

INCLUDES        :=
LIBS            :=

CFLAGS          += -Werror -Wall $(INCLUDES)
LFLAGS          += $(LIBS)

ifeq ($(CC), $(filter $(CC), clang gcc cc musl-gcc))
    CFLAGS += -std=c99 -pedantic
endif

# version info from git
REVCNT         := $(shell git rev-list --count master 2>/dev/null)
ifeq ($(REVCNT),)
	VERSION     := devel
else
	REVHASH     := $(shell git rev-parse --short HEAD 2>/dev/null)
	ISCLEAN     := $(shell git diff-index --quiet HEAD || echo " [devel]" 2>/dev/null)
	VERSION     := "$(REVCNT).$(REVHASH)$(ISCLEAN)"
endif

all: debug

debug: CFLAGS += -g -ggdb -DDEBUG
debug: LFLAGS += -g
debug: build

release: CFLAGS += -O3
release: clean build
	strip $(TARGET)

static: CFLAGS += -static
static: LFLAGS += -static
static: release

build: $(BUILD_HOST) $(TARGET)

$(BUILD_HOST):
	@echo "#define BUILD_HOST \"`hostname`\""      > $(BUILD_HOST)
	@echo "#define BUILD_OS \"`uname`\""          >> $(BUILD_HOST)
	@echo "#define BUILD_PLATFORM \"`uname -m`\"" >> $(BUILD_HOST)
	@echo "#define BUILD_KERNEL \"`uname -r`\""   >> $(BUILD_HOST)
	@echo "#define VERSION \"$(VERSION)\""        >> $(BUILD_HOST)

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

.PHONY : all debug release static build install clean
