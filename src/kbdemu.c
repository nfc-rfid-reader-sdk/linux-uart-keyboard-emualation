/*
 ============================================================================
 Project Name: linux-uart-keyboard-emualation
 Module Name : main.c
 Author      : d-logic
 Version     :
 Copyright   :
 Version     : 1.0
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <X11/keysymdef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "kbdemu.h"

static int serialPort;
//static int trash_bytes;
static Display *dpy;

static void xtest_key_press(unsigned char letter) {
	unsigned int shiftcode = XKeysymToKeycode(dpy, XStringToKeysym("Shift_L"));
	int upper = 0;
	int skip_lookup = 0;
	char s[2];
	s[0] = letter;
	s[1] = 0;
	KeySym sym = XStringToKeysym(s);
	KeyCode keycode;

	if (sym == 0) {
		sym = letter;
	}

	if (sym == '\n') {
		sym = XK_Return;
		skip_lookup = 1;
	} else if (sym == '\t') {
		sym = XK_Tab;
		skip_lookup = 1;
	}

	keycode = XKeysymToKeycode(dpy, sym);
	if (keycode == 0) {
		sym = 0xff00 | letter;
		keycode = XKeysymToKeycode(dpy, sym);
	}

  if (!skip_lookup) {
    // Here we try to determine if a keysym
    // needs a modifier key (shift), such as a
    // shifted letter or symbol.
    // The second keysym should be the shifted char
    KeySym *syms;
    int keysyms_per_keycode;
    syms = XGetKeyboardMapping(dpy, keycode, 1, &keysyms_per_keycode);
    int i = 0;
    for (i = 0; i <= keysyms_per_keycode; i++) {
      if (syms[i] == 0)
	break;
      
      if (i == 0 && syms[i] != letter)
	upper = 1;
      
      
    }
  }

  if (upper)
    XTestFakeKeyEvent(dpy, shiftcode, True, 0);	

  
  XTestFakeKeyEvent(dpy, keycode, True, 0);	
  XTestFakeKeyEvent(dpy, keycode, False, 0);

  if (upper)
	  XTestFakeKeyEvent(dpy, shiftcode, False, 0);
}

static void press_keys(char* string) {
  int len = strlen(string);
  int i = 0;
  for (i = 0; i < len; i++) {
    xtest_key_press(string[i]);
  }
  XFlush(dpy);
}

void PortSetRTS(int fd, int level) {
	int uart_status;

	/* GET the State of MODEM bits in Status */
	if (ioctl(fd, TIOCMGET, &uart_status) == -1)
	{
		perror("setRTS(): TIOCMGET");
		return;
	}

	if (level)
		uart_status |= TIOCM_RTS;	// Set the RTS pin
	else
		uart_status &= ~TIOCM_RTS;

	if (ioctl(fd, TIOCMSET, &uart_status) == -1)
	{
		perror("setRTS(): TIOCMSET");
	}
}

int get_linux_baudrate(int speed)
{
	int baudr;

	switch (speed)
	{
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

int sw_open_serial(const char *port, int baud_rate) {

	serialPort = open(port, O_RDONLY);
		if (serialPort < 0) {
		fprintf(stderr, "Can't open serial port: %s\n", port);
		exit(-1);
	}

	PortSetRTS(serialPort, 0);
	PortSetBaudRate(serialPort, baud_rate);
	usleep(1200000);
	tcflush(serialPort, TCIFLUSH);
	return 0;
}

void sw_init() {

  int xtest_major_version = 0;
  int xtest_minor_version = 0;
  int dummy;

  
  /*
   * Open the display using the $DISPLAY environment variable to locate
   * the X server.  See Section 2.1.
   */
  if ((dpy = XOpenDisplay(NULL)) == NULL) {
    fprintf(stderr, "%s: can't open %s\en", "softwedge", XDisplayName(NULL));
    exit(1);
  }
  
  Bool success = XTestQueryExtension(dpy, &dummy, &dummy,
				     &xtest_major_version, &xtest_minor_version);
  if(success == False || xtest_major_version < 2 ||
     (xtest_major_version <= 2 && xtest_minor_version < 2))
    {
      fprintf(stderr,"XTEST extension not supported. Can't continue\n");
      exit(1);
    }
}


void sw_read_loop() {

	char readbuf[2];
	readbuf[1] = 0;
	int rcv_num;

	while((rcv_num = read(serialPort, readbuf, 1)) >= 0) {
		if (rcv_num)
			press_keys(readbuf);
	}


  // We're done now
  close(serialPort);
}
