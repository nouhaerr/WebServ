
#include "HttpRequestParser.hpp"
#include <sstream>
#include <vector> // Include this header to use std::vector

void HttpRequestParser::parseHttpRequest(HttpRequest& request, const std::string& req) 
{
    std::istringstream requestStream(req);
    std::string line;
    std::getline(requestStream, line);
    std::istringstream lineStream(line);
    std::string method, uri, httpVersion;
    lineStream >> method >> uri >> httpVersion;

    request.setMethod(method);
    request.setUri(uri);
    request.setHttpVersion(httpVersion);

    while (std::getline(requestStream, line) && line != "\r" && !line.empty()) 
    {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) 
        {
            std::string headerName = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 2);
            request.setHeaderField(headerName, headerValue);
        }
    }

    if (request.getHeaderFields().find("Content-Length") != request.getHeaderFields().end()) 
    {
        long contentLength = std::strtol(request.getHeaderFields().at("Content-Length").c_str(), NULL, 10);
        if (contentLength > 0) {
            std::vector<char> buffer(contentLength);
            requestStream.read(&buffer[0], contentLength);
            std::string body(buffer.begin(), buffer.end());
            request.setBody(body);
        }
    }
}

void HttpRequestParser::parseBody(HttpRequest& request, size_t &bodypos) 
{
    long contentLength = 0;
    if (request.is_body(contentLength)) 
    {
        if (request.getIsChunked()) 
            getChunkedBody(request, bodypos);
        else 
        {
            std::string bodyContent(request.getBody().begin() + bodypos, request.getBody().end());
            request.setBody(bodyContent);
        }
    }
}

bool HttpRequestParser::is_body(HttpRequest& request, long& contentLength) 
{
    std::map<std::string, std::string>::const_iterator it = request.getHeaderFields().find("Content-Length");
    if (it != request.getHeaderFields().end()) 
    {
        contentLength = std::strtol(it->second.c_str(), NULL, 10);
        return true;
    }
    return false;
}

void HttpRequestParser::getChunkedBody(HttpRequest& request, size_t &bodypos) 
{
    (void)request;
    (void)bodypos;
}


// #include "HttpRequestParser.hpp"
// #include <sstream>
// #include <vector> // Include this header to use std::vector

// void HttpRequestParser::parseHttpRequest(HttpRequest& request, const std::string& req) 
// {
//     std::istringstream requestStream(req);
//     std::string line;
//     std::getline(requestStream, line);
//     std::istringstream lineStream(line);
//     std::string method, uri, httpVersion;
//     lineStream >> method >> uri >> httpVersion;

//     request.setMethod(method);
//     request.setUri(uri);
//     request.setHttpVersion(httpVersion);

//     while (std::getline(requestStream, line) && line != "\r" && !line.empty()) 
//     {
//         size_t colonPos = line.find(':');
//         if (colonPos != std::string::npos) 
//         {
//             std::string headerName = line.substr(0, colonPos);
//             std::string headerValue = line.substr(colonPos + 2);
//             request.setHeaderField(headerName, headerValue);
//         }
//     }

//     if (request.getHeaderFields().find("Content-Length") != request.getHeaderFields().end()) 
//     {
//         long contentLength = std::strtol(request.getHeaderFields().at("Content-Length").c_str(), NULL, 10);
//         if (contentLength > 0) {
//             std::vector<char> buffer(contentLength);
//             requestStream.read(&buffer[0], contentLength);
//             std::string body(buffer.begin(), buffer.end());
//             request.setBody(body);
//         }
//     }
// }

// void HttpRequestParser::parseBody(HttpRequest& request, size_t &bodypos) 
// {
//     long contentLength = 0;
//     if (request.is_body(contentLength)) 
//     {
//         if (request.getIsChunked()) 
//             getChunkedBody(request, bodypos);
//         else 
//         {
//             std::string bodyContent(request.getBody().begin() + bodypos, request.getBody().end());
//             request.setBody(bodyContent);
//         }
//     }
// }

// bool HttpRequestParser::is_body(HttpRequest& request, long& contentLength) 
// {
//     std::map<std::string, std::string>::const_iterator it = request.getHeaderFields().find("Content-Length");
//     if (it != request.getHeaderFields().end()) 
//     {
//         contentLength = std::strtol(it->second.c_str(), NULL, 10);
//         return true;
//     }
//     return false;
// }

// void HttpRequestParser::getChunkedBody(HttpRequest& request, size_t &bodypos) 
// {
//     (void)request;
//     (void)bodypos;
// }

