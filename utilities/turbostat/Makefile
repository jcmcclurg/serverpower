CC		= $(CROSS_COMPILE)gcc
BUILD_OUTPUT	:= $(CURDIR)
PREFIX		?= /usr
DESTDIR		?=

ifeq ("$(origin O)", "command line")
	BUILD_OUTPUT := $(O)
endif

turbostat_josiah : turbostat_josiah.c
CFLAGS +=	-Wall
CFLAGS +=	-DMSRHEADER='"./msr-index.h"'

%: %.c
	@mkdir -p $(BUILD_OUTPUT)
	$(CC) $(CFLAGS) $< -o $(BUILD_OUTPUT)/$@

.PHONY : clean
clean :
	@rm -f $(BUILD_OUTPUT)/turbostat_josiah

#install : turbostat
#	install -d  $(DESTDIR)$(PREFIX)/bin
#	install $(BUILD_OUTPUT)/turbostat $(DESTDIR)$(PREFIX)/bin/turbostat
#	install -d  $(DESTDIR)$(PREFIX)/share/man/man8
#	install turbostat.8 $(DESTDIR)$(PREFIX)/share/man/man8
