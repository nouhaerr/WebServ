#ifndef HTTPRESPONE_HPP
# define HTTPRESPONSE_HPP

#include "../Macros.hpp"
#include "../parsing/Config.hpp"
#include "../networking/HttpRequest.hpp"

class HttpRequest;
class HttpResponse {
	public:
		HttpResponse();
		HttpResponse(const ConfigServer &clientServer);
		~HttpResponse();

		void	generateResponse(HttpRequest &req);

	private:
		int				_clSocket;
		ConfigServer	_serv;
		std::string		_body;
		int				_errCode;
		std::string		_statusCode;
		bool			_isCgi;
		std::string		_root;
		std::string		_index;
		std::string		_error_page;
		int				_autoindex;
		std::string		_redirection;
		std::vector<ConfigLocation>	_loc;
		int			_fd;
		std::string _path;
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

#endif