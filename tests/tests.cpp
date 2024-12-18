#include "fixtures.h"

struct HTTPParserTestParams {
    std::string req;
    HTTPRequest expected;
};

TEST_F(BasicParserTest, BasicTest) {
  std::string req = "GET /cookies HTTP/1.1\r\nHost: 127.0.0.1:8090\r\nCookie: name=wookie\r\n\r\nthis is the body time to test";
  std::array<header, 50> expectedHeaders{{ 
      { "Host", "127.0.0.1:8090" },
      { "Cookie", "name=wookie"} }};
  HTTPRequest expectedRequest {
    "GET",
    "/cookies",
    1,
    1,
    expectedHeaders,
    "this is the body time to test",
    0,
    {},
    ErrorCode::OK
  };  
  int returnCode = httpRequest.parse(req);
  EXPECT_TRUE(httpRequest.equals(expectedRequest));
}

TEST_F(BasicParserTest, StrictSpaceRequestLine) {
  std::string req = "GET  /cookies HTTP/1.1\r\nHost: 127.0.0.1:8090\r\nCookie: name=wookie\r\n\r\nthis is the body time to test";
  HTTPRequest expectedRequest {
    "GET",
    "",
    1,
    1,
    {},
    "",
    0,
    {},
    ErrorCode::BAD_START_LINE
  };  
  int returnCode = httpRequest.parse(req);
  EXPECT_TRUE(httpRequest.equals(expectedRequest));
}

TEST_F(BasicParserTest, BadFormatHTTPVersion) {
  std::string req = "GET /cookies HTP/1.1\r\nHost: 127.0.0.1:8090\r\nCookie: name=wookie\r\n\r\nthis is the body time to test";
  int returnCode = httpRequest.parse(req);
  EXPECT_TRUE(httpRequest.return_code == BAD_HTTP_VERSION);
}

TEST_F(BasicParserTest, NonTokenHeaderField) {
  std::string req = "GET /cookies HTTP/1.1\r\nHo¢st: 127.0.0.1:8090\r\nCookie: name=wookie\r\n\r\nthis is the body time to test";
  int returnCode = httpRequest.parse(req);
  EXPECT_TRUE(httpRequest.return_code == BAD_HEADER_FIELD);
}

TEST_F(BasicParserTest, OWSHeaderValue) {
  std::string req = "GET /cookies HTTP/1.1\r\nHost:    \t 127.0.0.1:8090\t\r\nCookie: name=wookie\r\n\r\nthis is the body time to test";
  //Problem: Cannot get rid of trailing OWS without breaking one pass (how to determine correct state)
  std::array<header, 50> expectedHeaders{{ 
      { "Host", "127.0.0.1:8090\t" },
      { "Cookie", "name=wookie"} }};
  HTTPRequest expectedRequest {
    "GET",
    "/cookies",
    1,
    1,
    expectedHeaders,
    "this is the body time to test",
    0,
    {},
    ErrorCode::OK
  };  
  int returnCode = httpRequest.parse(req);
  EXPECT_TRUE(httpRequest.equals(expectedRequest));
}

TEST_F(BasicParserTest, BasicTransferCoding) {
  std::string req = "GET /cookies HTTP/1.1\r\nTransfer-Encoding: gzip, chunked  \r\n";
  std::array<header, 50> expectedHeaders{{ 
      { "Transfer-Encoding", "gzip, chunked  " } }};
  Flags expectedFlags{ false, true, true, false, false, false, false, false };
  HTTPRequest expectedRequest {
    "GET",
    "/cookies",
    1,
    1,
    expectedHeaders,
    "",
    0,
    expectedFlags,
    ErrorCode::OK
  };  
  
  int res = httpRequest.parse(req);
  
  EXPECT_TRUE(httpRequest.equals(expectedRequest));
}

TEST_F(BasicParserTest, BasicMultiHeader) {
  std::string req = "GET /cookies HTTP/1.1\r\nTransfer-Encoding: gzip, chunked  \r\nHost: 127.0.0.1:8090\r\n";
  std::array<header, 50> expectedHeaders{{ 
      { "Transfer-Encoding", "gzip, chunked  " },
      { "Host", "127.0.0.1:8090" }}};
  Flags expectedFlags{ false, true, true, false, false, false, false, false };
  HTTPRequest expectedRequest {
    "GET",
    "/cookies",
    1,
    1,
    expectedHeaders,
    "",
    0,
    expectedFlags,
    ErrorCode::OK
  };  
  
  int res = httpRequest.parse(req);
  
  EXPECT_TRUE(httpRequest.equals(expectedRequest));
}
int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


//start thinking about test suite:
/* 
1st test: Basic, generic http request, 1-2 headers GET
2nd test: Basic generic http request POST with body
3rd test: basic generic http request GET with OWS beteween fields
4th test: Incomplete request -> set up error number (have enum corresponding to each error) in our parser object
5th test: badly formatted request - have non token characters
6th test: badly formatted request - have extra colons in headers
7th test: badly formatted request - make HTTP version 3.1
8th test: completely random string
9th test: Some non-ascii character like japanese symbol, try to test for larger than octet characters also


Test for content length:
  * Normal case
  * Empty value 
  * Extremely large number (have limit on body size)
  * Multiple content lengths -> reject
  * Multipel content length values/invalid characters (...: 12, 15, 2) -> reject 
*/