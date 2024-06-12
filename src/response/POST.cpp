#include "HttpResponse.hpp"

bool	HttpResponse::_isSupportedUploadPath() {
    // Find upload path logic
	if (_uploadPath.empty()) {
		buildResponse(405);
		return 0;
	}
    return 1;
}

void	HttpResponse::_createFile() {
	std::ofstream file(_uploadPath.c_str());

    if (file) {
        file << _postBody;
        file.close();
		this->_errCode = 201;
		_headers["Location"] = _uploadPath;
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
	std::string	file = _generateTempFileName();
	if (_uploadPath[_uploadPath.size() - 1] != '/')
	{
       	_uploadPath += "/";
    }
	_uploadPath += file;
    std::map<std::string, std::string>::iterator it = _reqHeader.find("Content-Type");
	if(it != _reqHeader.end())
    {
		std::string value = trimHeader((*it).second);
        std::string contentType = getMimeTypes("second", value);
		if (contentType.empty()) {
			buildResponse(415);
			return ;
		}
        _uploadPath += contentType;
		_contentType = value;
    }
	// std::cout << "dyal resp: " << _uploadPath << "\n";
	_createFile();
}

void	HttpResponse::handlePostMethod(){
	if (!_isSupportedMethod("POST")) {
		buildResponse(405);
		return ;
	}

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
			// _isFile();
			return;
		}
		else if (type == FOLDER_TYPE) {
			// _postRequestFolder();
			return ;
		}
		else if (type == ERROR){
			buildResponse(404);
			return ;
		}
	}
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
                // _errCode = 200;
                _filePath = path;
                file.close();
				// _isFile();
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