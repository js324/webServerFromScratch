#include "../include/httpserver/http_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <string>

int main (void)
{
  int i;
  float start, end;
  size_t parsed;

//   parser = malloc(sizeof(http_parser));
    
  std::string buf = "GET /cookies HTTP/1.1\r\nHost: 127.0.0.1:8090\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.56 Safari/537.17\r\nAccept-Encoding: gzip,deflate,sdch\r\nAccept-Language: en-US,en;q=0.8\r\nAccept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.3\r\nCookie: name=wookie\r\n\r\nthis is the body time to test";
  start = (float)clock()/CLOCKS_PER_SEC;
  for (i = 0; i < 100000; i++) {
    HTTPRequest httpRequest{buf};
    // std::cout << httpRequest;
    if (httpRequest.return_code != OK) {
      std::cout << "-------ERROR: " << httpRequest.return_code << " ------" << std::endl;
      break;
    }
  }
  end = (float)clock()/CLOCKS_PER_SEC;
  

  printf("Elapsed %f seconds.\n", (end - start));

  return 0;
}

//as of 11/12/2024
//hit 0.134 seconds
//as of 11/19/2024
