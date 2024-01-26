#include <filesystem>
#include <fstream> 
#include <sstream>
#include <map>
#include <iostream>
#include <functional>
#include <locale>
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
    std::map<std::string, std::string> _kvParams{};
    std::function<std::string(std::map<std::string, std::string>)> Action{};
    Route(std::string_view verb, std::filesystem::path path): _verb {verb}, _path {path} { }
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
                return !(relPath.empty() || (relPath.string()[0] == '.' && relPath.string() != "."));
            }
            catch (const std::filesystem::filesystem_error& e) {
                return false;
			}
        }
        return false;
    }
    static response PageLoader(std::string fullPath, std::string ext, ExtensionInfo extInfo)
    {
        if (fullPath == "/") {
            std::cout << "test";
            return FileLoader("/html/index.html", ".html", ExtensionInfo{"text/html"});
        }
        else {
            return FileLoader(fullPath, ext, extInfo);
        }
    }
    static response ImageLoader(std::string fullPath, std::string ext, ExtensionInfo extInfo)
    {
        response resp{};

        std::ifstream t(fullPath.substr(1));//has to remove first / to get relative 
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
        {"", ExtensionInfo{"text/html", &(Router::PageLoader)}},
        {".css", ExtensionInfo{"text/css", (Router::FileLoader)}},
        {".js", ExtensionInfo{"text/javascript", (Router::FileLoader)}},

    };    
    

public:
    void AddRoute(Route route) {
        _routes.push_back(route);
    }


    response route(std::string verb,
    std::filesystem::path path,
    std::map<std::string, std::string> kvParams) {
        response resp{};
        
        
        //TO DO: Better cleaning of path url (clean up /// slashes if inputted)
        
        auto routeIt = std::find_if(_routes.begin(), _routes.end(), [&verb, &path](Route route) { 
            for (auto& c : verb) {
                c = std::tolower(c);
            }
            return route._verb == verb && route._path == path; 
        });
        // std::cout << "Found route!" << std::endl;
        //possibly outside check to see if extension is valid (but right now we would default to .html)
        if (_routes.end() != routeIt) { 
           
            Route route = *routeIt;
            resp.headers.push_back({"Connection", "close"});
            std::string extension = path.extension().string();
            // std::string mime_type = getMIMEType(extension); //either keep the getmimetype or get rid of extensioninfo, probably get rid of extension info
            std::cout << "PATH: " << path.string() << " extension: " << extension << std::endl;
            ExtensionInfo extInfo = _extFolderMap[extension];
            std::string redirect = route.Action ? route.Action(kvParams) : "";
            // std::cout << "Found route!" << std::endl;
            if (redirect.length()) {
                resp.redirect = redirect;
            }
            else if (path.string() == "/") { //ugly way to handle case of hitting root of site
                resp = extInfo._loader(path.string(), extension, extInfo);  
            }
            else {
                if (extension == "") { //edge case where there is no extension given, default to html
                    extension = ".html";
                    path = path.string() + extension; 
                }
                 std::cout << "Found route!" << std::endl;
                if (verifyPath(ROOT_DIR+path.string())) {
                    
                    resp = extInfo._loader(ROOT_DIR+path.string(), extension, extInfo);
                }
                else {
                    resp = getStockResponse(ServerError::FileNotFound);
                    return resp;
                }
            }
            // else {
            //     resp = getStockResponse(ServerError::InternalServerError);

            // }

    
                
        }
        else {
            resp = getStockResponse(ServerError::InternalServerError);
            return resp;
        }
        std::cout << "RETURNING!" << std::endl;
        resp.status_code = 200;
        resp.reason = "OK";
        return resp;
    }
};

