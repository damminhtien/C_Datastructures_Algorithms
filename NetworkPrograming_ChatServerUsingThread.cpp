// NetworkPrograming_ChatServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "winsock2.h"
#include "time.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

DWORD WINAPI ClientThread(LPVOID lpParam);

SOCKET clients[64];
int numClients = 0;

int main(int argc, char** argv)
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	if (argc > 1) {
		// if it has command line's argument, get it 
		addr.sin_family = AF_INET;
		printf("Connecting server on IP: %s port: %d \n", (char*)argv[1], atoi((char*)argv[2]));
		addr.sin_addr.s_addr = inet_addr((char*)argv[1]);
		addr.sin_port = htons(atoi((char*)argv[2]));
	}
	else {
		// if it hasn't, get default
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		addr.sin_port = htons(9000);
	}

	// listen ip address in port
	bind(listener, (SOCKADDR *)&addr, sizeof(addr));
	listen(listener, 5);

	// while(true) - always accept if get new clients
	while (1)
	{
		SOCKET client = accept(listener, NULL, NULL);

		printf("Co ket noi moi: %d\n", client);

		for (int i = 0; i < numClients; i++)
		{
			char msg[16] = "Co ket noi moi\n";
			send(clients[i], msg, strlen(msg), 0);
		}

		clients[numClients] = client;
		numClients++;

		CreateThread(0, 0, ClientThread, &client, 0, 0);
	}

	closesocket(listener);
	WSACleanup();
	return 0;
}

// return -1 if connect error, return 0 if client disconnect
DWORD WINAPI ClientThread(LPVOID lpParam)
{
	SOCKET client = *(SOCKET *)lpParam;

	int ret;
	char buf[256];
	char name[64];

	// login: client must pass form before send data
	while (1) {
		char msg[26] = "Enter id with true form: ";
		send(client, msg, strlen(msg), 0);
		ret = recv(client, buf, sizeof(buf), 0);
		if (ret <= 0)
			return -1;
		// copy the first 11 character from buf to subbuf, in correct form, subbuf will be 'client_id: '
		buf[ret] = 0;
		char subbuf[12];
		memcpy(subbuf, &buf, 11);
		subbuf[11] = '\0';
		// if client enters true form: 11 character is 'client_id: ' and client's name is not null 
		if (strcmp(subbuf, "client_id: ") == 0 && buf[11] != '\0') {
			// get client's name from 11th character to 'ret' 
			memcpy(name, &buf[11], ret - 12);
			name[ret - 12] = '\0';
			break;
		}
	}

	while (1)
	{
		ret = recv(client, buf, sizeof(buf), 0);
		if (ret <= 0)
			break;
		buf[ret] = 0;

		// get timestamp
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		char m[256];
		sprintf(m, "%d-%d-%d %d:%d:%d %s Received %s: %s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour % 12, tm.tm_min, tm.tm_sec, tm.tm_hour < 12 ? "AM" : "PM", name, buf);
		printf("%s", m);
		
		for (int i = 0; i < numClients; i++)
			send(clients[i], m, strlen(m), 0);
	}

	closesocket(client);
	return 0;
}
