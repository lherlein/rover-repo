cmake_minimum_required(VERSION 3.14)
project(robot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include the headers
include_directories(include)

# Find and link wiringPi and i2c libraries
find_library(WIRINGPI_LIB wiringPi REQUIRED)
find_library(I2C_LIB i2c REQUIRED)

# Define the executable and source files
add_executable(robot
  src/main.cpp
  src/mpu6050.cpp
  src/tinygps.cpp
  src/ultrasonic.cpp
)

# Link external libraries
target_link_libraries(robot
  ${WIRINGPI_LIB}
  ${I2C_LIB}
)
