#pragma once
#include <string>
#include <array>
#include <iostream>
#include <filesystem>
#include <fstream> 
#include <sstream>
#include <unordered_map>

const uint8_t MAX_HEADER_LENGTH = 50;
const uint32_t MAX_HEADER_SIZE = MAX_HEADER_LENGTH*8192; //if headers total size greater, return 4xx error
const uint32_t MAX_SIZE = 1000000; //if total size of request/response greater, return 4xx
const uint8_t MAX_HEADER_VALUE_COUNT = 50;

class HTTPRequest {
    public:
        struct header {
            std::string_view field;
            
            std::string_view value; 
            //we also treat quoted strings as one string (including the string)
            //keep in mind this can be CSV, even ';' separated, simply have multiple header fields with same name
            //But for simplicity keeping it as one string
            //Comma or ';' separated? https://stackoverflow.com/questions/78183179/when-are-http-header-values-comma-or-semi-colon-separated
                //Usually its CSV but ultimately based on spec of specific header field
                //Regardless the parsing doesn't care about what header field it is until later, 3.2.4 Field Parsing

        };
        std::string_view method;
        std::string_view URI;
        int maj_version;
        int min_version;
        std::array<header, MAX_HEADER_LENGTH> headers;
        std::string_view body;
        uint32_t content_length;

    friend std::ostream& operator<<(std::ostream& os, const HTTPRequest& obj) {
        os << "Method: " << obj.method << std::endl;
        os << "URI: " << obj.URI << std::endl;
        os << "HTTP Version: " << obj.maj_version << "." << obj.min_version << std::endl;
        for (auto header : obj.headers) {
            if (header.field == "")
                break;
            os << "Header Field/Value: " << header.field << ": " << header.value << std::endl;
        }
        os << "Body: " << obj.body << std::endl;
        return os;
    }      
};

enum ParsingState {
    METHOD,
    REQUEST_TARGET,
    HTTP_VERSION,
    HEADER_FIELD,
    HEADER_VALUE,
    BODY,
    END_PARSE,
};

bool isToken(const char c) { return static_cast<unsigned char>(c) < 127 && static_cast<unsigned char>(c) > 32; } //between Space and DEL in ASCII chart

//another thing to consider -> ASCII (token) vs non ASCII characters
int httpParse(std::string& req, HTTPRequest& httpRequest) { //return status as int

    size_t pos = 0;
    // std::string delimiter = "\r\n";
    ParsingState currState = METHOD;
    uint8_t headerCnt = 0;
    uint8_t headerValueCnt = 0;
    for (size_t i = 0; i < req.length(); i++ ) {
        pos = i;
        switch (currState) {
            //NO PREDEFINED LIMIT ON LENGTH OF REQUEST LINE, min 8000 octets (otherwise error if above max)
            case METHOD:
                while (req[i] != ' ' && req[i] != '\t') {
                    ++i;
                }    
                httpRequest.method =  std::string_view(req).substr(pos, i-pos);
                currState = REQUEST_TARGET;
            break;
            case REQUEST_TARGET:
                while (req[i] != ' ' && req[i] != '\t') {
                    ++i;
                }    
                httpRequest.URI =  std::string_view(req).substr(pos, i-pos);
                currState = HTTP_VERSION;
            break;
            case HTTP_VERSION:
                if (req[i] != 'H') {
                    return -3;
                }
                ++i;
                if (req[i] != 'T') {
                    return -3;
                }
                ++i;
                if (req[i] != 'T') {
                    return -3;
                }
                ++i;
                if (req[i] != 'P') {
                    return -3;
                }
                ++i;
                if (req[i] != '/') {
                    return -3;
                }
                ++i;
                if (req[i] != '0' && req[i] != '1' && req[i] != '2') {
                    return -3;
                }
                httpRequest.maj_version = i-'0';
                ++i;
                if (req[i] != '.') {
                    return -3;
                }
                ++i;
                if (req[i] != '0' && req[i] != '1' && req[i] != '2') {
                    return -3;
                }
                httpRequest.min_version = i-'0';
                ++i;
                if (req[i] != '\r') {
                    return -4; 
                }
                ++i;
                if (req[i] != '\n') {
                    return -4;
                }
                currState = HEADER_FIELD;
            break;
            //AGAIN IGNORE LARGER THAN MAX SIZE FOR FIELD AND VALUE
            case HEADER_FIELD:
                //HEADER FIELD STRICTLY TOKEN AND NO WHITESPACE IN BETWEEN FIELD AND ':'
                while (req[i] != ':') {
                    if (std::isspace(req[i])) {
                        return -5;
                    }
                    ++i;
                }
                //NO SPACES IN HEADER FIELD
                httpRequest.headers[headerCnt].field = std::string_view(req).substr(pos, i-pos);
                currState = HEADER_VALUE;
            break;
            case HEADER_VALUE:
                //Optional WS before and after value
                // while (req[i] == ' ' || req[i] == '\t') {
                //     ++i;
                //     ++pos;
                // }
                // //check that we actually had a value (some VCHAR/non delimiter, visible char), err if we dont
                // if (isspace(req[i])) {
                //     return -6; 
                // }
                // while (!isspace(req[i])) {
                //     ++i;
                // }
                // //at this point should check for space/htab for additional values
                // if (req[i] == ' ' || req[i] == '\t') {
                //     ++i; //go back to beginning while loop
                // }

                //this logic could get more complicated with delimiting, support for quoted strings, comments, obs-text (3.2.6)
                //but this is extremely naive
                while (req[i] != '\r' && (isToken(req[i]) || req[i] == ' ' || req[i] == '\t')) {
                    std::cout << req[i];
                    ++i;
                }
                if (req[i] != '\r') {
                    return -6; 
                }
                ++i;
                if (req[i] != '\n') {
                    return -6;
                }

                httpRequest.headers[headerCnt].value = std::string_view(req).substr(pos, (i-1)-pos); //i-1 beccause we included the \n
                ++headerCnt; //increment header length/counter
                currState = (req[i+1] == '\r' ? BODY : HEADER_FIELD); //read ahead [i+1] here maybe unnecessary
            break;
            case BODY:
                //skip through CRLF
                //initial carriage is skipped by for loop
                //Request check if Content-Length or Transfer-Encoding
                //Reponse see (RFC 7230 3.3), depends on request method and status code
                if (req[i] != '\r') {
                    return -7;
                }
                ++i;
                if (req[i] != '\n') {
                    return -7;
                }
                ++i;
                httpRequest.body = std::string_view(req).substr(i, req.length());
            break;
        }
    }
    // std::cout << httpRequest;
    return 0;
}
