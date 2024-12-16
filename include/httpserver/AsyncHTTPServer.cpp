/*
** server.c -- a stream socket server demo
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <sys/epoll.h> //epoll linux kernel specific (could use kqueue)
#include <fcntl.h>
#define PORT "3490"  // the port users will be connecting to
#define MAXDATASIZE 100
#define BACKLOG 10	 // how many pending connections queue will hold
#define MAX_EVENTS 10

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void serve_connection(int fd) {
	
	while (1) {
		try {
			if (send(fd, "Hello, world!", 13, 0) == -1) 
				perror("send");
			char buf[MAXDATASIZE];
			int numbytes;
			
			if ((numbytes = recv(fd, buf, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
				return;
			}
			buf[numbytes] = '\0';
			
			printf("%s\n", buf);
			if (numbytes == 0) {
				std::cout << "Connection closed!\n";
				return;
			}
		}
		catch (...) {
			std::cout << "Connection failure!";
		}
	}
}

int main(void)
{
	
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	struct epoll_event ev, events[MAX_EVENTS];
	int epoll_fd;
	if ((epoll_fd = epoll_create1(0)) == -1) {
		perror("epoll_create1");
	}
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
		perror("epoll_ctl");
		exit(1);
	}
	

	printf("server: waiting for connections...\n");
	AsyncHTTPServer(std::string ip, int port) {
		
	}
    void runServer();
    void closeServer();
	while(1) {  // main accept() loop
		new_fd = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (new_fd == -1) {
			perror("epoll_wait");
			exit(1);
		}
		// sin_size = sizeof their_addr; 
		// int new_socket = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		// std::cout << events[0].data.fd << std::endl;
		for (int i = 0; i < new_fd; ++i) {
			int conn_socket = -1;
			if (events[i].data.fd == sockfd) { //listening
				sin_size = sizeof their_addr;
				if ((conn_socket = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size)) == -1) {
					perror("accept");
					exit(1);
				}
				int status;
				std::cout << conn_socket << std::endl;
				if ((status = fcntl(conn_socket, F_SETFL, fcntl(conn_socket, F_GETFL, 0) | O_NONBLOCK)) == -1) {
					perror("fnctl");
					exit(1);
				}
				ev.events = EPOLLOUT | EPOLLIN | EPOLLET; //mark for reading and nonblocking
				ev.data.fd = conn_socket;
				
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_socket, &ev) == -1) {
					perror("epoll_ctl: conn_socket");
					exit(1);
				}
				
			}
			else {
				if (events[i].events  & EPOLLIN) {
					std::cout << "Reading event" << std::endl;
					
				}	
				if (events[i].events & EPOLLOUT) {
					
				}
			}
		}
		// inet_ntop(their_addr.ss_family,
		// 	get_in_addr((struct sockaddr *)&their_addr),
		// 	s, sizeof s);
		// printf("server: got connection from %s\n", s);
		
		// close(new_fd);  // parent doesn't need this
	}

	return 0;
}
