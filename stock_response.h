#pragma once
#include "header.h"
#include <string>


enum class ServerError
{
  OK = 200,
  Forbidden = 403,
  NotAuthorized = 401,
  FileNotFound = 404,
  ServerError = 500,
  UnknownType,
};

struct response {
    std::string HTTP_version;
    int status_code;
    std::string reason;
    std::vector<header> headers;
    std::string body;
    std::string redirect;
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
response respond(response& resp) {
    resp.HTTP_version="HTTP/1.1";
    resp.headers.push_back({"Connection", "close"});
    if (!resp.redirect.length()) {
        resp.status_code = 200;
    }
    else {
        resp.status_code = 301;
        resp.headers.push_back({"Location", "http://localhost:3490" + resp.redirect});
    }
    return resp;
}

response getStockResponse(int code) {
    response resp{};
    resp.headers.push_back({"Content-Type", "text/html"});
    switch (code) {
        case 404:
        {
            resp.reason="Not Found";
            resp.redirect = "/stock_resps/notFound.html";
            return resp;
        }
        case 401:
        {
            resp.reason="Unauthorized";
            
            return resp;
        }
        case 403:
        {
            resp.reason="Forbidden";
            
            return resp;
        }
        case 500:
        {
            resp.reason="Internal Server Error";
            
            return resp;
        }
        default:
        return resp;
    }
}