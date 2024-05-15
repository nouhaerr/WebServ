#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : isChunked(false), _errorCode(0) {}

HttpRequest::~HttpRequest() {}

void HttpRequest::parseHttpRequest(const std::string& req) 
{
    this->_request = req;
    std::istringstream requestStream(this->_request);
    std::string line;
    std::getline(requestStream, line);
    std::istringstream lineStream(line);
    lineStream >> this->_method >> this->_uri >> this->_httpVersion;

    while (std::getline(requestStream, line) && line != "\r" && !line.empty()) 
    {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) 
        {
            std::string headerName = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 2);
            _headerFields[headerName] = headerValue;
        }
    }

    if (_headerFields.find("Content-Length") != _headerFields.end()) 
    {
        int contentLength = std::stoi(_headerFields["Content-Length"]);
        if (contentLength > 0) 
        {
            std::vector<char> buffer(contentLength);
            requestStream.read(&buffer[0], contentLength);
            _body.assign(buffer.begin(), buffer.end());
        }
    }
}

void HttpRequest::printRequestDetails() const 
{
    std::cout << "Method: " << _method << std::endl;
    std::cout << "URI: " << _uri << std::endl;
    std::cout << "HTTP Version: " << _httpVersion << std::endl;
    printHeaders();
    std::cout << "Body: " << _body << std::endl;
}

std::string HttpRequest::getBody() const 
{
    return _body;
}

std::string HttpRequest::getHeader(const std::string& headerName) const 
{
    std::map<std::string, std::string>::const_iterator it = _headerFields.find(headerName);
    if (it != _headerFields.end()) 
        return it->second;
    else 
        return "";  // Retourne une chaîne vide si l'en-tête n'est pas trouvé
}


void HttpRequest::printHeaders() const 
{
    for (std::map<std::string, std::string>::const_iterator it = _headerFields.begin(); it != _headerFields.end(); ++it) 
        std::cout << it->first << ": " << it->second << std::endl;
}

void HttpRequest::parseBody(size_t &bodypos) 
{
    int contentLength = 0;
    if (is_body(contentLength)) 
    {
        if (this->isChunked) 
            _getChunkedBody(bodypos);
        else 
        {
            std::string bodyContent(this->_request.begin() + bodypos, this->_request.end());
            this->_body = bodyContent;
        }
    }
}

bool HttpRequest::is_body(int& contentLength) 
{
    std::map<std::string, std::string>::const_iterator it = _headerFields.find("Content-Length");
    if (it != _headerFields.end()) 
    {
        contentLength = std::stoi(it->second);
        return true;
    }
    return false;
}

void HttpRequest::_getChunkedBody(size_t &bodypos) 
{
    (void)bodypos;
    // Implementation for chunked transfer encoding
}

bool HttpRequest::isSupportedMethod() 
{
    // Implementation to check if the method is supported
    return true;
}

std::string HttpRequest::findUploadPath() 
{
    // Find upload path logic
    return std::string();
}

void HttpRequest::createFile(const std::string& name, const std::string& reqBody) 
{
    std::ofstream file(name.c_str());
    if (file) 
    {
        file << reqBody;
        file.close();
    } 
    else 
    {
        std::cerr << "Failed to create file." << std::endl;
        _errorCode = 500; // Internal Server Error
    }
}

std::string HttpRequest::generateTempFileName() 
{
    const char* alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string tempName;
    for (int i = 0; i < 10; ++i) 
        tempName += alphanum[rand() % (sizeof(alphanum) - 1)];
    return tempName;
}

int HttpRequest::hexToInt(const std::string& str) 
{
    int intValue;
    std::istringstream(str) >> std::hex >> intValue;
    return intValue;
}

std::string HttpRequest::getMethod() const 
{
    return this->_method;
}

std::string HttpRequest::getUri() const 
{
    return this->_uri;
}

std::string HttpRequest::getHttpVersion() const 
{
    return this->_httpVersion;
}