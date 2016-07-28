/*
 ============================================================================
 Project Name: linux-uart-keyboard-emualation
 Module Name : main.c
 Author      : d-logic
 Version     :
 Copyright   :
 Version     : 1.4
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <X11/keysymdef.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "kbdemu.h"

int main(int argc, char**argv)
{
	int c;
	int dontDaemon = 0;
	char *sport = NULL;
	int baud_rate;

	baud_rate = GetBaudrate(DEFAULT_BAUD_RATE);

	while ((c = getopt(argc, argv, "fvs:c:")) != -1) {
		switch (c) {
			case 'f':
				fprintf(stderr, "Start without daemonizing...\n");
				dontDaemon = 1;
				break;
			case 'v':
				fprintf(stderr, "Application version - %s: linux-uart-keyboard-emualation for X11.\n"
						"uFR device must be connected using FTDI usb to serial driver (add user to dialout group).\n"
						"uFR device must be in asynchronous UID mode.\n", APP_VERSION);
				fprintf(stderr, "Exiting...\n");
				exit(0);
			case 's':
				baud_rate = GetBaudrate(atoi(optarg));
				if (!baud_rate) {
					fprintf (stderr, "Using default baud rate (%d bps).\n", DEFAULT_BAUD_RATE);
					baud_rate = GetBaudrate(DEFAULT_BAUD_RATE);
				}
				break;
			case 'c':
				sport = optarg;
				break;
			case '?':
				if (optopt == 'c' || optopt == 's')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
				return 1;
			default:
				abort ();
		}
	}

	if (sport == NULL)
		sport = DEFAULT_PORT;

	InitDisp();
	OpenUART(sport, baud_rate);

	if (!dontDaemon) {
		if(fork()) {
			return 0;
		}
    
		close(0);
		close(1);
		close(2);
	}

	ReadWorker();

	return 0;
}
