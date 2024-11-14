#include <iostream>
#include <RadioLib.h>
#include "PiHal.h"
#include <cstdio>

// Create a new instance of the HAL class
PiHal* hal = new PiHal(0);

// NSS pin: WiringPi 10 (GPIO 8) WPI # 29 GPIO 21 for Pi hat
// DIO1 pin: WiringPi 2 (GPIO 27) WPI # 27 GPIO 16 for Pi hat
// NRST pin: WiringPi 21 (GPIO 5) WPI #1 GPIO 18
// BUSY pin: WiringPi 0 (GPIO 17) WPI #28 GPIO 20

// Create the radio module instance
Module* module = new Module(hal, 10, 2, 21, 0);
SX1262 radio(module);

// Flag to indicate a packet was received
volatile bool receivedFlag = false;

// ISR function to set the received flag
void setFlag(void) {
  receivedFlag = true;
}

int main(int argc, char** argv) {
  radio.XTAL = true;

  // Initialize the radio module
  printf("[SX1262] Initializing ... ");
  int state = radio.beginFSK(915.0, 4.8, 125.0, 467.0, 20.0, 16.0, 0.0, false); // FSK mode
  if (state != RADIOLIB_ERR_NONE) {
    printf("failed, code %d\n", state);
    return(1);
  }
  printf("success!\n");

  // Set the ISR for packet received
  radio.setPacketReceivedAction(setFlag);

  // Start receiving packets
  printf("[SX1262] Starting to listen ... ");
  state = radio.startReceive();
  if (state != RADIOLIB_ERR_NONE) {
    printf("failed, code %d\n", state);
    return(1);
  }
  printf("success!\n");

  // Main loop
  uint8_t buff[256] = { 0 };
  for(;;) {
    if(receivedFlag) {
      // Reset flag
      receivedFlag = false;
      
      // Get the length of the received packet
      size_t len = radio.getPacketLength();
      
      // Read the received packet
      int state = radio.readData(buff, len);
      if (state != RADIOLIB_ERR_NONE) {
        printf("Read failed, code %d\n", state);
      } else {
        printf("Data: %s\n", (char*)buff);
      }

    }
  }

  return(0);
}
