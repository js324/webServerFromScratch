#include <filesystem>
#include <fstream> 
#include <sstream>
#include <map>
#include <iostream>
#include <functional>
#include "mime_types.h"
#include "stock_response.h"
class Router;

struct ExtensionInfo
{
  std::string _contentType{};
  std::function<response(std::string, std::string, ExtensionInfo)> _loader{};
//   ExtensionInfo(std::string contentType, std::function<response(std::string, std::string)> loader): 
//     _contentType(contentType), _loader(loader) {} why constructor doesn't work? have no clue
};


class Route {
public:
    std::string_view _verb;
    std::filesystem::path _path; 
    std::map<std::string, std::string> _kvParams;
    Route(std::string_view verb, std::filesystem::path path, std::map<std::string, std::string> kvParams): 
        _verb {verb}, _path {path}, _kvParams {kvParams}  {
    }    
};


class Router {
private:
    static response PageLoader(std::string fullPath, std::string ext, ExtensionInfo extInfo)
    {
        response resp{};
        std::ifstream t(fullPath);
        std::stringstream buffer;
        resp.headers.push_back({"Content-Type", extInfo._contentType});
        header contentLength = {"Content-Length", std::to_string(buffer.str().length())};
        resp.headers.push_back(contentLength);
        resp.body = buffer.str();
        return resp;
    }
    static response ImageLoader(std::string fullPath, std::string ext, ExtensionInfo extInfo)
    {
        response resp{};
        std::ifstream t(fullPath);
        resp.headers.push_back({"Content-Type", extInfo._contentType});
        std::stringstream buffer;
        buffer << t.rdbuf();
        header contentLength = {"Content-Length", std::to_string(buffer.str().length())};
        resp.headers.push_back(contentLength);
        resp.body = buffer.str();
        return resp;
    }
    static response FileLoader(std::string fullPath, std::string ext, ExtensionInfo extInfo)
    {
        response resp{};
        std::ifstream t(fullPath);
        resp.headers.push_back({"Content-Type", extInfo._contentType});
        std::stringstream buffer;
        buffer << t.rdbuf();
        header contentLength = {"Content-Length", std::to_string(buffer.str().length())};
        resp.headers.push_back(contentLength);
        resp.body = buffer.str();
        return resp;
    }


    std::vector<Route> _routes{};
    std::unordered_map<std::string, ExtensionInfo> _extFolderMap{
        // {"png", ExtensionInfo{"image/ico", (Router::ImageLoader)}},
        {"html", ExtensionInfo{"text/html", &(Router::PageLoader)}},
        // {"css", ExtensionInfo{"text/css", (Router::FileLoader)}},
        // {"js", ExtensionInfo{"text/js", (Router::FileLoader)}},
    };    
    

public:
    void AddRoute(Route route) {
        _routes.push_back(route);
    }


    response route(std::string_view verb,
    std::filesystem::path path,
    std::map<std::string, std::string> kvParams) {
        response resp{};
        std::string dir = path.parent_path().string(); // "/home/dir1/dir2/dir3/dir4"
        std::string file = path.filename().string(); // "file"
        // std::cout << "DIR: " << dir << "FILE: " << file;
        
        
        if (verb == "GET") {

            resp.headers.push_back({"Connection", "close"});
            
            if (dir == "/html"  && file.length() == 0) {
                file = "html/index.html";
                std::ifstream t(file);
                std::stringstream buffer{};
                buffer << t.rdbuf();
                header contentLength = {"Content-Length", std::to_string(buffer.str().length())};
                resp.headers.push_back({"Content-Type", "text/html"});
                resp.headers.push_back(contentLength);
                resp.body = buffer.str();
               
            } 
            else {
                //create enum of mime struct later
                std::string extension = path.extension().string().substr(1);
                std::string mime_type = getMIMEType(extension);
               
                if (mime_type == "") {
                    return respond(resp = getStockResponse(ServerError::FileNotFound));
                }
                resp = _extFolderMap[extension]._loader(path.string().substr(1), extension, _extFolderMap[extension]);
                // path.string(), extension, _extFolderMap[extension]._loader
                
                
            }
            resp.status_code = 200;
            resp.reason = "OK";
            resp.HTTP_version = "HTTP/1.1";
            return resp;
        }
    }
};
