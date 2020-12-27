#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT 8000
#define MAX_LINE 4096
#define BUFFSIZE 4096

ssize_t total = 0;
int main(int argc, char const *argv[])
{
    // declaring server file descriptor
    int server_fd;

    // stores file descriptor to use for a single accepted connection
    int new_socket;

    // to store return value of read
    int valread;

    // IPv4 socket address structure
    struct sockaddr_in address;

    int opt = 1;
    int addrlen = sizeof(address);

    // char buffer[1024] = {0};
    // char *hello = "Hello from server";

    // Creating socket file descriptor
    // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    // SOL_SOCKET is the socket layer itself
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }


    // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc. 
    address.sin_family = AF_INET;
    // Accept connections from any IP address - listens from all interfaces.
    address.sin_addr.s_addr = INADDR_ANY; // 32 bit IPv4 address
    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian
    address.sin_port = htons( PORT ); // 16 bit TCP or UDP port number


    // bind->listen->accept->read->send


    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }


    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()

    // The listen() function converts an unconnected socket into a passive socket, 
    // indicating that the kernel should accept incoming connection requests directed to this socket.
    if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        // printf("loop\n");
        // retrieve a connect request and convert that into a request
        // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        while(1)
        {
            char filename[BUFFSIZE] = {0}; 
            
            if (read(new_socket, filename, BUFFSIZE) == -1)
                perror("Couldn't receive command");

            if(strcmp(filename, "exit") == 0)
            {
                printf("Client exited.\n");
                break;
            }

            // printf("%s\n", filename);
            FILE* fd = fopen(filename, "r"); 
          
            // checking if the file exist or not 
            if (fd == NULL) 
            {
                char conf[9] = "notfound";
                char buf[BUFFSIZE] = {0};
                strncpy(buf, conf, strlen(conf));
                if (send(new_socket, buf, BUFFSIZE, 0) == -1)
                    perror("Couldn't send file confirmation message.\n");
                continue;
            }
            else
            {
                char conf[6] = "found";
                char buf[BUFFSIZE] = {0};
                strncpy(buf, conf, strlen(conf));
                if (send(new_socket, buf, BUFFSIZE, 0) == -1)
                {
                    perror("Couldn't send file confirmation message.\n");
                    continue;
                }
            }

            // getting size of file
            fseek(fd, 0L, SEEK_END);
            long int sz = ftell(fd);
            fclose(fd);

            // opening file
            int fp = open(filename, O_RDONLY);
            if (fp == -1) 
            {
                perror("Couldn't open file");
                continue;
            }
            
            ssize_t n;
            ssize_t total = 0;

            // printf("size: %d", sz);
            // sending file size to client
            if (write(new_socket, &sz, sizeof(sz)) == -1) 
            {
                perror("write");
                break;
            }

            char buffer[BUFFSIZE] = {0}; 

            // reading from file and sending to client
            while (1) 
            {
                n = read(fp, buffer, BUFFSIZE);
                if (n == 0)
                    break;
                if (n == -1) 
                {
                    perror("read");
                    break;
                }
                if (write(new_socket, buffer, n) == -1) 
                {
                    perror("write");
                    break;
                }
            }
            close(fp);
        }
        close(new_socket);
    }
    return 0;
}

