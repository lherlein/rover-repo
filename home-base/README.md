# Ground Station Code

This ground station will make connections with drones and control them/monitor them. Currently, it will be connecting to a rover over the network.

## Network Comms

TCP tunnel is formed where the drone is the server and the ground station is the client. At the end of the setup process, the ground station will be told to connect to a specific IP on the network. This will be determined during a setup process not currently defined, so for now it will be manual.

```
rm -rf build && mkdir build && cd build
cmake ..
cmake --build .
```