#include "HttpRequest.hpp"

HttpRequest::HttpRequest() :
	_request(""),
	_method(""),
	_uri(""),
	_httpVersion(""),
	_body(""),
	isChunked(false),
	_bodySize(0),
	_errorCode(0) {}

HttpRequest::HttpRequest(const HttpRequest& other) :
	_request(other._request),
	_method(other._method),
	_uri(other._uri),
	_httpVersion(other._httpVersion),
	_headerFields(other._headerFields),
	_body(other._body),
	isChunked(other.isChunked),
	_bodySize(other._bodySize),
	_errorCode(other._errorCode) {}

HttpRequest& HttpRequest::operator=(const HttpRequest& other) {
	if (this != &other) 
	{
		_request = other._request;
		_method = other._method;
		_uri = other._uri;
		_httpVersion = other._httpVersion;
		_headerFields = other._headerFields;
		isChunked = other.isChunked;
		_body = other._body;
		_bodySize = other._bodySize;
		_errorCode = other._errorCode;
	}
	return *this;
}

HttpRequest::~HttpRequest() {
	_headerFields.clear();
}

std::string toLower(const std::string& str) 
{
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

void HttpRequest::parseHttpRequest(const std::string& req) 
{
    this->_request = req;
    std::istringstream requestStream(req);
    std::string line;
    std::getline(requestStream, line);
    std::istringstream lineStream(line);
    lineStream >> this->_method >> this->_uri >> this->_httpVersion;

    _parseMethod();
	// if (this->_errorCode != 501) {
		_parseURI();
		// if (this->_errorCode != 400 && this->_errorCode != 414)
		// {
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
		// }
	// }
}

void	HttpRequest::_parseMethod() {
	if (this->_method != "POST" && this->_method !="GET" && this->_method != "DELETE")
		this->_errorCode = 501; /*Not Implemented method*/
}

void	HttpRequest::_parseURI() {
	size_t questionMarkPos = this->_uri.find_first_of('?'); // Check for query parameters
	std::string	queryString;

	if (questionMarkPos != std::string::npos) {
		queryString = this->_uri.substr(questionMarkPos + 1);
		this->_uri = this->_uri.substr(0, questionMarkPos);
	}
	else {
		this->_uri = this->_uri;
		queryString.clear();
	}
	if (!this->_uri.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;="))
		this->_errorCode = 400; //Bad Request
	if (this->_uri.length() > 2048)
		this->_errorCode = 414; //414 URI Too Long
}

std::string HttpRequest::getHeader(const std::string& headerName) const 
{
    std::map<std::string, std::string>::const_iterator it = _headerFields.find(headerName);
    if (it != _headerFields.end()) 
        return it->second;
    return "";
}

void	HttpRequest::printRequestDetails() const {
    std::cout << "Method: " << _method << std::endl;
    std::cout << "URI: " << _uri << std::endl;
    std::cout << "HTTP Version: " << _httpVersion << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = _headerFields.begin(); it != _headerFields.end(); ++it) 
        std::cout << it->first << ": " << it->second << std::endl;
    std::cout << "Body: " << _body << std::endl;
}

void HttpRequest::setMethod(const std::string& m) { 
    this->_method = m; 
}

void HttpRequest::setUri(const std::string& u) { 
    this->_uri = u; 
}

void HttpRequest::setHttpVersion(const std::string& hv) { 
    this->_httpVersion = hv; 
}

void HttpRequest::setBody(const std::string& b) { 
    this->_body = b; 
}

void HttpRequest::setHeaderField(const std::string& name, const std::string& value) {
    this->_headerFields[name] = value;
}

std::string HttpRequest::getMethod() const { 
    return this->_method; 
}

std::string HttpRequest::getUri() const 
{ 
    return this->_uri; 
}
std::string HttpRequest::getHttpVersion() const { 
    return this->_httpVersion; 
}

const std::map<std::string, std::string>& HttpRequest::getHeaderFields() const { 
    return this->_headerFields; 
}

std::string HttpRequest::getBody() const { 
    return this->_body; 
}

bool HttpRequest::getIsChunked() const { 
    return this->isChunked; 
}

std::string HttpRequest::getRequest() const { 
    return this->_request; 
}

int	HttpRequest::getErrorCode() const {
	return this->_errorCode;
}
