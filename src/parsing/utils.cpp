#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <map>
#include <exception>
#include <sstream>

#include "ConfigServer.hpp"

bool isValidIPAddress(const std::string& ip) {
	int numSegments = 0;
	int numDigits = 0;
	int numDots = 0;
	int segmentValue = 0;

	for (size_t i = 0; i < ip.length(); ++i) {
		if (isdigit(ip[i])) {
			++numDigits;
			segmentValue = segmentValue * 10 + (ip[i] - '0');

			// Check if the segment value exceeds 255
			if (segmentValue > 255)
				return false;
			}
		else if (ip[i] == '.') {
			++numDots;
			if (numDigits == 0 || numDigits > 3 || numDots > 3)
				return false; // Each segment should contain at least 1 digit and at most 3 digits,
				// and there should be exactly 3 dots
			numSegments++;
			numDigits = 0;
			segmentValue = 0;
		} else
			return false; // Invalid character
	}
	// Ensure the last segment is valid
	if (numSegments != 3 || numDigits == 0 || numDigits > 3 || numDots != 3)
		return false;

	return true;
}

size_t	parseMaxBodySize(char &unit, std::string& bodySize, size_t sizeEnd) {
	size_t size;
	if (std::isdigit(unit)) {
		size = std::strtoll(bodySize.c_str(), NULL, 10);
		unit = ' ';
	} else
		size = std::strtoll(bodySize.substr(0, sizeEnd).c_str(), NULL, 10);

	switch (unit) {
		case 'k':
			size *= 1024LL; // 1 kilobyte = 1024 bytes
			break;
		case 'm':
			size *= 1024LL * 1024LL; // 1 megabyte = 1024 kilobytes
			break;
		case 'g':
			size *= 1024LL * 1024LL * 1024LL; // 1 gigabyte = 1024 megabytes
			break;
		case 't':
			size *= 1024LL * 1024LL * 1024LL * 1024LL; // 1 terabyte = 1024 gigabytes
			break;
		default:
		// No unit letter, so the size is in bytes by default
			break;
	}
	return size;
}

ConfigLocation	ConfigServer::parseLocation(std::vector<t_tokens> &tok, std::vector<t_tokens>::iterator& it) {
	ConfigLocation	loc;
	bool	root = false, index= false, errPage = false;
	bool	meth = false, ret = false, upload = false;
	bool	autind = false, bdSize = false;
	int rt = 0;
	loc.setLocationName(it->_value);
	it++;
	while (it != tok.end() && it->_type != "}") {
		if (!root) {
			if (it->_type == "root" )
				loc.setRoot(it->_value);
		}
		// if (it->_type == "}")
		// 	continue;
		
		// else if (it->_type.empty())
		// 	continue;
		// else
		// 	break;
		// if (it->_type == "location")
		// 	server.set_location(it);

        // Move to the next token
        it++;
    }
	it--;
	// std::cout << it->_type << "\n";
	if (it->_type != "}")
		throw ConfigServerException("Error: expected '}' in the end of server/location directive.");
	return (loc);
}
