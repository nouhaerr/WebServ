#include "CGI.hpp"

CGI::CGI(NetworkClient &client) : client(client), responseStatus(200) {
}

CGI::~CGI() {
}

CGI::CGI(const CGI& otherCGI) : client(otherCGI.client), responseStatus(200) {
    this->envData = otherCGI.envData;
}

CGI& CGI::operator=(const CGI& otherCGI) {
    if (this != &otherCGI) {
        this->envData = otherCGI.envData;
        this->client = otherCGI.client;
        this->responseStatus = otherCGI.responseStatus;
    }
    return *this;
}


void CGI::initializeEnvData(const std::vector<std::string>& envs) 
{
    this->envData = new char*[envs.size() + 1];
    size_t i = 0;
    for (; i < envs.size(); i++) 
	{
        this->envData[i] = new char[envs[i].size() + 1];
        std::strcpy(this->envData[i], envs[i].c_str());
    }
    this->envData[i] = NULL;
}

void CGI::configureEnvironment(std::string& script_name)
{
std::vector<std::string> envs;
    std::map<std::string, std::string>& headers = this->client.getRequest().getHeaderFields();
    HttpRequest& request = this->client.getRequest();

    // Ajout des variables d'environnement
    envs.push_back("CONTENT_TYPE=" + (headers.count("Content-Type") ? headers["Content-Type"] : "undefined"));
    envs.push_back("REDIRECT_STATUS=200");
    envs.push_back("CONTENT_LENGTH=" + (headers.count("Content-Length") ? headers["Content-Length"] : "0"));
    envs.push_back("HTTP_COOKIE=" + (headers.count("Cookie") ? headers["Cookie"] : "none"));
    envs.push_back("HTTP_USER_AGENT=" + (headers.count("User-Agent") ? headers["User-Agent"] : "unknown"));
    envs.push_back("PATH_INFO="); 
    envs.push_back("QUERY_STRING=" + request.get_queryString());
    envs.push_back("REMOTE_ADDR=");
    envs.push_back("REMOTE_HOST=");
    envs.push_back("REQUEST_METHOD=" + request.getMethod());
    envs.push_back("SCRIPT_NAME=" + script_name); // Utilisation du nouveau paramètre
    envs.push_back("SERVER_NAME=" + (headers.count("Host") ? headers["Host"] : "localhost"));
    envs.push_back("SERVER_SOFTWARE=HTTP/1.1");
    envs.push_back("SCRIPT_FILENAME=" + request.getUri());
    envs.push_back("REQUEST_URI=" + request.getUri());

    initializeEnvData(envs);
}

char** CGI::get_CGIenvironmentVariables()
{
	return (this->envData);
}

bool CGI::initializePipes(int pipeIn[2], int pipeOut[2]) {
    if (pipe(pipeOut) == -1) {
        this->responseStatus = 500;
        return false;
    }
    if (this->client.getRequest().getMethod() == "POST") {
        if (pipe(pipeIn) == -1) {
            this->responseStatus = 500;
            close(pipeOut[0]);
            close(pipeOut[1]);
            return false;
        }
    }
    return true;
}

void CGI::executeChildProcess(const char* scriptPath, char* args[], int pipeIn[2], int pipeOut[2]) {
    if (this->client.getRequest().getMethod() == "POST") {
        close(pipeIn[1]);
        dup2(pipeIn[0], STDIN_FILENO);
        close(pipeIn[0]);
    }
    
    close(pipeOut[0]);
    dup2(pipeOut[1], STDOUT_FILENO);
    close(pipeOut[1]);

    execve(scriptPath, args, this->envData);
    std::string errorMessage = "Content-Type: text/html\r\n\r\n<html><body style='text-align:center;'><h1>500 Internal Server Error</h1></body></html>";
    write(STDOUT_FILENO, errorMessage.c_str(), errorMessage.size());
    std::exit(EXIT_FAILURE);
}

void CGI::handleParentProcess(pid_t processId, int pipeIn[2], int pipeOut[2], const std::string& scriptArgument) {
    if (this->client.getRequest().getMethod() == "POST") {
        close(pipeIn[0]);
        int inputFileDescriptor = open(this->client.getRequest().get_bodyFileName().c_str(), O_RDONLY);
        if (inputFileDescriptor == -1) {
            this->responseStatus = 500;
            delete[] scriptArgument.c_str();
            std::exit(EXIT_FAILURE);
        }
        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(inputFileDescriptor, buffer, sizeof(buffer))) > 0) {
            write(pipeIn[1], buffer, bytesRead);
        }
        close(inputFileDescriptor);
        close(pipeIn[1]);
    }

    close(pipeOut[1]);

    int processStatus = 200;
    time_t startTime = time(NULL);
    while (time(NULL) - startTime < 5) {
        if (waitpid(processId, NULL, WNOHANG) == processId) {
            processStatus = -2;
            break;
        }
    }
    if (processStatus != -2 && waitpid(processId, NULL, WNOHANG) != processId) {
        this->responseStatus = 504;
        kill(processId, SIGKILL);
    }

    for (int i = 0; this->envData[i]; i++) {
        delete[] this->envData[i];
    }
    delete[] this->envData;

    char buffer[1024];
    ssize_t bytesRead;
    std::string response;

    while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }

    char *responseStr = new char[response.size() + 1];
    std::strcpy(responseStr, response.c_str());
    this->client.setResponse(responseStr);
    delete[] responseStr;

    close(pipeOut[0]);
}

void CGI::executeScript() {
    pid_t processId;
    int pipeIn[2], pipeOut[2];

    std::string scriptPath = "./src" + this->client.getRequest().getUri();
    char* scriptArgument = new char[scriptPath.size() + 1];
    std::strcpy(scriptArgument, scriptPath.c_str());

    char *arguments[] = {scriptArgument, NULL};

    if (!initializePipes(pipeIn, pipeOut)) {
        delete[] scriptArgument;
        std::exit(EXIT_FAILURE);
    }

    processId = fork();
    if (processId == -1) {
        std::string errorMessage = "Content-Type: text/html\r\n\r\n<html><body style='text-align:center;'><h1>500 Internal Server Error</h1></body></html>";
        write(STDOUT_FILENO, errorMessage.c_str(), errorMessage.size());
        delete[] scriptArgument;
        std::exit(EXIT_FAILURE);
    } else if (processId == 0) {
        executeChildProcess(scriptPath.c_str(), arguments, pipeIn, pipeOut);
    } else {
        handleParentProcess(processId, pipeIn, pipeOut, scriptPath);
        delete[] scriptArgument;
    }
}