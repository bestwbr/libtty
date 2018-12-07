libtty
======

The library of tty which make uart programming simple.

--------------------------------------------------------

simtty.py
----------
The simtty.py is used to simulate two virtual ttys for testing the library.

usage
------

.. code-block:: c

    #include <stdio.h>
    #include <string.h>
    #include <unistd.h>

    #include "tty.h"

    int main(int argc, char const* argv[])
    {
    	int i;
    	int len;
    	char rcvbuf[128] = {0};
    	char *sendbuf = "hello world, from orangepi!";
    	tty_t *tty;
    	tty_attrs_t attrs;

    	if (argc < 3) {
    		printf("%s [-r|-s] <tty path>\n", argv[0]);
    		goto err;
    	}

    	attrs.speed     = B115200;
    	attrs.flow_ctrl = 0;
    	attrs.databits  = 8;
    	attrs.stopbits  = 1;
    	attrs.parity    = 'N';

    	tty = tty_open(argv[2], attrs);
    	if (!tty) {
    		printf("tty_open failed\n");
    		tty_close(tty);
    		goto err;
    	}

    	if (!strcmp(argv[1], "-s")) {
    		for(i = 0; i < 10; i++) {
    			len = tty_send(tty, sendbuf, strlen(sendbuf));
    			if (len > 0)
    				printf(" %d time send %d data successful\n", i, len);
    			else
    				printf("send data failed!\n");

    			sleep(1);
    		}
    	} else {
    		for(i = 0; i < 1; i++) {
    			len = tty_recv(tty, rcvbuf, 127, 10);
    			if(len > 0) {
    				rcvbuf[len] = '\0';
    				printf("receive data is %s\n", rcvbuf);
    				printf("len = %d\n", len);
    			} else {
    				printf("cannot receive data\n");
    			}
    		}
    	}

    	tty_close(tty);
    	return 0;
    err:
    	return -1;
    }
