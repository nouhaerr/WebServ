#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <vector>
#include <string>
#include <map>
#include <netinet/in.h>
#include "NetworkClient.hpp"
#include "HttpRequest.hpp"
#include "../src/parsing/Config.hpp"
#include "../src/parsing/ConfigServer.hpp"

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
};
#endif
