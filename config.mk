VERSION = NaV

PREFIX    = /usr/local
BINPREFIX = $(PREFIX)/bin
MANPREFIX = $(PREFIX)/share/man
XSESSIONS = $(PREFIX)/share/xsessions

CC = gcc
LD = $(CC)

INCS = -Isrc/

DEFS = -D_POSIX_C_SOURCE=2 -DVERSION_STRING=\"$(VERSION)\"

CFLAGS   = -std=c11 -Wall -Wextra -pedantic -Os $(DEFS) $(INCS)
LDFLAGS  = -lxcb
CPPFLAGS = -MD -MP
