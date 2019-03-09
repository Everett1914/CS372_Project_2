
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
#define BLENGTH 1024
#define MAXDATASIZE 500
#define MAXFILENAMELENGTH 100
#define CMDSTRSIZE 100
#define PARAMS 4  //Number of command arguements received from client <cmd> <dataport> <hostname>

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
  char flip[6];
  memset(flip, '\0', sizeof(flip));
  strncpy(flip, commandStr[0], 5);
  if(strcmp(commandStr[1], "-l") == 0){
    printf("Connection from %s.\n", flip);
    printf("List directory requested on port %s.\n", commandStr[2]);
    printf("Sending directory contents to %s: %s\n", commandStr[0], commandStr[2]);
  }
  else{
    printf("Connection from %s.\n", flip);
    printf("File %s requested on port %s.\n", commandStr[2], commandStr[3]);
    printf("Sending %s to %s: %s\n", commandStr[2], flip, commandStr[3]);
  }
}

int openDataConnection(char commandStr[PARAMS][CMDSTRSIZE]){
  int rv, sock_fd, numbytes1;
  struct addrinfo hints1, *result1;
  memset(&hints1, 0, sizeof hints1);
  hints1.ai_family = AF_INET;  //Use IPv4
  hints1.ai_socktype = SOCK_STREAM;
  if(strcmp(commandStr[1], "-l") == 0){
    if ((rv = getaddrinfo(commandStr[0], commandStr[2], &hints1, &result1)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
  }
  else if(strcmp(commandStr[1], "-g") == 0){
    if ((rv = getaddrinfo(commandStr[0], commandStr[3], &hints1, &result1)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
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

//ftp://ftp.cs.umass.edu/pub/net/pub/kurose/ftpserver.c
//https://beej.us/guide/bgnet/html/multi/advanced.html#sendall
//https://stackoverflow.com/questions/2029103/correct-way-to-read-a-text-file-into-a-buffer-in-c
void handleRequest(int sock_fd, char commandStr[PARAMS][CMDSTRSIZE]){
  int numbytes0;
  if(strcmp(commandStr[1], "-l") == 0){
    char fileList[MAXFILENAMELENGTH];
    memset(fileList, '\0', sizeof(fileList));
    getDirectory(fileList);
    if ((numbytes0 = send(sock_fd, fileList, strlen(fileList), 0)) == -1) {
      perror("send: ");
      exit(1);
    }
  }
  else if(strcmp(commandStr[1], "-g") == 0){
    FILE *fp;
    char *source = NULL;
    char ch;
    fp = fopen(commandStr[2] , "r");
    if(fp == NULL){
      printf("Error Opening file\n");
      exit(1);
    }
    if (fp != NULL) {
      if (fseek(fp, 0L, SEEK_END) == 0) {
          long bufSize = ftell(fp);
          if (bufSize == -1) {
              perror("Invalid file");
              exit(1);
          }
          /* Allocate our buffer to that size. */
          source = malloc(sizeof(char) * (bufSize + 1));

          /* Go back to the start of the file. */
          if (fseek(fp, 0L, SEEK_SET) != 0) {
              perror("Unable to read file");
          }

          /* Read the entire file into memory. */
          size_t newLen = fread(source, sizeof(char), bufSize, fp);
          if ( ferror( fp ) != 0 ) {
              fputs("Error reading file", stderr);
          }
          else {
              source[newLen++] = '\0'; /* Just to be safe. */
          }
      }
    }
    int total = 0;        // how many bytes we've sent
    int len = strlen(source);
    int bytesleft = len; // how many we have left to send
    int n;
    while(total < len){
        n = send(sock_fd, source + total, bytesleft, 0);
        if (n == -1){
          perror("sendall");
          printf("We only sent %d bytes because of the error!\n", len);
          break;
        }
        total += n;
        bytesleft -= n;
    }
    memset(source, '\0', sizeof(source));
    strcpy(source, "<<stop>>");
    send(sock_fd, "<<stop>>", sizeof(source),0);
    fclose(fp);
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
  hints.ai_flags = AI_PASSIVE;

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
