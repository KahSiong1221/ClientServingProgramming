#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Practical.h"
#include <unistd.h>
#include <sys/stat.h>

#define HOME_PAGE "HTTP/1.0 200 File Found\r\nContent-Length: 131 \r\nConnection: close\r\nServer: httpserver\r\n\r\n<HTML><HEAD><TITLE>File Found</TITLE></HEAD><BODY><h2>FILE Found</h2><hr><p>Your requested INDEX FILE was found.</p></BODY></HTML>"

#define ERROR_PAGE "HTTP/1.0 404 File Not Found\r\nContent-Length: 142\r\nConnection: close\r\n\r\n<HTML><HEAD><TITLE>File NOT Found</TITLE></HEAD><BODY><h2>FILE NOT Found</h2><hr><p>Your requested INDEX FILE was NOT found.</p></BODY></HTML>"

static const int MAXPENDING = 3; // Maximum outstanding connection requests

int main(int argc, char *argv[]) {
	int recvLoop = 0;
	int numBytes = 0;
	int totalBytes = 0;
	char sendbuffer[BUFSIZE]; // Buffer for sending data to the client 
	char recvbuffer[BUFSIZE];
	char uri[200] = {""};
	char discard1[50];
	char discard2[50];
	
	if (argc != 2) // Test for correct number of arguments
		DieWithUserMessage("Parameter(s)", "<Server Port>");

	in_port_t servPort = atoi(argv[1]); // First arg:  local port

		// Create socket for incoming connections
	int servSock; // Socket descriptor for server
	if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithSystemMessage("socket() failed");

	// Construct local address structure
	struct sockaddr_in servAddr;                  // Local address
	memset(&servAddr, 0, sizeof(servAddr));       // Zero out structure
	servAddr.sin_family = AF_INET;                // IPv4 address family
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
	servAddr.sin_port = htons(servPort);          // Local port

	// Bind to the local address
	if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
		DieWithSystemMessage("bind() failed");

	// Mark the socket so it will listen for incoming connections
	if (listen(servSock, MAXPENDING) < 0)
		DieWithSystemMessage("listen() failed");

	for (;;) { // Infinite for loop; runs forever
    
    		// Wait for a client to connect
    		int clntSock = accept(servSock, (struct sockaddr *) NULL, NULL);
    		
		if (clntSock < 0)
      			DieWithSystemMessage("accept() failed");
		
		recvLoop = 1;
		totalBytes = 0;
		memset(uri, '\0', sizeof(uri));
		memset(sendbuffer, '\0', sizeof(sendbuffer));
		memset(recvbuffer, '\0', sizeof(recvbuffer));

		while (recvLoop > 0) {
			
			// note the one byte limitation in argument three
			numBytes = recv(clntSock, (recvbuffer + totalBytes), 1, 0);
			// updating the off-set
			totalBytes += numBytes;
			// do not exceed the size of the recvbuffer OR looking for "\r\n\r\n"
			if ((totalBytes >= (BUFSIZE - 2)) || (strstr(recvbuffer, "\r\n\r\n") > 0))
				recvLoop = 0;
		}

		if (numBytes < 0)
			DieWithSystemMessage("recv() failed");
		
		// parsing the incoming stream
		sscanf(recvbuffer, "%s %s %s\r\n", discard1, uri, discard2);

		if (strcmp(uri, "/favicon.ico") == 0) {
			
			printf("\n\nFound and ignored favicon.ico\n\n");
			close(clntSock);
		}
		else {
			
			recvbuffer[totalBytes] = '\0';
			fputs(recvbuffer, stdout);
			
			if (strcmp(uri, "/index.html") == 0) {
				snprintf(sendbuffer, sizeof(sendbuffer), HOME_PAGE);
			}
			else {
				snprintf(sendbuffer, sizeof(sendbuffer), ERROR_PAGE);
			}

			ssize_t numBytesSent = send(clntSock, sendbuffer, strlen(sendbuffer), 0);

			if (numBytesSent < 0)
				DieWithSystemMessage("send() failed");

			close(clntSock);
		}
	}
}
