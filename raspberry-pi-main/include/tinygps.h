#ifndef TINYGPS_H
#define TINYGPS_H

#include <string>

/**
 * Reads GPS data from a specified UART port.
 * @param port Path to the UART device (e.g., "/dev/ttyS0")
 * @return A valid NMEA sentence starting with '$' or an empty string on failure.
 */

std::string readGPS(const char *port);

#endif // TINYGPS_H
