#include <iostream>
#include <cstdlib>

// misc libs
#include <fstream>
#include <nlohmann/json.hpp>
#include <chrono>

// sensor libs
#include "ultrasonic.h"
#include "tinygps.h"
#include "mpu6050.h"

// define MPU
MPU6050 mpu6050(0x68);
int us_trig = 5;
int us_echo = 4;

// networking libs
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

// threading libs
#include <thread>
#include <mutex>
std::mutex socket_mutex;

#define PORT 5000
#define BUFFER_SIZE 1024

int myUID = 0001; // hard code for now

// Set up heartbeat messages
nlohmann::json formHeartbeatMessage() { // preshim crafting of message
  std::time_t now = std::time(nullptr);
  std::string time = std::ctime(&now);
  nlohmann::json message = {
    {"uid", myUID},
    {"type", "heartbeat"},
    {"time_sent", time}
  };
  return message;
}

// Function to set up the TCP server and return the client socket descriptor
int setupTCPServer() {
  int server_fd, client_fd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len = sizeof(client_addr);

  // Create socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("Socket creation failed");
    return -1;
  }

  // Configure server address
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
  server_addr.sin_port = htons(PORT);

  // Bind the socket
  if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("Bind failed");
    close(server_fd);
    return -1;
  }

  // Start listening
  if (listen(server_fd, 3) < 0) {
    perror("Listen failed");
    close(server_fd);
    return -1;
  }

  std::cout << "Drone server listening on port " << PORT << "...\n";

  // Accept client connection
  client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
  if (client_fd < 0) {
    perror("Accept failed");
    close(server_fd);
    return -1;
  }

  std::cout << "Connection established with ground station.\n";

  close(server_fd);  // Close server socket after accepting client
  return client_fd;
}

// Function to handle communication with the ground station
void handleIncomingMessages(int client_fd) {
  char buffer[BUFFER_SIZE] = {0};

  while (true) {
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_read = read(client_fd, buffer, BUFFER_SIZE);

    if (bytes_read > 0) {
      // Respond back to client
      //std::string response = "ACK: " + std::string(buffer);
      // craft response json
      nlohmann::json responseJSON = {
        {"uid", myUID},
        {"type", "ack"},
        {"message", buffer}
      };
      std::string response = responseJSON.dump();
      // std::lock_guard<std::mutex> lock(socket_mutex);
      send(client_fd, response.c_str(), response.length(), 0);
    } else {
      std::cout << "Connection closed or error occurred.\n";
      break;
    }
  }

  close(client_fd);
}

void sendHeartbeats(int client_fd) {
  while (true) {
    nlohmann::json message = formHeartbeatMessage();
    // std::lock_guard<std::mutex> lock(socket_mutex);
    send(client_fd, message.dump().c_str(), message.dump().length(), 0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

long getUltrasonicCm() {
  return readUltrasonic(us_trig, us_echo);
}

std::string getGPSData() {
  const char *port = "/dev/ttyS0";
  std::string gps_data = readGPS(port);
  return gps_data;
}

std::array <float, 2> callibrateMPU(int calib_n = 1000) {
  // get roll, pitch calib_n times and average
  std::cout << "Calibrating MPU6050...\n";
  float roll_calib = 0, pitch_calib = 0;
  for (int i = 0; i < calib_n; i++) {
    float roll, pitch;
    mpu6050.getAngle(0, &roll);
    mpu6050.getAngle(1, &pitch);
    roll_calib += roll;
    pitch_calib += pitch;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  std::cout << "Calibration complete.\n";
  return {roll_calib/calib_n, pitch_calib/calib_n};
}

std::array <float, 2> getGyroData(float roll_calib = 0, float pitch_calib = 0) {
  float roll, pitch;
  mpu6050.getAngle(0, &roll);
  mpu6050.getAngle(1, &pitch);
  return {roll-roll_calib, pitch-pitch_calib};
}

void grabAndSendTelem(float roll_calib, float pitch_calib, int client_fd) {
  while (true) {
    std::array <float, 2> gyro_data = getGyroData(roll_calib, pitch_calib);
    long ultrasonic_data = getUltrasonicCm();
    std::string gps_data = getGPSData();

    nlohmann::json sensorData = {
      {"pitch_roll", {gyro_data[0], gyro_data[1]}},
      {"ultrasonic_cm", ultrasonic_data},
      {"gps_NMEA", gps_data}
    };

    nlohmann::json telem = {
      {"uid", myUID},
      {"type", "telemetry"},
      {"data", sensorData}
    };

    // std::lock_guard<std::mutex> lock(socket_mutex);
    send(client_fd, telem.dump().c_str(), telem.dump().length(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

int main() {
  std::array <float, 2> calib = callibrateMPU();  // Calibrate MPU6050
  int client_fd = setupTCPServer();  // Initialize TCP server

  if (client_fd >= 0) {
    std::thread heartbeat_thread(sendHeartbeats, client_fd);
    std::thread telemetry_thread(grabAndSendTelem, calib[0], calib[1], client_fd);
    std::thread incoming_messages(handleIncomingMessages, client_fd);
    heartbeat_thread.join();
    telemetry_thread.join();
    incoming_messages.join();
  } else {
    std::cerr << "Failed to establish a TCP connection.\n";
  }

  return 0;
}

