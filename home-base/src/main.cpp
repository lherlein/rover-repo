#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <termios.h>  // For capturing key press without Enter key
#include <nlohmann/json.hpp>

#define SERVER_IP "192.168.2.118"
#define PORT 5000
#define BUFFER_SIZE 1024
int received_heartbeat_count = 0;

// Set up heartbeat messages
nlohmann::json formHeartbeatMessage(int uid) { // preshim crafting of message
  std::time_t now = std::time(nullptr);
  std::string time = std::ctime(&now);
  nlohmann::json message = {
    {"uid", uid},
    {"type", "heartbeat_response"},
    {"time_received", time}
  };
  return message;
}


// Function to capture a single key press
void waitForKeyPress() {
  std::cout << "Press any key to connect to the drone..." << std::endl;

  struct termios old_tio, new_tio;
  unsigned char c;

  // Get the terminal settings
  tcgetattr(STDIN_FILENO, &old_tio);

  // Disable buffered I/O and echo
  new_tio = old_tio;
  new_tio.c_lflag &= (~ICANON & ~ECHO);

  // Apply new settings
  tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

  // Wait for key press
  read(STDIN_FILENO, &c, 1);

  // Restore original settings
  tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
}

// Function to establish a TCP connection to the drone
int connectToDrone() {
  int sock = 0;
  struct sockaddr_in serv_addr;

  // Create socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Socket creation error");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // Convert IPv4 address from text to binary
  if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
    perror("Invalid address/Address not supported");
    close(sock);
    return -1;
  }

  // Connect to the server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Connection failed");
    close(sock);
    return -1;
  }

  std::cout << "Connected to the drone server.\n";
  return sock;
}

bool expectACK(int sock, int uid) {
  char buffer[BUFFER_SIZE] = {0};
  int bytes_read = read(sock, buffer, BUFFER_SIZE);
  if (bytes_read > 0) {
    nlohmann::json message = nlohmann::json::parse(buffer);
    if (message["type"] == "ack" && message["uid"] == uid) {
      received_heartbeat_count++;
      return true;
    } else {
      throw std::runtime_error("Invalid ACK received.");
      return false;
    }
  }
  return false;
}


void handleHeartBeat(int sock, nlohmann::json message) {
  // Handle heartbeat message
  // isolate uid
  int uid = message["uid"];
  // form response
  nlohmann::json response = formHeartbeatMessage(uid);
  // send response
  send(sock, response.dump().c_str(), response.dump().length(), 0);
  try {
    while(bool ack = false) {
      ack = expectACK(sock, uid);
    }
  } catch (std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
  }
}

void handleTelemetry(int sock, nlohmann::json message) {
  // Handle telemetry message
  /*
    incoming message:
    {
      "uid", myUID,
      "type", "telemetry",
      "data", {
        {"pitch_roll", {gyro_data[0], gyro_data[1]}},
        {"ultrasonic_cm", ultrasonic_data},
        {"gps_NMEA", gps_data}
      }
    };
  */

  int uid = message["uid"];
  // parse data to vars
  float pitch = message["data"]["pitch_roll"][0];
  float roll = message["data"]["pitch_roll"][1];
  float ultrasonic_cm = message["data"]["ultrasonic_cm"];
  std::string gps_NMEA = message["data"]["gps_NMEA"];

  // Print data on one line, minus NMEA
  std::cout << "UID: " << uid << " | Pitch: " << pitch << " | Roll: " << roll << " | Ultrasonic: " << ultrasonic_cm << std::endl;
}

// Handles communication with the drone server
void listenToDrone(int sock) {
  char buffer[BUFFER_SIZE] = {0};
  std::string input;

  while (true) {
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_read = read(sock, buffer, BUFFER_SIZE);
    if (bytes_read > 0) {
      try {
        // check if incoming is json
        nlohmann::json message = nlohmann::json::parse(buffer);
        //std::cout << "Received JSON: " << message.dump(4) << std::endl;
        // check type of message
        if (message["type"] == "heartbeat") {
          handleHeartBeat(sock, message);
        } else if (message["type"] == "telemetry") {
          // Handle data message
          handleTelemetry(sock, message);
        } else if (message["type"] == "ack"){
          // nothing here, ack's handled with 'expectACK'
        } else {
          // Handle unknown message type
          std::cout << "Received unknown message type.\n";
        }
      } catch (nlohmann::json::parse_error& e) {
        // Handle non-JSON messages
        std::cout << "Received non-JSON: " << buffer << std::endl;
      }
    }
  }

  close(sock);
}

int main() {
  waitForKeyPress();  // Wait for user input before connecting

  int sock = connectToDrone();  // Attempt connection

  if (sock >= 0) {
    listenToDrone(sock);  // Start communication
  } else {
    std::cerr << "Unable to connect to the drone. Exiting.\n";
  }

  return 0;
}
