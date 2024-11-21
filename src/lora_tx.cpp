#include <RadioLib.h>
#include "PiHal.h"
#include <chrono> // For timestamping
#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <sstream>

// Create a new instance of the HAL class
PiHal* hal = new PiHal(0); // 0 for SPI 0 , set to 1 if using SPI 1(this will change NSS pinout)

// Create the radio module instance/////////////////////////
// Pinout *****MBED SHIELD****************PI HAT************
// NSS pin:  WPI# 10 (GPIO 8)  WPI # 29 (GPIO 21) for Pi hat
// DIO1 pin: WPI# 2  (GPIO 27) WPI # 27 (GPIO 16) for Pi hat
// NRST pin: WPI# 21 (GPIO 5)  WPI # 1  (GPIO 18) for Pi hat
// BUSY pin: WPI# 0  (GPIO 17) WPI # 28 (GPIO 20) for Pi hat
////////////////////////////////////////////////////////////

// Radio initialization based on Pi Hat wiring
// change for MBED Shield use
// According to SX1262 radio = new Module(hal, NSS,DI01,NRST,BUSY)
SX1262 radio = new Module(hal, 29, 27, 1, 28);

void parseNMEA(const std::string &line, bool prntflag){
    if(line.rfind("GNGGA", 0) == 0){
        prntflag = true;
        std::istringstream ss(line);
        std::string token;
        int idx = 0;
        std::string lat, lon, lat_dir, lon_dir;

	while(std::getline(ss, token, ',')){
	    if(idx == 2) lat = token;
	    else if(idx == 3) lat_dir = token;
            else if(idx == 4) lon = token;
            else if(idx == 5) lon_dir = token;
            idx++;
        }
        std::cout<<"Latitude: " << lat + " " << lat_dir << ", Longitude: " << lon + " " << lon_dir << std::endl;
    }
    else{
         prntflag = false;
    }
}

int main() {
    // Initialize the radio module
    std::cout << "[SX1262] Initializing ... " << std::endl;
    int serial_port = open("/dev/serial0", O_RDWR | O_NOCTTY);

    if(serial_port < 0){
        std::cerr << "Error opening serial port!\n";
        return 1;
    }

    struct termios tty;
    tcgetattr(serial_port, &tty);
    tty.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    tty.c_iflag = IGNPAR;
    tcsetattr(serial_port, TCSANOW, &tty);


    int state = radio.begin(915.0, 125.0, 7, 5, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 0.0, false);
    if (state != RADIOLIB_ERR_NONE) {
        printf("Initialization failed, code %d\n", state);
        return 1;
    }
    //std::cout << "Initialization????? success!\n" << std::endl;
    
    int count = 0;
    char buf[256];
    while (true) {
        int n = read(serial_port, buf, sizeof(buf) - 1);
        
        bool prntflag =  false;
        if(n < 0){
            
            int n = read(serial_port, buf, sizeof(buf) - 1);
            buf[n] = '\0';
            std::string line(buf);
            parseNMEA(line, prntflag);
            if(prntflag == true){
                std::cout << "Transmitting: " << line << std::endl;
                char* transmit_array = new char[line.length()];
                strcpy(transmit_array, line.c_str());
                state = radio.transmit(transmit_array);

                if (state == RADIOLIB_ERR_NONE) {
                    std::cout << "success!" << std::endl;

            // Get and print the effective data rate for the transmitted packet
                    float effectiveDataRate = radio.getDataRate();
                    //printf("Effective Data Rate: %.2f bps\n", effectiveDataRate);
                } else {
                    //printf("failed, code %d\n", state);
                }
            }
        }
        //std::cout << "Loop done?" << std::endl;
        hal->delay(1000); // delay 1 second
    }

    close(serial_port);

    return 0;
}
