#ifndef ULTRASONIC_H
#define ULTRASONIC_H

/**
 * Measures the duration of a pulse on a pin.
 * @param pin GPIO pin to read from.
 * @param state HIGH or LOW state to wait for.
 * @param timeout Microseconds to wait before timing out (default 1 second).
 * @return Pulse duration in microseconds or 0 if timeout occurs.
 */
unsigned long pulseIn(int pin, int state, unsigned long timeout = 1000000);

/**
 * Reads distance in centimeters from the ultrasonic sensor.
 * @param trigPin GPIO pin connected to the trigger pin.
 * @param echoPin GPIO pin connected to the echo pin.
 * @return Distance in cm or -1 on timeout/error.
 */
long readUltrasonic(int trigPin = 8, int echoPin = 9);

#endif // ULTRASONIC_H
