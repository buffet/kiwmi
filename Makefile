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

.PHONY: all nodoc doc install uninstall clean

all: $(TARGETS)

$(KWTARGET): $(KWOBJ)
	$(LD) -o $@ $(LDFLAGS) $^

$(SETARGET): $(SEOBJ)
	$(LD) -o $@ $(LDFLAGS) $^

install: all misc/kiwmi.desktop
	install -Dm755 $(KWTARGET) "$(DESTDIR)$(BINPREFIX)/$(KWTARGET)"
	install -Dm755 $(SETARGET) "$(DESTDIR)$(BINPREFIX)/$(SETARGET)"
	install -Dm644 misc/kiwmi.desktop "$(DESTDIR)$(XSESSIONS)/kiwmi.desktop"

uninstall:
	$(RM) "$(DESTDIR)$(BINPREFIX)/$(KWTARGET)"
	$(RM) "$(DESTDIR)$(BINPREFIX)/$(SETARGET)"
	$(RM) "$(DESTDIR)$(XSESSIONS)/kiwmi.desktop"

clean:
	$(RM) $(DEPS)
	$(RM) $(OBJ)
	$(RM) $(TARGETS)

-include $(DEPS)
