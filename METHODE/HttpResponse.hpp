#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <iostream>

class HttpResponse {
public:
    void setStatusCode(int code) { statusCode = code; }
    void setStatusMessage(const std::string& message) { statusMessage = message; }
    void setBody(const std::string& bodyContent) { body = bodyContent; }
    void send() {
        std::cout << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
        std::cout << "Content-Length: " << body.size() << "\r\n";
        std::cout << "Connection: close\r\n";
        std::cout << "\r\n";
        std::cout << body;
    }

private:
    int statusCode;
    std::string statusMessage;
    std::string body;
};

#endif // HTTP_RESPONSE_HPP

