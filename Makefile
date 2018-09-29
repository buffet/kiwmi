include config.mk

WFLIT    = wmaffle.lit
WFSRC    = wmaffle.c
WFTARGET = wmaffle

SYLIT    = syrup.lit
SYSRC    = syrup.c
SYTARGET = syrup

SRC     = $(WFSRC) $(SYSRC)
DEPS    = $(SRC:.c=.d)
TARGETS = $(WFTARGET) $(SYTARGET)

VPATH = $(SRCPREFIX)/lit

ifeq ($(VERBOSE), 1)
	HIDE =
else
	HIDE = @
endif

.PHONY: all all-nodoc doc install uninstall clean

all: all-nodoc doc

all-nodoc: $(TARGETS)

$(WFTARGET): $(WFSRC)
	@echo "  [CC]  $@..."
	$(HIDE) $(CC) -o "$@" "$<" $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

$(SYTARGET): $(SYSRC)
	@echo "  [CC]  $@..."
	$(HIDE) $(CC) -o "$@" "$<" $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

$(WFSRC): $(WFLIT)
	@echo "  [LIT] $@..."
	$(HIDE) $(LIT) -t "$<"

$(SYSRC): $(SYLIT)
	@echo "  [LIT] $@..."
	$(HIDE) $(LIT) -t "$<"

doc: books

books: syrup.html wmaffle.html

wmaffle.html: $(WFLIT)
	@echo "  [LIT] $@..."
	$(HIDE) $(LIT) -w "$<"
	
syrup.html: $(SYLIT)
	@echo "  [LIT] $@..."
	$(HIDE) $(LIT) -w "$<"
	
install:
	install -d "$(DESTDIR)$(BINPREFIX)"
	install -m 755 "$(WFTARGET)" "$(DESTDIR)$(BINPREFIX)"
	install -m 755 "$(SYTARGET)" "$(DESTDIR)$(BINPREFIX)"

uninstall:
	@echo "  [RM]  $(WFTARGET)..."
	$(HIDE) $(RM) "$(DESTDIR)$(BINPREFIX)/$(WFTARGET)"
	@echo "  [RM]  $(SYTARGET)..."
	$(HIDE) $(RM) "$(DESTDIR)$(BINPREFIX)/$(SYTARGET)""

clean:
	@echo "  [RM]  $(DEPS)..."
	$(HIDE) $(RM) $(DEPS)
	@echo "  [RM]  $(SRC)..."
	$(HIDE) $(RM) $(SRC)
	@echo "  [RM]  $(TARGETS)..."
	$(HIDE) $(RM) $(TARGETS)
	@echo "  [RM]  wmaffle.html syrup.html..."
	$(HIDE) $(RM) wmaffle.html syrup.html

-include $(DEPS)
