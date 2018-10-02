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
	
install:
	install -d "$(DESTDIR)$(BINPREFIX)"
	install -m 755 "$(KWTARGET)" "$(DESTDIR)$(BINPREFIX)"
	install -m 755 "$(SETARGET)" "$(DESTDIR)$(BINPREFIX)"

uninstall:
	@echo "  [RM]  $(KWTARGET)..."
	$(HIDE) $(RM) "$(DESTDIR)$(BINPREFIX)/$(KWTARGET)"
	@echo "  [RM]  $(SETARGET)..."
	$(HIDE) $(RM) "$(DESTDIR)$(BINPREFIX)/$(SETARGET)""

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
