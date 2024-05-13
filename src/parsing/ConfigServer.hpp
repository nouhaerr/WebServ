#ifndef CONFIGSERVER_HPP
# define CONFIGSERVER_HPP

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
#include "ConfigLocation.hpp"

class Config;
class ConfigLocation;
class	ConfigServer {
	public:
		ConfigServer();
		~ConfigServer();
		ConfigServer(ConfigServer const &src);
		ConfigServer&	operator=(const ConfigServer &src);

		ConfigLocation	parseLocation(std::vector<t_tokens> &tok, std::vector<t_tokens>::iterator& it);
		
		//Getters
		const std::string &getHost() const;
		const size_t &getPort() const;
		const std::string &getServerName() const;
		const size_t &getMaxBodySize() const;

		// Setters
		void	setListen(std::string& listen);
		void	setServerName(std::string& serverName);
		void	setBodySize(std::string& bodySize);
		void	setLocation(std::vector<t_tokens> &tok, std::vector<t_tokens>::iterator& it);

		class	ConfigServerException: public std::exception{
			std::string message;
			public:
				ConfigServerException(std::string msg) : message(msg){};
			const char *what() const throw()
			{
				return (this->message.c_str());
			}
			~ConfigServerException() throw(){};
		};

    private:
        std::string		_host;
        size_t			_port;
        std::string		_serverName;
        size_t			_maxBodySize;
		std::vector<ConfigLocation> _location;
		std::string		_root;
		std::vector<std::pair<std::vector<std::string>, std::string> > _errorPage;
		std::string		_autoindex;
		std::vector<std::string> _index;
		int socket;
};

bool						isValidIPAddress(const std::string& ip);
size_t						parseMaxBodySize(char &unit, std::string& bodySize, size_t sizeEnd);
std::vector<std::string>	splitVal(std::string& str);
size_t						isNum(std::string str);
bool						isUrl(const std::string& str);
std::vector<std::string>	splitArgs(std::string val);

#endif