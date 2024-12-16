#include "threaded_server.h"
#include <string>
std::string ErrorHandler(HTTPStatusCode code)
{
  std::string ret;
  std::cout << "HTTPSTATUS " << (int) code << std::endl;
  switch (code) {
		case HTTPStatusCode::FileNotFound:
		{
			ret = "/stock_resps/notFound.html"; 
			break;
		}
		case HTTPStatusCode::NotAuthorized:
		{
			ret = "/stock_resps/.html";
			break;
		}
		case HTTPStatusCode::Forbidden:
		{
			ret = "/stock_resps/notFound.html";
			break;
		}
		case HTTPStatusCode::InternalServerError:
		{
			ret = "/stock_resps/serverError.html";
			break;
		}
		default: 
		{
			ret = "Unknown Error";
		}
    }

  return ret;
}


int main(void)
{
	ThreadedHTTPServer server("localhost", 3490);
	std::string websitePath = "/html";
	// server.hello();


	server.AddRoute({"get", "/index.html"});
	server.AddRoute({"get", "/test", [] (std::map<std::string, std::string>) { return "/index.html"; }});
	server.AddRoute({"post", "/demo/redirect", [] (std::map<std::string, std::string>) { return "/stock_resps/forbidden.html"; }});
	
	server.onError(&ErrorHandler);
	server.runServer(websitePath);	 

	

	return 0;
}
