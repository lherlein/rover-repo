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

// State machine - continuously poll incoming messages for state change command

int main() {
  std::cout << "Hello, World!" << std::endl;
  return 0;
}