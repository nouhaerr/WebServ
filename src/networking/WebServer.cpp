#include "WebServer.hpp"
#include "HttpRequest.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <algorithm>
#include <arpa/inet.h>

WebServer::WebServer(int port, const std::string& host) 
{
    setupServerSocket(port, host);
}

WebServer::~WebServer() 
{
    close(serverSocket);
    for (size_t i = 0; i < clients.size(); ++i) 
        close(clients[i].fetchConnectionSocket());
}

// void WebServer::setupServerSocket(int port, const std::string& host) 
// {
//     sockaddr_in serverAddr;
//     serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//     if (serverSocket == -1) 
//     {
//         std::cerr << "Failed to create socket." << std::endl;
//         exit(1);
//     }

//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_addr.s_addr = inet_addr(host.c_str());
//     serverAddr.sin_port = htons(port);

//     if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) 
//     {
//         std::cerr << "Bind failed." << std::endl;
//         exit(1);
//     }

//     listen(serverSocket, 5);
//     FD_ZERO(&masterSet);
//     FD_SET(serverSocket, &masterSet);
//     highestFd = serverSocket;
// }

void WebServer::setupServerSocket(int port, const std::string& host) {
    sockaddr_in serverAddr;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        exit(1);
    }

    int optval = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(host.c_str());
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed." << std::endl;
        exit(1);
    }

    listen(serverSocket, SOMAXCONN); // Set the maximum number of clients
    FD_ZERO(&masterSet);
    FD_SET(serverSocket, &masterSet);
    highestFd = serverSocket;
}


void WebServer::run() 
{
    while (true) 
    {
        readSet = masterSet;
        if (select(highestFd + 1, &readSet, NULL, NULL, NULL) < 0) 
        {
            std::cerr << "Error in select()." << std::endl;
            continue;
        }

        for (int i = 0; i <= highestFd; i++) 
        {
            if (FD_ISSET(i, &readSet)) 
            {
                if (i == serverSocket) 
                    acceptNewClient();
                else 
                {
                    NetworkClient* client = findClientBySocket(i);
                    if (client != NULL) 
                    {
                        if (!client->isResponsePrepared()) 
                            processClientRequests(*client); 
                        else 
                            sendDataToClient(*client);
                    }
                }
            }
        }
    }
}

void WebServer::acceptNewClient() 
{
    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrSize);

    if (clientSocket < 0) 
    {
        std::cerr << "Failed to accept client." << std::endl;
        return;
    }

    NetworkClient newClient(clientSocket);
    clients.push_back(newClient);
    FD_SET(clientSocket, &masterSet);
    highestFd = std::max(highestFd, clientSocket);
}

void WebServer::closeClient(NetworkClient& client) 
{
    int clientSocket = client.fetchConnectionSocket();
    close(clientSocket);
    FD_CLR(clientSocket, &masterSet);
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
}

void WebServer::processClientRequests(NetworkClient& client) 
{
    std::string requestString;
    char buffer[1024];
    int bytesRead = 0;

    // Première lecture pour obtenir les en-têtes
    while ((bytesRead = recv(client.fetchConnectionSocket(), buffer, sizeof(buffer), 0)) > 0) 
    {
        requestString.append(buffer, bytesRead);
        if (requestString.find("\r\n\r\n") != std::string::npos) 
            break; // En-têtes reçus complètement
    }

    if (bytesRead <= 0) 
    {
        std::cerr << "Initial read failed or connection closed by peer." << std::endl;
        closeClient(client);
        return;
    }

    // Analyse des en-têtes pour obtenir Content-Length
    HttpRequest request;
    request.parseHttpRequest(requestString);
    std::string contentLengthStr = request.getHeader("Content-Length");
    int contentLength = std::stoi(contentLengthStr.empty() ? "0" : contentLengthStr);
    size_t totalHeadersAndBodyLength = requestString.find("\r\n\r\n") + 4 + contentLength;

    // Imprimer les détails de la requête
    std::cout << "Method: " << request.getMethod() 
              << "\nURI: " << request.getUri() 
              << "\nHTTP Version: " << request.getHttpVersion() << std::endl;

    // Continuer la lecture jusqu'à ce que tout le corps soit reçu
    while (requestString.size() < totalHeadersAndBodyLength && (bytesRead = recv(client.fetchConnectionSocket(), buffer, sizeof(buffer), 0)) > 0) {
        requestString.append(buffer, bytesRead);
    }

    if (bytesRead < 0) 
    {
        std::cerr << "Error while reading from socket." << std::endl;
        closeClient(client);
        return;
    }

    // Afficher la requête pour vérification
    std::cout << "Complete Request Received: " << requestString << std::endl;

    // Envoyer la réponse
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: 70\r\nConnection: close\r\n\r\n<html><body><h1>Hello, World!</h1><p>This is a simple web server.</p></body></html>";
    send(client.fetchConnectionSocket(), response.c_str(), response.size(), 0);

    closeClient(client);
}


void WebServer::sendDataToClient(NetworkClient& client) 
{
    if (!client.isResponsePrepared()) return;

    std::string response = client.retrieveResponseContent();
    int sentBytes = send(client.fetchConnectionSocket(), response.c_str(), response.size(), 0);
    if (sentBytes < 0)
        std::cerr << "Failed to send response." << std::endl;

    closeClient(client);
}

NetworkClient* WebServer::findClientBySocket(int socket) 
{
    for (std::vector<NetworkClient>::iterator it = clients.begin(); it != clients.end(); ++it) 
    {
        NetworkClient& client = *it;
        if (client.fetchConnectionSocket() == socket) 
            return &client;
    }
    return NULL;  // Retourne NULL si aucun client correspondant n'est trouvé
}

