#include <iostream>
#include <cstdlib>

// misc libs
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

//network libs
#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

#define PORT 80

// Setup: RPi is an Access Point until a client connects and passes an SSID/Password combo, which is written to networkconfig.json in the home directory. When that happens, the RPi switches to client mode and connects to the specified network.
// If network connection fails, return to AP mode. If the client disconnects, return to AP mode. If the connection to the network is successful, change to main code.

void enableAPMode() {
  std::cout << "[INFO] Enabling or Verifying Access Point Mode..." << std::endl;

  // Check if hostapd is running
  int apStatus = system("sudo systemctl is-active --quiet hostapd");

  if (apStatus != 0) {
    std::cout << "[INFO] AP is not active. Enabling AP Mode..." << std::endl;
    system("sudo rfkill unblock all");
    system("sudo systemctl unmask hostapd");
    system("sudo systemctl start hostapd");
    system("sudo systemctl start dnsmasq");
    system("sudo ip link set wlan0 up");
  } else {
    std::cout << "[INFO] Access Point is already active." << std::endl;
  }
}

void disableAPMode() {
  std::cout << "[INFO] Disabling Access Point Mode..." << std::endl;

  system("sudo systemctl stop hostapd");
  system("sudo systemctl stop dnsmasq");
  system("sudo ip link set wlan0 down");

  std::cout << "[INFO] Access Point Mode disabled." << std::endl;
}

bool connectToWiFi() {
  std::cout << "[INFO] Connecting to Wi-Fi using stored credentials..." << std::endl;

  // Read JSON configuration
  std::ifstream file("/home/pi/network-config.json");
  if (!file.is_open()) {
    std::cerr << "[ERROR] Failed to open network-config.json." << std::endl;
    return false;
  }

  nlohmann::json config;
  try {
    file >> config;
  } catch (const std::exception &e) {
    std::cerr << "[ERROR] Failed to parse JSON: " << e.what() << std::endl;
    return false;
  }
  file.close();

  std::string ssid = config["ssid"];
  std::string password = config["password"];

  // Write to wpa_supplicant.conf
  std::ofstream wifiConfig("/etc/wpa_supplicant/wpa_supplicant.conf");
  if (!wifiConfig.is_open()) {
    std::cerr << "[ERROR] Failed to open wpa_supplicant.conf." << std::endl;
    return false;
  }

  wifiConfig << "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev\n"
            << "update_config=1\n\n"
            << "network={\n"
            << "    ssid=\"" << ssid << "\"\n"
            << "    psk=\"" << password << "\"\n"
            << "    key_mgmt=WPA-PSK\n"
            << "}\n";
  wifiConfig.close();

  // Bring up wlan0 and reload configuration
  system("sudo ip link set wlan0 up");
  system("sudo wpa_cli -i wlan0 reconfigure");
  system("sudo systemctl restart dhcpcd");

  // Check if connected to the internet
  int result = system("ping -c 2 8.8.8.8 > /dev/null 2>&1");
  if (result == 0) {
    std::cout << "[SUCCESS] Connected to the internet!" << std::endl;
    return true;
  } else {
    std::cerr << "[ERROR] Failed to connect to the internet." << std::endl;
    return false;
  }
}  

// Serve HTML form to the client
std::string generateHTMLPage() {
  return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<!DOCTYPE html>"
        "<html lang='en'>"
        "<head><meta charset='UTF-8'><title>Wi-Fi Configuration</title></head>"
        "<body>"
        "<h2>Connect to Wi-Fi</h2>"
        "<form method='POST'>"
        "SSID: <input type='text' name='ssid' required><br><br>"
        "Password: <input type='password' name='password' required><br><br>"
        "<input type='submit' value='Connect'>"
        "</form>"
        "</body>"
        "</html>";
}

// Handle HTTP client for credential submission
void handleClient(int client_socket) {
  char buffer[4096] = {0};
  read(client_socket, buffer, 4096);
  std::string request(buffer);

  if (request.find("POST") != std::string::npos) {
    size_t ssid_pos = request.find("ssid=");
    size_t pass_pos = request.find("password=");
    std::string ssid = request.substr(ssid_pos + 5, request.find("&", ssid_pos) - (ssid_pos + 5));
    std::string password = request.substr(pass_pos + 9, request.find(" ", pass_pos) - (pass_pos + 9));

    std::replace(ssid.begin(), ssid.end(), '+', ' ');
    std::replace(password.begin(), password.end(), '+', ' ');

    nlohmann::json config;
    config["ssid"] = ssid;
    config["password"] = password;

    std::ofstream configFile("/home/pi/network-config.json");
    configFile << config.dump(4);
    configFile.close();

    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                          "<html><body><h2>Connecting to Wi-Fi...</h2></body></html>";
    write(client_socket, response.c_str(), response.length());
    close(client_socket);

    disableAPMode();
    if (!connectToWiFi()) {
      std::cout << "[ERROR] Wi-Fi connection failed. Re-enabling AP Mode." << std::endl;
      enableAPMode();
    } else {
      std::cout << "[SUCCESS] Connected to Wi-Fi." << std::endl;
    }
  } else {
    std::string html = generateHTMLPage();
    write(client_socket, html.c_str(), html.length());
    close(client_socket);
  }
}

int main() {
  enableAPMode();

  int server_fd, client_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  bind(server_fd, (struct sockaddr*)&address, sizeof(address));
  listen(server_fd, 3);

  std::cout << "[INFO] Server running on port " << PORT << std::endl;

  while (true) {
    client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    handleClient(client_socket);
  }

  return 0;
}