// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define PORT 8000
#define MAX_LINE 4096
#define BUFFSIZE 4096

int sock = 0;

int run = 1;

void intHandler(int a) {
    run = 0;
    char msg[5] = "exit";
    char buffe[BUFFSIZE] = {0};
    strncpy(buffe, msg, strlen(msg));
    if (send(sock, buffe, BUFFSIZE, 0) == -1)
        perror("Couldn't send exit message.\n");
    exit(0);
}

int main(int argc, char const *argv[])
{
    signal(SIGINT, intHandler);

	// IPv4 socket address structure
    struct sockaddr_in address;

    // to store return value of read
    int valread;

    // IPv4 socket address structure
    struct sockaddr_in serv_addr;

    size_t inp_sz = 0;
    size_t buffer_size = 0;
    char* inp = NULL;

    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    // to make sure the struct is empty. Essentially sets sin_zero as 0
    memset(&serv_addr, '0', sizeof(serv_addr)); 

    // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian
    serv_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a 
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // establish a connection with the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    while(1)
    {
        printf("\nclient> ");
    	inp_sz = 0;
        inp_sz = getline(&inp, &buffer_size, stdin);
        
        char *token = strtok (inp, " \n\t\r");
        if(!strcmp(token, "get") == 0 && !strcmp(token, "exit") == 0)
        {
            printf("Invalid command.\n");
            continue;
        }
        else if(strcmp(token, "exit") == 0)
        {
            char msg[5] = "exit";
            char buffe[BUFFSIZE] = {0};
            strncpy(buffe, msg, strlen(msg));
            if (send(sock, buffe, BUFFSIZE, 0) == -1)
                perror("Couldn't send exit message.\n");
            break;
        }

        token = strtok(NULL, " \n\t\r");
        if(token == NULL)
            printf("Too few arguments.");

        // getting file names one by one
        while(token != NULL)
        {
        	// printf("token : %s\n", token);
            char *filename = token; 
            ssize_t total = 0;
            ssize_t n;
            
            char buff[BUFFSIZE] = {0};
            strncpy(buff, filename, strlen(filename));
            if (send(sock, buff, BUFFSIZE, 0) == -1)
            {
                perror("Couldn't send filename");
                token = strtok(NULL, " \n\t\r");
                continue;
            }
            
            char conf[9];
            if (read(sock, conf, BUFFSIZE) == -1)
            {
                perror("Couldn't receive confirmation message");
                token = strtok(NULL, " \n\t\r");
                continue;
            }

            // printf("Confirm: %s\n", conf);
            if(strcmp(conf, "notfound") == 0)
            {
                printf("No such file in server: %s\n", filename);
                token = strtok(NULL, " \n\t\r");
                continue;
            }

            // getting filename
            filename = basename(token);
            // printf("file : %s\n", filename);
		    
            // opening a file with needed filename
            int fp = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		    if (fp == -1) 
		    {
		        perror("Can't open file");
                token = strtok(NULL, " \n\t\r");
		        continue;
		    }

		    long int sz;

            // reading file size
            if (read(sock, &sz, sizeof(sz)) == -1) 
            {
                perror("read");
                token = strtok(NULL, " \n\t\r");
                continue;
            }

            printf("size: %ld", sz);

            // reading from socket and writing to file
            // block by block till all bytes are read
            do 
            {
                n = read(sock, buffer, BUFFSIZE);

                if (n == -1) 
                {
                    perror("read");
                    break;
                }
                if (write(fp, buffer, n) == -1) 
                {
                    perror("write");
                    break;
                }

                // percentage of file downloaded
                total += n;
                int percen = (total * 100) / sz;

                printf("\r%d %% downloaded.", percen);

            } while (n > 0 && total < sz);
            printf("\n");
            close(fp);
            // getting next argument
            token = strtok(NULL, " \n\t\r");
        }
    }
    return 0;
}

