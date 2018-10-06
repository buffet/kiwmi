include config.mk

KWLIT    = kiwmi.lit
KWSRC    = kiwmi.c
KWTARGET = kiwmi

SELIT    = seed.lit
SESRC    = seed.c
SETARGET = seed

SRC     = $(KWSRC) $(SESRC)
DEPS    = $(SRC:.c=.d)
TARGETS = $(KWTARGET) $(SETARGET)

VPATH = $(SRCPREFIX)/lit

ifeq ($(VERBOSE), 1)
	HIDE =
else
	HIDE = @
endif

.PHONY: all all-nodoc doc install uninstall clean

all: all-nodoc doc

all-nodoc: $(TARGETS)

$(KWTARGET): $(KWSRC)
	@echo "  [CC]  $@..."
	$(HIDE) $(CC) -o "$@" "$<" $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

$(SETARGET): $(SESRC)
	@echo "  [CC]  $@..."
	$(HIDE) $(CC) -o "$@" "$<" $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

$(KWSRC): $(KWLIT)
	@echo "  [LIT] $@..."
	$(HIDE) $(LIT) -t "$<"

$(SESRC): $(SELIT)
	@echo "  [LIT] $@..."
	$(HIDE) $(LIT) -t "$<"

doc: books

books: seed.html kiwmi.html

kiwmi.html: $(KWLIT)
	@echo "  [LIT] $@..."
	$(HIDE) $(LIT) -w "$<"
	
seed.html: $(SELIT)
	@echo "  [LIT] $@..."
	$(HIDE) $(LIT) -w "$<"
	
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
	@echo "  [RM]  $(SRC)..."
	$(HIDE) $(RM) $(SRC)
	@echo "  [RM]  $(TARGETS)..."
	$(HIDE) $(RM) $(TARGETS)
	@echo "  [RM]  kiwmi.html seed.html..."
	$(HIDE) $(RM) kiwmi.html seed.html

-include $(DEPS)
