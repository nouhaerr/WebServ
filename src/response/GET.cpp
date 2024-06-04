#include "HttpResponse.hpp"

std::string HttpResponse::_constructPath(const std::string& requestPath, const std::string &root, const std::string &index) {
	std::string path = requestPath;
	std::cout << path << "\n";
    if (path.empty() || path[0] != '/') {
        path = "/" + path;
    }
	std::cout << index << "\n";
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
	return getMimeTypes(extension);
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
                _contentType = getContentType(_filePath);
				std::string header = createResponseHeader(_errCode, "Nothing");
				_client.setResponseHeader(header);
				_client.setResponseBody(_filePath);
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
		// if (!directory.empty() && directory[directory.size() - 1] != '/')
    	// {
        // 	directory += '/';
    	// }
		struct dirent *ent;
    	while ((ent = readdir(dir)) != NULL)
    	{
			std::string	d_nm = ent->d_name;
			if (d_nm == ".." || d_nm == ".") {
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
				std::cerr << "Erreur lors de la création du fichier temporaire." << std::endl;
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

void	HttpResponse::handleGetMethod() {
	if (!_isSupportedMethod("GET")) {
		buildResponse(405);
		return ;
	}
	// std::cout << "f get: " << _filePath << "\n";
	struct stat path_stat;
	if (stat(_filePath.c_str(), &path_stat) != 0) {
        buildResponse(404);
        return;
    }
	if (S_ISREG(path_stat.st_mode)) {
        // Handle file
        std::ifstream file(_filePath.c_str(), std::ios::in | std::ios::binary);
		if (file) {
			printf("fiiiile\n");
			_contentType = getContentType(_filePath);
			std::string header = createResponseHeader(200, "Nothing");
			_client.setResponseHeader(header);
			_client.setResponseBody(_filePath);
		} 
	} else if (S_ISDIR(path_stat.st_mode))
	{
		std::cout << "foldeeer\n";
		isUrihasSlashInTHeEnd();
		if (isDirHasIndexFiles())
			return;
		else {
			_getAutoIndex();
			return ;
		}
	}
}

bool	HttpResponse::_isSupportedMethod(std::string meth) {
	size_t	len = _methods.size();

	for(size_t i = 0; i < len; i++) {
		if (_methods[i] == meth)
			return true ;
	}
    return false;
}
