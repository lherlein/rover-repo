#include <iostream>
#include <wiringPi.h>
#include <csignal>
#include <chrono>

using namespace std;

// pin definition
int trigPin = 8;
int echoPin = 9;

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

// setup
void setup() {
  wiringPiSetup();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

long readUltraSonic() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // read echo
  long duration;
  duration = pulseIn(echoPin, HIGH);
  long cm = duration * 0.0343 / 2;
  return cm;
}

int main() {
  setup();
  while(true) {
    long cm = readUltraSonic();
    cout << "Distance: " << cm << "cm" << endl;
    delay(100);
  }
}