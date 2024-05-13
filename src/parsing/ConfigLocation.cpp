#include "ConfigLocation.hpp"

ConfigLocation::ConfigLocation() :
_locationName(""),
_root(""),
_maxBodySize(0),
_autoindex("")
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
		this->_maxBodySize = src._maxBodySize;
		this->_autoindex = src._autoindex;
	}
	return *this;
}

ConfigLocation::~ConfigLocation() {}

void	ConfigLocation::setLocationName(std::string& locationName) {
	if (locationName.empty() || locationName.find_first_of(" \t") != std::string::npos)
		throw ConfigLocationException("Error: Empty location name.");
	this->_locationName = locationName;
}

std::string& ConfigLocation::getLocationName() {
	return this->_locationName;
}

void	ConfigLocation::setRoot(std::string& root) {
	if (root.empty() || root.find_first_of(" \t") != std::string::npos)
		throw ConfigLocationException("Error: Wrong root!");
	this->_root = root;
}

std::string& ConfigLocation::getRoot() {
	return this->_root;
}

void	ConfigLocation::setIndex(std::string& index) {
	if (index.empty())
		throw ConfigLocationException("Error: Wrong root!");
	std::string tmp;
	size_t pos = 0;

	while (pos < index.size()) {
		// Extract the word
		while (pos < index.size() && index[pos] != ' ' && index[pos] != '\t') {
			tmp += index[pos];
			pos++;
		}
		// Push the word into the result vector
		if (!tmp.empty()) {
			this->_index.push_back(tmp);
			tmp.clear();
		}
		pos++;
	}
}

std::vector<std::string>&	ConfigLocation::getIndex() {
	return this->_index;
}

void	ConfigLocation::setMethods(std::string& methods) {
	if (methods.empty())
		throw ConfigLocationException("Error: Wrong root!");
	std::string tmp;
	size_t pos = 0;

	while (pos < methods.size()) {
		// Extract the word
		while (pos < methods.size() && methods[pos] != ' ' && methods[pos] != '\t') {
			tmp += methods[pos];
			pos++;
		}
		// Push the word into the result vector
		if (!tmp.empty()) {
			this->_methods.push_back(tmp);
			tmp.clear();
		}
		pos++;
	}
}

std::vector<std::string>& ConfigLocation::getMethods() {
	return this->_methods;
}

void	ConfigLocation::setBodySize(std::string& bodySize) {
	if (bodySize.empty())
		throw ConfigLocationException("Empty max_body_size");
	std::size_t sizeEnd = bodySize.size() - 1;
    char unit = std::tolower(bodySize[sizeEnd]);

    for (std::string::size_type i = 0; i < sizeEnd; ++i) {
        if (!std::isdigit(bodySize[i]))
            throw  ConfigLocationException("Invalid max_body_size.");
    }
    if (unit != 'm' && unit != 'g' && unit != 't' && unit != 'k' && !std::isdigit(unit))
        throw ConfigLocationException("Invalid max_body_size.");
	this->_maxBodySize = parseMaxBodySize(unit, bodySize, sizeEnd);
}

size_t&	ConfigLocation::getMaxBodySize() {
	return this->_maxBodySize;
}

void	ConfigLocation::setAutoIndex(std::string& autoindex) {	
	if (autoindex != "ON" && autoindex != "OFF")
		throw ConfigLocationException("Error: Wrong Autoindex!");
	this->_autoindex = autoindex;
}

std::string&	ConfigLocation::getAutoIndex() {
	return this->_autoindex;
}

