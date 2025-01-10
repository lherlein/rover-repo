#include <iostream>
#include "tinygps.h"
#include "ultrasonic.h"
#include "mpu6050.h"

using namespace std;

int ultrasonic_trig_pin = 5;
int ultrasonic_echo_pin = 4;
int mpu6050_addr = 0x68;
const char* gps_port = "/dev/ttyserial0";

MPU6050 device(mpu6050_addr);

int main() {
	float ax, ay, az, gr, gp, gy; //Variables to store the accel, gyro and angle values

	sleep(1); //Wait for the MPU6050 to stabilize

	// //Calculate the offsets
	// std::cout << "Calculating the offsets...\n    Please keep the accelerometer level and still\n    This could take a couple of minutes...";
	// device.getOffsets(&ax, &ay, &az, &gr, &gp, &gy, 1000);
	// std::cout << "Gyroscope R,P: " << gr << "," << gp << "\nAccelerometer X,Y: " << ax << "," << ay << "\n";


	//Read the current yaw angle
	device.calc_yaw = true;

	for (int i = 0; i < 40; i++) {
		device.getAngle(0, &gr);
		device.getAngle(1, &gp);
		std::cout << "Current angle around the roll axis: " << gr << "\n";
		std::cout << "Current angle around the pitch axis: " << gp << "\n";
		usleep(250000); //0.25sec
	}
	return 0;
}