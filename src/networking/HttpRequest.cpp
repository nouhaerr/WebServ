#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : isChunked(false), errorCode(0) {}

HttpRequest::~HttpRequest() {}

void HttpRequest::parseHttpRequest(const std::string& req) 
{
    this->request = req;
    std::istringstream requestStream(this->request);
    std::string line;
    std::getline(requestStream, line);
    std::istringstream lineStream(line);
    lineStream >> this->method >> this->uri >> this->httpVersion;

    while (std::getline(requestStream, line) && line != "\r" && !line.empty()) 
    {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) 
        {
            std::string headerName = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 2);
            headerFields[headerName] = headerValue;
        }
    }

    if (headerFields.find("Content-Length") != headerFields.end()) 
    {
        int contentLength = std::stoi(headerFields["Content-Length"]);
        if (contentLength > 0) 
        {
            std::vector<char> buffer(contentLength);
            requestStream.read(&buffer[0], contentLength);
            body.assign(buffer.begin(), buffer.end());
        }
    }
}

void HttpRequest::printRequestDetails() const 
{
    std::cout << "Method: " << method << std::endl;
    std::cout << "URI: " << uri << std::endl;
    std::cout << "HTTP Version: " << httpVersion << std::endl;
    printHeaders();
    std::cout << "Body: " << body << std::endl;
}

std::string HttpRequest::getBody() const 
{
    return body;
}

std::string HttpRequest::getHeader(const std::string& headerName) const 
{
    std::map<std::string, std::string>::const_iterator it = headerFields.find(headerName);
    if (it != headerFields.end()) 
        return it->second;
    else 
        return "";  // Retourne une chaîne vide si l'en-tête n'est pas trouvé
}


void HttpRequest::printHeaders() const 
{
    for (std::map<std::string, std::string>::const_iterator it = headerFields.begin(); it != headerFields.end(); ++it) 
        std::cout << it->first << ": " << it->second << std::endl;
}

void HttpRequest::parseBody(size_t &bodypos) 
{
    int contentLength = 0;
    if (is_body(contentLength)) 
    {
        if (this->isChunked) 
            getChunkedBody(bodypos); 
        else 
        {
            std::string bodyContent(this->request.begin() + bodypos, this->request.end());
            this->body = bodyContent;
        }
    }
}

bool HttpRequest::is_body(int& contentLength) 
{
    std::map<std::string, std::string>::const_iterator it = headerFields.find("Content-Length");
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
        errorCode = 500; // Internal Server Error
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
    return this->method;
}

std::string HttpRequest::getUri() const 
{
    return this->uri;
}

std::string HttpRequest::getHttpVersion() const 
{
    return this->httpVersion;
}