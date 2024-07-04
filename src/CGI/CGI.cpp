#include "CGI.hpp"
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctime>
#include <csignal>
#include <iostream>

CGI::CGI(NetworkClient &client) :  envData(nullptr) ,client(client), responseStatus(200) {
}

CGI::~CGI() {
    if (envData) {
        for (size_t i = 0; envData[i] != nullptr; ++i) {
            delete[] envData[i];
        }
        delete[] envData;
    }
}

CGI::CGI(const CGI& otherCGI) : client(otherCGI.client), responseStatus(otherCGI.responseStatus) {
    if (otherCGI.envData) {
        size_t count = 0;
        while (otherCGI.envData[count]) ++count;
        envData = new char*[count + 1];
        for (size_t i = 0; i < count; ++i) {
            envData[i] = new char[strlen(otherCGI.envData[i]) + 1];
            strcpy(envData[i], otherCGI.envData[i]);
        }
        envData[count] = nullptr;
    } else {
        envData = nullptr;
    }
}

CGI& CGI::operator=(const CGI& otherCGI) 
{
    if (this != &otherCGI) {
        for (size_t i = 0; envData && envData[i] != nullptr; ++i) {
            delete[] envData[i];
        }
        delete[] envData;

        responseStatus = otherCGI.responseStatus;

        if (otherCGI.envData) {
            size_t count = 0;
            while (otherCGI.envData[count]) ++count;
            envData = new char*[count + 1];
            for (size_t i = 0; i < count; ++i) {
                envData[i] = new char[strlen(otherCGI.envData[i]) + 1];
                strcpy(envData[i], otherCGI.envData[i]);
            }
            envData[count] = nullptr;
        } else {
            envData = nullptr;
        }
    }
    return *this;
}

void CGI::initializeEnvData(const std::vector<std::string>& envs) 
{
    this->envData = new char*[envs.size() + 1];
    size_t i = 0;
    for (; i < envs.size(); ++i) {
        this->envData[i] = new char[envs[i].size() + 1];
        std::strcpy(this->envData[i], envs[i].c_str());
    }
    this->envData[i] = nullptr;
}

void CGI::configureEnvironment(const std::string& file_name) {
    std::vector<std::string> envs;
    std::map<std::string, std::string> &headers = this->client.getRequest().getHeaderFields();
    HttpRequest &request = this->client.getRequest();

    envs.push_back("CONTENT_TYPE=" + headers["Content-Type"]);
    envs.push_back("REDIRECT_STATUS=200");
    envs.push_back("CONTENT_LENGTH=" + headers["Content-Length"]);
    envs.push_back("HTTP_COOKIE=" + headers["Cookie"]);
    envs.push_back("HTTP_USER_AGENT=" + headers["User-Agent"]);
    envs.push_back("PATH_INFO=");
    envs.push_back("QUERY_STRING=" + request.get_queryString());
    envs.push_back("REMOTE_ADDR=");
    envs.push_back("REMOTE_HOST=");
    envs.push_back("REQUEST_METHOD=" + request.getMethod());
    std::cout << "FILE NAME " << file_name << std::endl;
    envs.push_back("SCRIPT_NAME=" + file_name);
    envs.push_back("SERVER_NAME=" + headers["Host"]);
    envs.push_back("SERVER_SOFTWARE=HTTP/1.1");
    envs.push_back("SCRIPT_FILENAME=" + std::string("/Users/user/Desktop/roro/src/cgi-bin/") + file_name);
    envs.push_back("REQUEST_URI=" + std::string("/Users/user/Desktop/roro/src/cgi-bin/") + file_name);

    this->envData = new char*[envs.size() + 1];
    for (size_t i = 0; i < envs.size(); i++) {
        this->envData[i] = new char[envs[i].size() + 1];
        std::strcpy(this->envData[i], envs[i].c_str());
    }
    this->envData[envs.size()] = NULL;
}

char** CGI::get_CGIenvironmentVariables() {
    return this->envData;
}

bool CGI::initializePipes(int pipeIn[2], int pipeOut[2]) {
    if (pipe(pipeOut) == -1) {
        responseStatus = 500;
        return false;
    }
    if (client.getRequest().getMethod() == "POST") {
        if (pipe(pipeIn) == -1) {
            responseStatus = 500;
            close(pipeOut[0]);
            close(pipeOut[1]);
            return false;
        }
    }
    return true;
}

void CGI::executeChildProcess(const char* scriptPath, char* args[], int pipeIn[2], int pipeOut[2]) {
    if (client.getRequest().getMethod() == "POST") {
        close(pipeIn[1]);
        dup2(pipeIn[0], STDIN_FILENO);
        close(pipeIn[0]);
    }
   // std::cout << args[0] << std::endl;
    close(pipeOut[0]);
    dup2(pipeOut[1], STDOUT_FILENO);
    close(pipeOut[1]);
    std::cout << "Executing script: " << scriptPath << std::endl; 
    (void) scriptPath;
        std::cout << "Checking script path: " << args[0] << std::endl;
// const char *arr[] = {"/Users/user/Desktop/roro/src/cgi-bin/test.php", NULL};



    execve(args[0], args, this->envData);

    std::string errorMessage = "Content-Type: text/html\r\n\r\n<html><body style='text-align:center;'><h1>500 Internal Error</h1></body></html>";
    write(STDOUT_FILENO, errorMessage.c_str(), errorMessage.size());
    std::exit(EXIT_FAILURE);
}

void CGI::handleParentProcess(pid_t processId, int pipeIn[2], int pipeOut[2], const std::string& scriptArgument) {
    if (client.getRequest().getMethod() == "POST") {
        close(pipeIn[0]);
        int inputFileDescriptor = open(client.getRequest().get_bodyFileName().c_str(), O_RDONLY);
        if (inputFileDescriptor == -1) {
            responseStatus = 500;
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
        responseStatus = 504;
        kill(processId, SIGKILL);
    }

    for (int i = 0; envData && envData[i]; ++i) {
        delete[] envData[i];
    }
    delete[] envData;
    envData = nullptr;

    char buffer[1024];
    ssize_t bytesRead;
    std::string response;

    while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }
     std::cout << "CGI Response: " << response << std::endl;

    client.setResponse(response.c_str());

    close(pipeOut[0]);
}

// void CGI::executeScript() {
//     pid_t processId;
//     int pipeIn[2] = {-1, -1}, pipeOut[2] = {-1, -1};

//     std::string scriptPath = "./src" + client.getRequest().getUri();
//     char* scriptArgument = new char[scriptPath.size() + 1];
//     std::strcpy(scriptArgument, scriptPath.c_str());

//     char *arguments[] = {scriptArgument, nullptr};

//     if (!initializePipes(pipeIn, pipeOut)) {
//         delete[] scriptArgument;
//         std::exit(EXIT_FAILURE);
//     }

//     processId = fork();
//     if (processId == -1) {
//         std::string errorMessage = "Content-Type: text/html\r\n\r\n<html><body style='text-align:center;'><h1>500 Internal Error</h1></body></html>";
//         write(STDOUT_FILENO, errorMessage.c_str(), errorMessage.size());
//         delete[] scriptArgument;
//         std::exit(EXIT_FAILURE);
//     } else if (processId == 0) {
//        // std::cout << scriptArgument << std::endl;
//         executeChildProcess(scriptPath.c_str(), arguments, pipeIn, pipeOut);
//     } else {
//         handleParentProcess(processId, pipeIn, pipeOut, scriptPath);
//         delete[] scriptArgument;
//     }
// }


void CGI::executeScript() {
    pid_t processId;
    int pipeIn[2] = {-1, -1}, pipeOut[2] = {-1, -1};

    std::string scriptPath = "./src" + client.getRequest().getUri();
    char* scriptArgument = new char[scriptPath.size() + 1];
    std::strcpy(scriptArgument, scriptPath.c_str());

    char *arguments[] = {scriptArgument, nullptr};

    if (!initializePipes(pipeIn, pipeOut)) {
        delete[] scriptArgument;
        std::exit(EXIT_FAILURE);
    }

    processId = fork();
    if (processId == -1) {
        std::string errorMessage = "Content-Type: text/html\r\n\r\n<html><body style='text-align:center;'><h1>500 Internal Error</h1></body></html>";
        write(STDOUT_FILENO, errorMessage.c_str(), errorMessage.size());
        delete[] scriptArgument;
        std::exit(EXIT_FAILURE);
    } else if (processId == 0) {
       // std::cout << scriptArgument << std::endl;
        executeChildProcess(scriptPath.c_str(), arguments, pipeIn, pipeOut);
    } else {
        handleParentProcess(processId, pipeIn, pipeOut, scriptPath);
        delete[] scriptArgument;
    }
}
