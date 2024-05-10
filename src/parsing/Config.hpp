#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <limits.h>
#include <map>
#include <stdexcept>
#include "ConfigServer.hpp"

typedef struct s_tokens {
	std::string	_type;
	std::string _value;
} t_tokens;

class ConfigServer;
// class Location;
class ParseFile;

class Config {
    public:
		Config();
		Config(const char *fileName);
		Config(const Config &other);
		Config& operator=(const Config &other);
		~Config();

		void	parse();
		void incrementServerCount() { serverCount++; }
		int getServerCount() const { return serverCount; }
		ConfigServer	parseServerConfig(std::vector<t_tokens>::iterator& it);
		// void	checkServer();
		// bool	look(const std::string &type);
		// t_tokens match(const std::string &type);
		// ConfigServer	parseServer();
		// ConfigServer	parseServerBody();
		// std::string		parseStringRules(std::string key);
		// size_t			parseNumRules(std::string key);
		class	ParseServerException: public std::exception{
			std::string message;
			public:
				ParseServerException(std::string msg) : message(msg){};
			const char *what() const throw()
			{
				return (this->message.c_str());
			}
			~ParseServerException() throw(){};
		};
	private:
		int serverCount;
		int index;
		const char* _fileName;
		typedef std::vector<t_tokens>	tokenVector;
		tokenVector	_tokens;
		
        std::vector<ConfigServer>  _servers;
       
};


#endif