
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>

#define BACKLOG 1
#define MAXDATASIZE 500
#define MAXFILENAMELENGTH 10000
#define CMDSTRSIZE 100
#define PARAMS 3  //Number of command arguements received from client <cmd> <dataport> <hostname>

int createBindSocket(int sockfd, struct addrinfo *servinfo, int yes){
  //Step2: Create Socket
  if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,servinfo->ai_protocol)) == -1) {
    perror("server: socket");
    exit(1);
  }

  //Alt Step: Reuse socket if socket hasn't completed shut down from previous use.
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }

  //Step3: Bind (associate) socket with port
  if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
    close(sockfd);
    perror("server: bind");
    exit(1);
  }
  return sockfd;
}

void listen4Connection(int sockfd, char *port){
  //Step4: Listen on port for incoming connections
  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }
  printf("Server open on %s\n", port);
}

void printConnectionInfo(char commandStr[PARAMS][CMDSTRSIZE]){
  if(strcmp(commandStr[1], "-l") == 0){
    printf("Connection from %s.\n", commandStr[0]);
    printf("List directory requested on port %s.\n", commandStr[2]);
    printf("Sending directory contents to %s: %s\n", commandStr[0], commandStr[2]);
  }
  else{
  }
}

int openDataConnection(char commandStr[PARAMS][CMDSTRSIZE]){
  int rv, sock_fd, numbytes1;
  struct addrinfo hints1, *result1;
  memset(&hints1, 0, sizeof hints1);
  hints1.ai_family = AF_INET;  //Use IPv4
  hints1.ai_socktype = SOCK_STREAM;
  if ((rv = getaddrinfo(commandStr[0], commandStr[2], &hints1, &result1)) != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
  }
  //Create the socket, if error creating socket exit
  if ((sock_fd = socket(result1->ai_family, result1->ai_socktype, result1->ai_protocol)) == -1){
    perror("Socket Creation Error: ");
  }
  //Create the connection, if error creating connection exit
  if (connect(sock_fd, result1->ai_addr, result1->ai_addrlen) == -1) {
    close(sock_fd);
    perror("Connection Creation Error: ");
  }
  return sock_fd;
}

//https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
int getDirectory(char* fileList){
  struct dirent *de;  // Pointer for directory entry
  DIR *dr = opendir(".");  // opendir() returns a pointer of DIR type.
  if (dr == NULL){  // opendir returns NULL if couldn't open directory
    printf("Could not open current directory" );
    return 1;
  }
  while ((de = readdir(dr)) != NULL){
    if (strcmp(de->d_name,".") != 0 && strcmp(de->d_name,"..") != 0){
      strcat(fileList, de->d_name);
      strcat(fileList, "\n");
    }
  }
  closedir(dr);
  return 0;
}

void handleRequest(int sock_fd, char commandStr[PARAMS][CMDSTRSIZE]){
  int numbytes0;
  char fileList[MAXFILENAMELENGTH];
  memset(fileList, '\0', sizeof(fileList));
  getDirectory(fileList);
  if(strcmp(commandStr[1], "-l") == 0){
    if ((numbytes0 = send(sock_fd, fileList, strlen(fileList), 0)) == -1) {
      send(sock_fd, fileList, strlen(fileList), 0);
      perror("send: ");
      exit(1);
    }
  }
  close(sock_fd);
}

int main(int argc, char *argv[]) {
  int sockfd, new_fd, sock_fd;  // listen on sockfd, new connection on new_fd
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  char commandStr[PARAMS][CMDSTRSIZE];
  char* token;
  char* rest;
  int yes = 1;
  int rv, numbytes;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;  //Use IPv4
  hints.ai_socktype = SOCK_STREAM;

  if(argc != 2){
    fprintf(stderr, "Arguement count is incorrect.  Must enter: ./ftserver <portnumber>\n");
    exit(1);
  }

  //Step1: Create pointer (servinfo) to linked list containing address info
  if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  //Step 2-3:  Create Socket and bind to port
  sockfd = createBindSocket(sockfd, servinfo, yes);
  //Step4: Listen for incoming connections
  listen4Connection(sockfd, argv[1]);

  while(1){  //while loop allows the software to continue to accept new connections on the server port
    sin_size = sizeof their_addr;
    if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1){
      perror("accept");
      continue;
    }
    if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
      perror("recv");
      exit(1);
    }
    buf[numbytes] = '\0';
    rest = buf;
    int i = 0;
    while ((token = strtok_r(rest, " ", &rest))){
      memset(commandStr[i], '\0', sizeof(commandStr[i]));
      strcpy(commandStr[i], token);
      i++;
    }
    printConnectionInfo(commandStr);
    sock_fd = openDataConnection(commandStr);
    handleRequest(sock_fd, commandStr);
  }
  return 0;
}
