#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <vector>
#include <map>
#include <string>
#include <netinet/in.h>
#include "NetworkClient.hpp"
#include "../parsing/Config.hpp"
#include "../parsing/ConfigServer.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestParser.hpp"
#include <arpa/inet.h>
#include <cctype>
#include <cstring>
#include <sys/sendfile.h>
#include "../response/HttpResponse.hpp"

class WebServer {
public:
    WebServer(const Config& config);
    ~WebServer();

    void run();
    NetworkClient& GetRightClient(int fd); 

private:
    void setupServerSockets();
    void acceptNewClient(int serverSocket);
    void closeClient(int clientSocket);
   void processClientRequests(int clientSocket);
    void sendDataToClient(NetworkClient& client);
    NetworkClient* findClientBySocket(int socket);
    const ConfigServer& matchServerByFd(int fd);
    // const ConfigServer& matchServerByName(const std::string& host);
    const ConfigServer& matchServerByName(const std::string& host, int port);
    // std::string generateResponse(const ConfigServer& server);

    void sendResponse(HttpRequest &req, NetworkClient &client);
    int sendResponseBody(NetworkClient &client);

    fd_set masterSet, readSet, writeSet;
    int highestFd;
    std::vector<int> serverSockets;
    std::map<int, NetworkClient> clients;  // Changer std::vector en std::map
    // std::vector<ConfigServer> serverConfigs;
    std::vector<ConfigServer> *serverConfigs;
};

#endif // WEBSERVER_HPP
