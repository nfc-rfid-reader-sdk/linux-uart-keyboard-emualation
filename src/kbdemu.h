
#define DEFAULT_PORT 		"/dev/ttyUSB0"
#define DEFAULT_BAUD_RATE	1000000
#define APP_VERSION			"1.1"

int sw_open_serial(const char *port, int baud_rate);
void sw_init();
void sw_read_loop();
void PortSetRTS(int fd, int level);
void PortSetBaudRate(int fd, int br);
int get_linux_baudrate(int speed);
