cmake_minimum_required(VERSION 3.14)
project(ground-station)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include the headers
include_directories(include)

find_package(nlohmann_json 3.2.0 REQUIRED)

add_executable(ground-station
  src/main.cpp
)

# Link external libraries
target_link_libraries(ground-station
  nlohmann_json::nlohmann_json
)