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
	// std::cout << _uploadPath << "\n";
	_createFile();
	// std::ofstream file(_uploadPath.c_str());
	// file << _postBody;
	// file.close();
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
		_postBody = filecontent.str();
		bodyfile.close();
		std::remove(_bodyFileName.c_str());
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