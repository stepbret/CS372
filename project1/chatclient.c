/*********************************************************************
 *                      chatclient
 * Description: Takes a hostname and port from the command line
 *              uses the the info to open a connection with the server.
 *              will continue until 'exit' is scanned in
 * Name: Brett Stephenson
 * Date: 10/30/2016
 * ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h> 
#include <arpa/inet.h>

#define MAXLENGTH 501
//#define PORT 9009
#define HANDLESIZE 10 // max number of bytes allowed in client's handle

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/****************************************************************************
 *                          setupConnectio
 * Description: This is pretty much straight out of beej's guide to networking
 *              Takes the hostname passed and the port number starts the
 *              connection
 * Source: beej.us/guide/bgnet
 * **************************************************************************/
int setupConnect(char* hostname, char* portno){
        int sockfd;
    char buf[MAXLENGTH];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN]; 
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname, portno, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    //couldn't connect
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        exit(1);
    }

    //found a connection
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure
        
        return sockfd;
}

void chat(int socket_fd, char* handle){
        char buf[MAXLENGTH];
        int numbytes;
        int quit;
        while(1){
        
                //send the information tp a
                //Attach the handle to the beginning of the message requires a little
                //manipulation
                char handleMes[MAXLENGTH + 12];
                memset(buf, 0, MAXLENGTH);
                printf("%s> ", handle);
                fgets(buf, MAXLENGTH-1, stdin);
                quit = strncmp(buf, "\\quit", 5);
                printf("Value of quit: %d\n", quit);
                strcpy(handleMes, handle);
                strcat(handleMes, "> ");
                strcat(handleMes, buf);
                if (quit == 0){
                        if(send(socket_fd, "Connection closed by Client\n", 28, 0) == -1){
                                perror("send");
                                exit(1);
                        }
                        close(socket_fd);
                        exit(0);
                }
                else{
                        if(send(socket_fd, handleMes, strlen(handleMes), 0) == -1){
                                perror("send");
                                exit(1);
                        }
                }
                
                //receive
                if ((numbytes = recv(socket_fd, buf, MAXLENGTH, 0)) == -1) {
                        perror("recv");
                        exit(1);
                }
                if (strncmp(buf, "Connection closed by Server", 27) == 0){
                        printf("%s\n", buf);
                        close(socket_fd);
                        exit(0);
                }
                buf[numbytes] = '\0';
                printf("Server> %s\n",buf);
        }
        return;
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXLENGTH];
    char handle[HANDLESIZE];
        size_t ln;
    
    if (argc != 3) {
        fprintf(stderr,"usage: client hostname portnumber\n");
        exit(1);
    }

        //connect to Server
        sockfd = setupConnect(argv[1], argv[2]);

        //get handle and send to Server
        printf("Please enter a handle up to %d characters: ", HANDLESIZE);
        fgets(handle, HANDLESIZE, stdin);
        ln = strlen(handle) - 1;
        if (handle[ln] == '\n')
    handle[ln] = '\0';
    if(send(sockfd, handle, strlen(handle), 0) == -1){
                perror("send");
                exit(1);
        }
        
        //Begin chat
        printf("\nWait for prompt to begin typing message\n");
        printf("Type '\\quit' to quit\n\n");
        chat(sockfd, handle);      

    close(sockfd);

    return 0;
}
