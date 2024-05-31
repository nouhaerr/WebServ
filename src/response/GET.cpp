#include "HttpResponse.hpp"

void	HttpResponse::handleGetMethod() {
	if (!_isSupportedMethod("GET")) {
		buildResponse(405);
		return ;
	}
}

bool	HttpResponse::_isSupportedMethod(std::string meth) {
	size_t	len = _methods.size();

	for(size_t i = 0; i < len; i++) {
		if (_methods[i] == meth)
			return true ;
	}
	// Check if the request method is found in the vect of allowed methods
    return false;
}
