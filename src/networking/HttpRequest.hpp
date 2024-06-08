#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "../Macros.hpp"
#include "../parsing/ConfigServer.hpp"

class HttpRequest {
	private:
		std::string _request;
		std::string _method;
		std::string _uri;
		std::string	_httpVersion;
		std::map<std::string, std::string> _headerFields;
		std::string		_body;
		bool			isChunked;
		size_t			_bodySize;
		int				_errorCode;
		std::vector<ConfigServer>	_confServ;
		ConfigServer	_serv;
		// int				_port;
		bool parsingFinished;

		void		_parseMethod();
		void		_parseURI();
		// void		_matchServer();
		void		_getChunkedBody(size_t &bodypos);
		std::string	_generateTempFileName();
		void		_createFile(const std::string& name, const std::string& reqBody);

	public:
		HttpRequest();
		// HttpRequest(std::vector<ConfigServer> serverConfig);
		HttpRequest(ConfigServer serverConfig);
		HttpRequest(const HttpRequest&);
		HttpRequest& operator=(const HttpRequest&);
		~HttpRequest();

		void	parseHttpRequest(const std::string& req);
		void	printRequestDetails() const;
		void	parseBody(size_t &bodypos);
		bool	is_body(long& contentLength);
		void	getChunkedBody(size_t &bodypos);


		//Setters
		void	setMethod(const std::string& m);
		void	setUri(const std::string& u);
		void	setHttpVersion(const std::string& hv);
		void	setBody(const std::string& b);
		void	setHeaderField(const std::string& name, const std::string& value);

		//Getters
		std::string	getRequest() const;
		std::string	getMethod() const;
		std::string	getUri() const;
		std::string	getHttpVersion() const;
		std::string	getHeader(const std::string& headerName) const;
		const std::map<std::string, std::string>& getHeaderFields() const;
		std::string	getBody() const;
		bool		getIsChunked() const;
		int			getErrorCode() const;

		static int	hexToInt(const std::string& str);

		void setParsingFinished(bool finished);
    	bool isParsingFinished() const;
};

std::string trimHeader(const std::string& str);

#endif
