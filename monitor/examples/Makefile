#
#  Example libmonitor clients.
#
#  Copyright (c) 2019-2020, Rice University.
#  See LICENSE for details.
#

CC = gcc
CFLAGS = -g -O

LIBS = libreal.so real.o

INCL = -I../src

all: $(LIBS)

libreal.so: realtime.c
	$(CC) $(CFLAGS) -fPIC -shared $(INCL) $< -o $@ -lrt -lpthread

real.o: realtime.c
	$(CC) -c $(CFLAGS) $(INCL) $< -o $@

clean:
	rm -f $(LIBS) *.o *.so

