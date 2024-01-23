#pragma once
#include "header.h"
#include <string>


enum class ServerError
{
  OK = 200,
  Forbidden = 403,
  NotAuthorized = 401,
  FileNotFound = 404,
  InternalServerError = 500,
  BadRequest = 400,
};

struct response {
    std::string HTTP_version = "HTTP/1.1";
    int status_code;
    std::string reason;
    std::vector<header> headers;
    std::string body;
    std::string redirect;
    std::string encoding;
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
    std::cout << "RESPOND: " << resp.redirect;
    if (!resp.redirect.length()) {
        resp.status_code = 200;
    }
    else {
        resp.status_code = 301;
        resp.headers.push_back({"Location", "http://localhost:3490/" + resp.redirect});
    }
    return resp;
}

response getStockResponse(ServerError code) {
    response resp{};
    resp.headers.push_back({"Content-Type", "text/html"});
    switch (code) {
        case ServerError::FileNotFound:
        {
            resp.reason="Not Found";
            // resp.status_code = 404;

            resp.redirect = "stock_resps/notFound.html";
            return resp;
        }
        case ServerError::NotAuthorized:
        {
            resp.reason="Unauthorized";
            
            return resp;
        }
        case ServerError::Forbidden:
        {
            resp.reason="Forbidden";
            
            return resp;
        }
        case ServerError::InternalServerError:
        {
            resp.reason="Internal Server Error";
            
            return resp;
        }
        default:
        return resp;
    }
}