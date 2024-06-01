#include "NetworkClient.hpp"
#include <cstring>
#include "../src/parsing/ConfigServer.hpp"

NetworkClient::NetworkClient()
  : serverSocketId(-1), connectionSocketId(-1), clientAddressSize(0),
    headerDispatched(false), fileAccessed(false) {
    std::memset(&clientDetails, 0, sizeof(clientDetails));
}

NetworkClient::NetworkClient(int socketDescriptor, int serverSocket)
  : serverSocketId(serverSocket),
    connectionSocketId(socketDescriptor),
    clientAddressSize(0),
    headerDispatched(false),
    fileAccessed(false) {
    std::memset(&clientDetails, 0, sizeof(clientDetails));
}

NetworkClient::NetworkClient(const NetworkClient& source)
  : serverSocketId(source.serverSocketId), connectionSocketId(source.connectionSocketId),
    clientAddressSize(source.clientAddressSize), clientDetails(source.clientDetails),
    responseHeader(source.responseHeader), responseBody(source.responseBody),
    fullResponse(source.fullResponse), headerDispatched(source.headerDispatched),
    fileAccessed(source.fileAccessed) {
}

bool operator==(const NetworkClient& lhs, const NetworkClient& rhs) 
{
    return lhs.fetchConnectionSocket() == rhs.fetchConnectionSocket();
}


NetworkClient::~NetworkClient() {}

NetworkClient& NetworkClient::operator=(const NetworkClient& source) 
{
    if (this != &source) 
    {
        serverSocketId = source.serverSocketId;
        connectionSocketId = source.connectionSocketId;
        clientAddressSize = source.clientAddressSize;
        clientDetails = source.clientDetails;
        responseHeader = source.responseHeader;
        responseBody = source.responseBody;
        fullResponse = source.fullResponse;
        headerDispatched = source.headerDispatched;
        fileAccessed = source.fileAccessed;
    }
    return *this;
}

bool NetworkClient::hasFileBeenAccessed() const 
{
    return fileAccessed;
}

void NetworkClient::markFileAsAccessed(bool accessed) 
{
    fileAccessed = accessed;
}

bool NetworkClient::wasHeaderDispatched() const 
{
    return headerDispatched;
}

void NetworkClient::markHeaderAsDispatched(bool dispatched) 
{
    headerDispatched = dispatched;
}

std::string NetworkClient::retrieveResponseContent() const 
{
    return fullResponse;
}

void NetworkClient::updateResponseContent(const std::string& content) 
{
    fullResponse = content;
}

std::string NetworkClient::retrieveBodyContent() const 
{
    return responseBody;
}

void NetworkClient::updateBodyContent(const std::string& content) 
{
    responseBody = content;
}

std::string NetworkClient::retrieveHeaderContent() const {
    return responseHeader;
}

void NetworkClient::updateHeaderContent(const std::string& content)
{
    responseHeader = content;
}

int NetworkClient::fetchServerSocket() const 
{
    return serverSocketId;
}

int NetworkClient::fetchConnectionSocket() const 
{
    return connectionSocketId;
}

void NetworkClient::assignConnectionSocket(int socket) 
{
    connectionSocketId = socket;
}

sockaddr_in* NetworkClient::fetchClientInfo() 
{
    return &clientDetails;
}

socklen_t* NetworkClient::fetchAddressLength() 
{
    return &clientAddressSize;
}

bool NetworkClient::isResponsePrepared() const 
{
    return !fullResponse.empty();
}

void NetworkClient::setServer(const ConfigServer& server) 
{
    this->server = server; 
}

const ConfigServer& NetworkClient::getConfigServer() const 
{
    return server;
}

ConfigServer& NetworkClient::getServer()
{
    return this->server;
}
