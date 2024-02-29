#include "httpserver/threaded_server.h"

int main() {
    std::string req = "test"; 
    ThreadedHTTPServer server("localhost", 3490);
	std::string websitePath = "/html";
    server.AddRoute({"get", "/index.html"});   
    server.runServer(websitePath);	 
}