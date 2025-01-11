cmake_minimum_required(VERSION 3.14)
project(robot-setup)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include the headers
include_directories(include)

find_package(nlohmann_json 3.2.0 REQUIRED)

add_executable(robot-setup
  src/setup.cpp
)

# Link external libraries
target_link_libraries(robot-setup
  nlohmann_json::nlohmann_json
)