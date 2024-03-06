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
#include <thread>
#include "threadpool.h"
#include "httpservers.h"

#define MAXDATASIZE 10000




class ThreadedHTTPServer: public HTTPServer::TCPServer {
private:
	ThreadPool pool;
	void serve_connection(int fd) {
		std::cout << "serving connection\n";
		std::cout.flush();
		while (1) {
			try {
				char buf[MAXDATASIZE];
				int numbytes;
				//handle partial reads if too big 
				if ((numbytes = recv(fd, buf, MAXDATASIZE-1, 0)) == -1) {
					perror("recv");
					throw;
				}
				
				buf[numbytes] = '\0';
				if (numbytes == 0) {
					std::cout << "Connection closed!\n";
					return;
				}
				std::string req = std::string(buf, numbytes);
				std::string resp = serve_request(req);
				//still need to handle partial sends

				std::cout << numbytes << std::endl;
				if (send(fd, &resp[0], resp.length(), 0) == -1) {
					perror("send");
					throw;
				}
				
			}
			catch (...) {
				std::cout << "Connection failure!";
				close(fd);
				return;
			}
		}
}
public:
	//default const
	ThreadedHTTPServer(std::string ip, int port): TCPServer(ip, port) {}
	void startConnectionHandler() {
		pool.startUp();
		int sockfd = bindAndListen();
		struct sockaddr_storage their_addr; // connector's address information
        socklen_t sin_size;
		while(1) {  // main accept() loop
			sin_size = sizeof their_addr;
			int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
			if (new_fd == -1) {
				perror("accept");
				continue;
			}

			// inet_ntop(their_addr.ss_family,
			// 	get_in_addr((struct sockaddr *)&their_addr),
			// 	s, sizeof s);
			// printf("server: got connection from %s\n", s);
			
			pool.queueJob([new_fd, this]() { this->serve_connection(new_fd); });
		}
	}
	void closeServer() {
		pool.close();
	}
};