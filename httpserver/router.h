#include <filesystem>
#include <fstream> 
#include <sstream>
#include <map>
#include <iostream>
#include <functional>
#include "mime_types.h"
#include "stock_response.h"
class Router;

class ExtensionInfo
{
public:
  std::string _contentType{};
  std::function<response(std::string, std::string, ExtensionInfo)> _loader{};
  ExtensionInfo(std::string contentType, std::function<response(const Router&, std::string, std::string, ExtensionInfo)> loader): 
    _contentType(contentType), _loader(std::move(loader)) {}
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
    static response ImageLoader(std::string fullPath, std::string ext, ExtensionInfo extInfo)
    {
    // FileStream fStream = new FileStream(fullPath, FileMode.Open, FileAccess.Read);
    // BinaryReader br = new BinaryReader(fStream);
    // ResponsePacket ret = new ResponsePacket() { Data = br.ReadBytes((int)fStream.Length), ContentType = extInfo.ContentType };
    // br.Close();
    // fStream.Close();
    response resp{};
    return resp;
    }

    std::vector<Route> _routes{};
    // public std::string WebsitePath { get; set; } prob don't need since we pass path relative to site
    // std::function<response(std::string, std::string, ExtensionInfo)> loader {&ImageLoader};

    // <int(A&)> f5 = &A::get;
    std::unordered_map<std::string, ExtensionInfo> _extFolderMap{
        {"png", ExtensionInfo{"image/ico", std::bind(&(Router::ImageLoader))}}}


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
        std::cout << "DIR: " << dir << "FILE: " << file;
        
        resp.HTTP_version = "HTTP/1.1";
        if (verb == "GET") {
            resp.status_code = 200;
            resp.reason = "OK";
            
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
                return resp;
            } 
            else {
                std::ifstream t(path.string().substr(1));
                std::string extension = path.extension();
                //create enum of mime struct later

                std::string mime_type = getMIMEType(extension);
                if (mime_type == "") {
                    return respond(resp = getStockResponse(ServerError::FileNotFound));
                }
                resp.headers.push_back({"Content-Type", mime_type});
                std::stringstream buffer;
                buffer << t.rdbuf();
                header contentLength = {"Content-Length", std::to_string(buffer.str().length())};
                resp.headers.push_back(contentLength);
                resp.body = buffer.str();
                return resp;
            }
        }
    }
};
