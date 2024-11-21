#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <RadioLib.h>
#include "PiHal.h"
#include <vector>

// Create a new instance of the HAL class
PiHal* hal = new PiHal(0);

// Create the radio module instance
// NSS pin: WiringPi 10 (GPIO 8)
// DIO1 pin: WiringPi 2 (GPIO 27)
// NRST pin: WiringPi 21 (GPIO 5)
// BUSY pin: WiringPi 0 (GPIO 17)
SX1262 radio = new Module(hal, 10, 2, 21, 0);

std::map<std::string, std::string> parseConfigFile(const std::string &filename) {
    std::map<std::string, std::string> config;
    std::ifstream infile(filename);

    if (!infile.is_open()) {
        printf("Failed to open %s\n", filename.c_str());
        return config;
    }

    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            config[key] = value;
        }
    }
    infile.close();
    return config;
}

int main() {
    // Parse the config file to get the parameters
    std::map<std::string, std::string> config = parseConfigFile("config.txt");

    // Extract parameters from the config map and convert to appropriate types
    float freq = std::stof(config["freq"]);
    float br = std::stof(config["br"]);
    float freqDev = std::stof(config["freqDev"]);
    float rxBw = std::stof(config["rxBw"]);
    int8_t power = static_cast<int8_t>(std::stoi(config["power"]));
    uint16_t preambleLength = static_cast<uint16_t>(std::stoi(config["preambleLength"]));
    float tcxoVoltage = std::stof(config["tcxoVoltage"]);
    bool useRegulatorLDO = (config["useRegulatorLDO"] == "true");

    // Initialize the radio module with parameters from the config file
    printf("[SX1262] Initializing ... ");
    int state = radio.beginFSK(freq, br, freqDev, rxBw, power, preambleLength, tcxoVoltage, useRegulatorLDO);
    if (state != RADIOLIB_ERR_NONE) {
        printf("Initialization failed, code %d\n", state);
        return 1;
    }
    printf("Initialization success!\n");

    // Set the FSK sync word (e.g., 0x12, 0x34)
    uint8_t syncWord[] = {0x12, 0x34};
    state = radio.setSyncWord(syncWord, sizeof(syncWord));
    if (state != RADIOLIB_ERR_NONE) {
        printf("Setting sync word failed, code %d\n", state);
        return 1;
    }
    printf("Sync word set successfully!\n");

    // Set the CRC configuration (1-byte CRC, polynomial 0x07, no inversion)
    state = radio.setCRC(1, 0x00, 0x07, false);
    if (state != RADIOLIB_ERR_NONE) {
        printf("Setting CRC failed, code %d\n", state);
        return 1;
    }
    printf("CRC configuration set!\n");

    // Open the file msg.txt to read the entire message
    std::ifstream infile("msg.txt", std::ios::in | std::ios::binary);
    if (!infile.is_open()) {
        printf("Failed to open msg.txt\n");
        return 1;
    }

    // Read the entire file into a string
    std::string message((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    infile.close();

    // Check if the message is larger than 229 bytes, split into chunks if necessary
    const size_t maxChunkSize = 229;
    std::vector<std::string> chunks;
    for (size_t i = 0; i < message.size(); i += maxChunkSize) {
        chunks.push_back(message.substr(i, maxChunkSize));
    }

    // Loop to send each chunk
    int count = 0;
    for (const auto& chunk : chunks) {
        printf("[SX1262] Transmitting chunk #%d ... ", count++);
        state = radio.transmit(chunk.c_str());
        if (state == RADIOLIB_ERR_NONE) {
            // The chunk was successfully transmitted
            printf("success!\n");
        } else {
            printf("failed, code %d\n", state);
        }

        // Wait for a second before transmitting the next chunk
        hal->delay(1000);
    }

    return 0;
}
