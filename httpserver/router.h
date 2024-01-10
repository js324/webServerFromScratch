#include <filesystem>
#include <fstream> 
#include <sstream>
#include <map>
#include <iostream>
#include <functional>
#include "mime_types.h"
#include "stock_response.h"

inline const std::string ROOT_DIR = "/html";

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
    bool verifyPath(std::filesystem::path path) {
        
        if (path.compare(path.root_directory())) {
            try { //really should avoid exceptions (why?)
                auto relPath = std::filesystem::canonical(path.string().substr(1));
                return !(relPath.empty() || relPath.string()[0] == '.' && relPath.string() != ".");
            }
            catch (const std::filesystem::filesystem_error& e) {
                return false;
			}
        }
    }
    static response PageLoader(std::string fullPath, std::string ext, ExtensionInfo extInfo)
    {
        if (fullPath == ROOT_DIR+"/") {
            return FileLoader("/html/index.html", ".html", ExtensionInfo{"text/html"});
        }
        else {
            return FileLoader(fullPath, ext, extInfo);
        }
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

        std::ifstream t(fullPath.substr(1));//has to remove first / to get relative 
        resp.headers.push_back({"Content-Type", extInfo._contentType});
        std::stringstream buffer;
        buffer << t.rdbuf();
        header contentLength = {"Content-Length", std::to_string(buffer.str().length())};
        header charset = {"charset", "utf-8"};
        resp.headers.push_back(charset);
        resp.headers.push_back(contentLength);
        resp.body = buffer.str();
        return resp;
    }


    std::vector<Route> _routes{};
    std::unordered_map<std::string, ExtensionInfo> _extFolderMap{
        {".png", ExtensionInfo{"image/png", (Router::ImageLoader)}}, 
        {".html", ExtensionInfo{"text/html", &(Router::PageLoader)}},
        {".css", ExtensionInfo{"text/css", (Router::FileLoader)}},
        {".js", ExtensionInfo{"text/javascript", (Router::FileLoader)}},

    };    
    

public:
    void AddRoute(Route route) {
        _routes.push_back(route);
    }


    response route(std::string_view verb,
    std::filesystem::path path,
    std::map<std::string, std::string> kvParams) {
        response resp{};
        
        path = ROOT_DIR + path.string();

        
        //TO DO: Better cleaning of path url (clean up /// slashes if inputted)
        

        if (verb == "GET") {
            resp.headers.push_back({"Connection", "close"});
            std::string extension = path.extension().string();
            // std::string mime_type = getMIMEType(extension); //either keep the getmimetype or get rid of extensioninfo, probably get rid of extension info
            std::cout << "PATH: " << path.string() << " extension: " << extension << std::endl;
            if (extension == "") {
                extension = ".html";
                path = path.string() + extension; 
            }
            if (verifyPath(path)) {
                resp = _extFolderMap[extension]._loader(path.string(), extension, _extFolderMap[extension]);   
            }
            else {
                resp = getStockResponse(ServerError::FileNotFound);
                return respond(resp);
            }

     
                
        }
        std::cout << "RETURNING!" << std::endl;
        resp.status_code = 200;
        resp.reason = "OK";
        return resp;
    }
};

