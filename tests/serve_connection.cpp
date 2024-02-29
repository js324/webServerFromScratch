#include "../httpserver/threaded_server.h"





int main() {
    std::string req = "test";
    std::string resp = HTTPServer::serve_request(req);  
    ThreadedHTTPServer server("localhost", 3490);
	std::string websitePath = "/html";
    server.AddRoute({"get", "/index.html"});   
}