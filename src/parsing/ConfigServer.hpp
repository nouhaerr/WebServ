#ifndef CONFIGSERVER_HPP
# define CONFIGSERVER_HPP

#include "../Macros.hpp"
#include <stdexcept>
#include <cstdlib>

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
		const std::string			&getHost() const;
		const size_t				&getPort() const;
		const std::string			&getServerName() const;
		const size_t				&getMaxBodySize() const;
		std::vector<ConfigLocation>	&getLocation();
		bool						&getAutoIndex();
		std::string					&getRoot();
		std::map<int, std::string>	&getErrorPage();

		// Setters
		void	setListen(std::string& listen);
		void	setServerName(std::string& serverName);
		void	setBodySize(std::string& bodySize);
		void	setLocation(std::vector<t_tokens> &tok, std::vector<t_tokens>::iterator& it);
		void	setAutoIndex(std::string& autoindex);
		void	setRoot(std::string& root);
		void	setErrorPage(std::string& errorPage);

		int getSocket() const 
		{
        	return socketFD;
    	}

		void setSocket(int sock) 
		{
			socketFD = sock;
		}

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
        std::string					_host;
        size_t						_port;
        std::string					_serverName;
        size_t						_maxBodySize;
		std::vector<ConfigLocation> _location;
		bool						_autoindex;
		std::string					_root;
		std::vector<std::string>	_index;
		std::map<int, std::string>	_errorPage;
		int _socket;
		int socketFD;
};

bool						isValidIPAddress(const std::string& ip);
size_t						parseMaxBodySize(char &unit, std::string& bodySize, size_t sizeEnd);
std::vector<std::string>	splitVal(std::string& str);
size_t						isNum(std::string str);
bool						isUrl(const std::string& str);
std::vector<std::string>	splitArgs(std::string val);

#endif