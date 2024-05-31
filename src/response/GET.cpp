#include "HttpResponse.hpp"

void	HttpResponse::handleGetMethod() {
	if (_isSupportedMethod()) {

	}
}

bool	HttpResponse::_isSupportedMethod() {
	std::vector<std::string> _isAllowedMeth = this->_confServ.getMethods();
	size_t	len = this->_confServ.getLocation().size();

	std::vector<std::string>::iterator it = this->_confServ.getMethods().begin();
	for(; it != this->_confServ.getMethods().end(); it++)
		std::cout << *it << " ";
	printf("\n");
	for(size_t i = 0; i < len; i++) {
		if (this->_uri.find(this->_confServ.getLocation()[i].name) != std::string::npos
			&& this->_confServ.getLocation()[i].getMethods().empty()) {//find the location Name in the uri
			_isAllowedMeth = this->_confServ.getLocation()[i].getMethods();
			break ;
		} //update the method vector
	}
	// Check if the request method is found in the vect of allowed methods
    return (std::find(_isAllowedMeth.begin(), _isAllowedMeth.end(), this->_method) != _isAllowedMeth.end());
}
