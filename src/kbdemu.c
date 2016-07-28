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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <X11/keysymdef.h>
#include "kbdemu.h"

static int uart;
static Display *disp;

void PortSetRTS(int fd, int level) {
	int uart_status;

	// GET the State of MODEM bits in Status
	if (ioctl(fd, TIOCMGET, &uart_status) == -1) {
		perror("setRTS(): TIOCMGET");
		return;
	}

	if (level)
		uart_status |= TIOCM_RTS;	// Set the RTS pin
	else
		uart_status &= ~TIOCM_RTS;

	if (ioctl(fd, TIOCMSET, &uart_status) == -1) {
		perror("setRTS(): TIOCMSET");
	}
}

int GetBaudrate(int speed) {
	int baudr;

	switch (speed) {
	case 50:
		baudr = B50;
		break;
	case 75:
		baudr = B75;
		break;
	case 110:
		baudr = B110;
		break;
	case 134:
		baudr = B134;
		break;
	case 150:
		baudr = B150;
		break;
	case 200:
		baudr = B200;
		break;
	case 300:
		baudr = B300;
		break;
	case 600:
		baudr = B600;
		break;
	case 1200:
		baudr = B1200;
		break;
	case 1800:
		baudr = B1800;
		break;
	case 2400:
		baudr = B2400;
		break;
	case 4800:
		baudr = B4800;
		break;
	case 9600:
		baudr = B9600;
		break;
	case 19200:
		baudr = B19200;
		break;
	case 38400:
		baudr = B38400;
		break;
	case 57600:
		baudr = B57600;
		break;
	case 115200:
		baudr = B115200;
		break;
	case 230400:
		baudr = B230400;
		break;

#ifndef __APPLE__
	case 460800:
		baudr = B460800;
		break;
	case 500000:
		baudr = B500000;
		break;
	case 576000:
		baudr = B576000;
		break;
	case 921600:
		baudr = B921600;
		break;
	case 1000000:
		baudr = B1000000;
		break;
	case 1152000:
		baudr = B1152000;
		break;
	case 1500000:
		baudr = B1500000;
		break;
	case 2000000:
		baudr = B2000000;
		break;
	case 2500000:
		baudr = B2500000;
		break;
	case 3000000:
		baudr = B3000000;
		break;
	case 3500000:
		baudr = B3500000;
		break;
	case 4000000:
		baudr = B4000000;
		break;
#endif // __APPLE__

	default:
		fprintf(stderr, "invalid baudrate\n");
		return (0);
		break;
	}

	return baudr;
}

void PortSetBaudRate(int fd, int br) {
	struct termios options;

	tcgetattr(fd, &options);
	cfsetispeed(&options, br);
	cfsetospeed(&options, br);
	tcsetattr(fd, TCSANOW, &options);
}

int OpenUART(const char *port, int baud_rate) {

	uart = open(port, O_RDONLY);
	if (uart < 0) {
		fprintf(stderr, "Can't open UART: %s\n", port);
		exit(-1);
	}

	PortSetRTS(uart, 0);
	PortSetBaudRate(uart, baud_rate);
	usleep(1200000);
	tcflush(uart, TCIFLUSH);
	return 0;
}

void InitDisp() {

	int ver_major = 0;
	int ver_minor = 0;
	int not_care;

	// Opening the display:
	if ((disp = XOpenDisplay(NULL)) == NULL) {
		fprintf(stderr, "Can't open %s. Exiting...\n", XDisplayName(NULL));
		exit(1);
	}

	// Get XTEST version:
	Bool success = XTestQueryExtension(disp, &not_care, &not_care, &ver_major, &ver_minor);
	if (success == False || ver_major < 2
			|| (ver_major <= 2 && ver_minor < 2)) {
		fprintf(stderr, "XTEST not supported. Exiting...\n");
		exit(1);
	}
}

void ReadWorker() {

	char readbuf[2];
	readbuf[1] = 0;
	ssize_t rcv_num;

	while ((rcv_num = read(uart, readbuf, 1)) >= 0) {
		if (rcv_num)
			PressKeys(readbuf);
	}

	// Closing UART:
	close(uart);
}

void XKeyPress(unsigned char ch) {
	unsigned int shiftcode = XKeysymToKeycode(disp, XStringToKeysym("Shift_L"));
	int uppercase = 0;
	int skip = 0;
	char s[2];
	KeyCode scan_code;
	KeySym key_sym = XStringToKeysym(s);

	s[0] = ch;
	s[1] = 0;
	if (key_sym == 0) {
		key_sym = ch;
	}

	if (key_sym == '\n') {
		key_sym = XK_Return;
		skip = 1;
	} else if (key_sym == '\t') {
		key_sym = XK_Tab;
		skip = 1;
	}

	scan_code = XKeysymToKeycode(disp, key_sym);
	if (scan_code == 0) {
		key_sym = ch | 0xff00;
		scan_code = XKeysymToKeycode(disp, key_sym);
	}

	if (!skip) {
		// Is SHIFT needed?
		KeySym *sym;
		int test;
		int i;

		sym = XGetKeyboardMapping(disp, scan_code, 1, &test);
		for (i = 0; i <= test; i++) {
			if (sym[i] == 0)
				break;

			if (i == 0 && sym[i] != ch)
				uppercase = 1;
		}
	}

	if (uppercase)
		XTestFakeKeyEvent(disp, shiftcode, True, 0);

	XTestFakeKeyEvent(disp, scan_code, True, 0);
	XTestFakeKeyEvent(disp, scan_code, False, 0);

	if (uppercase)
		XTestFakeKeyEvent(disp, shiftcode, False, 0);
}

void PressKeys(char* str) {
	int str_len = strlen(str);
	int i = 0;

	for (i = 0; i < str_len; i++) {
		XKeyPress(str[i]);
	}
	XFlush(disp);
}
