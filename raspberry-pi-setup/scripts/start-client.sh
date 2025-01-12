sudo ip link set wlan0 up
sudo wpa_cli -i wlan0 reconfigure
sudo systemctl start dhcpcd
