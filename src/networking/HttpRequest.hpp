#ifndef REQUEST_HPP
#define REQUEST_HPP
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream> 

class HttpRequest
{
	public:
	enum REQUEST_STATE
	{
		HEADERS,
		BODY,
		REQUEST_READY	
	};

	enum BODY_STATE
	{
		CHUNKED,
		CONTENT_LENGTH,
		NONE,
	};

	private:
	std::string httpMethod;
	std::string httpVersion;
	std::string uri;
	std::map<std::string, std::string> headers;
	std::string bodyFileName;
	std::string queryString;
	REQUEST_STATE request_status;
	BODY_STATE body_status;
	std::string requestData;
	int _errorCode;
	size_t bodySize;

	public:


	void parse(std::string&);
	HttpRequest();
	HttpRequest(const HttpRequest&);
	HttpRequest& operator=(const HttpRequest&);
	~HttpRequest();

	/*setters*/
	void set_request(std::string&);
	void set_httpMethod(std::string&);
	void set_httpVersion(std::string&);
	void set_uri(std::string&);
	void set_headers(std::string&);
	bool set_body(std::string&);
	void set_queryString();
	void set_bodyStatus(BODY_STATE);
	void set_requestStatus(REQUEST_STATE);
	void setRequestData(std::string&);
	



	/*getters*/
	std::string& get_httpMethod();
	int getErrorCode() const {
    return this->_errorCode;
}
	std::string& get_httpVersion();
	std::string& get_uri();
	std::string& get_body();
	std::string& get_queryString();
	std::map<std::string, std::string>& get_headers();
	std::string& get_bodyFileName();
	size_t& get_bodysize();
	int get_bodyStatus();
	int get_requestStatus();
	void is_method(std::string&);
	void printHeaders();
	void unchunkBody(std::string&);
	std::string& getRequestData();

	void is_body();
};
#endif
