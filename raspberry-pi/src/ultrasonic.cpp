#include <iostream>
#include <wiringPi.h>
#include <csignal>
#include <chrono>

using namespace std;
const float SOUND_SPEED_CM_PER_US = 0.0343f;  // Speed of sound in cm/us
const int DEFAULT_TRIG_PIN = 8;
const int DEFAULT_ECHO_PIN = 9;

unsigned long pulseIn(int pin, int state, unsigned long timeout = 1000000) {
  auto start = std::chrono::high_resolution_clock::now();
  auto timeout_time = start + std::chrono::microseconds(timeout);

  // Wait for the pin to match the desired state
  while (digitalRead(pin) != state) {
    if (std::chrono::high_resolution_clock::now() > timeout_time) {
      return 0; // Timeout
    }
  }

  auto pulse_start = std::chrono::high_resolution_clock::now();

  // Measure how long the pin stays in the desired state
  while (digitalRead(pin) == state) {
    if (std::chrono::high_resolution_clock::now() > timeout_time) {
      return 0; // Timeout
    }
  }

  auto pulse_end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(pulse_end - pulse_start).count();
}

// Reads distance in cm from the ultrasonic sensor
long readUltrasonic(int trigPin = DEFAULT_TRIG_PIN, int echoPin = DEFAULT_ECHO_PIN) {
  static bool isInitialized = false;
  if (!isInitialized) {
    wiringPiSetup();
    isInitialized = true;
  }

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  if (duration == 0) {
    std::cerr << "Ultrasonic sensor timeout." << std::endl;
    return -1;  // Indicate error
  }

  long cm = duration * SOUND_SPEED_CM_PER_US / 2;
  return cm;
}