#include "common.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

const char *argv0;

void
die(char *fmt, ...)
{
	fprintf(stderr, "%s: ", argv0);
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}
