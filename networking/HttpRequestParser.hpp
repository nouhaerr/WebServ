#ifndef HTTPREQUESTPARSER_HPP
#define HTTPREQUESTPARSER_HPP

#include "HttpRequest.hpp"

class HttpRequestParser {
public:
    static void parseHttpRequest(HttpRequest& request, const std::string& req);
    static void parseBody(HttpRequest& request, size_t &bodypos);
    static bool is_body(HttpRequest& request, int& contentLength);
    static void getChunkedBody(HttpRequest& request, size_t &bodypos);
};

#endif
