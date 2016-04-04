#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

int main(int argc, char const *argv[])
{

	// ignore child's close
	signal(SIGCHLD, SIG_IGN);

	// socket file descriptor
	int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serv_sock < 0)
	{
		perror("socket");
		exit(1);
	}

	int opt = 1;
	setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	// bind socket and ip:port
	sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(1234);
	int res = bind(serv_sock, (sockaddr*)&serv_addr, sizeof(serv_addr));
	if (res < 0)
	{
		perror("bind");
		exit(1);
	}

	// 10 : size of request queue
	listen(serv_sock, 10);

	while (1)
	{
		
		sockaddr_in client_addr;
		socklen_t client_addr_size = sizeof(client_addr);

		// wait for request
		int client_sock = accept(serv_sock, (sockaddr*)&client_addr, &client_addr_size);
		char* client_addr_str = inet_ntoa(client_addr.sin_addr);
		int client_port = ntohs(client_addr.sin_port);
		printf("Accept %s : %d\n", client_addr_str, client_port);

		auto pid = fork();
		if (pid < 0)
		{
			perror("fork failed");
		}
		else if (pid == 0)
		{
			// child process
			close(serv_sock);
			char buffer[1024];
			while (1)
			{
				memset(buffer, 0, sizeof(buffer));
				int data_size = read(client_sock, buffer, sizeof(buffer));
				if (data_size == 0)
				{
					printf("client stop\n");
					break;
				}
				write(client_sock, buffer, data_size);
			}
			exit(EXIT_SUCCESS);
		}
		else
			// parent process
			close(client_sock);

	}

	close(serv_sock);

	return 0;
}