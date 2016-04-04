#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char const *argv[])
{

	// socket file descriptor
	int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		
	// bind socket and ip:port
	sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(9999);
	
	connect(serv_sock, (sockaddr*)&serv_addr, sizeof(serv_addr));

	char cmd[1024];
	char buffer[1024];
	while (std::cin>>cmd)
	{
		bzero(buffer, sizeof(buffer));

		write(serv_sock, cmd, strlen(cmd) + 1);
		int data_size = read(serv_sock, buffer, sizeof(buffer));
		if (data_size == 0)
		{
			printf("server close");
			break;
		}
		printf("%s\n", buffer);

		bzero(cmd, sizeof(cmd));
	}
	close(serv_sock);
	return 0;
}