#include "Config.hpp"
#include "ParseFile.hpp"

Config::Config() {}

Config::Config(const char *fileName) :
	_serverCount(0),
	_fileName(fileName)
{}

Config::Config(const Config &other) {
	*this = other;
}

Config&	Config::operator=(const Config &other) {
	if (this != &other) {
		this->_fileName = other._fileName;
	}
	return *this;
}

Config::~Config() {}

ConfigServer Config::parseServerConfig(std::vector<t_tokens>::iterator& it) {

	++it;
	ConfigServer	server;
    // Parse server configuration settings
	while (it != this->_tokens.end() && it->_type != "}") {
		std::cout << it->_type << "\n";
		if (it->_type == "location") {
			server.setLocation(this->_tokens ,it);
			it++;
			std::cout << "after location block:" <<it->_type << "\n";
			continue;
		}
		else if (it->_type == "listen")
		{
			server.setListen(it->_value);
		}
		else if (it->_type == "server_name")
			server.setServerName(it->_value);
		else if (it->_type == "client_max_body_size")
			server.setBodySize(it->_value);
		else if (it->_type == "autoindex")
			server.setAutoIndex(it->_value);
		else if (it->_type == "root")
			server.setRoot(it->_value);
		else if (it->_type == "error_page")
			server.setErrorPage(it->_value);
		else if (it->_type.empty()) {
			it++;
			continue;
		}
		else
			break;
		it++;
	}
	std::cout << "end of server\n";
	if (it->_type != "}")
		throw ParseServerException("Error: expected '}' in the end of server directive.");
	return (server);
}

void	Config::parse()
{
	try {
		this->_tokens = ParseFile::readFile(this->_fileName);
		std::vector<t_tokens>::iterator it = _tokens.begin();
		if (it->_type != "server")
			throw ParseServerException("Error: Unexpected token.");
		while (it != this->_tokens.end())
		{
			if (it->_type == "server")
				this->_servers.push_back(parseServerConfig(it));
			it++;
		}
		this->_serverCount = this->_servers.size();
		if (this->_serverCount == 0)
			throw ParseServerException("Error: Must have at least one server.");
		while (this->_serverCount > 1) {
			for (size_t i = 0; i < this->_serverCount - 1; i++) {
				for (size_t j = i + 1; j < this->_serverCount; j++) {
					if (this->_servers[i].getPort() == this->_servers[j].getPort()
						&& this->_servers[i].getServerName() == this->_servers[j].getServerName())
						throw ParseServerException("Error: Same Server.");
				}
			}
			break;
		}
		std::cout << "Host: " << _servers[0].getHost() << ", Port: " << _servers[0].getPort() 
		<< ", ServerName: " << _servers[0].getServerName()
		<< ", BodySize: " << _servers[0].getMaxBodySize() << std::endl;
		// for (std::vector<t_tokens>::iterator it =_tokens.begin(); it != this->_tokens.end(); ++it) {
		// 	std::cout << "type: " << it->_type << ", Value: " << it->_value << std::endl;
        // }
	} catch(const std::exception &e) {
		std::cout << e.what() << std::endl;
		exit(1);
	}
}
