
#define DEFAULT_PORT 		"/dev/ttyUSB0"
#define DEFAULT_BAUD_RATE	1000000
#define APP_VERSION			"1.4"

int OpenUART(const char *port, int baud_rate);
void InitDisp();
void ReadWorker();
void PortSetRTS(int fd, int level);
void PortSetBaudRate(int fd, int br);
int GetBaudrate(int speed);
void XKeyPress(unsigned char letter);
void PressKeys(char* string);
