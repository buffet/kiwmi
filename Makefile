include config.mk

SRCPREFIX = src

KWSRC   := $(wildcard $(SRCPREFIX)/kiwmi/*.c) $(SRCPREFIX)/common.c
KWOBJ   := $(KWSRC:.c=.o)
KWTARGET = kiwmi

SESRC   := $(wildcard $(SRCPREFIX)/seed/*.c) $(SRCPREFIX)/common.c
SEOBJ   := $(SESRC:.c=.o)
SETARGET = seed

SRC     = $(KWSRC) $(SESRC)
OBJ     = $(KWOBJ) $(SEOBJ)
DEPS    = $(OBJ:.o=.d)
TARGETS = $(KWTARGET) $(SETARGET)

ifeq ($(VERBOSE), 1)
	HIDE =
else
	HIDE = @
endif

.PHONY: all all-nodoc doc install uninstall clean

all: all-nodoc doc

all-nodoc: $(TARGETS)

$(KWTARGET): $(KWOBJ)
	@echo "  [LD]  $@..."
	$(HIDE) $(LD) -o "$@" $^ $(LDFLAGS)

$(SETARGET): $(SEOBJ)
	@echo "  [LD]  $@..."
	$(HIDE) $(LD) -o "$@" $^ $(LDFLAGS)

.c.o:
	@echo "  [CC]  $@..."
	$(HIDE) $(CC) -o "$@" "$<" -c $(CFLAGS) $(CPPFLAGS)

doc:

install: all misc/kiwmi.desktop
	install -Dm755 "$(KWTARGET)" "$(DESTDIR)$(BINPREFIX)/$(KWTARGET)"
	install -Dm755 "$(SETARGET)" "$(DESTDIR)$(BINPREFIX)/$(SETARGET)"
	install -Dm644 misc/kiwmi.desktop "$(DESTDIR)$(XSESSIONS)/kiwmi.desktop"

uninstall:
	@echo "  [RM]  $(KWTARGET)..."
	$(HIDE) $(RM) "$(DESTDIR)$(BINPREFIX)/$(KWTARGET)"
	@echo "  [RM]  $(SETARGET)..."
	$(HIDE) $(RM) "$(DESTDIR)$(BINPREFIX)/$(SETARGET)"
	@echo "  [RM]  kiwmi.desktop..."
	$(HIDE) $(RM) "$(DESTDIR)$(XSESSIONS)/kiwmi.desktop"

clean:
	@echo "  [RM]  $(DEPS)..."
	$(HIDE) $(RM) $(DEPS)
	@echo "  [RM]  $(OBJ)..."
	$(HIDE) $(RM) $(OBJ)
	@echo "  [RM]  $(TARGETS)..."
	$(HIDE) $(RM) $(TARGETS)

-include $(DEPS)
