// TelnetServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "winsock2.h"

DWORD WINAPI ClientThread(LPVOID);

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
	char filebuf[256];

	char *msg = "Sai user hoac pass. Hay nhap lai\n";

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

		// Xoa ky tu xuong dong neu co
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0;

		// Kiem tra user va pass
		int found = 0;
		FILE *f = fopen("C:\\Test_server\\users.txt", "r");
		while (fgets(filebuf, sizeof(filebuf), f))
		{
			// Xoa ky tu xuong dong neu co
			if (filebuf[strlen(filebuf) - 1] == '\n')
				filebuf[strlen(filebuf) - 1] = 0;

			if (strcmp(filebuf, buf) == 0)
			{
				found = 1;
				break;
			}
		}
		fclose(f);

		if (found == 1)
		{
			send(client, "OK\n", 3, 0);
			break;
		}
		else
			send(client, msg, strlen(msg), 0);
	}

	char cmdbuf[256];

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

		// xoa ky tu xuong dong neu co
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0;

		sprintf(cmdbuf, "%s > c:\\test_server\\out.txt", buf);
		system(cmdbuf);

		FILE *f = fopen("c:\\test_server\\out.txt", "r");
		while (fgets(filebuf, sizeof(buf), f))
			send(client, filebuf, strlen(filebuf), 0);
		fclose(f);
	}
}

