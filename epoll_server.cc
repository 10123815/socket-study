#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>

typedef std::vector<epoll_event> EpollEventSet;
typedef std::vector<int> Clients;

// set file descriptor non block
void setnonblocking(int sock)
{
     int opts;
     opts = fcntl(sock,F_GETFL);
     if(opts < 0)
     {
          perror("fcntl(sock,GETFL)");
          exit(1);
     }
     opts = opts|O_NONBLOCK;
     if(fcntl(sock, F_SETFL,opts)<0)
     {
          perror("fcntl(sock,SETFL,opts)");
          exit(1);
     }  
}

int main(int argc, char const *argv[])
{
	int serv_sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serv_sock_fd == -1)
	{
		perror("socket error");
	}

	sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(9999);

	int on = 1;
	int res = setsockopt(serv_sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (res == -1)
	{
		perror("setsockopt");
	}

	res = bind(serv_sock_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
	if (res == -1)
	{
		perror("bind error");
	}

	res = listen(serv_sock_fd, SOMAXCONN);
	if (res == -1)
	{
		perror("listen");
	}

	int epoll_fd = epoll_create1(EPOLL_CLOEXEC);

	Clients client_socks;

	epoll_event event;
	event.data.fd = serv_sock_fd;
	event.events = EPOLLIN | EPOLLET;	// listen read event
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv_sock_fd, &event);

	EpollEventSet events(16);
	sockaddr_in client_addr;
	socklen_t client_addr_len;
	int client_sock_fd;

	int nfds = 0;
	while (1)
	{
		// events里面将储存所有的读写事件
		nfds = epoll_wait(epoll_fd, &*events.begin(), events.size(), -1);
		if (nfds == -1)
		{
			perror("epoll wait");
		}

		if (nfds == events.size())
			events.resize(nfds * 2);

		for (int i = 0; i < nfds; ++i)
		{
			if (events[i].data.fd == serv_sock_fd)
			{
				// 新连接进入
				client_addr_len = sizeof(client_addr);
				// read request queue
				client_sock_fd = accept(serv_sock_fd, (sockaddr*)&client_addr, &client_addr_len);
				if (client_sock_fd == -1)
					perror("accept");

				char* addr_str = inet_ntoa(client_addr.sin_addr);
				short port = ntohs(client_addr.sin_port);
				printf("%s : %d connected.\n", addr_str, port);

				client_socks.push_back(client_sock_fd);

				setnonblocking(client_sock_fd);

				// add this connection to events
				event.data.fd = client_sock_fd;
				event.events = EPOLLIN | EPOLLET;
				res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock_fd, &event);
				if (res == -1)
				{
					perror("add epoll event");
				}

			}
			else if (events[i].events & EPOLLIN)
			{
				// read from the connected client
				client_sock_fd = events[i].data.fd;
				if (client_sock_fd < 0)
				{
					perror("client error");
				}

				char recv_buf[1024];
				int data_size = read(client_sock_fd, recv_buf, sizeof(recv_buf));
				if (data_size == -1)
					perror("read");
				else if (data_size == 0)
				{
					printf("client close\n");
					close(client_sock_fd);
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_sock_fd, &(events[i]));
					client_socks.erase(std::remove(client_socks.begin(), client_socks.end(), client_sock_fd), client_socks.end());
				}
				else
				{
					printf("%s\n", recv_buf);
					write(client_sock_fd, recv_buf, data_size);
				}

			}
		}
	}

	return 0;
}