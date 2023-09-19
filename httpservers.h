#pragma once
#include <string>
#include <filesystem>
#include <fstream> 
#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 10	 // how many pending connections queue will hold
struct header {
        std::string name;
        std::string value;
    };
struct request {
    std::string method;
    std::string URI;
    std::vector<header> headers;
    std::string body;
};

struct response {
    std::string HTTP_version;
    int status_code;
    std::string reason;
    std::vector<header> headers;
    std::string body;
    std::string toString() {
        std::string res;
        res = HTTP_version + " " + std::to_string(status_code) + " " + reason + "\r\n"; //bad to append so many times (copies)
        for (auto& head : headers) {
            res += head.name + ": " + head.value + "\r\n";
        }
        res += "\r\n";
        if (body.length()) {
            res += body; 
        }
        return res;
    }
};
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
    std::string serve_request(std::string req) {
        // std::cout<<request;
        size_t pos = 0;
        
        size_t prev = 0;
        std::string token;
        response resp;
        std::string delimiter = "\r\n";
        request reqParsed{};
        int first_time = 1;
        while ((pos = req.find(delimiter, prev)) != std::string::npos) {
            size_t linePos{};
            size_t linePrev{};
            token = req.substr(prev, pos-prev);
            
            // std::cout << token << std::endl;
            std::string word{};
            while ((linePos = token.find(" ", linePrev)) != std::string::npos) {
                if (first_time) {
                     //loop unroll
                    word = token.substr(linePrev, linePos-linePrev);
                    reqParsed.method = word;
                    linePrev = linePos+1;
                    // std::cout << reqParsed.method << std::endl;
                   
                    linePos = token.find(" ", linePrev);
                    word = token.substr(linePrev, linePos-linePrev);
                    reqParsed.URI = word;
                    linePrev = linePos+1;
                    // std::cout << reqParsed.URI << std::endl;
                    
                    linePos = token.find(" ", linePrev);
                    word = token.substr(linePrev, linePos-linePrev);
                    // std::cout << word << std::endl;
                    if (word != "HTTP/1.1" && word != "HTTP/1.0") {
                        return "400";
                    }
                    linePrev = linePos+1;
                    
                    first_time = 0;
                    break;
                }
                header indHeader;
                word = token.substr(linePrev, linePos-linePrev);
                indHeader.name = word;
                linePrev = linePos+1;
                // std::cout << indHeader.name << std::endl;
                
                word = token.substr(linePrev, token.length()-linePrev);
                indHeader.value = word;
                linePrev = linePos+1;
                // std::cout << indHeader.value << std::endl;
                
            }
            
            prev = pos+2;
        }
        std::filesystem::path path(reqParsed.URI);
        std::string dir = path.parent_path().string(); // "/home/dir1/dir2/dir3/dir4"
        std::string file = path.filename().string(); // "file"
        std::cout << dir << " " << file << std::endl;
        resp.HTTP_version = "HTTP/1.1";
        resp.status_code = 200;
        resp.reason = "OK";
        resp.headers.push_back({"Content-Type", "text/plain"});
        resp.headers.push_back({"Content-Length", "6"});
        resp.headers.push_back({"Connection", "close"});
        resp.body = "Hello!";
        if (dir == "/"  && file.length() == 0) {
            std::cout << resp.toString() << std::endl;
            return resp.toString();
        } 

        std::cout << "FIN" << std::endl;
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