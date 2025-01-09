#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

int main() {
  const char *port = "/dev/serial0";
  int uart0 = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
  if (uart0 == -1) {
    std::cerr << "Failed to open UART." << std::endl;
    return 1;
  }

  struct termios options;
  tcgetattr(uart0, &options);
  options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  tcflush(uart0, TCIFLUSH);
  tcsetattr(uart0, TCSANOW, &options);

  char buffer[256];
  std::string line;

  while (true) {
    int length = read(uart0, buffer, sizeof(buffer) - 1);
    if (length > 0) {
      buffer[length] = '\0';
      line += buffer;

      size_t pos;
      while ((pos = line.find('\n')) != std::string::npos) {
        std::string sentence = line.substr(0, pos + 1);
        line.erase(0, pos + 1);

        // Validate and print the complete sentence
        if (sentence[0] == '$') {
          std::cout << "Received: " << sentence << std::endl;
        }
      }
    }
  }

  close(uart0);
  return 0;
}
