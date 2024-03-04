#include "httpserver/threaded_server.h"
#include <unistd.h>
#include <chrono>
#include <thread>
void sendRequest(std::string input, std::string hostname);
int main() {
    if (fork() == 0) { //yikes this is really hacky but should look into how other ppl test
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::ifstream t("request.txt");
        std::stringstream buffer;
        buffer << t.rdbuf();
        
        sendRequest(buffer.str(), "localhost");
    }
    else {
        ThreadedHTTPServer server("localhost", 3490);
        std::string websitePath = "/html";
        server.AddRoute({"get", "/index.html"});   
        server.runServer(websitePath);	 
    }
}