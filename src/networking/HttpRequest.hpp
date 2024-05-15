#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "../Macros.hpp"
#include <cstdlib>
#include <ctime>
#include <cstring>

class HttpRequest {
	public:
		HttpRequest();
		HttpRequest(const HttpRequest&);
		HttpRequest& operator=(const HttpRequest&);
		~HttpRequest();

		void		parseHttpRequest(const std::string& request);
		void		parseBody(size_t &bodypos);
		bool		is_body(long long& contentLength);
		static int	hexToInt(const std::string& str);
		std::string	getHeader(const std::string& headerName) const;

		void printHeaders() const;
		void printRequestDetails() const;

		std::string getMethod() const;
		std::string getUri() const;
		std::string getHttpVersion() const;
		std::string getBody() const;

	private:
		std::string _request;
		std::string _method;
		std::string _uri;
		std::string	_httpVersion;
		std::map<std::string, std::string> _headerFields;
		bool			isChunked;
		std::string		_body;
		int				_bodySize;
		int				_errorCode;
		// ConfigServer	_confServ;
		// int				_port;

		void		_parseMethod();
		void		_parseURI();
		void		_getChunkedBody(size_t &bodypos);
		bool		_isSupportedMethod();
		std::string	_generateTempFileName();
		std::string _findUploadPath();
		void		_createFile(const std::string& name, const std::string& reqBody);
};

#endif
