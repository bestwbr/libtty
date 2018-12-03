CROSS_COMPILE ?=

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
CFLAG = -O2 -fPIC -Wall

all: libtty.a libtty.so
	@echo make successfully!

libtty.a: tty.o
	$(AR) crv $@ $<

libtty.so: libtty.so.0.1
	@ln -s libtty.so.0.1 libtty.so

libtty.so.0.1: tty.o
	$(CC) -shared $< -o $@

tty.o: tty.c
	$(CC) $(CFLAG) -c $<

.PHONY: all clean

clean:
	@rm *.o libtty*
