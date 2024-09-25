#pragma once
#include <string>
#include <filesystem>
#include <fstream> 
#include <sstream>
#include "stock_response.h"
#include "header.h"
#include "mime_types.h"
#include "router.h"
#include "http_parser.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <netdb.h>
#include <signal.h>
#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 10	 // how many pending connections queue will hold

namespace HTTPServer {

class TCPServer {

public:
    TCPServer(std::string ip, int port): m_ip {ip}, m_port{port}{};
    virtual void startConnectionHandler() = 0;
    void runServer(std::string websitePath) {
        _router.SetWebsitePath(websitePath);
        startConnectionHandler();
        
    }
    virtual void closeServer() = 0;//virtual good, bad?
    void AddRoute(Route route) {
        _router.AddRoute(route);
    }
    virtual ~TCPServer() = default;

    void onError(std::function<std::string(HTTPStatusCode)> onError) {
        _router.onError(onError);
    }
private:
    std::string m_ip;
    int m_port;   
    Router _router{};

    // get sockaddr, IPv4 or IPv6:
    void *get_in_addr(struct sockaddr *sa)
    {
        if (sa->sa_family == AF_INET) {
            return &(((struct sockaddr_in*)sa)->sin_addr);
        }

        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }
public:
    
    std::string serve_request(std::string req) {
        
        request reqParsed = httpParse(req);
        auto path = std::filesystem::path(reqParsed.URI);
        //need its own router here that will return response packet, 
        //if response packet has error flag, set response packet to be error stock response
        //instatitae res = router.route(verb, path, params like body, headers, etc. w/e)
        //if resp.status != OK, resp = Redirect
        
        
        std::string dir = path.parent_path().string(); // "/home/dir1/dir2/dir3/dir4"
        std::string file = path.filename().string(); // "file"
        //get canonical make sure, it matches to current (doesn't break out)
        response resp = _router.RouteReq(reqParsed.method, path, {});
        
        return respond(resp).toString();
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
};
}