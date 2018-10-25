VERSION = NaV

PREFIX    = /usr/local
BINPREFIX = $(PREFIX)/bin
MANPREFIX = $(PREFIX)/share/man
XSESSIONS = $(PREFIX)/share/xsessions

CC = gcc
LD = $(CC)

INCS = -Isrc/
LIBS = `pkg-config --libs xcb`

DEFS = -D_POSIX_C_SOURCE=2 -DVERSION_STRING=\"$(VERSION)\"

CFLAGS   += -std=c11 -Wall -Wextra -pedantic $(DEFS) $(INCS)
LDFLAGS  += $(LIBS)
CPPFLAGS += -MD -MP
