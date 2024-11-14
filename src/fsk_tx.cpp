#include <RadioLib.h>
#include "PiHal.h"
#include <chrono> // For timestamping

// Create a new instance of the HAL class
PiHal* hal = new PiHal(0);

// Create the radio module instance
// Pinout corresponds to your SX1262 setup
// NSS pin: WiringPi 10 (GPIO 8) WPI # 29 GPIO 21 for Pi hat
// DIO1 pin: WiringPi 2 (GPIO 27) WPI # 27 GPIO 16 for Pi hat
// NRST pin: WiringPi 21 (GPIO 5) WPI #1 GPIO 18
// BUSY pin: WiringPi 0 (GPIO 17) WPI #28 GPIO 20
SX1262 radio = new Module(hal, 10, 2, 21, 0);

int main() {
    // Initialize the radio module with XTAL configuration
    printf("[SX1262] Initializing ... ");
    int state = radio.beginFSK(915.0, 4.8, 125.0, 467.0, 20.0, 16.0, 0.0, false);
    if (state != RADIOLIB_ERR_NONE) {
        printf("Initialization failed, code %d\n", state);
        return 1;
    }
    printf("Initialization success!\n");

    int count = 0;

    while (true) {
        // Get the current time 
        auto now = std::chrono::high_resolution_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

        // Print the timestamp and packet number
        printf("[SX1262] Transmitting packet #%d at time %lld ms ... ", count, timestamp);

        // Create a packet with the timestamp and message
        char str[64];
        sprintf(str, "Timestamp: %lld, Hello World! #%d", timestamp, count++);

        // Send the packet
        state = radio.transmit(str);
        if (state == RADIOLIB_ERR_NONE) {
            printf("success!\n");

            // Get and print the effective data rate for the transmitted packet
            float effectiveDataRate = radio.getDataRate();
            printf("Effective Data Rate: %.2f bps\n", effectiveDataRate);
        } else {
            printf("failed, code %d\n", state);
        }

        // Wait for a second before transmitting again
        hal->delay(1000);
    }

    return 0;
}
