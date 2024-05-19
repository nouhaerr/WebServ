#include "HttpRequest.hpp"
#include <iostream>
#include <sstream>

HttpRequest::HttpRequest() : method(""), uri(""), httpVersion(""), body(""), isChunked(false) {}

HttpRequest::~HttpRequest() {}

std::string toLower(const std::string& str) 
{
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

void HttpRequest::parseHttpRequest(const std::string& req) 
{
    std::istringstream requestStream(req);
    std::string line;
    std::getline(requestStream, line);
    std::istringstream lineStream(line);
    lineStream >> method >> uri >> httpVersion;

    while (std::getline(requestStream, line) && line != "\r" && !line.empty()) 
    {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) 
        {
            std::string headerName = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 2);
            headerFields[toLower(headerName)] = headerValue;
        }
    }
}

std::string HttpRequest::getHeader(const std::string& headerName) const 
{
    std::map<std::string, std::string>::const_iterator it = headerFields.find(headerName);
    if (it != headerFields.end()) 
        return it->second;
    return "";
}

void HttpRequest::printRequestDetails() const 
{
    std::cout << "Method: " << method << std::endl;
    std::cout << "URI: " << uri << std::endl;
    std::cout << "HTTP Version: " << httpVersion << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = headerFields.begin(); it != headerFields.end(); ++it)
        std::cout << it->first << ": " << it->second << std::endl;
    std::cout << "Body: " << body << std::endl;
}

void HttpRequest::setMethod(const std::string& m) 
{ 
    method = m; 
}
void HttpRequest::setUri(const std::string& u) 
{ 
    uri = u; 
}
void HttpRequest::setHttpVersion(const std::string& hv) 
{ 
    httpVersion = hv; 
}
void HttpRequest::setBody(const std::string& b) 
{ 
    body = b; 
}
void HttpRequest::setHeaderField(const std::string& name, const std::string& value) { headerFields[name] = value; }

std::string HttpRequest::getMethod() const 
{ 
    return method; 
}
std::string HttpRequest::getUri() const 
{ 
    return uri; 
}
std::string HttpRequest::getHttpVersion() const 
{ 
    return httpVersion; 
}
const std::map<std::string, std::string>& HttpRequest::getHeaderFields() const 
{ 
    return headerFields; 
}
std::string HttpRequest::getBody() const 
{ 
    return body; 
}

bool HttpRequest::getIsChunked() const 
{ 
    return isChunked; 
}
std::string HttpRequest::getRequest() const 
{ 
    return ""; 
}

void HttpRequest::parseBody(size_t &bodypos, const std::string &requestString) 
{
    int contentLength = 0;
    if (is_body(contentLength)) 
    {
        if (isChunked) 
            getChunkedBody(bodypos);
        else 
        {
            if (bodypos + contentLength <= requestString.size()) 
            {
                std::string bodyContent = requestString.substr(bodypos, contentLength);
                body = bodyContent;
                std::cout << "Extracted body: " << bodyContent << std::endl; 
            } 
            else 
                std::cerr << "Error: bodypos is out of range. Request size: " << requestString.size() << " bodypos: " << bodypos << std::endl;
        }
    }
}

bool HttpRequest::is_body(int& contentLength) 
{
    std::map<std::string, std::string>::const_iterator it = headerFields.find("content-length");
    if (it != headerFields.end()) 
    {
        contentLength = std::stoi(it->second);
        return true;
    }
    return false;
}

void HttpRequest::getChunkedBody(size_t &bodypos) 
{
    (void)bodypos;
    // Implementation for chunked transfer encoding
}
