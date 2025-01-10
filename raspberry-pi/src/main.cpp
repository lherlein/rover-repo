#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

// Include sensor libraries
#include "ultrasonic.h"
#include "tinygps.h"
#include "mpu6050.h"

// Include PWM and Wi-Fi setup libraries
// #include "PWMController.h"
// #include "WiFiManager.h"

// // Configuration Constants
// #define TELEMETRY_IP "192.168.1.100"
// #define TELEMETRY_PORT 5005
// #define MOTOR_CONTROL_PORT 6006

// // Global Instances
// UltrasonicSensor ultrasonic;
// GPSSensor gps;
// Accelerometer accel;
// PWMController leftMotorPWM;
// PWMController rightMotorPWM;

// // Function to send telemetry data
// void sendTelemetry(int sock, struct sockaddr_in &addr) {
//   std::string data = "Ultrasonic:" + std::to_string(ultrasonic.readDistance()) +
//                     ",GPS:" + gps.readCoordinates() +
//                     ",Accel:" + accel.readValues();
  
//   sendto(sock, data.c_str(), data.length(), 0, (struct sockaddr*)&addr, sizeof(addr));
// }

// // Function to handle incoming UDP packets for motor control
// void handleMotorControl(int sock) {
//   char buffer[1024];
//   struct sockaddr_in senderAddr;
//   socklen_t addrLen = sizeof(senderAddr);

//   int bytesReceived = recvfrom(sock, buffer, sizeof(buffer) - 1, MSG_DONTWAIT, 
//                               (struct sockaddr*)&senderAddr, &addrLen);
//   if (bytesReceived > 0) {
//     buffer[bytesReceived] = '\0';
//     std::string command(buffer);
    
//     // Example command format: "LEFT:1500;RIGHT:1500"
//     size_t leftPos = command.find("LEFT:");
//     size_t rightPos = command.find("RIGHT:");

//     if (leftPos != std::string::npos && rightPos != std::string::npos) {
//       int leftSpeed = std::stoi(command.substr(leftPos + 5, command.find(';', leftPos) - (leftPos + 5)));
//       int rightSpeed = std::stoi(command.substr(rightPos + 6));

//       leftMotorPWM.setDutyCycle(leftSpeed);
//       rightMotorPWM.setDutyCycle(rightSpeed);
//     }
//   }
// }

// int main() {
//   // Step 1: Initialize Wi-Fi
//   WiFiManager wifi;
//   if (!wifi.connect("SSID", "PASSWORD")) {
//     std::cerr << "Failed to connect to Wi-Fi." << std::endl;
//     return -1;
//   }

//   // Step 2: Set up UDP socket for telemetry
//   int telemetrySock = socket(AF_INET, SOCK_DGRAM, 0);
//   struct sockaddr_in telemetryAddr{};
//   telemetryAddr.sin_family = AF_INET;
//   telemetryAddr.sin_port = htons(TELEMETRY_PORT);
//   inet_pton(AF_INET, TELEMETRY_IP, &telemetryAddr.sin_addr);

//   // Step 3: Set up UDP socket for motor control
//   int motorControlSock = socket(AF_INET, SOCK_DGRAM, 0);
//   struct sockaddr_in motorControlAddr{};
//   motorControlAddr.sin_family = AF_INET;
//   motorControlAddr.sin_port = htons(MOTOR_CONTROL_PORT);
//   motorControlAddr.sin_addr.s_addr = INADDR_ANY;

//   bind(motorControlSock, (struct sockaddr*)&motorControlAddr, sizeof(motorControlAddr));

//   // Step 4: Main loop
//   while (true) {
//     // 1. Sensor Reading
//     ultrasonic.update();
//     gps.update();
//     accel.update();

//     // 2. Send Telemetry
//     sendTelemetry(telemetrySock, telemetryAddr);

//     // 3. Read UDP Input
//     handleMotorControl(motorControlSock);

//     // 4. Control Motors (Handled in handleMotorControl)

//     std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Loop delay
//   }

//   close(telemetrySock);
//   close(motorControlSock);

//   return 0;
// }
