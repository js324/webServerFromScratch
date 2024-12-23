#pragma once
#include <gtest/gtest.h> 
#include "httpserver/http_parser.h"

class BasicParserTest : public testing::Test {
 protected:
  // You can remove any or all of the following functions if their bodies would
  // be empty.
  HTTPRequest httpRequest{};
  
  // ~FooTest() override {
  //    // You can do clean-up work that doesn't throw exceptions here.
  // }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  void SetUp() override {
     // Code here will be called immediately after the constructor (right
     // before each test).
  }

  void TearDown() override {
     // Code here will be called immediately after each test (right
     // before the destructor).
  }

  // Class members declared here can be used by all tests in the test suite
  // for Foo.
public: 
   void compareResults(HTTPRequest expected) {
      std::cout << "EXPECTED: " << std::endl
         << expected << std::endl
         << "RESULT: " << std::endl
         << httpRequest << std::endl;
   }
};

class ContentLengthTest: public BasicParserTest {};

class TransferChunkedTest: public BasicParserTest {};