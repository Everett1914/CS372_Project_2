
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

#define BACKLOG 1
#define MAXDATASIZE 500

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

int main(int argc, char *argv[]) {
  int sockfd, new_fd;  // listen on sockfd, new connection on new_fd
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  int yes = 1;
  //char s[INET6_ADDRSTRLEN];
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
    printf("Received '%s'\n",buf);
  }

  return 0;
}
