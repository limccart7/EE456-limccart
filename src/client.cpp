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

void parseNMEA(const std::string &line, int sock) {
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
            send(sock, gps_data, strlen(gps_data), 0);
            cout << "GPS data successfully sent" << endl;

            char buf[1024] = {0};
            read(sock, buf, 1024);
            cout << "Message from server: " << buf << endl;
            } 
        }
    }
}

int main(){
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        perror("Socket creation error");
        return -1;
    }

    cout << "Socket created successfully." << endl;

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "10.40.127.139", &serv_addr.sin_addr) <= 0){
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

    std::string buffer;
    char c;
    while (true) {
        // Read one character at a time
        if (read(serial_port, &c, 1) > 0) {
            if (c == '\n') {  // End of line detected
                parseNMEA(buffer, sock);  // Parse the complete line
                buffer.clear();     // Clear the buffer for the next line
            } else {
                buffer += c;  // Append character to buffer
            }
        }
    }

    close(serial_port);
    close(sock);
    return 0;
}
