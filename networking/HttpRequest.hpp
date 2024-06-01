#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

class HttpRequest {
private:
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headerFields;
    std::string body;
    bool isChunked;

public:
    HttpRequest();
    ~HttpRequest();

    void parseHttpRequest(const std::string& req);
    std::string getHeader(const std::string& headerName) const;
    void printRequestDetails() const;

    void setMethod(const std::string& m);
    void setUri(const std::string& u);
    void setHttpVersion(const std::string& hv);
    void setBody(const std::string& b);
    void setHeaderField(const std::string& name, const std::string& value);

    std::string getMethod() const;
    std::string getUri() const;
    std::string getHttpVersion() const;
    const std::map<std::string, std::string>& getHeaderFields() const;
    std::string getBody() const;

    bool getIsChunked() const;
    std::string getRequest() const;

    void parseBody(size_t &bodypos, const std::string &requestString);
    bool is_body(int& contentLength);
    void getChunkedBody(size_t &bodypos);
};

#endif
