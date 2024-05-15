#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "../Macros.hpp"
#include <cstdlib>
#include <ctime>
#include <cstring>

class HttpRequest {
public:
    HttpRequest();
    ~HttpRequest();

    void parseHttpRequest(const std::string& request);
    void parseBody(size_t &bodypos);
    bool is_body(int& contentLength);
    void getChunkedBody(size_t &bodypos);
    bool isSupportedMethod();
    std::string findUploadPath();
    void createFile(const std::string& name, const std::string& reqBody);
    std::string generateTempFileName();
    static int hexToInt(const std::string& str);
    std::string getHeader(const std::string& headerName) const;

    void printHeaders() const;
    void printRequestDetails() const;

    std::string getMethod() const;
    std::string getUri() const;
    std::string getHttpVersion() const;
    std::string getBody() const;
private:
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headerFields;
    bool isChunked;
    std::string body;
    int errorCode;
    std::string request;
};

#endif
