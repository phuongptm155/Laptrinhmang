// ChatServer.cpp : Su dung mo hinh select
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
	addr.sin_port = htons(8000);

	bind(listener, (SOCKADDR *)&addr, sizeof(addr));
	listen(listener, 5);

	fd_set fdread;

	SOCKET clients[64];
	int numClients = 0;
	int ret;
	char buf[256];

	char* ids[64];
	SOCKET registeredClients[64];
	int numRegisteredClients = 0;

	char cmd[16], id[64], tmp[64];
	char *msg = "Sai cu phap. Hay nhap lai.\n";
	char targetId[64];
	char sendbuf[256];

	while (1)
	{
		FD_ZERO(&fdread);
		FD_SET(listener, &fdread);
		for (int i = 0; i < numClients; i++)
			FD_SET(clients[i], &fdread);

		ret = select(0, &fdread, NULL, NULL, NULL);
		if (ret == SOCKET_ERROR)
		{
			printf("Error!");
			break;
		}
		if (ret > 0)
		{
			if (FD_ISSET(listener, &fdread))
			{
				SOCKET client = accept(listener, NULL, NULL);
				printf("New client connected: %d\n", client);
				clients[numClients] = client;
				numClients++;
			}

			for (int i = 0; i < numClients; i++)
				if (FD_ISSET(clients[i], &fdread))
				{
					ret = recv(clients[i], buf, sizeof(buf), 0);
					if (ret <= 0)
					{
						// Xoa socket khoi mang
						continue;
					}

					buf[ret] = 0;
					printf("Received %d: %s", clients[i], buf);

					// Kiem tra clients[i] da dang nhap hay chua
					int found = 0;
					int j = 0;
					for (; j < numRegisteredClients; j++)
						if (clients[i] == registeredClients[j])
						{
							found = 1;
							break;
						}
					
					if (found == 0)	// chua dang nhap
					{
						ret = sscanf(buf, "%s %s %s", cmd, id, tmp);
						if (ret == 2)
						{
							if (strcmp(cmd, "client_id:") == 0)
							{
								send(clients[i], "OK\n", 3, 0);

								ids[numRegisteredClients] = (char *)malloc(64);
								memcpy(ids[numRegisteredClients], id, strlen(id) + 1);
								registeredClients[numRegisteredClients] = clients[i];
								numRegisteredClients++;
							}
							else
								send(clients[i], msg, strlen(msg), 0);
						}
						else
							send(clients[i], msg, strlen(msg), 0);
					}
					else // found == 1 => da dang nhap
					{
						// tim id ma client muon gui
						sscanf(buf, "%s", targetId);

						sprintf(sendbuf, "%s: %s", ids[j], buf + strlen(targetId) + 1);

						if (strcmp(targetId, "@all") == 0)
						{
							// forward den tat ca cac client khac
							for (int j = 0; j < numRegisteredClients; j++)
								if (clients[i] != registeredClients[j])
									send(registeredClients[j], sendbuf, strlen(sendbuf), 0);
						}
						else
						{
							// forward den client co id la targetId
							for (int j = 0; j < numRegisteredClients; j++)
								if (strcmp(targetId + 1, ids[j]) == 0)
									send(registeredClients[j], sendbuf, strlen(sendbuf), 0);
						}
					}
				}
		}
	}

	WSACleanup();
    return 0;
}

