#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <conio.h>

#include <fstream>
#include <iostream>
#include <conio.h>
#define MAXDATASIZE 1033

int GetPasswordFromServer(SOCKET tcpSocket)
{
  	//send a string to the server and wait for the password
  	char *login = "login:00010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899";
  	char tempBuffer[MAXDATASIZE];
  	int bytesReceived = 0,bytesBuffered = 0;
  	char serverResponse[201];
  
  	send(tcpSocket, login, strlen(login), 0);
  
  	printf("waiting for password \n");
  	while (1)
  	{
  		//Server sends yes or no for each number. Eg 00,01,02..
  		//Only one of the numbers will be a yes
  		memset(tempBuffer, 0, MAXDATASIZE);
  		if ((bytesReceived = recv(tcpSocket, tempBuffer, MAXDATASIZE, 0)) < 0)
  			continue;
  
  		memcpy(&serverResponse[bytesBuffered], tempBuffer, strlen(tempBuffer));
  		bytesBuffered += bytesReceived;
  
  		if (bytesBuffered == 201)
  			break;
  	}
  
  	int k = 0;
  	for (k = 0; k < 201; k++)
  	{
    		//Get the password using server's reply
    		printf("Parsing input tcp stream..its a no\n");
    		if (serverResponse[k] == 'y')
    			break;
  	}
	  return  k/2;
}

int GetImageSize(SOCKET tcpSocket)
{
  	int bytesBuffered = 0, bytesReceived = 0;
  	//9 digit iamge size
  	char imageSize[9];
  	char tempBuffer[MAXDATASIZE];
  	printf("Waiting to receive Image size \n");
  	while (1)
  	{
  		//Get the image size from the server
  		memset(tempBuffer, 0, MAXDATASIZE);
  		if ((bytesReceived = recv(tcpSocket, tempBuffer, MAXDATASIZE, 0)) < 0)
  			continue;
  
  
  		memcpy(&imageSize[bytesBuffered], tempBuffer, strlen(tempBuffer));
  		bytesBuffered += bytesReceived;
  
  		if (bytesBuffered == 9)
  			break;
  	}
  	std::string imageSizeString(imageSize);
  	return std::stoi(imageSizeString);
}

void GetImage(SOCKET udpSocket, int imageSize,  sockaddr_in  destinationAddr, sockaddr_in serverAddr)
{
	int receivedImageSize = 0, bytesBuffered = 0, bytesSent = 0,  bytesReceived = 0;
	char *image = new char[imageSize];
	char *imageBuffer = new char[1024];
	char tempBuffer[MAXDATASIZE];
	socklen_t serverAddrLength = sizeof(serverAddr);

	fd_set readfds, masterfds;
	struct timeval tv;

	tv.tv_sec = 3;
	tv.tv_usec = 0;

	//For Select()
	FD_ZERO(&readfds);
	FD_ZERO(&masterfds);
	FD_SET(udpSocket, &readfds);
	FD_SET(udpSocket, &masterfds);
	
	std::string defaultStr = "00000000";
	std::string packet;
	int packetNum = 0;
	packet = defaultStr + std::to_string(packetNum);

	//strings can't be sent over sockets directly. Need a buffer. string.c_str() will work but this requires the length of
	//string be sent first and then the actual string. The server cannot handle such a transmission.
	char message[9];
	strcpy(message, packet.c_str());
	bool packetNotReceived;

	std::ofstream outputFile("output.jpg", std::ios::binary);

	while (receivedImageSize != imageSize)
	{
  		//Tell the server the next message we want. The message string will indicate the packet number.
  		//Asking for packet one tells the server that packet 0 has been received. If packet 0 was dropped,
  		//request for packet 0 had to be sent again until its received
  		if ((bytesSent = sendto(udpSocket, message, strlen(message), 0, (SOCKADDR*)&destinationAddr, sizeof(destinationAddr))) == -1)
  		{
  			wprintf(L"broadcast sendto failed with error: %d\n", WSAGetLastError());
  			getchar();
  			return;
  		}
  
  		while (1)
  		{
    			readfds = masterfds;
    			//Poll the socket for read
    			if (select(udpSocket + 1, &readfds, NULL, NULL, &tv) == -1)
    			{
    				wprintf(L"select failed with error: %d\n", WSAGetLastError());
    				getchar();
    				return;
    			}
    			//If something received
    			if (FD_ISSET(udpSocket, &readfds))
    			{
    				memset(tempBuffer, 0, MAXDATASIZE);
    				memset(imageBuffer, 0, 1024);
    
    				//Receive the message
    				if ((bytesReceived = recvfrom(udpSocket, tempBuffer, MAXDATASIZE, 0, (SOCKADDR*)&serverAddr, &serverAddrLength)) < 0)
    				{
    					wprintf(L"recvfrom failed with error: %d\n", WSAGetLastError());
    					continue;
    				}
    
    				//Only the last receive will not be 1033 bytes
    				if (bytesReceived != 1033)
    				{
    					tempBuffer[bytesReceived] = '\0';
    				}
    				int receivedNum = (int)tempBuffer[8] - 48;
    
    				bytesBuffered = bytesBuffered + (bytesReceived - 9);
    				memcpy(&image[receivedImageSize], &tempBuffer[9], (bytesReceived - 9));
    				int i;
    				int j = 0;
    				//The first nine bytes are not required
    				for (i = 9; i < (bytesReceived); i++)
    				{
    					imageBuffer[j++] = tempBuffer[i];
    				}
    				packetNum = ++receivedNum;
    				outputFile.write(imageBuffer, bytesReceived - 9);
    				packetNotReceived = false;
    				break;
    			}
    			packetNotReceived = true;
    			break;
    		}
    		if (!packetNotReceived)
    		{
    			//Update the packet number
    			receivedImageSize = bytesBuffered;
    			packet = defaultStr + std::to_string(packetNum);
    			strcpy(message, packet.c_str());
    		}
    		else
    		{
    			//Did not receive a packet. So wait for it. 
    			printf("Did not receive packet %d\n", packetNum);
    		}
  	}
  
  	outputFile.close();
}

int main()
{
  	int numericPassword = 0;
  	int bytesBuffered = 0;
  	WSADATA wsaData;
  	SOCKET udpSocket, tcpSocket;
  	struct sockaddr_in  broadcastAddr, broadcastToAddr, serverAddr, receiveAddr;
  	int broadcast = 1, bytesSent = 0, bytesReceived = 0;
  	char tempBuffer[MAXDATASIZE];
  	socklen_t serveraddr_len;
  	char serverResponse[201];
  
  	printf("\nInitialising Winsock...\n");
  	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  	{
  		  printf("Failed. Error Code : %d", WSAGetLastError());
  		  return -1;
  	}
  
  	//Create sockets required for different purposes
  	if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
  	{
  		  printf("Could not create socket : %d", WSAGetLastError());
  		  return -1;
  	}
  
  	if ((tcpSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
  	{
    		printf("Could not create socket : %d", WSAGetLastError());
    		return -1
  	}
  
  	//Bind the socket that needs to broadcast to this port
  	broadcastAddr.sin_family = AF_INET;
  	broadcastAddr.sin_port = htons(4492);
  	inet_pton(AF_INET, "127.0.0.1", &(broadcastAddr.sin_addr));
  
  	//Bind the socket which will be used for broadcasting
  	if (bind(udpSocket, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) == SOCKET_ERROR)
  	{
    		printf("ack Bind failed with error code : %d", WSAGetLastError());
    		getchar();
    		return 1;
  	}
  
  	receiveAddr.sin_family = AF_INET;
  	receiveAddr.sin_port = htons(4492);
  	inet_pton(AF_INET, "127.0.0.1", &(receiveAddr.sin_addr));
  
  	//It is known that server is listening at port 4000
  	broadcastToAddr.sin_family = AF_INET;
  	//Given port number
  	broadcastToAddr.sin_port = htons(4000);
  	inet_pton(AF_INET, "255.255.255.255", &(broadcastToAddr.sin_addr));
  	memset(broadcastToAddr.sin_zero, '\0', sizeof broadcastAddr.sin_zero);
  
  	//Set socket option
  	if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast,
  		sizeof broadcast) == -1) {
  		perror("setsockopt (SO_BROADCAST)");
  		exit(1);
  	}
  
  	//Send the message where to server
  	if ((bytesSent = sendto(udpSocket, "where", strlen("where"), 0, (SOCKADDR*)&broadcastToAddr, sizeof(broadcastToAddr))) == -1)
  	{
  		wprintf(L"broadcast sendto failed with error: %d\n", WSAGetLastError());
  		getchar();
  		return 1;
  	}
  
  	serveraddr_len = sizeof(serverAddr);
  	while (1)
  	{
    		//Get the first message from server
    		if ((bytesReceived = recvfrom(udpSocket, tempBuffer, MAXDATASIZE, 0, (SOCKADDR*)&serverAddr, &serveraddr_len)) < 0)
    		{
    			wprintf(L"recvfrom failed with error: %d\n", WSAGetLastError());
    			continue;
    		}
    		printf("Received first message \n");
    		break;
  	}
  
  	//we now know the port at which server is waiting for TCP and also the IP address of server
  	int port = ntohs(serverAddr.sin_port);
  	serverAddr.sin_family = AF_INET;
  	//Server listens for TCP at port + 1
  	serverAddr.sin_port = htons(port + 1);
  
  	if (connect(tcpSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)		//Connect to that IP = google.com
  	{
    		wprintf(L"connect failed with error: %d\n", WSAGetLastError());
    		_getch();
  	}
  	printf("Connected successfully\n");
  	
  	numericPassword = GetPasswordFromServer(tcpSocket) / 2;
  
  	std::string stringPassword = "image:";
  	stringPassword += std::to_string(numericPassword);
  
  	const char *charPassword;
  	charPassword = stringPassword.c_str();
  	//Send the password to the server
  	send(tcpSocket, charPassword, strlen(charPassword), 0);
  
  	//Turn off broadcast
  	if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast,
  		sizeof broadcast) == -1) {
  		perror("setsockopt (SO_BROADCAST)");
  		exit(1);
  	}
  
  	int imageSize = GetImageSize(tcpSocket);
  	
  	GetImage(udpSocket,imageSize,serverAddr,broadcastToAddr);
  
  	getchar();
}
