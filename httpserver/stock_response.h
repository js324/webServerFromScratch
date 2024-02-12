#pragma once
#include "header.h"
#include <string>


enum class HTTPStatusCode
{
  OK = 200,
  PermanentRedirect = 301,
  Forbidden = 403,
  NotAuthorized = 401,
  FileNotFound = 404,
  InternalServerError = 500,
  BadRequest = 400,
};

struct response {
    std::string HTTP_version = "HTTP/1.1";
    std::string reason;
    std::vector<header> headers;
    std::string body;
    std::string redirect;
    std::string encoding;
    int status_code;
    HTTPStatusCode error{HTTPStatusCode::OK};
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
    resp.headers.push_back({"Connection", "close"});

    if (resp.error != HTTPStatusCode::OK) {
        resp.status_code = (int) resp.error;
        resp.reason = "File Not Found"; //definitely should NOT be the default
    }
    else if (resp.redirect.length()) {
        resp.status_code = (int) HTTPStatusCode::PermanentRedirect;
        resp.reason="Not Found";
        resp.headers.push_back({"Location", "http://localhost:3490" + resp.redirect});
    }
    else {
        resp.status_code = (int) HTTPStatusCode::OK;
        resp.reason = "OK";
    }
    return resp;
}
