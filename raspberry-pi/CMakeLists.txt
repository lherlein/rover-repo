# Minimum CMake version and project definition
cmake_minimum_required(VERSION 3.10)
project(RaspberryPiRobot)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for custom headers
include_directories(
    ${CMAKE_SOURCE_DIR}/include           # Custom headers
)

# Find WiringPi (already installed on the Pi)
find_library(WIRINGPI_LIB wiringPi)
if (NOT WIRINGPI_LIB)
    message(FATAL_ERROR "WiringPi library not found!")
endif()

# Find I2C library (already installed on the Pi)
find_library(I2C_LIB i2c)
if (NOT I2C_LIB)
    message(FATAL_ERROR "I2C library not found!")
endif()

# Gather source files
file(GLOB SRC_FILES "src/*.cpp")           # Main source files
file(GLOB LIB_FILES "libs/sensors/*.cpp")   # Custom library source files

# Create a static library from custom code
add_library(sensors STATIC ${LIB_FILES})

# Create executable
add_executable(robot ${SRC_FILES})

# Link custom library, WiringPi, and I2C to the executable
target_link_libraries(robot
  sensors     # Custom C++ logic
  ${WIRINGPI_LIB} # WiringPi (GPIO control)
  ${I2C_LIB}      # I2C communication
  pthread         # Threading support
  rt              # Real-time clock support
)
