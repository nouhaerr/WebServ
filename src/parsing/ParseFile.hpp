#ifndef ParseFILE_HPP
# define ParseFILE_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <map>
#include <stdexcept>

#include "Config.hpp"

typedef std::vector<t_tokens>	tokenVector;

class ParseFile{
	public:
		virtual	~ParseFile();
		static std::vector<t_tokens>  	readFile(const char *fileName);
		static t_tokens		setToken(std::string& line);

		class	ParseFileException: public std::exception{
			std::string message;
			public:
				ParseFileException(std::string msg) : message(msg){};
			const char *what() const throw()
			{
				return (this->message.c_str());
			}
			~ParseFileException() throw(){};
		};

	private:
		ParseFile();
		ParseFile(ParseFile const &src);
		ParseFile	&operator=(ParseFile const &src);
};

void	trim(std::string& line);

#endif