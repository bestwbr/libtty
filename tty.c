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
#include <poll.h>

#include "tty.h"

static int tty_openat(const char *devp)
{
	int fd;

	fd = open(devp, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0) {
		perror("tty_openat, open");
		goto out;
	}

	return fd;

out:
	return -1;
}

static bool tty_set(tty_t *tty)
{
	struct termios options;

	if (tcgetattr(tty->fd, &options)) {
		perror("tty_set, tcgetattr");
		goto out;
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
		goto out;
	}

	return true;
out:
	close(tty->fd);
	return false;
}

tty_t *tty_open(const char *devp, tty_attrs_t attrs)
{
	static tty_t tty;

	tty.fd = tty_openat(devp);
	if (tty.fd == -1) {
		fprintf(stderr, "tty_openat %s failed\n", devp);
		goto out;
	}

	tty.attrs = &attrs;
	if (!tty_set(&tty)) {
		fprintf(stderr, "tty set attributes failed\n");
		goto close_out;
	}

	return &tty;

close_out:
	close(tty.fd);
out:
	return NULL;
}

void tty_close(tty_t *tty)
{
	close(tty->fd);
	tty = NULL;
}

int tty_recv(tty_t *tty, char *buffer, int len, unsigned int timeout)
{
	int rlen;
	struct pollfd fdset[1];
	int nfds = 1;
	int rc;
	fdset[0].fd = tty->fd;
	fdset[0].events = POLLOUT;

	rc = poll(fdset, nfds, timeout);
	if (rc < 0) {
		fprintf(stderr, "poll failed!\n");
		return -1;
	}
	if (rc == 0) {
		fprintf(stderr, "poll timeout\n");
		return 0;
	}

	if (fdset[0].revents & POLLOUT) {
		rlen = read(tty->fd, buffer, len);
		return rlen;
	}
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
