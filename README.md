Comand line tool usage:
linux-uart-keyboard-emulation -c uart_port_name -s baud_rate

Example:
linux-uart-keyboard-emulation -c /dev/ttyUSB0 -s 230400

If you omit -c parameter, app use default uart port which is /dev/ttyUSB0.
If you omit -s parameter, app use default baud rate which is 1000000.

To start app without "daemonizing" use:
linux-uart-keyboard-emulation -f

which can be combined for -c and -s parameters



To get app version use:
linux-uart-keyboard-emulation -v

(Fixing log error:
 stated:
  Version 1.1
  +Linux executables
 should be:
  Version 1.2
  +Linux executables
)
