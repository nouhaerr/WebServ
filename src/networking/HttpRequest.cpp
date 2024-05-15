#include "HttpRequest.hpp"

HttpRequest::HttpRequest() :
	_request(""),
	_method(""),
	_uri(""),
	_httpVersion(""),
	isChunked(false),
	_body(""),
	_bodySize(0),
	_errorCode(0)
{}

HttpRequest::HttpRequest(const HttpRequest& other) :
	_request(other._request),
	_method(other._method),
	_uri(other._uri),
	_httpVersion(other._httpVersion),
	_headerFields(other._headerFields),
	isChunked(other.isChunked),
	_body(other._body),
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

HttpRequest::~HttpRequest() {}

void HttpRequest::parseHttpRequest(const std::string& req) 
{
    this->_request = req;
    std::istringstream	requestStream(this->_request);
    std::string			line;
    std::getline(requestStream, line);
    std::istringstream lineStream(line);
    lineStream >> this->_method >> this->_uri >> this->_httpVersion;

	_parseMethod();
	if (this->_errorCode != 501) {
		_parseURI();
		if (this->_errorCode != 400 && this->_errorCode != 414)
		{
			while (std::getline(requestStream, line) && line != "\r" && !line.empty()) 
			{
				size_t colonPos = line.find(':');
				if (colonPos != std::string::npos) {
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
			// parseBody(bodypos);
			// handleDelete;
		}
	}
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
	// else if (_uri.find("..") != std::string::npos)
	// 	this->_errCode = 403; //403 Forbidden: Accès interdit. Traversée de répertoire non autorisée.
}

void	HttpRequest::printRequestDetails() const 
{
    std::cout << "Method: " << _method << std::endl;
    std::cout << "URI: " << _uri << std::endl;
    std::cout << "HTTP Version: " << _httpVersion << std::endl;
    printHeaders();
    std::cout << "Body: " << _body << std::endl;
}

void HttpRequest::printHeaders() const 
{
    for (std::map<std::string, std::string>::const_iterator it = _headerFields.begin(); it != _headerFields.end(); ++it) 
        std::cout << it->first << ": " << it->second << std::endl;
}

std::string	HttpRequest::getBody() const {
    return this->_body;
}

std::string	HttpRequest::getHeader(const std::string& headerName) const {
    std::map<std::string, std::string>::const_iterator it = _headerFields.find(headerName);
    if (it != _headerFields.end()) 
        return it->second;
    else 
        return "";  // Retourne une chaîne vide si l'en-tête n'est pas trouvé
}

std::string HttpRequest::getMethod() const 
{
    return this->_method;
}

std::string HttpRequest::getUri() const {
    return this->_uri;
}

std::string HttpRequest::getHttpVersion() const {
    return this->_httpVersion;
}