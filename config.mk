VERBOSE = 0

PREFIX    = /usr/local
BINPREFIX = $(PREFIX)/bin
MANPREFIX = $(PREFIX)/share/man
XSESSIONS = $(PREFIX)/share/xsessions

SRCPREFIX = .

CC  = gcc
LIT = lit

CFLAGS   = -std=c99 -Wall -Wextra -pedantic -Os -D_POSIX_C_SOURCE=2
LDFLAGS  =
CPPFLAGS = -MD -MP
