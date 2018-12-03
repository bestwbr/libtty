/*
 * Copyright (C) 2018  Jason Wang
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "tty.h"

#define dprintf(fmt, ...)

static int tty_openat(const char *devp)
{
	int fd;

	fd = open(devp, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0) {
		perror("tty_openat, open");
		goto err0;
	}

	return fd;

err0:
	return -1;
}

static bool tty_set(tty_t *tty)
{
	struct termios options;

	if (tcgetattr(tty->fd, &options)) {
		perror("tty_set, tcgetattr");
		goto err0;
	}

	cfsetispeed(&options, tty->attrs->speed);
	cfsetospeed(&options, tty->attrs->speed);

	options.c_cflag |= CLOCAL;
	options.c_cflag |= CREAD;
	switch (tty->attrs->flow_ctrl) {
	case 0:
		options.c_cflag &= ~CRTSCTS;
		break;
	case 1:
		options.c_cflag |= CRTSCTS;
		break;
	case 2:
		options.c_cflag |= IXON | IXOFF | IXANY;
		break;
	default:
		options.c_cflag &= ~CRTSCTS;
	}
	options.c_cflag &= ~CSIZE;
	switch (tty->attrs->databits) {
	case 5:
		options.c_cflag |= CS5;
		break;
	case 6:
		options.c_cflag |= CS6;
		break;
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		options.c_cflag |= CS8;
	}
	switch (tty->attrs->parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;
		break;
	case 's':
	case 'S':
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
	}
	switch (tty->attrs->stopbits) {
	case 1:
		options.c_cflag &= ~CSTOPB; break;
	case 2:
		options.c_cflag |= CSTOPB; break;
	default:
		options.c_cflag &= ~CSTOPB;
	}
	options.c_oflag &= ~OPOST;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_cc[VTIME] = 1;
	options.c_cc[VMIN]  = 1;
	tcflush(tty->fd, TCIFLUSH);
	if (tcsetattr(tty->fd, TCSANOW, &options)) {
		perror("tty_set, tcsetattr");
		goto err0;
	}

	return true;
err0:
	close(tty->fd);
	return false;
}

tty_t *tty_open(const char *devp, tty_attrs_t attrs)
{
	tty_t *tty;

	tty = malloc(sizeof (*tty));
	if (!tty) {
		fprintf(stderr, "malloc tty structure failed\n");
		goto err0;
	}
	memset(tty, 0, sizeof(*tty));
	/* debug */
	dprintf("openat %s\n", devp);
	tty->fd = tty_openat(devp);
	if (tty->fd == -1) {
		fprintf(stderr, "tty_openat %s failed\n", devp);
		goto err1;
	}

	tty->attrs = &attrs;
	/* debug */
	dprintf("attrs: %d, %d, %d, %d, %c\n", tty->attrs->speed, tty->attrs->flow_ctrl, tty->attrs->databits, tty->attrs->stopbits, tty->attrs->parity);
	if (!tty_set(tty)) {
		fprintf(stderr, "tty set attributes failed\n");
		goto err2;
	}

	return tty;

err2:
	close(tty->fd);
err1:
	free(tty);
	tty = NULL;
err0:
	return NULL;
}

void tty_close(tty_t *tty)
{
	close(tty->fd);
	free(tty);
	tty = NULL;
}

int tty_recv(tty_t *tty, char *buffer, int len)
{
	int rlen,fs_sel;
	fd_set fs_read;
	struct timeval time;
	FD_ZERO(&fs_read);
	FD_SET(tty->fd, &fs_read);
	time.tv_sec = 10;
	time.tv_usec = 0;

	fs_sel = select(tty->fd+1, &fs_read, NULL, NULL, &time);
	if(fs_sel) {
		rlen = read(tty->fd, buffer, len);
		return rlen;
	}

	fprintf(stderr, "receive data failed\n");
	return -1;
}

int tty_send(tty_t *tty, char *buffer, int len)
{
	int rlen = 0;

	rlen = write(tty->fd, buffer, len);
	if (rlen == len) {
		return len;
	} else {
		tcflush(tty->fd, TCOFLUSH);
		fprintf(stderr, "send data failed\n");
		return -1;
	}
}
