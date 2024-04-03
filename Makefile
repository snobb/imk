TARGET          := imk
CC              ?= cc
BUILD_HOST      := build_host.h
SRC             != (ls *.c compat/poll_bsd.c || true)
OBJ             := $(SRC:.c=.o)
INSTALL         := install
INSTALL_ARGS    := -o root -g wheel -m 755
INSTALL_DIR     := /usr/local/bin/

INCLUDES        :=
LIBS            :=
CFLAGS          := -Wall $(INCLUDES)
LFLAGS          := $(LIBS)

.if $(CC) == cc || $(CC) == clang || $(CC) == gcc
    CFLAGS := -std=c99 -pedantic
.endif

.if make(release) || make(install)
    CFLAGS += -O3
.elif make(static)
    CFLAGS += -static
    LFLAGS += -static
.else # debug
    CFLAGS += -g -ggdb -DDEBUG
    LFLAGS += -g
.endif

# version info from git
REVCNT         != (git rev-list --count main 2>/dev/null)
.if !defined(REVCNT)
    VERSION := devel
.else
    REVHASH != (git rev-parse --short HEAD 2>/dev/null)
    ISCLEAN != (git diff-index --quiet HEAD || echo " [devel]" 2>/dev/null)
    VERSION := "$(REVCNT).$(REVHASH)$(ISCLEAN)"
.endif

all: debug
debug: build
release: clean build
	strip $(TARGET)

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

install:
	$(INSTALL) $(INSTALL_ARGS) $(TARGET) $(INSTALL_DIR)
	@echo "DONE"

clean:
	-rm -f *.core
	-rm -f $(BUILD_HOST)
	-rm -f $(TARGET)
	-rm -f *.o compat/*.o

.PHONY : all debug release static build install clean
