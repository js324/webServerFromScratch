#include <filesystem>
#include <fstream> 
#include <sstream>
#include <map>
#include <iostream>
#include <functional>
#include <locale>
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
    std::function<std::string(std::map<std::string, std::string>)> _action{};
    Route(std::string_view verb, std::filesystem::path path): _verb {verb}, _path {path} { }
    Route(std::string_view verb, std::filesystem::path path, std::function<std::string(std::map<std::string, std::string>)> action): 
        _verb {verb}, _path {path}, _action {action}  {
    }    
};


class Router {
private:
    std::string _websitePath{};
    std::function<std::string(HTTPStatusCode)> _onError;
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
    static response CustomResponse(std::string actionRes) {
        response resp{};
        resp.headers.push_back({"Content-Type", "application/json"});
        header contentLength = {"Content-Length", std::to_string(actionRes.length())};
        
        resp.headers.push_back(contentLength);
        resp.body = actionRes;
        return resp;

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
    void onError(std::function<std::string(HTTPStatusCode)> onError) {
        _onError = onError;
    }
    void AddRoute(Route route) {
        _routes.push_back(route);
    }
    std::string GetWebsitePath() {
        return _websitePath;
    }
    void SetWebsitePath(std::string websitePath) {
        _websitePath = websitePath;
    }

    response RouteReq(std::string verb,
    std::filesystem::path path,
    std::map<std::string, std::string> kvParams) {
        response resp{};
        std::string extension = path.extension().string();
        // std::string mime_type = getMIMEType(extension); //either keep the getmimetype or get rid of extensioninfo, probably get rid of extension info

        //TO DO: Better cleaning of path url (clean up /// slashes if inputted)
        
        auto routeIt = std::find_if(_routes.begin(), _routes.end(), [&verb, &path](Route route) { 
            for (auto& c : verb) {
                c = std::tolower(c);
            }
            return route._verb == verb && route._path == path; 
        });
        // std::cout << "Found route!" << std::endl;
        //possibly outside check to see if extension is valid (but right now we would default to .html)
        if (_extFolderMap.contains(extension)) {
            ExtensionInfo extInfo = _extFolderMap[extension];
            if (_routes.end() != routeIt) { 
                Route route = *routeIt;
                std::string actionRes = route._action ? route._action(kvParams) : "";

                if (actionRes.size()) {
                    std::cout << "testing!!" << std::endl;
                    resp = CustomResponse(actionRes);
                    return resp;
                }
                else {
                    if (extension == "") { //edge case where there is no extension given, default to html
                        extension = ".html";
                        path = path.string() + extension; 
                    }
                    
                    if (verifyPath(_websitePath+path.string())) {
                        resp = extInfo._loader(_websitePath+path.string(), extension, extInfo);
                    }
                    else {
                        std::cout << "Couldn't verify path: " << _websitePath+path.string() << std::endl;
                        resp = RouteReq("get", _onError(HTTPStatusCode::FileNotFound), {});
                        resp.error = HTTPStatusCode::FileNotFound;
                        return resp;
                    }
                }   
            }
            else if (verifyPath(_websitePath+path.string())) {
                resp = extInfo._loader(_websitePath+path.string(), extension, extInfo);
                return resp;
            }
            else {
                std::cout << "Error Thrown: No valid route or file found" << std::endl;
                resp = RouteReq("get", _onError(HTTPStatusCode::FileNotFound), {});
                resp.error = HTTPStatusCode::FileNotFound;
                return resp;
            }
        }
        else {
            std::cout << "Error Thrown: No valid extension" << std::endl;
            resp = RouteReq("get", _onError(HTTPStatusCode::FileNotFound), {});
            resp.error = HTTPStatusCode::FileNotFound;
            return resp;
        }
        return resp;
    }
};

