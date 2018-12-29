// ChatServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "winsock2.h"

DWORD WINAPI ClientThread(LPVOID);

char* ids[64];
SOCKET clients[64];
int numClients;

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	bind(listener, (SOCKADDR *)&addr, sizeof(addr));
	listen(listener, 5);

	numClients = 0;

	while (1)
	{
		SOCKET client = accept(listener, NULL, NULL);
		CreateThread(0, 0, ClientThread, &client, 0, 0);
	}

	closesocket(listener);
	WSACleanup();
    return 0;
}

DWORD WINAPI ClientThread(LPVOID lpParam)
{
	SOCKET client = *(SOCKET *)lpParam;
	int ret;
	char buf[256];
	char sendbuf[256];

	char cmd[16], id[64], tmp[64];
	char *msg = "Sai cu phap. Hay nhap lai\n";

	char targetId[64];

	// Yeu cau client gui dung cu phap
	while (1)
	{
		ret = recv(client, buf, sizeof(buf), 0);
		if (ret <= 0)
		{
			// ngat ket noi
			closesocket(client);
			return 1;
		}

		buf[ret] = 0;
		printf("Received: %s\n", buf);

		// Kiem tra cu phap client_id: id
		ret = sscanf(buf, "%s %s %s", cmd, id, tmp);
		if (ret == 2)
		{
			if (strcmp(cmd, "client_id:") == 0)
			{
				send(client, "OK\n", 3, 0);

				ids[numClients] = id;
				clients[numClients] = client;
				numClients++;

				break;	// Thoat khoi vong lap 1
			}
			else
				send(client, msg, strlen(msg), 0);
		}
		else
			send(client, msg, strlen(msg), 0);
	}

	// Nhan va chuyen tiep tin nhan
	while (1)
	{
		ret = recv(client, buf, sizeof(buf), 0);
		if (ret <= 0)
		{
			// ngat ket noi
			// xoa phan tu khoi mang
			closesocket(client);
			return 1;
		}

		buf[ret] = 0;
		printf("Received: %s\n", buf);

		// tim id ma client muon gui
		sscanf(buf, "%s", targetId);

		sprintf(sendbuf, "%s: %s", id, buf + strlen(targetId) + 1);

		if (strcmp(targetId, "@all") == 0)
		{
			for (int i = 0; i < numClients; i++)
				if (clients[i] != client)
					send(clients[i], sendbuf, strlen(sendbuf), 0);
		}
		else
		{
			for (int i = 0; i < numClients; i++)
				if (strcmp(targetId + 1, ids[i]) == 0)
					send(clients[i], sendbuf, strlen(sendbuf), 0);
		}
	}
}