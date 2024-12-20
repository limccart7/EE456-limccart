#include <iostream>
#include <RadioLib.h>
#include "PiHal.h"
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <fcntl.h>
#include <termios.h>
#include <sstream>

using namespace std;

#define PORT 8080
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
Module* module = new Module(hal, 29, 27, 1, 28);
SX1262 radio(module);

// Flag to indicate packet has been received
volatile bool receivedFlag = false;

// ISR function to set received flag
void setFlag(void) {
  receivedFlag = true;
}

int main(int argc, char** argv) {
  hal->init(); 
  radio.XTAL = true;

  // Initialize the radio module
  printf("[SX1262] Initializing ... ");
  int state = radio.begin(915.0, 125.0, 7, 5, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 0.0, false);
  if (state != RADIOLIB_ERR_NONE) {
    printf("failed, code %d\n", state);
    return(1);
  }
  printf("success!\n");
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0){
      perror("Socket creation error");
      return -1;
  }

  cout << "Socket created successfully." << endl;

  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if(inet_pton(AF_INET, "10.40.115.112", &serv_addr.sin_addr) <= 0){ //change this to end destination IP
      perror("Invalid address / Address not supported");
      return -1;
  }

  if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
      perror("Connection Failed");
      return -1;
  }

  cout << "Connected to server successfully" << endl;


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
  while(true) {
    if(receivedFlag) {
      // Reset flag
      receivedFlag = false;
      
      // Get the length of the received packet
      size_t len = radio.getPacketLength();
      
      // Read the packet
      int state = radio.readData(buff, len);
      if (state != RADIOLIB_ERR_NONE) {
        printf("Read failed, code %d\n", state);
      } else {
        printf("Data: %s\n", (char*)buff);
        send(sock, buff, 256, 0);
        cout << "GPS data successfully received and sent over TCP/IP" << endl;
        char buf[1024] = {0};
        read(sock, buf, 1024);
        cout << "Message from server: " << buf << endl;
      }
    }
    hal->delay(500);
  }

  return(0);
}
