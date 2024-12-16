#include "fixtures.h"

TEST_F(ContentLengthTest, BasicContentLength) {
  // Exercises the Xyz feature of Foo.
  std::string req = "GET /cookies HTTP/1.1\r\nContent-Length: 1234\r\n";
  std::array<header, 50> expectedHeaders{{ 
      { "Content-Length", "1234" } }};
  Flags expectedFlags{ true, false, false, false, false };
  HTTPRequest expectedRequest {
    "GET",
    "/cookies",
    1,
    1,
    expectedHeaders,
    "",
    1234,
    expectedFlags,
    ErrorCode::OK
  };  
  
  httpRequest.parse(req);
  
  EXPECT_TRUE(httpRequest.equals(expectedRequest));
}

