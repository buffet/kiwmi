include config.mk

SRCPREFIX = src

KWSRC   := $(wildcard $(SRCPREFIX)/kiwmi/*.c) $(SRCPREFIX)/common.c
KWOBJ   := $(KWSRC:.c=.o)
KWTARGET = kiwmi

SESRC   := $(wildcard $(SRCPREFIX)/seed/*.c) $(SRCPREFIX)/common.c
SEOBJ   := $(SESRC:.c=.o)
SETARGET = seed

SRC      = $(KWSRC) $(SESRC)
OBJ      = $(KWOBJ) $(SEOBJ)
DEPS     = $(OBJ:.o=.d)
TARGETS  = $(KWTARGET) $(SETARGET)

MANTARGET = kiwmi.1.gz

.PHONY: all all-nodoc doc install uninstall clean

all: all-nodoc doc

all-nodoc: $(TARGETS)

$(KWTARGET): $(KWOBJ)
	$(LD) -o $@ $^ $(LDFLAGS)

$(SETARGET): $(SEOBJ)
	$(LD) -o $@ $^ $(LDFLAGS)

doc: $(MANTARGET)

$(MANTARGET): misc/kiwmi.1
	sed 's/{{VERSION}}/v$(VERSION)/g' $< | gzip >$@

install: all misc/kiwmi.desktop
	install -Dm755 $(KWTARGET) "$(DESTDIR)$(BINPREFIX)/$(KWTARGET)"
	install -Dm755 $(SETARGET) "$(DESTDIR)$(BINPREFIX)/$(SETARGET)"
	install -Dm644 misc/kiwmi.desktop "$(DESTDIR)$(XSESSIONS)/kiwmi.desktop"
	install -Dm644 $(MANTARGET) "$(DESTDIR)$(MANPREFIX)/$(MANPREFIX)"

uninstall:
	$(RM) "$(DESTDIR)$(BINPREFIX)/$(KWTARGET)"
	$(RM) "$(DESTDIR)$(BINPREFIX)/$(SETARGET)"
	$(RM) "$(DESTDIR)$(XSESSIONS)/kiwmi.desktop"
	$(RM) "$(DESTDIR)$(MANPREFIX)/$(MANTARGET)"

clean:
	$(RM) $(DEPS)
	$(RM) $(OBJ)
	$(RM) $(TARGETS)

-include $(DEPS)
