#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <vector>
#include <map>
#include <string>
#include <netinet/in.h>
#include "NetworkClient.hpp"
#include "../parsing/Config.hpp"
class WebServer {
public:
    WebServer(const Config& config);
    ~WebServer();

    void run();
    NetworkClient& GetRightClient(int fd);

void CheckRequestStatus(NetworkClient &client);

private:
    void setupServerSockets();
    void acceptNewClient(int serverSocket);
    void closeClient(int clientSocket);
    void processClientRequests(int clientSocket);
    void sendDataToClient(NetworkClient& client);
    NetworkClient* findClientBySocket(int socket);
    const ConfigServer& matchServerByFd(int fd);
    const ConfigServer& matchServerByName(const std::string& host, int port);
    std::string generateResponse(const ConfigServer& server);
    void readFromClient(int clientSocket, std::string &requestString);

    fd_set masterSet, readSet, writeSet;
    int highestFd;
    std::vector<int> serverSockets;
    std::map<int, NetworkClient> clients;
    std::vector<ConfigServer>* serverConfigs;
    std::map<int, std::string> clientRequests; // Stockage des requêtes par client
    int currentClientIndex;
};
#endif