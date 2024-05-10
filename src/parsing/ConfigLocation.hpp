#ifndef CONFIGLOCATION_HPP
# define CONFIGLOCATION_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <map>
#include <exception>
#include <cstdlib>
#include <sstream>

class ConfigLocation {
	public:
		ConfigLocation();
		ConfigLocation(const ConfigLocation &src);
		ConfigLocation& operator=(const ConfigLocation &src);
		~ConfigLocation();

		// Setters
		void	setLocationName(std::string& locationName);

		// Getters
		std::string& getLocationName();

		class	ConfigLocationException: public std::exception{
			std::string message;
			public:
				ConfigLocationException(std::string msg) : message(msg){};
			const char *what() const throw()
			{
				return (this->message.c_str());
			}
			~ConfigLocationException() throw(){};
		};

	private:
		std::string	_locationName;
		std::string	_root;
		std::string	_index;
		std::string	_methods;
};

#endif
