#include "HttpResponse.hpp"

bool	HttpResponse::_isSupportedUploadPath() {
    // Find upload path logic
	if (_uploadPath.empty()) {
		buildResponse(405);
		return 0;
	}
    return 1;
}

void	HttpResponse::_createFile(std::string &filename) {
	std::ofstream file(_uploadPath.c_str());

    if (file) {
        file << _postBody;
        file.close();
		this->_errCode = 201;
		std::string hostt = _serv.getHost() + ":" + toString(_serv.getPort());
		std::string dirdir = findDirname(_uploadPath, _root) + "/";
		// std::cout << "dir " << dirdir << "\n";
		_headers["Location"] = "http://" + hostt + dirdir + filename;
		buildResponse(201);
		// std::cout << _uploadPath << "\n";
		 /*201 Created success status response code indicates
		that the request has succeeded and has led to the creation of a resource*/
    } 
    else {
        std::cerr << "Failed to create file." << std::endl;
        this->_errCode = 500; // Internal Server Error
		buildResponse(_errCode);
    }
}

void	HttpResponse::processPostMethod() {
	std::string	filename = _generateTempFileName();
	if (_uploadPath[_uploadPath.size() - 1] != '/')
	{
       	_uploadPath += "/";
    }
    std::map<std::string, std::string>::iterator it = _reqHeader.find("Content-Type");
	if(it != _reqHeader.end())
    {
		std::string value = trimHeader((*it).second);
        std::string contentType = getMimeTypes("second", value);
		if (contentType.empty()) {
			buildResponse(415);
			return ;
		}
		filename += contentType;
		_contentType = value;
    }
	// std::cout << "dyal resp: " << _uploadPath << "\n";
    _uploadPath += filename;
	_createFile(filename);
}

void	HttpResponse::handlePostMethod(){
	if (!_isSupportedMethod("POST")) {
		buildResponse(405);
		return ;
	}
			// 	if (extension == ".php" || extension == ".py")
			// {
			// //	std::cout<< "CGI FOUND !" << std::endl;
			// 	size_t pos;
			// 	CGI cgi(_client);
			// 		cgi.set_environmentVariables(file_name);
			// 		cgi.run();
			// 		if (cgi.status_code != 200)
			// 		{  
			// 			std::cout << "ERROCODE CGI " << std::endl;
			// 			buildResponse(cgi.status_code);
			// 			return;
			// 		}
			// 	std::string cgi_headers = extractHeaders(_client.getResponse());
			// 	pos = cgi_headers.find("Set-Cookie");
			// 	if (pos != std::string::npos)
			// 	{
			// 		cgi_headers = cgi_headers.substr(pos);
			// 		pos = cgi_headers.find("\r\n");
			// 		this->cookies = cgi_headers.substr(0, pos); 
			// 	}
			// 	std::string response_cgi = _client.getResponse();
			// 					//std::cout << _client.getResponse() << std::endl;

			// 	std::string c_t = findContentType(response_cgi);
			// 	_client.setResponseBody(extractBody(_client.getResponse()));
			// 	//std::cout << _client.getResponseBody() << std::endl;
			// 	std::stringstream ss;
			// 	ss << _client.getResponseBody().length();
			// 	std::string body_length = ss.str();
			// 	_client.setResponseHeader(createResponseHeader(200, c_t));
			// 	_isText = true;
			// 	return;
			// }
	if (_isSupportedUploadPath() && _filePath.find(".py") == std::string::npos && _filePath.find(".php") == std::string::npos) {
		std::ifstream bodyfile(_bodyFileName.c_str());
		std::ostringstream filecontent;
		filecontent << bodyfile.rdbuf();
		_postBody += filecontent.str();
		bodyfile.close();
		processPostMethod();
		return ;
	}
	else {
		int type = _checkRequestedType();
		if (type == FILE_TYPE) {
			_postRequestFile();
			return;
		}
		else if (type == FOLDER_TYPE) {
			// std::cout << "okkkkk\n";
			_postRequestFolder();
			return ;
		}
		else if (type == ERROR){
			buildResponse(404);
			return ;
		}
	}
}

void	HttpResponse::_postRequestFile() {
	// if (_location.)
}

void	HttpResponse::isUrihasSlashInTHeEnd() {
	size_t urisize = _client.getRequest().getUri().size();
	if ((_root[_root.size() - 1]) != '/' && _client.getRequest().getUri()[urisize - 1] != '/')
	{
		std::string hostt = _serv.getHost() + ":" + toString(_serv.getPort());
        std::string dirdir = _location.getLocationName().empty() ? findDirname(_filePath, _root) + "/" : _location.getLocationName() + findDirname(_filePath, _root) + "/";
        // std::cout << _filePath << " lastdir: " << dirdir<< "\n";
       _redirection = "http://" + hostt + dirdir;
	   std::string header = createResponseHeader(301, "Default");
    	_client.setResponseHeader(header);
        _client.setResponseBody(_errorPath);
		_slashSetted = true;
		return ;
	}

	// if (_filePath[_filePath.size() - 1] != '/')
    // {
	// 	std::cout << "kidkhol l hadi?\n";
    //    _filePath += "/";
    //     buildResponse(301);
	// 	_slashSetted = true;
    // }
}

void	HttpResponse::_postRequestFolder() {
	isUrihasSlashInTHeEnd();
	if (_slashSetted == true) {
		return ;
	}
	if (isPostDirHasIndexFiles())
		return;
	else {
		buildResponse(403);
	}
}

bool HttpResponse::isPostDirHasIndexFiles() {
	if (_idxFiles.size() != 0) {
		for (size_t i = 0; i <_idxFiles.size(); i++) {
			std::string path = _filePath + _idxFiles[i];
			// std::cout << "haas index\n" << path << "\n";
			path = deleteRedundantSlash(path);
			std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);

            if (file.is_open())
            {
                 _errCode = 200;
                _filePath = path;
                file.close();
				 _isFile();
                return true;
            }
			else {
				buildResponse(404);
				return true;
			}
		}
		return true;
	}
	return false;
}

std::string	HttpResponse::_generateTempFileName() {
	const char* alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-";
    const int charsetSize = sizeof(alphanum) - 1;
	std::string	tempName;

	std::srand(static_cast<unsigned int>(time(NULL)));
    for (int i = 0; i < 10; ++i) {
		int randomIndex = std::rand() % charsetSize;
        tempName += alphanum[randomIndex];
	}
    return tempName;
}