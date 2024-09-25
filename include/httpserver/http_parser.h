#pragma once
#include <string>
#include <filesystem>
#include <fstream> 
#include <sstream>
#include <unordered_map>
#include "stock_response.h"
struct request {
    std::string method;
    std::string URI;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    std::string reqUrl;
    std::string userHostAddress;
};
request httpParse(std::string req) {

    size_t pos = 0;
    size_t prev = 0;
    std::string token;
    std::string delimiter = "\r\n";
    request reqParsed{};
    int first_time = 1;
    //should still fix parsing, should error if its not well formed http req
    while ((pos = req.find(delimiter, prev)) != std::string::npos) {
        size_t linePos{};
        size_t linePrev{};
        token = req.substr(prev, pos-prev);
        
        std::string word{};
        while ((linePos = token.find(" ", linePrev)) != std::string::npos) {
            if (first_time) {
                    //loop unroll
                word = token.substr(linePrev, linePos-linePrev);
                reqParsed.method = word;
                linePrev = linePos+1;
                linePos = token.find(" ", linePrev);
                word = token.substr(linePrev, linePos-linePrev);
                reqParsed.URI = word;
                linePrev = linePos+1;
                linePos = token.find(" ", linePrev);
                word = token.substr(linePrev, linePos-linePrev);
                if (word != "HTTP/1.1" && word != "HTTP/1.0") { // should throw
                    return reqParsed;
                }
                linePrev = linePos+1;
                
                first_time = 0;
                break;
            }
            header indHeader;
            word = token.substr(linePrev, linePos-linePrev);
            indHeader.name = word;
            linePrev = linePos+1;
            
            word = token.substr(linePrev, token.length()-linePrev);
            indHeader.value = word;
            linePrev = linePos+1;
            
        }
        
        prev = pos+2;
    }
    return reqParsed;
}
