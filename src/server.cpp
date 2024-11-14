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

int main(){
    int server_fd = socket(AF_NET, SOCK_STREAM, 0);

    if(server_fd == -1){
        perror("Socket failed!");
        exit(EXIT_FAILURE);
    }

    cout << "Socket created successfully" << endl;

    struct sockaddr_in addr;

    memset(&addr, -, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;



    if(connect(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    cout << "Bound to server successfully" << endl;

    if(listen(server_fd, 3) < 0){
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    cout << "Listening for Connections" << endl;

    int new_socket;
    socketlen_t addrlen = sizeof(addr);

    new_socket = accept(server_fd, (struct sockaddr *)&addr, &addrlen);

    if(new_socket < 0){
        perror("Error accepting");
        exit(EXIT_FAILURE);
    }

    cout << "Connection accepted" << endl;

    char buf[1024] = {0};

    read(new_socket, buf, 1024);

    cout << "Message from Client: " << buf << endl;

    const char *reply = "Message Ack";

    send(new_socket, reply, strlen(reply), 0);

    cout << "Reply sent." << endl;

    close(new_socket);

    close(server_fd);

    return 0;
}
