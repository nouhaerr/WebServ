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
        file << _bodyPost;
        file.close();
		this->_errCode = 201;
		buildResponse(201);
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
	std::cout << _uploadPath << "\n";
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
    }
	std::ofstream file(_uploadPath.c_str());
	file << _bd;
	file.close();
}

void	HttpResponse::handlePostMethod(){
	if (!_isSupportedMethod("POST")) {
		buildResponse(405);
		return ;
	}

	if (_isSupportedUploadPath() && _filePath.find(".py") == std::string::npos && _filePath.find(".php") == std::string::npos) {
		std::string	file = _generateTempFileName();
		if (_uploadPath[_uploadPath.size() - 1] != '/')
    	{
       		_uploadPath += "/";
    	}
		_uploadPath += file;
		_createFile();
		std::ifstream bodyfile(_uploadPath.c_str());
		std::ostringstream filecontent;
		filecontent << bodyfile.rdbuf();
		_bd = filecontent.str();
		bodyfile.close();
		// std::remove(_uploadPath.c_str());
		processPostMethod();
		return ;
	}
	// else {

	// }
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