#include "HttpResponse.hpp"

HttpResponse::HttpResponse(NetworkClient &client) :
	_client(client),
	_serv(client.getConfigServer()),
	_bodyPost(""),
    _bd(""),
	_errCode(0),
	_statusCode(""),
	_isCgi(false),
	_root(""),
    _uploadPath(""),
	_index(""),
    _idxFiles(),
	_errorPath(""),
	_autoindex(0),
    _methods(),
    _uri(""),
	_fd(0),
	_filePath(""),
    _buffer(""),
    _isfile(false),
	_contentType(""),
    _reqHeader()
    {}

HttpResponse::~HttpResponse(){}

void	HttpResponse::locateErrorPage(int errCode) {
	for (std::map<int, std::string>::iterator it = _errorPage.begin(); it != _errorPage.end(); it++) {
		if (it->first == errCode) {
			_errorPath = it->second;
			_errorPath = deleteRedundantSlash(_errorPath);
			std::cout << _errorPath << "\n";
			return ;
		}
	}
	_errorPath = "";
}

void	HttpResponse::_handleDefaultErrors() {
	if (_errCode == 505) {
		buildResponse(505);
		return ;
	}
	if (_errCode == 501) {
		buildResponse(501);
		return ;
	}
	if (_errCode == 400 || _errCode == 414 || _errCode == 413) {
		buildResponse(_errCode);
		return ;
	}
}

void	HttpResponse::generateResponse(HttpRequest &req) {
	_errCode = req.getErrorCode();
	std::cout << "errcode result from req:" << _errCode << "\n";
	_uri = getRequestedResource(req);
	_filePath = deleteRedundantSlash(_uri);
	std::cout << "filePath: "<< _filePath << "\n";
	if (_filePath.empty()) {
		buildResponse(404);
		return;
	}
	checkHttpVersion(req);
	if (_errCode != 0 && _errCode != 201) {
		_handleDefaultErrors();
		return ;
	}
	if (!_redirection.empty()) {
		std::string header = createResponseHeader(301, "Default");
    	_client.setResponseHeader(header);
		return;
	}
	if (req.getMethod() == "GET") {
	 	handleGetMethod();
		return ;
	}
	if (req.getMethod() == "POST") {
        _bodyPost = req.getBody();
        _reqHeader = req.getHeaderFields();
		handlePostMethod();
		return ;
	}
	if (req.getMethod() == "DELETE") {
		handleDeleteMethod(); //where u put the Delete method
		return ;
	}
	return ;
}

void	HttpResponse::findStatusCode(int code) {
    switch (code)
    {
        case 0:
            _statusCode = "200 OK\r\n";
            break;
        case 201:
            _statusCode = "201 Created\r\n";
            break;
        case 400:
            _statusCode = "400 Bad Request\r\n";
            break;
        case 301:
            _statusCode = "301 Moved Permanently\r\n";
            break;
        case 302:
            _statusCode = "302 Moved Temporarily\r\n";
            break;
        case 403:
            _statusCode = "403 Forbidden\r\n";
            break;
        case 404:
            _statusCode = "404 Not Found\r\n";
            break;
        case 405:
            _statusCode = "405 Method Not Allowed\r\n";
            break;
        case 413:
            _statusCode = "413 Payload Too Large\r\n";
            break;
		case 414:
			_statusCode = "414 URI Too Long\r\n";
			break;
        case 415:
			_statusCode = "415 Unsupported Media Type\r\n";
            break;
        case 500:
            _statusCode = "500 Internal Server Error\r\n";
            break;
        case 501:
            _statusCode = "501 Not Implemented\r\n";
            break;
        case 505:
            _statusCode = "505 HTTP Version Not Supported\r\n";
            break;
        default:
            _statusCode = "200 OK\r\n";
            break;
    }
}

std::string	HttpResponse::getContentLength(std::string path) {
	struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == 0) 
    {
		_fileSize = fileStat.st_size;
		std::cout << "file exist of size: " << _fileSize << "\n";
        std::ostringstream oss;
        oss << fileStat.st_size;
        return oss.str();
    }
    return "0";
}

std::string HttpResponse::generateDate()
{
    time_t t = time(0);
    struct tm *current_time = localtime(&t);
    char date_buffer[80];
    strftime(date_buffer, sizeof(date_buffer), "%a, %d %b %Y %H:%M:%S GMT", current_time);
    return (std::string(date_buffer));
}

std::string	HttpResponse::createResponseHeader(int errCode, std::string flag) {
	std::string	respHeader;

	_headers["server"] = "Webserv/1.0";
	if (!_redirection.empty()) {
		_headers["Location"] = _redirection;
		_errCode = 301;
	}
    if (flag == "Default") {
		_headers["Content-Length"] = getContentLength(_errorPath);
    	_headers["Content-Type"] = "text/html";
	}
    else {
		_headers["Content-Length"] = getContentLength(_filePath);
		_headers["Content-Type"] = _contentType;
	}
	_headers["Date"] = generateDate();
	std::stringstream ss;

	findStatusCode(errCode);
	_errCode = errCode;
    if (_errCode == 0)
        findStatusCode(0);	
    ss << "HTTP/1.1 " << _statusCode;
    for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++)
	{
		ss << it->first << ": " << it->second << "\r\n";
	}
	ss << "\r\n";
	_buffer = ss.str();
	return _buffer;
}

void	HttpResponse::buildResponse(int errCode) {
	_errCode = errCode;
	std::string statusCodeStr = toString(_errCode);
    locateErrorPage(_errCode);
    std::string header = createResponseHeader(_errCode, "Default");

    _client.setResponseHeader(header);
    _client.setResponseBody(_errorPath);
}

void	HttpResponse::checkHttpVersion(HttpRequest &req) {
	std::string const &version = req.getHttpVersion();
	if (version != "HTTP/1.1") {
		_errCode = 505;
		return ;
	}
}

std::string HttpResponse::deleteRedundantSlash(std::string uri)
{
    std::string newUri;
    std::string::size_type i = 0;

    while (i < uri.size()) {
		// Add current character to newUri
        newUri.push_back(uri[i]);
        // If the current character is a slash, skip all subsequent slashes
        if (uri[i] == '/') {
            while (i < uri.size() && uri[i] == '/') {
                ++i;
            }
        } else {
            ++i;
        }
    }
    return newUri;
}

std::string	HttpResponse::getRequestedResource(HttpRequest &req) {

	_locations = _serv.getLocation();
	std::string location;
    for (std::vector<ConfigLocation>::iterator it = _locations.begin(); it != _locations.end(); ++it)
    {
		std::cout << req.getUri();
		std::cout << " --> " << it->getLocationName() << "\n";
        size_t pos = req.getUri().find(it->getLocationName());
        if (pos != std::string::npos && pos == 0)
        {
			std::cout << "dkhaaaal\n";
			if (req.getUri().size() > it->getLocationName().size()
				&& it->getLocationName() != "/"
				&& req.getUri()[it->getLocationName().size()] != '/');
			else if (it->getLocationName().size() > location.size())
			{
            	_location = *it;
				location = it->getLocationName();
			}
		}
	}
	if (req.getUri().find(_location.getRoot()) == std::string::npos)
	{
            _root = _location.getRoot();
            _idxFiles = _location.getIndex();
            _autoindex =_location.getAutoIndex();
            _errorPage =_location.getErrorPage();
            _methods = _location.getMethods();
            _uploadPath = _location.getUpload();
            // std::cout << "inMatch " << _uploadPath << "\n";
			if (_location.getRedirect() == true)
				_redirection = _location.getRedirection();

            int idx = req.getUri().find(_location.getLocationName());
            std::string locationName = _location.getLocationName();
            std::string relativePath = (req.getUri().find_first_of(locationName) == 0) ?
                                      req.getUri().substr(locationName.length()) :
                                      req.getUri().substr(0, idx);
            _filePath = _constructPath(relativePath, _root, "");

            if (_filePath[_filePath.length() - 1] == '/')
                _filePath = _filePath.substr(0, _filePath.length() - 1);

            if (_autoindex)
            {
                _filePath = _constructPath(relativePath, _root, "");
                return _filePath;
            }

        return _filePath;
	}
	_root = _serv.getRoot();
    _idxFiles = _serv.getIndex();
    _autoindex = _serv.getAutoIndex();
    _errorPage = _serv.getErrorPage();
    _methods.push_back("POST");
    _methods.push_back("GET");
    _methods.push_back("DELETE");
	_autoindex = _serv.getAutoIndex();
    _filePath = _constructPath(req.getUri(), _root, _index);
 	if (_autoindex)
    {
		std::cout << "AUTOINDEX\n";
		if (req.getUri().find(_root) == std::string::npos)
		{
		std::cout << "root" << _root << "\n";
        _filePath = _constructPath(req.getUri(), _root, "");
		}
		return _filePath;
    }
    else
    {
        _filePath = _constructPath(req.getUri(), _root, _index);
    }
    // std::cout << _filePath << "\n";
    return _filePath;
}
