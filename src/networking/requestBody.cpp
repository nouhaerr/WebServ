#include "HttpRequest.hpp"

void	HttpRequest::parseBody(size_t &bodypos) 
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