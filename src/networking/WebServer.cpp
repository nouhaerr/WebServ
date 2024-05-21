#include "WebServer.hpp"

std::string trimm(const std::string& str) 
{
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");

    if (first == std::string::npos || last == std::string::npos)
        return "";

    return str.substr(first, (last - first + 1));
}

WebServer::WebServer(const Config& config)
    : highestFd(0), serverConfigs(config.getServers()) {
    FD_ZERO(&masterSet);
    setupServerSockets();  // Setup all servers
}

WebServer::~WebServer() 
{
    for (size_t i = 0; i < serverSockets.size(); ++i) 
        close(serverSockets[i]);
    for (size_t i = 0; i < clients.size(); ++i) 
        close(clients[i].fetchConnectionSocket());
}

void WebServer::setupServerSockets() 
{
    for (size_t i = 0; i < serverConfigs.size(); ++i) 
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) 
        {
            std::cerr << "Error opening socket" << std::endl;
            continue;
        }

        int optval = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(serverConfigs[i].getHost().c_str());
        serv_addr.sin_port = htons(serverConfigs[i].getPort());

        if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        {
            std::cerr << "ERROR on binding" << std::endl;
            close(sockfd);
            continue;
        }

        listen(sockfd, 10);
        FD_SET(sockfd, &masterSet);
        if (sockfd > highestFd) highestFd = sockfd;
        serverSockets.push_back(sockfd);
        serverConfigs[i].setSocket(sockfd);
        
        std::cout << "Server " << serverConfigs[i].getServerName() 
                  << " set up on " << serverConfigs[i].getHost() 
                  << ":" << serverConfigs[i].getPort() << " with socket: " << sockfd << std::endl;
    }
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
                if (std::find(serverSockets.begin(), serverSockets.end(), i) != serverSockets.end()) 
                    acceptNewClient(i);
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

const ConfigServer& WebServer::matchServerByFd(int fd) 
{
    for (size_t i = 0; i < serverConfigs.size(); ++i) 
    {
        if (serverConfigs[i].getSocket() == fd) 
        {
            std::cout << "Matched server with fd: " << fd << " to server: " << serverConfigs[i].getServerName() << std::endl;
            return serverConfigs[i];
        }
    }
    std::cerr << "No matching server found for socket fd: " << fd << std::endl;
    return serverConfigs[0];
}

void WebServer::acceptNewClient(int serverSocket) 
{
    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrSize);

    if (clientSocket < 0) 
    {
        std::cerr << "Failed to accept client." << std::endl;
        return;
    }

    NetworkClient newClient(clientSocket, serverSocket);

    std::cout << "Server socket for new client: " << newClient.fetchServerSocket() << std::endl;
    const ConfigServer& associatedServer = matchServerByFd(serverSocket);
    if (associatedServer.getSocket() != -1)
        std::cout << "Socket number of associated server: " << associatedServer.getSocket() << std::endl;
    else 
        std::cerr << "Invalid socket number received." << std::endl;

    newClient.setServer(associatedServer);
    clients.push_back(newClient);
    FD_SET(clientSocket, &masterSet);
    highestFd = std::max(highestFd, clientSocket);

    std::cout << "New client accepted on server: " << associatedServer.getServerName() << std::endl;
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

    while ((bytesRead = recv(client.fetchConnectionSocket(), buffer, sizeof(buffer), 0)) > 0) 
    {
        requestString.append(buffer, bytesRead);
        if (requestString.find("\r\n\r\n") != std::string::npos) 
            break;
    }

    if (bytesRead <= 0) 
    {
        std::cerr << "Initial read failed or connection closed by peer." << std::endl;
        closeClient(client);
        return;
    }

    std::cout << "Full request received:\n" << requestString << std::endl;
    // std::cout << "Request size: " << requestString.size() << std::endl;

    HttpRequest request;
    request.parseHttpRequest(requestString);

    size_t bodypos = requestString.find("\r\n\r\n");
    if (bodypos != std::string::npos) 
    {
        bodypos += 4;
        // std::cout << "Body starts at position: " << bodypos << std::endl;

        // if (bodypos < requestString.size()) 
        // {
        //     std::cout << "Character at position " << bodypos << ": " << requestString[bodypos] << std::endl;
        //     std::cout << "Substring starting at position " << bodypos << ": " << requestString.substr(bodypos) << std::endl;
        // } 
        // else 
        //     std::cerr << "Error: bodypos is out of range." << std::endl;
    } 
    else 
    {
        std::cerr << "Error: Could not find the end of the headers in the request." << std::endl;
        closeClient(client);
        return;
    }

    if (bodypos > requestString.size()) 
    {
        std::cerr << "Error: Calculated body position exceeds request size." << std::endl;
        closeClient(client);
        return;
    }
    request.parseBody(bodypos, requestString);
    // std::cout << "Processed body: " << request.getBody() << std::endl;

    std::string hostHeader = request.getHeader("Host");
    if (!hostHeader.empty())
    {
        // std::cout << "Received Host header: " << hostHeader << std::endl;
        hostHeader = trimm(hostHeader);
        const ConfigServer& clientServer = matchServerByName(hostHeader);
        client.setServer(clientServer);
        // HttpResponse	resp(clientServer);
		// resp.generateResponse(request);
        std::string response = generateResponse(clientServer);
        send(client.fetchConnectionSocket(), response.c_str(), response.size(), 0);
    }
    else 
    {
        std::cerr << "Host header not found in the request." << std::endl;
        closeClient(client);
        return;
    }
    closeClient(client);
}

void WebServer::sendDataToClient(NetworkClient& client) 
{
    if (!client.isResponsePrepared()) 
        return;

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
    return NULL;
}

const ConfigServer& WebServer::matchServerByName(const std::string& host) 
{
    std::string hostName = host;
    size_t pos = host.find(":");
    if (pos != std::string::npos) 
        hostName = host.substr(0, pos);

    hostName = trimm(hostName);

    std::cout << "Looking for server: " << hostName << std::endl;

    for (std::vector<ConfigServer>::const_iterator it = serverConfigs.begin(); it != serverConfigs.end(); ++it) 
    {
        std::string serverName = trimm(it->getServerName());
        std::string serverHost = trimm(it->getHost());

        std::cout << "Checking server: " << serverName << " with host: " << serverHost << std::endl;

        if (serverName == hostName || serverHost == hostName) 
        {
            std::cout << "Matched server: " << serverName << " with host: " << serverHost << std::endl;
            return *it;
        }
    }

    std::cout << "Defaulting to first server." << std::endl;
    return serverConfigs[0]; 
}

std::string WebServer::generateResponse(const ConfigServer& server) 
{
    std::string responseContent = "<html><body><h1>Hello, World!</h1><p>This is a simple web server.</p>";
    responseContent += "<p>Served by: " + server.getServerName() + "</p></body></html>";

    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html; charset=UTF-8\r\n";
    response += "Content-Length: " + toString(responseContent.size()) + "\r\n";
    response += "Connection: close\r\n\r\n";
    response += responseContent;

    return response;
}

