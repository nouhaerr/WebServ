#include "ConfigLocation.hpp"

ConfigLocation::ConfigLocation() :
_locationName(""),
_root(""),
_index(""),
_methods("")
{}

ConfigLocation::ConfigLocation(const ConfigLocation &src) {
	*this = src;
}

ConfigLocation& ConfigLocation::operator=(const ConfigLocation &src) {
	if (this != &src) {
		this->_locationName = src._locationName;
		this->_root = src._root;
		this->_index = src._index;
		this->_methods = src._methods;
	}
	return *this;
}

ConfigLocation::~ConfigLocation() {}

void	ConfigLocation::setLocationName(std::string& locationName) {
	if (locationName.empty())
		throw ConfigLocationException("Error: Empty location name.");
	this->_locationName = locationName;
}

std::string& ConfigLocation::getLocationName() {
	return this->_locationName;
}
