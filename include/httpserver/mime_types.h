#pragma once
#include <string>
#include <unordered_map>
std::unordered_map<std::string, std::string> mime_types{
    {"html", "text/html"},
    {"png", "image/png"},
    {"", "text/html"}
};

std::string getMIMEType(std::string extension) {
    if (mime_types.find(extension) != mime_types.end()) {
        return mime_types[extension];
    }
    return "";
}