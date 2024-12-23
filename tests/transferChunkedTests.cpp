#include "fixtures.h"

TEST_F(TransferChunkedTest, BasicTransferChunkedTest) {
  // Exercises the Xyz feature of Foo.
  std::string req = "GET /cookies HTTP/1.1\r\nTransfer-Encoding: chunked \r\n\r\n5\r\nasdfg\r\n";
  std::array<header, 50> expectedHeaders{{ 
      { "Transfer-Encoding", "chunked " } }};
  HTTPRequest::Flags expectedFlags{ false, true, false, false, false, false };
  HTTPRequest expectedRequest {
    "GET",
    "/cookies",
    1,
    1,
    expectedHeaders,
    "asdfg",
    5,
    expectedFlags,
    ErrorCode::OK
  };  
  
  httpRequest.parse(req);
    compareResults(expectedRequest);
  EXPECT_TRUE(httpRequest.equals(expectedRequest));
}

