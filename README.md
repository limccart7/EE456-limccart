# EE456  
# LoRa 1262 Project  

**Class:** EE456  
**Project Name:** LoRa 1262 GPS Data Transmission

This project leverages the SX1262 LoRa module and Raspberry Pi to transmit GPS data wirelessly using LoRa technology. The data is then forwarded via TCP/IP to another Raspberry Pi, where it is stored for further use.

***********VERY IMPORTANT***********  

- Enable SPI before attempting to transmit or receive.  
Run the following commands:  
`sudo raspi-config`  
Navigate to `Interfacing Options` -> `SPI` and enable it.  
Reboot your Raspberry Pi.  
**********************************************************  

## Installation Instructions  

--Run  `sudo apt update && sudo apt install -y cmake git`  
1. Enable SPI on the Raspberry Pi:   
   Run the following commands to enable SPI:   
   `sudo raspi-config`   
   Navigate to Interfacing Options -> SPI and enable it. Reboot   
   your Raspberry Pi.   
2. Install Required Dependencies:   
   Update your system and install necessary tools:   
   `sudo apt update && sudo apt install -y cmake git`   
3. Clone the GitHub Repository for EE456:   
`git clone https://github.com/limccart7/EE456.git`   
`cd EE456`   
  
Navigate into the 'EE456' directory.  

4. Navigate to 'WiringPi' directory:  
Run:  
`./build`  
  

5. Navigate to 'RadioLib/examples/NonAudrino/Raspberry/'  
Run:  
`mkdir build`  
`cd build`  
`cmake ..`  
`make`    

4. Make and build the Project:  
From EE456 directory run:  
`mkdir build`  
Navigate to the `build` directory:    
`cd build`  
Run:  
`cmake ../src`  **(cmake must be ran after any changes to CMakeLists.txt such as adding a new executable!)**  
Then run:  
`make` **(make must be ran after all changes to .cpp files)**  


## Wiring Diagram for WiringPi Library & SX1262 LORAWAN Pi Hat  

- NSS: GPIO 21, WPI# 29  
- Reset: GPIO 18, WPI# 1  
- DI01: GPIO 16, WPI# 27  
- Busy: GPIO 20, WPI# 28  

Ensure your wiring matches the above configuration for proper operation with the RPI and SX1262 LORAWAN Pi Hat.  

## Running the System  

1. **Start the GPS Transmitter**:  
On the transmitting Raspberry Pi, run the GPS transmission script:     
`sudo ./lora_gps_tx`  

2. **Start the Lora Receiver/Client**:  
On the receiving Raspberry Pi, initialize the LoRa receiver/Client:  
`sudo ./rx_client`  
Check for errors and verify successful reception.   

3. **Start the TCP/IP Server**:   
On the third Raspberry Pi, run the server script to receive and store data:   
`sudo ./server`   

## Troubleshooting   
- SPI Issues:   
  Ensure SPI is enabled on all Raspberry Pis. Reboot after enabling SPI.   
- Transmission Errors:   
  Verify the wiring and power connections for the SX1262 LoRa module.   
- TCP/IP Connection Errors:   
  Check the IP addresses and ports in the client and server scripts.   
  Ensure all devices are on the same network.  

## Additional Resources  

- [RadioLib Documentation](https://jgromes.github.io/RadioLib/index.html)  
- [RadioLib GitHub Repository](https://github.com/jgromes/RadioLib)  
- [WiringPi Pinout](https://pinout.xyz/pinout/wiringpi)  
- [WiringPi GitHub Repository](https://github.com/WiringPi/WiringPi)  
- [Waveshare SX1262 LoRaWAN Hat](https://www.waveshare.com/sx1262-lorawan-hat.htm)  
- [Semtech SX1262 Product Page](https://www.semtech.com/products/wireless-rf/lora-connect/sx1262)  
