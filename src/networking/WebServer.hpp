#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "NetworkClient.hpp"
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>

class WebServer {
public:
    WebServer(int port, const std::string& host);
    ~WebServer();
    void run();

private:
    std::vector<NetworkClient> clients;
    fd_set masterSet, readSet;
    int serverSocket;
    int highestFd;

    void setupServerSocket(int port, const std::string& host);
    void acceptNewClient();
    void closeClient(NetworkClient& client);
    void processClientRequests(NetworkClient& client);
    void sendDataToClient(NetworkClient& client);
    NetworkClient* findClientBySocket(int socket);

};

#endif

