#include "HttpResponse.hpp"

void	HttpResponse::handleGetMethod() {
	if (!_isSupportedMethod("GET")) {
		buildResponse(405);
		return ;
	}
	// if (isFile(_uri)) {

	// }
}

bool	HttpResponse::_isSupportedMethod(std::string meth) {
	size_t	len = _methods.size();

	for(size_t i = 0; i < len; i++) {
		if (_methods[i] == meth)
			return true ;
	}
    return false;
}
