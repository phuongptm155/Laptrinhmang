// Server1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "winsock2.h"

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

	fd_set fdread;
	int ret;

	SOCKET clients[1024];
	int numClients = 0;

	char buf[256];

	timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	while (1)
	{
		FD_ZERO(&fdread);
		FD_SET(listener, &fdread);
		for (int i = 0; i < numClients; i++)
			FD_SET(clients[i], &fdread);

		ret = select(0, &fdread, NULL, NULL, &tv);
		if (ret == SOCKET_ERROR)
		{
			printf("Error\n");
			break;
		}
		else if (ret == 0)
		{
			printf("Timed out!\n");
		}
		else if (ret > 0)
		{
			if (FD_ISSET(listener, &fdread))
			{
				SOCKET client = accept(listener, NULL, NULL);
				printf("New client accepted: %d\n", client);

				clients[numClients] = client;
				numClients++;
			}
			
			for (int i = 0; i < numClients; i++)
				if (FD_ISSET(clients[i], &fdread))
				{
					ret = recv(clients[i], buf, sizeof(buf), 0);
					if (ret <= 0)
					{
						// Xoa client khoi mang clients
						printf("Loi ket noi!");
					}
					else
					{
						buf[ret] = 0;
						printf("Received %d: %s\n", clients[i], buf);
					}
				}
		}
	}

	closesocket(listener);
	WSACleanup();
    return 0;
}

