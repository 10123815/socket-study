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
	serv_addr.sin_port = htons(1234);
	
	connect(serv_sock, (sockaddr*)&serv_addr, sizeof(serv_addr));

	char cmd[1024];
	char buffer[1024];
	while (std::cin>>cmd)
	{
		write(serv_sock, cmd, strlen(cmd));
		read(serv_sock, buffer, sizeof(buffer));
		printf("%s\n", buffer);

		bzero(cmd, sizeof(cmd));
		bzero(buffer, sizeof(buffer));
	}
	close(serv_sock);
	return 0;
}