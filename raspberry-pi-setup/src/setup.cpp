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
bool IN_SETUP = true;

// Setup: RPi is an Access Point until a client connects and passes an SSID/Password combo, which is written to networkconfig.json in the home directory. When that happens, the RPi switches to client mode and connects to the specified network.
// If network connection fails, return to AP mode. If the client disconnects, return to AP mode. If the connection to the network is successful, change to main code.
void enableAPMode() {
  std::cout << "[INFO] Enabling Access Point Mode, this will take a few seconds" << std::endl;
  system("sudo rfkill unblock all");
  system("sudo ip link set wlan0 up");
  sleep(3);
  system("sudo systemctl restart dhcpcd");
  system("sudo systemctl restart hostapd");
  system("sudo systemctl restart dnsmasq");
  system("sudo systemctl unmask hostapd");
  std::cout << "[INFO] Access Point Mode enabled." << std::endl;
}

void restartAPMode() {
  std::cout << "[INFO] Restarting Access Point Mode..." << std::endl;

  system("sudo ip link set wlan0 down");
  system("sudo systemctl restart hostapd");
  sleep(2);
  system("sudo systemctl restart dnsmasq");
  system("sudo systemctl unmask hostapd");
  system("sudo ip link set wlan0 up");
  sleep(3);

  std::cout << "[INFO] Access Point Mode restarted." << std::endl;
}

void disableAPMode() {
  std::cout << "[INFO] Disabling Access Point Mode..." << std::endl;

  system("sudo systemctl stop hostapd");
  system("sudo systemctl stop dnsmasq");
  system("sudo systemctl restart dhcpcd");

  std::cout << "[INFO] Access Point Mode disabled." << std::endl;
}

bool connectToWiFi() {
  system("sudo ip link set wlan0 down");
  system("sudo systemctl stop hostapd");
  system("sudo systemctl stop dhcpcd");
  std::cout << "[INFO] Connecting to Wi-Fi using stored credentials..." << std::endl;

  // Read JSON configuration
  std::ifstream file("/home/luca/network-config.json");
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

  std::cout << "[DEBUG] Connecting to Wi-Fi SSID: " << ssid << std::endl;

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
  sleep(3);
  system("sudo systemctl restart wpa_supplicant");
  system("sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf");
  system("sudo wpa_cli -i wlan0 reconfigure");
  system("sudo dhclient wlan0");

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
        "<head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<title>Wi-Fi Configuration</title>"
        "</head>"
        "<body>"
        "<h2>Enter Wi-Fi Credentials</h2>"
        "<form id='wifiForm'>"
        "<label for='ssid'>SSID:</label><br>"
        "<input type='text' id='ssid' name='ssid' required><br><br>"
        "<label for='password'>Password:</label><br>"
        "<input type='password' id='password' name='password' required><br><br>"
        "<button type='button' onclick='submitForm()'>Connect</button>"
        "</form>"
        "<p id='status'></p>"
        "<script>"
        "function submitForm() {"
        "  const ssid = document.getElementById('ssid').value;"
        "  const password = document.getElementById('password').value;"
        "  fetch(`/ssid=${ssid}&password=${password}`, {"
        "    method: 'POST',"
        "    headers: {"
        "      'Content-Type': 'application/x-www-form-urlencoded'"
        "    },"
        "  })"
        "  .then(response => {"
        "    if (response.ok) {"
        "      document.getElementById('status').innerText = 'Credentials sent successfully!';"
        "    } else {"
        "      document.getElementById('status').innerText = 'Failed to send credentials.';"
        "    }"
        "  })"
        "  .catch(error => {"
        "    document.getElementById('status').innerText = 'Error: ' + error;"
        "  });"
        "}"
        "</script>"
        "</body>"
        "</html>";
}

void handleClient(int client_socket) {
  char buffer[4096] = {0};
  int bytes_read = read(client_socket, buffer, sizeof(buffer));
  std::string request(buffer, bytes_read);

  // Check for POST Request
  if (request.find("POST") != std::string::npos) {

    // Extract POST Body
    // ssid and password contained in POSTing URL
    // Parse Form Data (ssid=...&password=...)
    size_t ssid_pos = request.find("ssid=");
    size_t pass_pos = request.find("password=");

    if (ssid_pos == std::string::npos || pass_pos == std::string::npos) {
      std::cerr << "[ERROR] SSID or Password not found in POST data." << std::endl;
      close(client_socket);
      return;
    }

    std::string ssid = request.substr(ssid_pos + 5, request.find("&", ssid_pos) - (ssid_pos + 5));
    std::string password = request.substr(pass_pos + 9, request.find(" ", pass_pos) - (pass_pos + 9));

    // Replace '+' with ' ' in form data
    // std::replace(ssid.begin(), ssid.end(), '+', ' ');
    // std::replace(password.begin(), password.end(), '+', ' ');

    // Debug: Print parsed SSID and Password
    std::cout << "[DEBUG] SSID: " << ssid << std::endl;
    std::cout << "[DEBUG] Password: " << password << std::endl;

    // Save to JSON
    nlohmann::json config;
    config["ssid"] = ssid;
    config["password"] = password;

    std::ofstream configFile("/home/luca/network-config.json");
    if (configFile.is_open()) {
      configFile << config.dump(4);
      configFile.close();
      std::cout << "[INFO] Wi-Fi credentials saved.\n";
    } else {
      std::cerr << "[ERROR] Failed to save configuration.\n";
    }

    // Respond to Client
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                            "<html><body><h2>Connecting to Wi-Fi...</h2></body></html>";
    write(client_socket, response.c_str(), response.length());
    close(client_socket);

    // Attempt Wi-Fi connection
    disableAPMode();
    if (!connectToWiFi()) {
      std::cout << "[ERROR] Wi-Fi connection failed. Re-enabling AP Mode." << std::endl;
      restartAPMode();
      std::string html = generateHTMLPage();
      write(client_socket, html.c_str(), html.length());
      close(client_socket);
    } else {
      std::cout << "[SUCCESS] Connected to Wi-Fi." << std::endl;
      IN_SETUP = false;
    }

  } else if (request.find("GET") != std::string::npos) {
    // Serve the HTML Form for GET request
    std::string html = generateHTMLPage();
    write(client_socket, html.c_str(), html.length());
    close(client_socket);
  } else {
    // Handle unsupported request types
    std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid Request.";
    write(client_socket, response.c_str(), response.length());
    close(client_socket);
  }
}

int main() {
  system("sudo rfkill unblock all");
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

  while (IN_SETUP) {
    client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    handleClient(client_socket);
  }

  std::cout << "[INFO] Setup complete. Switching to main code." << std::endl;

  return 0;
}