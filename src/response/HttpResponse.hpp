#ifndef HTTPRESPONE_HPP
# define HTTPRESPONSE_HPP

#include "../Macros.hpp"
#include "../parsing/Config.hpp"
#include "../networking/NetworkClient.hpp"
#include "../networking/HttpRequest.hpp"
#include <sys/sendfile.h>

#define FILE_TYPE 1
#define FOLDER_TYPE 0
#define ERROR 2

class NetworkClient;
class HttpRequest;
class HttpResponse {
	public:
		HttpResponse(NetworkClient &client);
		~HttpResponse();

		void	generateResponse(HttpRequest &req);
		void	buildResponse(int errCode);
		void	locateErrorPage(int errCode);
		void	checkHttpVersion(HttpRequest &req);
		std::string	getContentLength(std::string path);
		std::string	createResponseHeader(int errCode, std::string flag);
		void	findStatusCode(int code);
		std::string	getRequestedResource(HttpRequest &req);
		std::string generateDate();
		std::string deleteRedundantSlash(std::string uri);

		void	handleGetMethod();
		void	isUrihasSlashInTHeEnd();
		bool	isDirHasIndexFiles();

		void	handlePostMethod();
		void	processPostMethod();

		void	handleDeleteMethod();
	private:
		NetworkClient&	_client;
		ConfigServer	_serv;
		std::string		_bodyPost;
		std::string		_bd;
		int				_errCode;
		std::string		_statusCode;
		bool			_isCgi;
		std::string		_root;
		std::string		_uploadPath;
		std::string		_index;
		std::vector<std::string>	_idxFiles;
		std::map<int, std::string>	_errorPage;
		std::string		_errorPath;
		int				_autoindex;
		std::vector<std::string>	_methods;
		std::string		_redirection;
		std::vector<ConfigLocation>	_locations;
		ConfigLocation	_location;
		std::string	_uri;
		int			_fd;
		std::map<std::string, std::string> _headers;
		std::string _filePath;
		std::string _buffer;
		off_t		_fileSize;
		off_t		_offset;
		bool		_isfile;
		std::string _contentType;
		std::map<std::string, std::string> _reqHeader;

		void	_handleDefaultErrors();
		bool	_isSupportedMethod(std::string meth);
		bool	_isSupportedUploadPath();
		std::string	_constructPath(const std::string& requestPath, const std::string &root, const std::string &index);
		int		_checkRequestedType();
		void	_isFile();
		void	_isFolder();
		void	_getAutoIndex();
		std::string	_findDirectoryName();
		std::string	_generateTempFileName();
		void	_createFile();
};

class HttpException : public std::exception {
	public:
		HttpException(const std::string& message, int code) : message_(message),_code(code)  {}

		virtual const char* what() const throw() {
			return message_.c_str();
		}
		virtual ~HttpException() throw() {}

		int getErrorCode() const { return _code; }
	private:
		std::string message_;
		int _code;
};

std::string	getMimeTypes(std::string flag, std::string extension);

#endif