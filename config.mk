VERBOSE = 0

PREFIX    = /usr/local
BINPREFIX = $(PREFIX)/bin
MANPREFIX = $(PREFIX)/share/man
XSESSIONS = $(PREFIX)/share/xsessions

CC = gcc
LD = $(CC)

INCS = -Isrc/

CFLAGS   = -std=c99 -Wall -Wextra -pedantic -Os -D_POSIX_C_SOURCE=2 $(INCS)
LDFLAGS  = -lxcb
CPPFLAGS = -MD -MP
