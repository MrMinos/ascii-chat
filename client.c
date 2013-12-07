#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "ascii.h"
#include "client.h"
#include "options.h"

int sockfd = 0;

void sig_handler(int signo) {
    if (signo == SIGINT) {
        printf("Closing connection to server...");
        close(sockfd);
        ascii_write_destroy();
    }
}

int main(int argc, char *argv[]) {
    options_init(argc, argv);
    char *address = opt_address;
    int   port    = (int) strtol(opt_port, (char **)NULL, 10);

    char recvBuff[40000];
    struct sockaddr_in serv_addr;

    static const struct timespec
    ASCII_SLEEP_START = {
        .tv_sec  = 3,
        .tv_nsec = 0
    },
    ASCII_SLEEP_STOP = {
        .tv_sec  = 0,
        .tv_nsec = 0
    };

    /* read from the socket as long as the size of the read is > 0 */
    while(1) {
        // Close socket on program exit.
        if (signal(SIGINT, sig_handler) == SIG_ERR) {
            printf("Couldn't catch SIGINT\n");
        }

        // try to open a socket
        memset(recvBuff, '0',sizeof(recvBuff));
        // error creating socket
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Error: could not create socket \n");
            return 1;
        }

        // reserve memory space to store IP address
        memset(&serv_addr, '0', sizeof(serv_addr));

        // set type of address to IPV4 and port to 5000
        printf("Connecting on port %d\n", port);

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        // an error occurred when trying to set server address and port number
        if(inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
            printf("\n Error: inet_pton \n");
            return 1;
        }

        int read_result = 0;
        ascii_write_init();

        printf("Attempting to connect...\n");

        // failed when trying to connect to the server
        if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("\n Error: connect failed \n");
        }

        while (0 < (read_result = read(sockfd, recvBuff, sizeof(recvBuff)-1))) {
            recvBuff[read_result] = 0;
            ascii_write(recvBuff);
        }

        ascii_write_destroy();

        nanosleep((struct timespec *)&ASCII_SLEEP_START,(struct timespec *)&ASCII_SLEEP_STOP);
    }

    return 0;
}

