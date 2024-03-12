#include "httpserver/threaded_server.h"
#include <unistd.h>
#include <chrono>
#include <thread>
void sendRequest(std::string input, std::string hostname);
ThreadedHTTPServer tcpserver{"", 3490};
    


void basicRequest(std::string req) {
    std::cout << "RESPONSE: " << tcpserver.serve_request(req);
}
    
int main() {
    std::ifstream t("request.txt");
    std::stringstream buffer;
    buffer << t.rdbuf();

    // if (fork() == 0) { //yikes this is really hacky but should look into how other ppl test
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));    
    //     sendRequest(buffer.str(), "localhost");
    // }
    // else {
    //     ThreadedHTTPServer server("localhost", 3490);
    //     std::string websitePath = "/html";
    //     server.AddRoute({"get", "/index.html"});   
    //     server.runServer(websitePath);	 
    // }

    //Test serve_request
    
    basicRequest(buffer.str());
}
