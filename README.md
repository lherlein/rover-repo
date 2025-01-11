# Rover MonoRepo

This repository contains C++ code to control the raspberry pi alongside homebase code written in Node.js. The code in this repository will allow your computer to communicate with and control the raspberry pi and all attached sensors and motors. 

## rapsberry-pi

This directory contains all source code for the robot. 

This code is compiled on the raspberry-pi. Compiling on a non pi device will cause problems. Copy the file structure within `raspberry-pi/` onto your pi, go to the code's working directory, and run:

```
sudo apt-get update
sudo apt-get install wiringpi libi2c-dev build-essential cmake
```

```
rm -rf build && mkdir build && cd build
cmake ..
cmake --build .
```

## home-base

This directory contains all source code for the control station. 