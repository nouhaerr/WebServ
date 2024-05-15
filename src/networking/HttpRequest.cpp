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
	// parseBody(bodypos);
}

void	HttpRequest::printRequestDetails() const 
{
    std::cout << "Method: " << _method << std::endl;
    std::cout << "URI: " << _uri << std::endl;
    std::cout << "HTTP Version: " << _httpVersion << std::endl;
    printHeaders();
    std::cout << "Body: " << _body << std::endl;
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


void HttpRequest::printHeaders() const 
{
    for (std::map<std::string, std::string>::const_iterator it = _headerFields.begin(); it != _headerFields.end(); ++it) 
        std::cout << it->first << ": " << it->second << std::endl;
}

void HttpRequest::parseBody(size_t &bodypos) 
{
    long long contentLength = 0;
    if (is_body(contentLength)) 
    {
        if (this->isChunked) {
            _getChunkedBody(bodypos);
			this->_bodySize = this->_body.size();
			// if (this->_confServ.getMaxBodySize() >= this->_bodySize) {//should check the max body in the conf file >= _bodySize
			// 	if (this->_method == "POST")
			// 		{
			// 			if (_isSupportedMethod())
			// 			{
			// 				std::string	file = _generateTempFileName();
			// 				std::string	uploadPath = _findUploadPath();
			// 				_createFile(uploadPath + file, this->_body);
			// 				printf("creyina l fileeee\n");
			// 			}
			// 			else
			// 				this->_errorCode = 405; //Method Not Allowed
			// 	}
			// }
			// else
			// 	this->_errorCode = 413; /*Content Too Large response status code indicates that
			// the request entity is larger than limits defined by server*/
		}
        else if (this->_method == "POST" && contentLength > 0)
        {
            std::string bodyContent(this->_request.begin() + bodypos, this->_request.end());
            this->_body = bodyContent;
			this->_bodySize = contentLength;
        }
    }
}

bool	HttpRequest::is_body(long long& contentLength) {
	std::map<std::string, std::string>::const_iterator it = _headerFields.find("Content-Length");

	if (it != _headerFields.end()) {
        contentLength = std::strtol(it->second.c_str(), NULL, 10);
        return true;
    }
	std::string	transfer_encod("Transfer-Encoding");

	if (_headerFields.find("Transfer-Encoding") != _headerFields.end() \
		&& _headerFields[transfer_encod].find("chunked") != std::string::npos) {
		this->isChunked = true;
		return true;
	}
	else if (_headerFields.find("Transfer-Encoding") != _headerFields.end() \
		&& _headerFields[transfer_encod].find("chunked") == std::string::npos) {
		this->_errorCode = 501; //Not implemented
		return false;
	}
	else if (this->_method == "post" && _headerFields.find("Content-Length") == _headerFields.end()
		&& _headerFields.find("Transfer-Encoding") == _headerFields.end()) {
		this->_errorCode = 400; //Bad Request
		return false;
	}
	return false; /*ayaetina makaynch body*/
}

void	HttpRequest::_getChunkedBody(size_t &bodypos) {
	// Implementation for chunked transfer encoding
	std::string	tmp = this->_request.substr(bodypos);
	size_t	bodySize = tmp.size();

	for (size_t i = 0; i < bodySize; i++) {
		std::string	chunk = "";
		size_t	j = i;
		for (; tmp[j] != '\r'; j++) {
			chunk += tmp[j];
		}
		i = j + 2;
		int	chunkedSize = hexToInt(chunk);
		if (chunkedSize == 0)
			break ;
		this->_body += tmp.substr(i, chunkedSize);
		i += chunkedSize + 1;
	}
}

// bool	HttpRequest::_isSupportedMethod() {
	// std::vector<std::string> _isAllowedMeth = this->_confServ._methods;
	// size_t	len = this->_confServ.loc.size();

	// std::vector<std::string>::iterator it = this->_confServ._methods.begin();
	// for(; it != this->_confServ._methods.end(); it++)
	// 	std::cout << *it << " ";
	// printf("\n");
	// for(size_t i = 0; i < len; i++) {
	// 	if (this->_uri.find(this->_confServ.loc[i].name) != std::string::npos
	// 		&& this->_confServ.loc[i]._methods.empty()) {//find the location Name in the uri
	// 		_isAllowedMeth = this->_confServ.loc[i]._methods;
	// 		break ;
	// 	} //update the method vector
	// }
		// Check if the request method is found in the vect of allowed methods
    // return (std::find(_isAllowedMeth.begin(), _isAllowedMeth.end(), this->_method) != _isAllowedMeth.end());
// }

std::string	HttpRequest::_findUploadPath() {
    // Find upload path logic
	// size_t	len = this->_confServ.getLocation.size();
	// std::string	download = this->_confServ.uploads; // get the uploadPath in the conf File if there is a section named upload

	// std::cout << "Upload Path: " << download << std::endl;

	// for(size_t i = 0; i < len; i++) {
	// 	if (this->_uri.find(this->_confServ.loc[i].name) != std::string::npos) //find the location Name in the uri
	// 		download = this->_confServ.loc[i].uploads; //get the upload Path
	// }
	// return download;
    return std::string();
}

void	HttpRequest::_createFile(const std::string& name, const std::string& reqBody) {
	std::ofstream file(name.c_str());

    if (file) {
        file << reqBody;
        file.close();
		this->_errorCode = 201; /*201 Created success status response code indicates
		that the request has succeeded and has led to the creation of a resource*/
    } 
    else {
        std::cerr << "Failed to create file." << std::endl;
        this->_errorCode = 500; // Internal Server Error
    }
}

std::string	HttpRequest::_generateTempFileName() {
	const char* alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-";
    std::string	tempName = "";
	std::srand(static_cast<unsigned int>(time(NULL)));
    for (int i = 0; i < 10; ++i) 
        tempName += alphanum[std::rand() % (sizeof(alphanum) - 1)];
    return tempName;
}

int	HttpRequest::hexToInt(const std::string& str) {
    int intValue;

    std::istringstream(str) >> std::hex >> intValue;
    return intValue;
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