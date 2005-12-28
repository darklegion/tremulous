ARCH=$(shell uname -m | sed -e s/i.86/i386/)
PLATFORM=$(shell uname|sed -e s/_.*//|tr A-Z a-z)
BD_DEBUG=debug-$(ARCH)$(PLATFORM)
BD_RELEASE=release-$(ARCH)$(PLATFORM)

ifeq ($(PLATFORM),mingw32)
  BINEXT=.exe
	RELEASE_LDFLAGS=-lwsock32
	DEBUG_LDFLAGS=-lwsock32
	RM=del
	MKDIR=mkdir
else
  BINEXT=
	RELEASE_LDFLAGS=
	DEBUG_LDFLAGS=
	RM=rm -f
	MKDIR=mkdir
endif

CC=gcc
RELEASE_CFLAGS=-Wall -O2
DEBUG_CFLAGS=-g
OBJECTS= \
	$(BD)/master.o \
	$(BD)/messages.o \
	$(BD)/servers.o

release: makedirs
	$(MAKE) $(BD_RELEASE)/tremmaster BD=$(BD_RELEASE) \
	CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)"

debug: makedirs
	$(MAKE) $(BD_DEBUG)/tremmaster BD=$(BD_DEBUG) \
	CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)"

all: release debug

$(BD)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(BD)/tremmaster: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	-$(RM) $(BD_DEBUG)/*
	-$(RM) $(BD_RELEASE)/*

makedirs:
	@if [ ! -d $(BD_RELEASE) ];then $(MKDIR) $(BD_RELEASE);fi
	@if [ ! -d $(BD_DEBUG) ];then $(MKDIR) $(BD_DEBUG);fi

.PHONY: all clean
