#include "CGI.hpp"

CGI::CGI(NetworkClient &client, std::string &filePath) : env(NULL) ,client(client), _filePath(filePath), status_code(200) {}


CGI::~CGI()
{
}

CGI::CGI(const CGI& cgi) : client(cgi.client), status_code(200)
{
	this->env = cgi.env;
}

CGI& CGI::operator=(const CGI& cgi)
{
	if (this != &cgi)
	{
		this->env = cgi.env;
		this->client = cgi.client;
		this->status_code = cgi.status_code;
	}
	return (*this);
}

void CGI::set_environmentVariables(std::string& file_name)
{
    std::vector<std::string> envs;
    std::map<std::string, std::string> &headers = this->client.getRequest().getHeaderFields();
    HttpRequest &request = this->client.getRequest();

    std::string method = request.getMethod();
    // std::cout << "Requestt method: " << method << std::endl;

    // std::cout << "Content-Type: " << headers["Content-Type"] << std::endl;
    // std::cout << "Content-Length: " << headers["Content-Length"] << std::endl;

    envs.push_back("CONTENT_TYPE=" + headers["Content-Type"]);
    envs.push_back("REDIRECT_STATUS=200");
    envs.push_back("CONTENT_LENGTH=" + headers["Content-Length"]);
    envs.push_back("HTTP_COOKIE=" + headers["Cookie"]);
    envs.push_back("HTTP_USER_AGENT=" + headers["User-Agent"]);
    envs.push_back("PATH_INFO=");
    envs.push_back("QUERY_STRING=" + request.get_queryString());
    envs.push_back("REMOTE_ADDR=");
    envs.push_back("REMOTE_HOST=");
    envs.push_back("REQUEST_METHOD=" + method);
    envs.push_back("SCRIPT_NAME=" + file_name);
    envs.push_back("SERVER_NAME=" + headers["Host"]);
    envs.push_back("SERVER_SOFTWARE=HTTP/1.1");
    envs.push_back("SCRIPT_FILENAME=" + this->_filePath);
    envs.push_back("REQUEST_URI=" + this->_filePath);

    this->env = new char*[envs.size() + 1];
    size_t i = 0;
    for (i = 0; i < envs.size(); i++)
    {
        this->env[i] = new char[envs[i].size() + 1];
        strcpy(this->env[i], envs[i].c_str());
    }
    this->env[i] = NULL;
}


char** CGI::get_CGIenvironmentVariables()
{
	return (this->env);
}

void CGI::RUN()
{
	pid_t pid;
	int fdIn = 0;
	int fdOut = 1;
	FILE* tmp = std::tmpfile();


	std::string path(this->_filePath);
	char* tmp_arg = new char[path.size() + 1];
	strcpy(tmp_arg, path.c_str());

	char *args[] = {tmp_arg, NULL};
	if (this->client.getRequest().getMethod() == "POST")
	{
		fdIn = open(this->client.getRequest().get_bodyFileName().c_str(), O_RDONLY);
		if (!fdIn)
		{
			this->status_code = 500;
			std::exit(EXIT_FAILURE);
		}
			
	}
		
	// std::cout << this->client.getRequest().get_bodyFileName().c_str() << std::endl;
	tmp = std::tmpfile();
	fdOut = fileno(tmp);
	pid = fork();
	
	if (pid == -1)
	{
        // std::cout << "ERROR" << std::endl;
		std::string errorContent = "Content-Type: text/html\r\n\r\n<html><body style='text-align:center;'><h1>500 IIIIIInternal Server Error</h1></body></html>";
        write(STDOUT_FILENO, errorContent.c_str(), errorContent.size());
		this->status_code = 500;
		std::exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
        // 	std::cout << args[0] << "ARGR 0"<<  std::endl;
		// if(args[1])
        // {
        //     std::cout << args[1] << "ARGR 1"<<  std::endl;
        // }
		if (dup2(fdIn, 0) == -1 || dup2(fdOut, 1) == -1)
		{
			std::string errorContent = "Content-Type: text/html\r\n\r\n<html><body style='text-align:center;'><h1>500 Internal Server Errrrrrror</h1></body></html>";
       		write(STDOUT_FILENO, errorContent.c_str(), errorContent.size());
			this->status_code = 500;
			std::exit(EXIT_FAILURE);
		}
			
		if (this->client.getRequest().getMethod() == "POST")
			close(fdIn);
		close(fdOut);
		fclose(tmp);
	
		execve(args[0], args, this->env);
		std::string errorContent = "Content-Type: text/html\r\n\r\n<html><body style='text-align:center;'><h1>500 Internal Server Eeeeeerror</h1></body></html>";
        write(STDOUT_FILENO, errorContent.c_str(), errorContent.size());
		this->status_code = 500;
		std::exit(EXIT_FAILURE);
	}
	else if (pid > 0)
	{
		int change_status = 200;
		time_t start_time = time(NULL);
		while (time(NULL) - start_time < 5)
		{
			if (waitpid(pid, NULL, WNOHANG) == pid)
			{
				change_status = -2;
				break;
			}
		}
		if (change_status != -2 && waitpid(pid, NULL, WNOHANG) != pid)
		{
			this->status_code = 504;
			kill(pid, SIGKILL);
		}

		delete[] tmp_arg;

		/*free array of cgi environment variables*/
		int i = 0;
		while(this->env[i])
		{
			delete[] this->env[i];
			i++;
		}
		delete[] this->env;

		/*read response*/
		char buffer[1024];
		ssize_t bytes;
		std::string response;

		lseek(fdOut, 0, SEEK_SET);
		while ((bytes = read(fdOut, buffer, 1023)) > 0)	
		{
			buffer[bytes] = '\0';
			response += buffer;
		}
		char *tmp_str = new char[response.size() + 1];
		strcpy(tmp_str, response.c_str());
		this->client.set_Response(tmp_str, response.size());
		// std::cout << tmp_str << response.size() << std::endl;
		delete[] tmp_str;
		if (this->client.getRequest().getMethod() == "POST")
			close(fdIn);
		close(fdOut);
		fclose(tmp);
	}
}
