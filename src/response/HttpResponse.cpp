#include "HttpResponse.hpp"

HttpResponse::HttpResponse(NetworkClient &client) :
	_client(client),
	_serv(client.getConfigServer()),
	_body(""),
	_errCode(0),
	_statusCode(""),
	_isCgi(false),
	_root(""),
	_index(""),
    _idxFiles(),
	_errorPath(""),
	_autoindex(0),
	_fd(0),
	_filePath(""),
	_contentType("") {}

HttpResponse::~HttpResponse(){}

void	HttpResponse::locateErrorPage(int errCode) {
	for (std::map<int, std::string>::iterator it = _errorPage.begin(); it != _errorPage.end(); it++) {
		if (it->first == errCode) {
			_errorPath = _root + it->second;
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
	std::cout << _errCode << "\n";
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
	// if (req.getMethod() == "POST") {
	// 	handlePostMethod();
	// 	return ;
	// }
	// if (req.getMethod() == "DELETE") {
	// 	handleDeleteMethod();
	// 	return ;
	// }
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
	// std::cout << getContentLength() << "\n";
    if (flag == "Default") {
		_headers["Content-Length"] = getContentLength(_errorPath);
    	_headers["Content-Type"] = "text/html";
	}
    else {
		_headers["Content-Length"] = getContentLength(_filePath);
		_headers["Content-Type"] = _contentType;
	}
	_headers["Date"] = generateDate();
	// _headers["Content-Type"] = getContentType(_filePath);
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
    for (std::vector<ConfigLocation>::iterator it = _locations.begin(); it != _locations.end(); ++it)
    {
		std::cout << req.getUri();
		std::cout << " --> " << it->getLocationName() << "\n";
        if (req.getUri().find(it->getLocationName()) == 0)
        {
			std::cout << "dkhaaaal\n";
            _location = *it;
            _root = it->getRoot();
            // _index = it->getIndex()[0];
            _idxFiles = it->getIndex();
            _autoindex = it->getAutoIndex();
            _errorPage = it->getErrorPage();
            _methods = it->getMethods();
			if (_location.getRedirect() == true)
				_redirection = _location.getRedirection();

            int idx = req.getUri().find(it->getLocationName());
            std::string locationName = it->getLocationName();
            std::string relativePath = (req.getUri().find_first_of(locationName) == 0) ?
                                      req.getUri().substr(locationName.length()) :
                                      req.getUri().substr(0, idx);
            _filePath = _constructPath(relativePath, _root, "");

            // if (isDirectory(_filePath.c_str()))
            //     _filePath = constructFilePath(relativePath, _root, _index);

            if (_filePath[_filePath.length() - 1] == '/')
                _filePath = _filePath.substr(0, _filePath.length() - 1);

            // if (!_redirect.empty() && (req.getPath().substr(it->getLocationName().length()) == "/" ||
            //                            req.getPath().substr(it->getLocationName().length()) == "") &&
            //     req.getMethod() == "GET")
            // {
            //     _errCode = 301;
            //     return ;
            // }

            if (_autoindex)
            {
                _filePath = _constructPath(relativePath, _root, "");
                return _filePath;
            }

            return _filePath;
        }
    }
	_root = _serv.getRoot();
    // _index = _serv.getIndex()[0];
    _idxFiles = _serv.getIndex();
    _autoindex = _serv.getAutoIndex();
    _errorPage = _serv.getErrorPage();
    // _methods = _serv.getMethods();
    _filePath = _constructPath(req.getUri(), _root, _index);

    // std::cout << _filePath << "\n";
    return _filePath;


	// _uri = req.getUri();
	// _locations = _serv.getLocation();
	// size_t match_index = _locations.size(); // Initialize to an invalid index
	// std::string	location;

	// for (size_t i = 0; i < _locations.size(); ++i) {
	// 	const std::string &locationName = _locations[i].getLocationName();
	// 	size_t foundPos = _uri.find(locationName);
	// 	if (foundPos == 0)
	// 	{
	// 		bool is_longer_uri = _uri.size() > locationName.size();
	// 		bool not_slash_terminated = locationName != "/" && _uri[locationName.size()] != '/';

    //         if (is_longer_uri && not_slash_terminated) {
    //             continue;
    //         }
    //         if (locationName.size() > location.size()) {
	// 			std::cout << "FoundPos: "<< foundPos << "\n";
    //             location = locationName;
    //             match_index = i;
    //         }
    //     }
    // }

    // if (match_index == _locations.size() || _serv.getLocation()[match_index].getRoot().empty()) {
    //     req.getUri() = "";
    // } else {
    //     const std::string& root = _serv.getLocation()[match_index].getRoot();
    //     size_t root_pos = _uri.find(root);
    //     if (root_pos != std::string::npos) {
    //         req.getUri() = _uri;
    //     } else {
    //         req.getUri() = root + "/" + _uri.substr(location.size());
    //     }
    // }

	// 	if (_locations[match_index].getLocationName() == location) {
	// 		_methods = _locations[match_index].getMethods();
	// 		_autoindex = _locations[match_index].getAutoIndex();
	// 		_root = _locations[match_index].getRoot();
	// 		_errorPage = _locations[match_index].getErrorPage();
	// 		_indexes = _locations[match_index].getIndex();
	// 		if (_locations[match_index].getRedirect() == true)
	// 			_redirection = _locations[match_index].getRedirection();
	// 	}
    // req.getUri() = deleteRedundantSlash(req.getUri());
	// std::cout << "uri: "<< req.getUri() << "\n";
    // return req.getUri();
}
