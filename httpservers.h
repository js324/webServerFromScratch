#pragma once
#include <string>

#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 10	 // how many pending connections queue will hold

class TCPServer {
private:
    std::string m_ip;
    int m_port;    
    // void sigchld_handler(int s)
    // {
    //     (void)s; // quiet unused variable warning

    //     // waitpid() might overwrite errno, so we save and restore it:
    //     int saved_errno = errno;

    //     while(waitpid(-1, NULL, WNOHANG) > 0);

    //     errno = saved_errno;
    // }


    // get sockaddr, IPv4 or IPv6:
    void *get_in_addr(struct sockaddr *sa)
    {
        if (sa->sa_family == AF_INET) {
            return &(((struct sockaddr_in*)sa)->sin_addr);
        }

        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }
protected:
    std::string serve_request(std::string request) {
        size_t pos = 0;
        std::string token;
        std::string resp;
        std::string delimiter = "\r\n";
        while ((pos = request.find(delimiter)) != std::string::npos) {
            token = request.substr(0, pos);
            std::cout << token << std::endl;
            request.erase(0, pos + delimiter.length());
        }
        return "";
    }
    int bindAndListen() {
        int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
        struct addrinfo hints, *servinfo, *p;
        
        struct sigaction sa;
        int yes=1;
        char s[INET6_ADDRSTRLEN];
        int rv;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE; // use my IP
        
        if ((rv = getaddrinfo(m_ip.c_str(), PORT, &hints, &servinfo)) != 0) {
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

        
        return sockfd;
    }
public:
    TCPServer(std::string ip, int port): m_ip {ip}, m_port{port}{};
    virtual void runServer() = 0;//will be inherited, then have event loop
    virtual void closeServer() = 0;//virtual good, bad?
    virtual ~TCPServer() = default;
};