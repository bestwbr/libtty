/*
 * Copyright (C) 2018  Jason Wang
 */


#ifndef __TTY_H
#define __TTY_H

#include <termios.h>
#include <stdbool.h>

typedef struct tty tty_t;
typedef struct tty_attrs tty_attrs_t;

struct tty_attrs {
	int speed;
	int flow_ctrl;
	int databits;
	int stopbits;
	int parity;
};

struct tty {
	int fd;
	struct tty_attrs *attrs;
};


/* APIs */
tty_t *tty_open(const char *devp, tty_attrs_t attrs);
void tty_close(tty_t *tty);
int tty_recv(tty_t *tty, char *buffer, int len);
int tty_send(tty_t *tty, char *buffer, int len);


#endif
