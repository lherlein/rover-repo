#include <iostream>
#include <wiringPi.h>
#include <csignal>

using namespace std;

//bool RUNNING = true;

// blink led
void blinkLed(int ledPin, int delayMs) {
  digitalWrite(ledPin, HIGH);
  delay(delayMs);
  digitalWrite(ledPin, LOW);
  delay(delayMs);
}

int main() {

  wiringPiSetup();
  
  std::cout << "Blinking LED" << std::endl;

  int ledPin = 25;
  pinMode(ledPin, OUTPUT);

  int delayMs = 500;
  while(true) {
    blinkLed(ledPin, delayMs);
  }
}