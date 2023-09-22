#pragma once
#include "header.h"
#include <string>
struct response {
    std::string HTTP_version;
    int status_code;
    std::string reason;
    std::vector<header> headers;
    std::string body;
    std::string toString() {
        std::string res;
        res = HTTP_version + " " + std::to_string(status_code) + " " + reason + "\r\n"; //bad to append so many times (copies)
        for (auto& head : headers) {
            res += head.name + ": " + head.value + "\r\n";
        }
        res += "\r\n";
        if (body.length()) {
            res += body; 
        }
        return res;
    }
};

response getStockResponse(int code) {
    response resp{};
    resp.status_code = code;
    resp.headers.push_back({"Connection", "close"});
    resp.headers.push_back({"Content-Type", "text/html"});
            
    switch (code) {
        case 404:
        {
            resp.HTTP_version="HTTP/1.1";
            resp.reason="Not Found";
            resp.body = "Resource Not Found";
            header contentLength = {"Content-Length", std::to_string(resp.body.length())};
            resp.headers.push_back(contentLength);
            return resp;
        }
        default:
        return resp;
    }
}