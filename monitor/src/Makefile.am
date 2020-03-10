##
##  Libmonitor src/Makefile.am.
##
##  Copyright (c) 2019-2020, Rice University.
##  All rights reserved.
##
##  Redistribution and use in source and binary forms, with or without
##  modification, are permitted provided that the following conditions are
##  met:
##
##  * Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer.
##
##  * Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in the
##    documentation and/or other materials provided with the distribution.
##
##  * Neither the name of Rice University (RICE) nor the names of its
##    contributors may be used to endorse or promote products derived from
##    this software without specific prior written permission.
##
##  This software is provided by RICE and contributors "as is" and any
##  express or implied warranties, including, but not limited to, the
##  implied warranties of merchantability and fitness for a particular
##  purpose are disclaimed. In no event shall RICE or contributors be
##  liable for any direct, indirect, incidental, special, exemplary, or
##  consequential damages (including, but not limited to, procurement of
##  substitute goods or services; loss of use, data, or profits; or
##  business interruption) however caused and on any theory of liability,
##  whether in contract, strict liability, or tort (including negligence
##  or otherwise) arising in any way out of the use of this software, even
##  if advised of the possibility of such damage.
##

GOTCHA_IFLAGS = @GOTCHA_IFLAGS@
GOTCHA_LFLAGS = @GOTCHA_LFLAGS@
GOTCHA_LIBDIR = @GOTCHA_LIBDIR@

MONITOR_SCRIPT_FILES = monitor-run monitor-link

MONITOR_SRC_FILES = main.c pthread.c callback.c

bin_SCRIPTS = $(MONITOR_SCRIPT_FILES)

CLEANFILES = $(MONITOR_SCRIPT_FILES)

include_HEADERS = monitor.h

lib_LTLIBRARIES = libmonitor-preload.la libmonitor-pure-preload.la

libmonitor_preload_la_SOURCES = $(MONITOR_SRC_FILES)
libmonitor_preload_la_CPPFLAGS = -DMONITOR_GOTCHA_PRELOAD $(GOTCHA_IFLAGS)
libmonitor_preload_la_LDFLAGS = -ldl $(GOTCHA_LFLAGS) -Wl,-rpath=$(GOTCHA_LIBDIR)

libmonitor_pure_preload_la_SOURCES = $(MONITOR_SRC_FILES)
libmonitor_pure_preload_la_CPPFLAGS = -DMONITOR_PURE_PRELOAD
libmonitor_pure_preload_la_LDFLAGS = -ldl

# Automake won't install a program (.o) into libdir, so we use noinst
# and install it manually.

noinst_PROGRAMS = libmonitor-link.o libmonitor-static.o

libmonitor_link_o_SOURCES = $(MONITOR_SRC_FILES)
libmonitor_link_o_CPPFLAGS = -DMONITOR_GOTCHA_LINK $(GOTCHA_IFLAGS)

libmonitor_static_o_SOURCES = $(MONITOR_SRC_FILES)
libmonitor_static_o_CPPFLAGS = -DMONITOR_STATIC

if MONITOR_COND_USE_DLOPEN
libmonitor_pure_preload_la_SOURCES += dlopen.c
endif

install-exec-hook:
	$(INSTALL) libmonitor-link.o $(DESTDIR)$(libdir)
	$(INSTALL) libmonitor-static.o $(DESTDIR)$(libdir)

