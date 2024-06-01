#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "../Macros.hpp"
#include <netinet/in.h>
#include "NetworkClient.hpp"
#include "HttpRequest.hpp"
#include "../parsing/Config.hpp"
#include "../parsing/ConfigServer.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestParser.hpp"
#include <arpa/inet.h>
#include <cctype>
#include "../response/HttpResponse.hpp"
#include <sys/sendfile.h>

class NetworkClient;

class WebServer {
public:
    WebServer(const Config& config);
    ~WebServer();

    void run();

private:
     fd_set masterSet, readSet;
    int highestFd;  
    std::vector<int> serverSockets;
    std::vector<NetworkClient> clients;
    std::vector<ConfigServer> serverConfigs;

    void setupServerSockets();
    void acceptNewClient(int serverSocket);
    void closeClient(NetworkClient& client);
    void processClientRequests(NetworkClient& client);
    void sendDataToClient(NetworkClient& client);
    NetworkClient* findClientBySocket(int socket);
    const ConfigServer& matchServerByName(const std::string& host);
    const ConfigServer& matchServerByFd(int fd);

    std::string generateResponse(const ConfigServer& server);
    void sendResponse(HttpRequest &req, NetworkClient &client);
    int sendResponseBody(NetworkClient &client);
};
#endif
