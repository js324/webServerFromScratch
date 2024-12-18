#include "fixtures.h"

TEST_F(ContentLengthTest, BasicContentLengthWithOWS) {
  // Exercises the Xyz feature of Foo.
  std::string req = "GET /cookies HTTP/1.1\r\nContent-Length: 1234  \r\n";
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

TEST_F(ContentLengthTest, StrictMultipleContentLength) {
  std::string req = "GET /cookies HTTP/1.1\r\nContent-Length: 1234\r\nContent-Length: 1234\r\n";
  std::array<header, 50> expectedHeaders{{ 
      { "Content-Length", "1234" },
      { "Content-Length", "" } }};

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
    ErrorCode::IMPROPER_CONTENT_LENGTH
  };
  int returnCode = httpRequest.parse(req);
  EXPECT_TRUE(httpRequest.equals(expectedRequest));
}

TEST_F(BasicParserTest, StrictTEandContentLength) {
  std::string req = "GET /cookies HTTP/1.1\r\nTransfer-Encoding: gzip, chunked  \r\nContent-Length: 1234\r\n";
  std::array<header, 50> expectedHeaders{{ 
      { "Transfer-Encoding", "gzip, chunked  " },
      { "Content-Length", "" } }};

  Flags expectedFlags{ false, true, true, false, false };
  HTTPRequest expectedRequest {
    "GET",
    "/cookies",
    1,
    1,
    expectedHeaders,
    "",
    0,
    expectedFlags,
    ErrorCode::IMPROPER_CONTENT_LENGTH
  };
  int returnCode = httpRequest.parse(req);
  EXPECT_TRUE(httpRequest.equals(expectedRequest));
}