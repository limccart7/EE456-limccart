#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sstream>
#include <RadioLib.h>
#include "PiHal.h"

// Create a new instance of the HAL class
PiHal* hal = new PiHal(0); // 0 for SPI 0

// Radio initialization based on Pi Hat wiring
SX1262 radio = new Module(hal, 29, 27, 1, 28);

void parseNMEA(const std::string &line) {
    if (line.rfind("$GNGGA", 0) == 0) {  // Check if line starts with $GNGGA
        std::istringstream ss(line);
        std::string token;
        int index = 0;
        std::string latitude, longitude, lat_dir, lon_dir;

        while (std::getline(ss, token, ',')) {
            if (index == 2) latitude = token;        // Latitude
            else if (index == 3) lat_dir = token;    // Latitude Direction
            else if (index == 4) longitude = token;  // Longitude
            else if (index == 5) lon_dir = token;    // Longitude Direction
            index++;
        }

        if (!latitude.empty() && !longitude.empty()) {
            // Create a formatted GPS data packet in a char array
            char gps_data[256];
            snprintf(gps_data, sizeof(gps_data), "Lat: %s %s, Lon: %s %s", 
                     latitude.c_str(), lat_dir.c_str(), longitude.c_str(), lon_dir.c_str());

            // Transmit the GPS data packet
            int state = radio.transmit(gps_data);
            if (state == RADIOLIB_ERR_NONE) {
                printf("Transmission success!\n");

                // Get and print the effective data rate for the transmitted packet
                float effectiveDataRate = radio.getDataRate();
                printf("Effective Data Rate: %.2f bps\n", effectiveDataRate);
            } else {
                printf("Transmission failed, code %d\n", state);
            }

            std::cout << "Latitude: " << latitude << " " << lat_dir
                      << ", Longitude: " << longitude << " " << lon_dir << std::endl;
        }
    }
}

int main() {
    // Initialize the radio module
    printf("[SX1262] Initializing ... ");
    int state = radio.begin(915.0, 125.0, 7, 5, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 0.0, false);
    if (state != RADIOLIB_ERR_NONE) {
        printf("Initialization failed, code %d\n", state);
        return 1;
    }
    printf("Initialization success!\n");

    // Open the serial port for GNSS data
    int serial_port = open("/dev/serial0", O_RDWR | O_NOCTTY);
    if (serial_port < 0) {
        std::cerr << "Error opening serial port\n";
        return 1;
    }

    struct termios tty;
    tcgetattr(serial_port, &tty);
    tty.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    tty.c_iflag = IGNPAR;
    tcsetattr(serial_port, TCSANOW, &tty);

    std::string buffer;
    char c;
    while (true) {
        // Read one character at a time
        if (read(serial_port, &c, 1) > 0) {
            if (c == '\n') {  // End of line detected
                parseNMEA(buffer);  // Parse the complete line
                buffer.clear();     // Clear the buffer for the next line
            } else {
                buffer += c;  // Append character to buffer
            }
        }
    }

    close(serial_port);
    return 0;
}
