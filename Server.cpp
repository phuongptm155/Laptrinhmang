
#include "pch.h"
#define _CRT_SECURE_NO_WARNINGS
constexpr auto MAXPLAYERS = 1024;
constexpr auto MAXMATCHES = 512;
#include "stdio.h"
#include <iostream>
#include <WinSock2.h>

SOCKET clients[MAXPLAYERS];
Player players[MAXPLAYERS];
Match matches[MAXMATCHES];
AskingMatch askings[MAXMATCHES];
int numPlayers = 0;
int numMatches = 0;
int numAskings = 0;

int main()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	bind(listener, (SOCKADDR*)&addr, sizeof(addr));
	listen(listener, 5);

	fd_set fdread;
	timeval tv; tv.tv_sec = 5; tv.tv_usec = 0;

	int ret;
	char buf[256];

	while (1) {
		FD_ZERO(&fdread);
		FD_SET(listener, &fdread);
		for (int i = 0; i < numPlayers; i++) {
			FD_SET(clients[i], &fdread);
		}
		ret = select(0, &fdread, NULL, NULL, &tv);
		if (ret == SOCKET_ERROR) {
			printf("Error\n");
			break;
		}
		else if (ret == 0) {
			//timed out
		}
		else if (ret > 0) {
			if (FD_ISSET(listener, &fdread)) {
				//accept client
				SOCKET client = accept(listener, NULL, NULL);
				printf("New client accepted: %d\n", client);
				clients[numPlayers] = client;
				players[numPlayers].clientID = client;
				players[numPlayers].matching = false;
				players[numPlayers].playing = false;

				numPlayers++;
			}

			for (int i = 0; i < numPlayers; i++) {
				if (FD_ISSET(clients[i], &fdread)) {
					ret = recv(clients[i], buf, sizeof(buf), 0);
					if (ret <= 0) {
						int idx = getPlayerIdxByClientID(clients[i], players, numPlayers);
						players[idx] = players[numPlayers];
						//Xoa client
						clients[i] = clients[numPlayers];
						numPlayers--;
					}
					else {
						if(ret==256)
							buf[ret-1] = 0;
						else buf[ret] = 0;
						printf("Received %d: %s\n", clients[i], buf);
						int p = processPacket(buf, clients[i], players, numPlayers, matches, &numMatches, askings, &numAskings);
					}
				}
			}
		}
	}

	closesocket(listener);
	WSACleanup();
	return 0;
}
