#include "HttpResponse.hpp"

HttpResponse::HttpResponse() {}

HttpResponse::HttpResponse(const ConfigServer &clientServer) :
	_serv(clientServer),
	_body(""),
	_errCode(0),
	_statusCode(""),
	_isCgi(false),
	_root(""),
	_index(""),
	_fd(0),
	_path("") {}

HttpResponse::~HttpResponse(){}

void	HttpResponse::generateResponse(HttpRequest &req) {
	// if (_isCgi == false)
	// 	_respFile(req);
	// int re = get_requested_resource(req);
	try {
		// if (_fd == 0)
			// initHeader(req);
	} catch(const HttpException &e) {
		_errCode = e.getErrorCode();
		std::cerr << e.what() << std::endl;
	}
	return ;
}