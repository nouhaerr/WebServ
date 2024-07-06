#ifndef CGI_HPP
#define CGI_HPP

#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <sys/wait.h>
#include "../networking/NetworkClient.hpp"
#include "../response/HttpResponse.hpp"
#include <signal.h>
#include <unistd.h>

class NetworkClient;
class HttpRequest;
class HttpResponse;

class CGI
{
	char** envData;
	NetworkClient &client;
	std::string _filePath;

	public:
	int responseStatus;
	CGI(NetworkClient&, std::string &filePath);

	CGI(const CGI&);
	CGI& operator=(const CGI&);
	~CGI();

	/*setters*/
	void configureEnvironment(const std::string& script_name);
	void initializeEnvData(const std::vector<std::string>& envs);

	/*getters*/
	char** get_CGIenvironmentVariables();

	void executeScript();
	bool initializePipes(int pipeIn[2], int pipeOut[2]);
	void executeChildProcess(const char* scriptPath, char* args[], int pipeIn[2], int pipeOut[2]);
	void handleParentProcess(pid_t processId, int pipeIn[2], int pipeOut[2], const std::string& scriptArgument);

	std::string	getContentLength(std::string path);
};

#endif

