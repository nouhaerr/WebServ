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
#include "Config.hpp"
#include "ConfigServer.hpp"

class Config;
class ConfigServer;
class ConfigLocation {
	public:
		ConfigLocation();
		ConfigLocation(const ConfigLocation &src);
		ConfigLocation& operator=(const ConfigLocation &src);
		~ConfigLocation();

		// Setters
		void	setLocationName(std::string& locationName);
		void	setRoot(std::string& root);
		void	setIndex(std::string& index);
		void	setMethods(std::string& methods);
		void	setBodySize(std::string& bodySize);
		void	setAutoIndex(std::string& autoindex);
		void	setUpload(std::string& upload);
		void	setErrorPage(std::string& errorPage);
		void	setRedirection(std::string& redirection);

		// Getters
		std::string& 				getLocationName();
		std::string& 				getRoot();
		std::vector<std::string>&	getIndex();
		std::vector<std::string>&	getMethods();
		size_t&						getMaxBodySize();
		bool&						getAutoIndex();
		std::string&				getUpload();
		std::map<int, std::string>&	getErrorPage();
		int&						getRedirectCode();
		std::string&				getRedirection();


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
		std::string					_locationName;
		std::string					_root;
		std::vector<std::string>	_index;
		std::vector<std::string>	_methods;
		size_t						_maxBodySize;
		bool						_autoindex;
		std::string					_upload;
		std::map<int, std::string>	_errorPage;
		std::string					_redirection;
		int							_redirectCode;
};

#endif
