BD_DEBUG=debug-$(PLATFORM)-$(ARCH)
BD_RELEASE=release-$(PLATFORM)-$(ARCH)

ifeq ($(PLATFORM),mingw32)
  BINEXT=.exe
  RELEASE_LDFLAGS=-lwsock32
  DEBUG_LDFLAGS=-lwsock32
  RM=rm -f
  MKDIR=mkdir
else
  BINEXT=
  RELEASE_LDFLAGS=-ltdb
  DEBUG_LDFLAGS=-ltdb
  RM=rm -f
  MKDIR=mkdir
endif

CC=gcc
RELEASE_CFLAGS=-Wall -O2
DEBUG_CFLAGS=-g
OBJECTS= \
	$(BD)/master.o \
	$(BD)/messages.o \
	$(BD)/stats.o \
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

.PHONY: all clean release debug makedirs
