
#############################################################################
# QVM BUILD TOOLS
#############################################################################

ifndef TOOLS_CC
# A compiler which probably produces native binaries
TOOLS_CC = $(CC)
endif

ifndef YACC
YACC = yacc
endif

B=.
Q3ASMDIR=asm
LBURGDIR=lcc/lburg
Q3CPPDIR=lcc/cpp
Q3LCCETCDIR=lcc/etc
Q3LCCSRCDIR=lcc/src

TOOLS_OPTIMIZE = -O3 -Wall -fno-strict-aliasing
TOOLS_CFLAGS += $(TOOLS_OPTIMIZE) \
                -DTEMPDIR=\"$(TEMPDIR)\" -DSYSTEM=\"\" \
                -I$(Q3LCCSRCDIR) \
                -I$(LBURGDIR)
TOOLS_LIBS =
TOOLS_LDFLAGS =

ifeq ($(GENERATE_DEPENDENCIES),1)
  TOOLS_CFLAGS += -MMD
endif

define DO_YACC
$(YACC) $<
mv -f y.tab.c $@
endef

define DO_TOOLS_CC
$(TOOLS_CC) $(TOOLS_CFLAGS) -o $@ -c $<
endef

define DO_TOOLS_CC_DAGCHECK
$(TOOLS_CC) $(TOOLS_CFLAGS) -I${Q3LCCSRCDIR} -Wno-unused -o $@ -c $<
endef

LBURG       = $(B)/lcc/lburg/lburg
DAGCHECK_C  = $(B)/dagcheck.c
Q3RCC       = $(B)/q3rcc
Q3CPP       = $(B)/q3cpp
Q3LCC       = $(B)/q3lcc
Q3ASM       = $(B)/q3asm

LBURGOBJ= \
  $(B)/lcc/lburg/lburg.o \
  $(B)/lcc/lburg/gram.o

# override GNU Make built-in rule for converting gram.y to gram.c
%.c: %.y
ifeq ($(USE_YACC),1)
	$(DO_YACC)
endif

$(B)/lburg/%.o: $(LBURGDIR)/%.c
	$(DO_TOOLS_CC)

$(LBURG): $(LBURGOBJ)
	$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

Q3RCCOBJ = \
  $(B)/lcc/src/alloc.o \
  $(B)/lcc/src/bind.o \
  $(B)/lcc/src/bytecode.o \
  $(B)/lcc/src/dag.o \
  $(B)/lcc/src/dagcheck.o \
  $(B)/lcc/src/decl.o \
  $(B)/lcc/src/enode.o \
  $(B)/lcc/src/error.o \
  $(B)/lcc/src/event.o \
  $(B)/lcc/src/expr.o \
  $(B)/lcc/src/gen.o \
  $(B)/lcc/src/init.o \
  $(B)/lcc/src/inits.o \
  $(B)/lcc/src/input.o \
  $(B)/lcc/src/lex.o \
  $(B)/lcc/src/list.o \
  $(B)/lcc/src/main.o \
  $(B)/lcc/src/null.o \
  $(B)/lcc/src/output.o \
  $(B)/lcc/src/prof.o \
  $(B)/lcc/src/profio.o \
  $(B)/lcc/src/simp.o \
  $(B)/lcc/src/stmt.o \
  $(B)/lcc/src/string.o \
  $(B)/lcc/src/sym.o \
  $(B)/lcc/src/symbolic.o \
  $(B)/lcc/src/trace.o \
  $(B)/lcc/src/tree.o \
  $(B)/lcc/src/types.o

$(DAGCHECK_C): $(LBURG) $(Q3LCCSRCDIR)/dagcheck.md
	$(LBURG) $(Q3LCCSRCDIR)/dagcheck.md $@

$(B)/lcc/src/dagcheck.o: $(DAGCHECK_C)
	$(DO_TOOLS_CC_DAGCHECK)

$(B)/lcc/src/%.o: $(Q3LCCSRCDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3RCC): $(Q3RCCOBJ)
	$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

Q3CPPOBJ = \
  $(B)/lcc/cpp/cpp.o \
  $(B)/lcc/cpp/lex.o \
  $(B)/lcc/cpp/nlist.o \
  $(B)/lcc/cpp/tokens.o \
  $(B)/lcc/cpp/macro.o \
  $(B)/lcc/cpp/eval.o \
  $(B)/lcc/cpp/include.o \
  $(B)/lcc/cpp/hideset.o \
  $(B)/lcc/cpp/getopt.o \
  $(B)/lcc/cpp/unix.o

$(B)/lcc/cpp/%.o: $(Q3CPPDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3CPP): $(Q3CPPOBJ)
	$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

Q3LCCOBJ = \
	$(B)/lcc/etc/lcc.o \
	$(B)/lcc/etc/bytecode.o

$(B)/etc/%.o: $(Q3LCCETCDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3LCC): $(Q3LCCOBJ) $(Q3RCC) $(Q3CPP)
	$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(Q3LCCOBJ) $(TOOLS_LIBS)

define DO_Q3LCC
$(echo_cmd) "Q3LCC $<"
$(Q3LCC) $(BASEGAME_CFLAGS) -o $@ $<
endef

define DO_CGAME_Q3LCC
$(echo_cmd) "CGAME_Q3LCC $<"
$(Q3LCC) $(BASEGAME_CFLAGS) -DCGAME -o $@ $<
endef

define DO_GAME_Q3LCC
$(echo_cmd) "GAME_Q3LCC $<"
$(Q3LCC) $(BASEGAME_CFLAGS) -DGAME -o $@ $<
endef

define DO_UI_Q3LCC
$(echo_cmd) "UI_Q3LCC $<"
$(Q3LCC) $(BASEGAME_CFLAGS) -DUI -o $@ $<
endef

define DO_CGAME_Q3LCC_11
$(echo_cmd) "CGAME_Q3LCC_11 $<"
$(Q3LCC) $(BASEGAME_CFLAGS) -DMODULE_INTERFACE_11 -DCGAME -o $@ $<
endef

define DO_UI_Q3LCC_11
$(echo_cmd) "UI_Q3LCC_11 $<"
$(Q3LCC) $(BASEGAME_CFLAGS) -DMODULE_INTERFACE_11 -DUI -o $@ $<
endef

Q3ASMOBJ = \
  $(B)/asm/q3asm.o \
  $(B)/asm/cmdlib.o

$(B)/asm/%.o: $(Q3ASMDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3ASM): $(Q3ASMOBJ)
	$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

all: $(LBURG) $(Q3ASM) $(Q3LCC) $(Q3CPP) $(Q3RCC)
