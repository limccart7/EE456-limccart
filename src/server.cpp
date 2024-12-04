#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

using namespace std;

#define PORT 8080

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("Socket creation failed!");
        exit(EXIT_FAILURE);
    }

    cout << "Socket created successfully" << endl;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Binding failed!");
        exit(EXIT_FAILURE);
    }

    cout << "Bound to port successfully." << endl;

    if (listen(server_fd, 3) < 0) {
        perror("Listening failed!");
        exit(EXIT_FAILURE);
    }

    cout << "Listening for connections..." << endl;

    int new_socket;
    socklen_t addrlen = sizeof(addr);

    new_socket = accept(server_fd, (struct sockaddr *)&addr, &addrlen);

    if (new_socket < 0) {
        perror("Error accepting connection!");
        exit(EXIT_FAILURE);
    }

    cout << "Connection accepted" << endl;

    // Continuous communication loop
    while (true) {
        char buf[1024] = {0};
        ssize_t bytes_read = read(new_socket, buf, sizeof(buf) - 1);

        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                cout << "Client disconnected." << endl;
            } else {
                perror("Error reading from socket");
            }
            break; // Exit loop if client disconnects or an error occurs
        }

        cout << "Message from Client: " << buf << endl;

        const char *reply = "Message Ack";
        send(new_socket, reply, strlen(reply), 0);
        cout << "Reply sent." << endl;
    }

    close(new_socket);
    close(server_fd);

    return 0;
}
