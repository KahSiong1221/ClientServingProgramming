#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Practical.h"
#include <netdb.h>


int main(int argc, char *argv[]) {
	char sendbuffer[BUFSIZE], recvbuffer[BUFSIZE], discard[20]; // I/O buffer
	int numBytes = 0, sock; 

	if (argc < 3 || argc > 4) 
    DieWithUserMessage("Parameter(s)",
        "<Server Address> <Server Port> <HTTP Request>");

	char *host = argv[1];   // Server address/name
	char *service = argv[2];   // Server port/service
	char *httpRequest = argv[3]; // Second arg: HTTP Request
	
	struct addrinfo *servAddr; // Holder for returned list of server addrs
	int rtnVal = getaddrinfo(host, service, NULL, &servAddr);
	if (rtnVal != 0)
		DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));

	for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next) {
    // Create a reliable, stream socket using TCP

    sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (sock < 0)
      continue;  // Socket creation failed; try next address
  
    // Establish the connection to the echo server
    if (connect(sock, addr->ai_addr, addr->ai_addrlen) == 0)
      break;     // Socket connection succeeded; break and return socket

    close(sock); // Socket connection failed; try next address
    sock = -1;
  }

  freeaddrinfo(servAddr); // Free addrinfo allocated in getaddrinfo()

  if (sock < 0)
    DieWithSystemMessage("socket() failed");

	snprintf(sendbuffer, sizeof(sendbuffer), httpRequest, strlen(httpRequest)); 
  
	ssize_t numBytesSent = send(sock, sendbuffer, strlen(sendbuffer), 0);

    if (numBytesSent < 0)
      DieWithSystemMessage("send() failed");


  while ((numBytes = recv(sock, recvbuffer, BUFSIZE, 0)) > 0) {
    /* Receive up to the buffer size (minus 1 to leave space for
     a null terminator) bytes from the sender */
		recvbuffer[numBytes] = '\0';    // Terminate the string!
    	fputs(recvbuffer, stdout);      // Print the echo buffer

	}
     if (numBytes < 0)
		DieWithSystemMessage("recv() failed");
       	
	fputc('\n', stdout); // Print a final linefeed

	close(sock);
	exit(0);
}
