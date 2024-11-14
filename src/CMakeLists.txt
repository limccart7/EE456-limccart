cmake_minimum_required(VERSION 3.18)

# Create the project
project(main)

# Include the RadioLib directory
include_directories("../RadioLib/src")

# Add the executables
add_executable(lora_rx lora_rx.cpp)
add_executable(lora_gps_tx lora_gps_tx.cpp)
add_executable(lora_tx lora_tx.cpp)
add_executable(fsk_tx fsk_tx.cpp)
add_executable(fsk_rx fsk_rx.cpp)
add_executable(client_gps client.cpp)
add_executable(server server.cpp)

# Find and link the WiringPi library
find_library(WIRINGPI_LIB wiringPi)

if(NOT WIRINGPI_LIB)
  message(FATAL_ERROR "WiringPi library not found")
endif()

# Link the WiringPi library to the executables
target_link_libraries(lora_rx ${WIRINGPI_LIB})
target_link_libraries(lora_tx ${WIRINGPI_LIB})
target_link_libraries(lora_gps_tx ${WIRINGPI_LIB})
target_link_libraries(fsk_tx ${WIRINGPI_LIB})
target_link_libraries(fsk_rx ${WIRINGPI_LIB})
target_link_libraries(client_gps ${WIRINGPI_LIB})


# Include all RadioLib source files
file(GLOB_RECURSE RADIO_LIB_SOURCES "../RadioLib/src/*.cpp")
target_sources(lora_rx PRIVATE ${RADIO_LIB_SOURCES})
target_sources(lora_tx PRIVATE ${RADIO_LIB_SOURCES})
target_sources(lora_gps_tx PRIVATE ${RADIO_LIB_SOURCES})
target_sources(fsk_tx PRIVATE ${RADIO_LIB_SOURCES})
target_sources(fsk_rx PRIVATE ${RADIO_LIB_SOURCES})
