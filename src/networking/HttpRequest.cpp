
#include "./HttpRequest.hpp"


void trim_front(std::string &str)
{
	std::size_t i = 0;
	while (i < str.size() && (str[i] == ' ' || str[i] == '\t'))
		i++;
	str = str.substr(i);
}

void trim_back(std::string &str)
{
	std::size_t pos;

	pos = str.size();
	while (pos > 0 && (str[pos - 1] == ' ' || str[pos - 1] == '\t'))
		pos--;
	if (pos == 0)
		return;
	// sus error
	str = str.substr(0, pos);
}

void str_trim(std::string &str)
{
	trim_front(str);
	trim_back(str);
}

std::vector<std::string> split(std::string &str, char delim)
{
	std::vector<std::string> split_str;
	std::string tmp;
	size_t pos;

	pos = str.find(delim);
	while (pos != std::string::npos)
	{
		tmp = str.substr(0, pos);
		str = str.substr(pos + 1);
		split_str.push_back(tmp);
		pos = str.find(delim);
	}
	split_str.push_back(str);
	return split_str;
}

bool isHexadecimal(std::string &str)
{

	if (str.empty())
		return false;
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!std::isxdigit(str[i]))
			return false;
	}
	return true;
}

bool isNumber(std::string &str)
{

	if (str.empty())
		return false;
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!std::isdigit(str[i]))
			return false;
	}
	return true;
}

std::string Generate_Random_File_Name()
{
	const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	const int charsetSize = sizeof(charset) - 1;
	const int fileNameLength = 7;
	std::string randomFileName;

	std::srand(static_cast<unsigned int>(std::time(0)));

	for (int i = 0; i < fileNameLength; ++i)
	{
		int randomIndex = std::rand() % charsetSize;
		randomFileName += charset[randomIndex];
	}
	return randomFileName;
}

HttpRequest::HttpRequest() : httpMethod(""),
	httpVersion(""),
	uri(""),
	bodyFileName(""),
	queryString(""),
	request_status(HEADERS),
	body_status(NONE),
	_errorCode(0),
	bodySize(0)

{
}

HttpRequest::HttpRequest(const HttpRequest &httprequest)
{
	this->httpMethod = httprequest.httpMethod;
	this->httpVersion = httprequest.httpVersion;
	this->uri = httprequest.uri;
	this->headers = httprequest.headers;
	this->bodyFileName = httprequest.bodyFileName;
	this->queryString = httprequest.queryString;
}

HttpRequest &HttpRequest::operator=(const HttpRequest &httprequest)
{
	if (this != &httprequest)
	{
		this->httpMethod = httprequest.httpMethod;
		this->httpVersion = httprequest.httpVersion;
		this->uri = httprequest.uri;
		this->headers = httprequest.headers;
		this->bodyFileName = httprequest.bodyFileName;
		this->queryString = httprequest.queryString;
		this->bodySize = httprequest.bodySize;
	}
	return (*this);
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::is_method(std::string &method)
{
	std::vector<std::string> methods;
	methods.push_back("GET");
	methods.push_back("POST");
	methods.push_back("DELETE");
	methods.push_back("PUT");
	for (size_t i = 0; i < methods.size(); i++)
	{
		if (method == methods[i])
			return;
	}
}

void HttpRequest::is_body()
{
	std::map<std::string, std::string>::iterator it_length = this->headers.find("Content-Length");
	std::map<std::string, std::string>::iterator it_chunk = this->headers.find("Transfer-Encoding");
	if (it_length != this->headers.end())
		this->body_status = HttpRequest::CONTENT_LENGTH;
	if (it_chunk != this->headers.end() && it_chunk->second == "chunked")
		this->body_status = HttpRequest::CHUNKED;
}

void HttpRequest::set_httpMethod(std::string &method)
{
	is_method(method);
	this->httpMethod = method;
}

void HttpRequest::set_uri(std::string &uri)
{
	this->uri = uri;
}

void HttpRequest::set_httpVersion(std::string &version)
{
	this->httpVersion = version;
}

void HttpRequest::set_request(std::string &request_line)
{
	std::vector<std::string> request = split(request_line, ' ');
	set_httpMethod(request[0]);
	set_uri(request[1]);
	set_httpVersion(request[2]);
}

void HttpRequest::printHeaders()
{
	std::map<std::string, std::string>::iterator it;

	for (it = this->headers.begin(); it != this->headers.end(); ++it)
		std::cout << it->first << ":" << it->second << std::endl;
}

void HttpRequest::set_headers(std::string &headers)
{
	std::string CRLF("\r\n");
	size_t crlf_pos, delim_pos;
	;
	std::string key, value;
	std::string header_line;

	while (headers.find(CRLF) != std::string::npos)
	{
		crlf_pos = headers.find(CRLF);
		header_line = headers.substr(0, crlf_pos);
		headers = headers.substr(crlf_pos + 2);
		if (header_line.empty())
			break;
		delim_pos = header_line.find(":");
		if (delim_pos != std::string::npos)
		{
			key = header_line.substr(0, delim_pos);
			value = header_line.substr(delim_pos + 1);
			trim_front(value);
			this->headers.insert(std::pair<std::string, std::string>(key, value));
		}
	}
}

void HttpRequest::unchunkBody(std::string &body)
{
	std::string chunk;
	std::string CRLF("\r\n");
	size_t crlf_pos;
	std::string line;
	std::fstream bodyDataFile;

	bodyDataFile.open(this->bodyFileName.c_str(), std::fstream::app | std::fstream::out | std::fstream::in);
	while (1)
	{
		crlf_pos = body.find(CRLF);
		if (crlf_pos == 0)
			break;
		chunk = body.substr(0, crlf_pos);
		body = body.substr(crlf_pos + 2);
		if (chunk == "0")
		{
			this->request_status = HttpRequest::REQUEST_READY;
			break;
		}

		else if (isHexadecimal(chunk))
			continue;
		this->bodySize += chunk.size();
		bodyDataFile << chunk;
	}
	bodyDataFile.close();
}

bool HttpRequest::set_body(std::string &body)
{
	if (this->bodyFileName.empty())
		this->bodyFileName = "/tmp/" + Generate_Random_File_Name();
	if (this->body_status == HttpRequest::CHUNKED)
	{
		unchunkBody(body);
		if (this->request_status == HttpRequest::REQUEST_READY)
			return true;
	}
	else
	{
		std::map<std::string, std::string>::iterator it_chunk = this->headers.find("Content-Length");
		size_t content_length = atoi(it_chunk->second.c_str());
		size_t bytes_left = content_length - (size_t)this->bodySize;
		std::fstream bodyDataFile;
		bodyDataFile.open(this->bodyFileName.c_str(), std::fstream::app | std::fstream::out | std::fstream::in);
		if (bytes_left > 0 && body.size() <= bytes_left)
		{
			bodyDataFile << body;
			this->bodySize += body.size();
			body = "";
		}
		bodyDataFile.close();
		if (this->bodySize == content_length)
			return true;
	}
	return false;
}

void HttpRequest::set_queryString()
{
	size_t pos;

	pos = this->uri.find("?");
	if (pos != std::string::npos)
		this->queryString = this->uri.substr(pos + 1);
	else
		this->queryString = "";
}

void HttpRequest::setRequestData(std::string &data)
{
	this->requestData += data;
}

void HttpRequest::parse(std::string &read_request)
{
	std::string CRLF("\r\n");
	std::string request_line;
	std::string headers_lines;
	std::string rest;
	size_t pos;

	pos = read_request.find(CRLF);
	if (pos != std::string::npos)
	{
		request_line = read_request.substr(0, pos);
		headers_lines = read_request.substr(pos + 2);
		set_request(request_line);
		set_queryString();
		set_headers(headers_lines);
		read_request = headers_lines;
		if (this->httpMethod == "POST")
			this->request_status = HttpRequest::BODY;
		else
			this->request_status = HttpRequest::REQUEST_READY;
	}
}

void HttpRequest::set_bodyStatus(BODY_STATE status)
{
	this->body_status = status;
}

void HttpRequest::set_requestStatus(REQUEST_STATE status)
{
	this->request_status = status;
}

std::string &HttpRequest::get_httpMethod()
{
	return (this->httpMethod);
}

std::string &HttpRequest::get_httpVersion()
{
	return (this->httpVersion);
}

std::string &HttpRequest::get_uri()
{
	return (this->uri);
}

std::map<std::string, std::string> &HttpRequest::get_headers()
{
	return (this->headers);
}

size_t &HttpRequest::get_bodysize()
{
	return (this->bodySize);
}

std::string &HttpRequest::get_bodyFileName()
{
	return (this->bodyFileName);
}

std::string &HttpRequest::get_queryString()
{
	return (this->queryString);
}

std::string &HttpRequest::getRequestData()
{
	return (this->requestData);
}

int HttpRequest::get_bodyStatus()
{
	return (this->body_status);
}

int HttpRequest::get_requestStatus()
{
	return (this->request_status);
}

