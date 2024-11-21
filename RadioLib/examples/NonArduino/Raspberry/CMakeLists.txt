cmake_minimum_required(VERSION 3.18)

# create the project
project(rpi-sx1262)

# add the RadioLib source directory
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../../../RadioLib" "${CMAKE_CURRENT_BINARY_DIR}/RadioLib")

# add the executable
add_executable(${PROJECT_NAME} main.cpp)

# find and link the WiringPi library
find_library(WIRINGPI_LIB wiringPi)

if(NOT WIRINGPI_LIB)
  message(FATAL_ERROR "WiringPi library not found")
endif()

# link the libraries
target_link_libraries(${PROJECT_NAME} RadioLib ${WIRINGPI_LIB})

# include directories for RadioLib
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../RadioLib/src)