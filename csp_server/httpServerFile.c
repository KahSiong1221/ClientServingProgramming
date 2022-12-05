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

static const int MAXPENDING = 5; // Maximum outstanding connection requests

int main(int argc, char *argv[]) {
	int recvLoop = 0;
	int totalBytes = 0;
	int numBytes = 0;
	int count = 0;
	int size = 0;
	int char_in;
	char sendbuffer[BUFSIZE]; // Buffer for sending data to the client 
	char recvbuffer[BUFSIZE];
	char path[200] = {'.'};
	char discard1[50];
	char discard2[50];
	struct stat st;
	FILE * hFile;
	
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
		sscanf(recvbuffer, "%s %s %s", discard1, (path+1), discard2);

		if (strcmp(path, "./favicon.ico") == 0) {	
			printf("\n\nFound favicon.ico\n\n");
			close(clntSock);
			continue;
		}

		if (strcmp(path, "./") == 0) {
			strcpy(path, "./index.html");
		}

		hFile = fopen(path, "r");

		if (hFile == NULL) {
			strcpy(path, "./error.html");

			hFile = fopen(path, "r");

			stat(path, &st);
			size = st.st_size;

			printf("\n\nERROR.HTML File size is: %d\n\n", size);
			snprintf(sendbuffer, sizeof(sendbuffer), "HTTP/1.1 404 File Not Found\r\nContent-Length: %d\r\nCache-Control: no-cache\r\nConnection: close\r\nServer: httpserver\r\n\r\n", size);
			printf("\n\nsendbuffer contents:\n%s\n", sendbuffer);
		}
		else {
			stat(path, &st);
			size = st.st_size;

			printf("\n\nINDEX.HTML File size is: %d\n\n", size);
			snprintf(sendbuffer, sizeof(sendbuffer), "HTTP/1.1 200 Okay\r\nContent-Length: %d\r\nCache-Control: no-cache\r\nConnection: close\r\nServer: httpserver\r\n\r\n", size);
			printf("\n\nsendbuffer contents:\n%s\n", sendbuffer);
			
		}

		ssize_t numBytesSent = send(clntSock, sendbuffer, strlen(sendbuffer), 0);

		if (numBytesSent < 0)
			DieWithSystemMessage("send() failed");

		strcpy(sendbuffer, "");

		while ((char_in = fgetc(hFile)) != EOF) {
			sendbuffer[count] = char_in;
			count++;
		}

		sendbuffer[count] = '\0';

		numBytesSent = send(clntSock, sendbuffer, strlen(sendbuffer), 0);
			
		if (numBytesSent < 0)
			DieWithSystemMessage("send() failed");

		size = 0;
		count = 0;
		fclose(hFile);
		strcpy(path, ".");
		close(clntSock);
	}
}
