cmake_minimum_required(VERSION 3.14)
project(robot-main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include the headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find and link wiringPi and i2c libraries
find_library(WIRINGPI_LIB wiringPi REQUIRED)
find_library(I2C_LIB i2c REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Define the executable and source files
add_executable(robot-main
  src/main.cpp
  src/sensors/ultrasonic.cpp
  src/sensors/tinygps.cpp
  src/sensors/mpu6050.cpp
)

# Link external libraries
target_link_libraries(robot-main
  ${WIRINGPI_LIB}
  ${I2C_LIB}
  nlohmann_json::nlohmann_json
)
