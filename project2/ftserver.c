/************************************************************************************
 *                              ftserver
 * Name: Brett Stephenson
 * desc: this program acts as a simple server for file transfers
 * Class: CS372 Intro to networks
 * Sources: Lectures and slides, Beej's guide to Networking, also some suggestions
 *          Stackoverflow.com
 * **********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>

/***********************************************************************************
 *                      getrequest
 * Description: Validates user input to be valid. Either '-g' or '-l' and sends
 *              it to the client
 **********************************************************************************/
void getRequest(int sock, char* cmd, char* fname, char *response);

/**********************************************************************************
 *                      dataTransfer
 * Description: Transfers requested data over the specified connection
 **********************************************************************************/
void dataTransfer(int sock, char* comm, char* fname, int portnum, char* hostName);


//The connection is set up in main
//most of the code sourced for the connection comes from beej's guide
int main(int argc, char *argv[]){
        int sockfd, newsockfd, datasockfd, portno, pid; 
        socklen_t clilen;
        struct sockaddr_in serv_addr2; 
        struct sockaddr_in serv_addr, cli_addr;
        struct hostent *server;
        char userCMD[10] = ""; 
        char dportno[10] = "";
        char command[10] ="";
        char fileName[50]= "";
        char hostname[1024];

       //proper usage 
         if (argc < 2) {
                fprintf(stderr,"Error: PORT NUMBER NOT SPECIFIED. EXITING.\n");
                exit(1);
         }

        while(1){
                sockfd = socket(AF_INET, SOCK_STREAM, 0);       
                if (sockfd < 0) {
                        fprintf(stderr, "ERROR: Opening socket, EXITING.\n");
                        exit(1);
                }

                bzero((char *) &serv_addr, sizeof(serv_addr));

                //get and store the portnumber
                portno = atoi(argv[1]); 

                int yes=1;
                if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
                        fprintf(stderr, "setsockopt\n");
                        exit(1);
                }

                //sets up server info
                serv_addr.sin_family = AF_INET; 
                serv_addr.sin_addr.s_addr = INADDR_ANY;
                serv_addr.sin_port = htons(portno);

                if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) { 
                        fprintf(stderr, "ERROR: on binding.\n");
                        exit(1);
                }
                //waiting for connection 
                listen(sockfd,10); 
                printf("Listening on port: %d", portno);
                clilen = sizeof(cli_addr); 
               
                //established connection 
                newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

                if (newsockfd < 0) {
                        fprintf(stderr, "ERROR: on accept. Connection closed. EXITING.\n");
                        close(newsockfd); // Close upon error and exit.
                        close(sockfd);
                        exit(1);
                }

                printf("Client connected to port: %d\n", portno);
                
                //get the data port number
                int x=0;
                x=read(newsockfd,dportno,sizeof(dportno));
                if (x < 0){
                        fprintf(stderr, "ERROR: reading from socket.\n");
                        close(newsockfd);
                        exit(1);
                }
        
                x = write(newsockfd,"Recieved Confirmed.",20);
                if (x < 0){
                        fprintf(stderr, "ERROR: writing to socket: %d\n", x);
                        close(newsockfd);
                        exit(1);
                }
                printf("Data transfer connection ready on port: %s\n", dportno);

                //gets the info and opens a new connection
                getRequest(newsockfd, command, fileName, userCMD); 
                if(strcmp(userCMD, "t") == 0){
                        int data_port_number = atoi(dportno);  
                        datasockfd = socket(AF_INET, SOCK_STREAM, 0); 

                        if (datasockfd < 0) {
                                fprintf(stderr,"ERROR: opening socket.\n");
                                exit(1);
                        }

                        //gets the hostname and sets up the next connection
                        gethostname(hostname, 1024); 
                        server = gethostbyname(hostname);
                        if (server == NULL) {
                                fprintf(stderr,"ERROR: no such host.\n");
                                exit(1);
                        }

                        bzero((char *) &serv_addr2, sizeof(serv_addr2)); 
                        serv_addr2.sin_family = AF_INET; 

                        bcopy((char *)server->h_addr, (char *)&serv_addr2.sin_addr.s_addr, server->h_length);
                        serv_addr2.sin_port = htons(data_port_number); 

                        //checking if anything went wrong
                        //closes the connections
                        //the server still listens for connections
                        if (connect(datasockfd,(struct sockaddr *) &serv_addr2,sizeof(serv_addr2)) < 0) { 
                                fprintf(stderr,"ERROR: connecting data connection.\n");
                                close(datasockfd);
                                close(newsockfd);
                                close(sockfd);
                                exit(1);
                        }
                        
                        dataTransfer(datasockfd, command, fileName, data_port_number, hostname);
                        close(datasockfd);  
                        close(sockfd);
                }else{
                        close(newsockfd);
                        close(sockfd);}
        }
        
        return 0;
}

void getRequest(int sock, char* cmd, char* fname, char *response){
        int n;
        n = read(sock,cmd,10);
        if (n < 0) {
                fprintf(stderr, "ERROR: reading from socket.\n");
                close(sock);
                exit(1);
        }

        //if the command is list
        if((strcmp(cmd, "-l") == 0)){ 
                n = write(sock,"ok",3);
                if (n < 0) { 
                        fprintf(stderr, "ERROR: writing to socket.\n");
                        close(sock);
                        exit(1);
                }
                strcpy(response, "t");
        }

        //if the command is get
        else if(strcmp(cmd, "-g") == 0){
                n = write(sock,"ok",3);
                if (n < 0) { 
                        fprintf(stderr, "ERROR: writing to socket.\n");
                        close(sock);
                        exit(1);
                }

                //get the file the user wants to get
                n = read(sock,fname,50); 
                if (n < 0) {
                        fprintf(stderr, "ERROR: reading from socket.\n");
                        close(sock);
                        exit(1);
                }
                n = write(sock,"ok",3);
                if (n < 0) { 
                        fprintf(stderr, "ERROR: writing to socket.\n");
                        close(sock);
                        exit(1);
                }
                strcpy(response, "t");

                //if it is not one of those commands send back bad command
        }else {         
                n = write(sock,"INVALID COMMAND.",17);
                if (n < 0) { 
                        fprintf(stderr, "ERROR: writing to socket.\n");
                        close(sock);
                        exit(1);
                }
                strcpy(response, "f");
        }
}

void dataTransfer(int sock, char* comm, char* fname, int portnum, char* hostName){
        char myDir[1024] = ""; 
        DIR *dir;       
        struct dirent *ent;

        int n;

        //if the command is list, then format the output 
        if((strcmp(comm, "-l") == 0)){
                char newLine[2] = "\n"; 
                printf("Sending a list of contents to port:  %d\n", portnum);

                char cwd[1024];
                getcwd(cwd, sizeof(cwd));

                if ((dir = opendir (cwd)) != NULL) {
                        while ((ent = readdir (dir)) != NULL) {
                                strcat(myDir, ent->d_name); 
                                strcat(myDir, newLine); // Concat new line char to end of C-String.
                        }

                        closedir (dir);

                        n = write(sock,myDir,strlen(myDir));

                        //error checking
                        if (n < 0){  
                                fprintf(stderr, "ERROR: writing to Data socket: %d\n", n);
                                exit(1);
                        }
                }
                
        }else{
                //finding the existing file
                FILE *myFD;

                myFD = fopen(fname,"r");

                //problem if it was not found
                if(myFD==0){ 
                        char badResponse[20];
                        n = write(sock,"FILE NOT FOUND",15);

                        if (n < 0) { 
                                fprintf(stderr, "ERROR: writing to Data socket: %d\n",n);
                                close(sock);
                                exit(1);
                        }

                        n = read(sock,badResponse,20);
                        if (n < 0) {
                                fprintf(stderr, "ERROR: reading from socket.\n");
                                close(sock);
                                exit(1);
                        }
                }else{
                       //otherwise go through with the transfer
                        char okResponse[20];
                        n = write(sock,"Sending File",13);

                        if (n < 0) { 
                                fprintf(stderr, "ERROR: writing to socket: %d\n",n);
                                close(sock);
                                exit(1);
                        }

                        n = read(sock,okResponse,20);
                        if (n < 0) {
                                fprintf(stderr, "ERROR: reading from socket: %d\n",n);
                                close(sock);
                                exit(1);
                        }
                        printf("Sending \"%s\" to Client:%d\n", fname, portnum);
                        
                        while(1){ 
                                unsigned char buff[1024]={0};

                                int nread = fread(buff,1,1024,myFD);      
                                if(nread > 0){
                                        write(sock, buff, nread); // Writing to client in 1024 byte portions.
                                }
                                if (nread < 1024){
                                        if (feof(myFD)){
                                                break; // Break from loop at EOF.
                                        }
                                        if (ferror(myFD)){
                                                fprintf(stderr, "ERROR: reading file.\n");
                                                break;
                                        }
                                }
                        }
                }
        }

        close(sock);
}
