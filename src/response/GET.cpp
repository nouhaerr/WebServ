#include "HttpResponse.hpp"

std::string HttpResponse::_constructPath(const std::string& requestPath, const std::string &root, const std::string &index) {
	std::string path = requestPath;
	// std::cout << path << "\n";
    if (path.empty() || path[0] != '/') {
        path = "/" + path;
    }
	// std::cout << index << "\n";
	if (!path.empty() && path[path.length() - 1] == '/') {
        path += index;
    }
    // Check if there is no extension by finding the last dot in the string
    else if (path.find_last_of('.') == std::string::npos) {
        path += "/" + index;
    }

    std::string filePath = root + path;
    return filePath;
}

std::string getContentType(std::string filename) {
    std::string extension;
    if (filename.empty()) {
        return "text/html";
    }
    try {
	    extension = filename.substr(filename.find_last_of('.'));
    }
    catch (std::exception &e) {
        return "text/html";
    }
	return getMimeTypes("", extension);
}

void	HttpResponse::isUrihasSlashInTHeEnd() {
	//    std::cout << "adding slash: "<< _filePath[_filePath.size() - 1] << "\n";
	// std::cout << _filePath.size() << "\n";
	if (_filePath[_filePath.size() - 1] != '/')
    {
       _filePath += "/";
	//    std::cout << "adding slash: "<< _filePath << "\n";
        buildResponse(301);
    }
}

bool HttpResponse::isDirHasIndexFiles() {
	if (_idxFiles.size() != 0) {
		for (size_t i = 0; i <_idxFiles.size(); i++) {
			std::string path = _filePath + _idxFiles[i];
			std::cout << "haas index\n" << path << "\n";
			path = deleteRedundantSlash(path);
			std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);

            if (file.is_open())
            {
                _errCode = 200;
                _filePath = path;
                file.close();
				_isFile();
                // _contentType = getContentType(_filePath);
				// std::string header = createResponseHeader(_errCode, "Nothing");
				// _client.setResponseHeader(header);
				// _client.setResponseBody(_filePath);
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

std::string	HttpResponse::_findDirectoryName() {
	// Create a string stream from the path
    std::istringstream iss(_filePath);

    // Tokenize the path by '/'
    std::string token;
    std::string lastDirName;
    while (std::getline(iss, token, '/')) {
        if (!token.empty()) {
            lastDirName = token;
        }
    }
    return lastDirName;
}

void	HttpResponse::_getAutoIndex() {
	if (_autoindex) {
		std::string path = _filePath;
    	DIR *dir = opendir(path.c_str());

    	if (dir == NULL)
    	{
        	return;
    	}
		std::size_t lastSlashPos = path.find_last_of('/');
    	std::string pathff = (lastSlashPos != std::string::npos) ? path.substr(0, lastSlashPos + 1) : "";
		std::string directory = _findDirectoryName();

		std::ostringstream listeningfile;
		listeningfile << "<!DOCTYPE html>\n" << "<html lang=\"en\">\n" << "\t<head>\n" << "\t\t<meta charset=\"UTF-8\">\n";
		listeningfile << "\t\t\t<title>Index of " << directory << "</title>\n";
		listeningfile << "</head>\n<body bgcolor=\"white\"><h1>Index of " << directory << "</h1><hr><pre>";
		struct dirent *ent;
    	while ((ent = readdir(dir)) != NULL)
    	{
			std::string	d_nm = ent->d_name;
			if (d_nm == ".." || d_nm == ".")
			{
				d_nm += '/';
			std::cout << d_nm << "\n";
        	listeningfile << "<a href=\"" <<  d_nm << directory<< "\">" << ent->d_name << "</a><br>";

			}
			else
        		listeningfile << "<a href=\"" << directory + '/' << ent->d_name << "\">" << ent->d_name << "</a><br>";
    	}
		closedir(dir);
		listeningfile << "</pre><hr></body></html>";
		_fileSize = listeningfile.str().size();
		char tempDirTemplate[] = "/tmp/tmpDirXXXXXX";
    	char *tempDir = mkdtemp(tempDirTemplate);
		if (tempDir)
		{
			chmod(tempDir, S_IRWXU | S_IRWXG | S_IRWXO);
			// Créer le chemin complet du fichier HTML
			char tempFileName[1024];
			strcpy(tempFileName, tempDir);
			strcat(tempFileName, "/index.html");

			FILE *tempFile = std::fopen(tempFileName, "w");
			if (tempFile)
			{
				std::fputs(listeningfile.str().c_str(), tempFile);
				std::fclose(tempFile);
				_filePath = tempFileName;
				_client.setResponseBody(tempFileName);
			} else
			{
				_errCode = 500;
				buildResponse(_errCode);
				std::cerr << "Error: Failed to Creat Temporary file." << std::endl;
				return ;
			}
		}
    	else {
			_errCode = 500;
			printf("haaadi hhh\n");
			buildResponse(_errCode);
			return;
		}
		_client.setResponseHeader(createResponseHeader(200, "Nothing"));
	}
	else
		buildResponse(403);
}

void	HttpResponse::_isFile() {
    // Handle file
    std::ifstream file(_filePath.c_str(), std::ios::in | std::ios::binary);
	if (file) {
		std::string extension = _filePath.substr(_filePath.find_last_of('.'));
		// if (extension == ".py" || extension == ".rb" || extension == ".php") {
		// 	_isCgi = true;
		// 	return ;
		// }
		std::cout << "the file exist: " << _filePath<< "\n";
		_contentType = getContentType(_filePath);
		std::string header = createResponseHeader(200, "Nothing");
		_client.setResponseHeader(header);
		_client.setResponseBody(_filePath);
		// _isfile = true;
		return ;
	}
	else {
		buildResponse(404);
		return ;
	}
}

#include <cerrno>

int	HttpResponse::_checkRequestedType() {
	struct stat path_stat;
	if (stat(_filePath.c_str(), &path_stat) != 0) {
		std::cerr << "Error accessing file: " << strerror(errno) << std::endl;
		_errCode = 404;
        return ERROR;
    }
	if (S_ISREG(path_stat.st_mode))
		return FILE_TYPE;
	else if (S_ISDIR(path_stat.st_mode))
		return FOLDER_TYPE;
	return 3;
}

void	HttpResponse::_isFolder() {
	std::cout << "foldeeer\n";
	isUrihasSlashInTHeEnd();
	if (isDirHasIndexFiles())
		return;
	else {
		_getAutoIndex();
		return ;
	}
}

void	HttpResponse::handleGetMethod() {
	if (!_isSupportedMethod("GET")) {
		buildResponse(405);
		return ;
	}
	int type = _checkRequestedType();
	std::cout << "type: "<<type <<std::endl;
	if (type == FILE_TYPE) {
		_isFile();
		return;
	}
	else if (type == FOLDER_TYPE) {
		_isFolder();
		return ;
	}
	else if (type == ERROR){
		buildResponse(404);
		return ;
	}
}

bool	HttpResponse::_isSupportedMethod(std::string meth) {
	size_t	len = _methods.size();

	for(size_t i = 0; i < len; i++) {
		if (_methods[i] == meth) {
			return true ;
		}
	}
    return false;
}
