#include "WebServer.hpp"

std::string trimm(const std::string& str) 
{
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");

    if (first == std::string::npos || last == std::string::npos)
        return "";

    return str.substr(first, (last - first + 1));
}

NetworkClient& WebServer::GetRightClient(int fd) 
{
    std::map<int, NetworkClient>::iterator it = this->clients.find(fd);
    if (it != this->clients.end())
    {
        std::cout << "Client found on fd: " << fd << std::endl;
        return it->second;
    }
    else
        throw std::runtime_error("BUG: Potential Server error");
}

WebServer::WebServer(const Config& config)
    : highestFd(0) 
{
    serverConfigs = new std::vector<ConfigServer>(config.getServers());

	fd_set masterSet, readSet, writeSet;
    FD_ZERO(&masterSet);
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);

    setupServerSockets();
}

WebServer::~WebServer() 
{
    for (size_t i = 0; i < serverSockets.size(); ++i) 
        close(serverSockets[i]);
    for (std::map<int, NetworkClient>::iterator it = clients.begin(); it != clients.end(); ++it)
        close(it->second.fetchConnectionSocket());
    delete serverConfigs;
}

void WebServer::setupServerSockets() 
{
    for (size_t i = 0; i < serverConfigs->size(); ++i) 
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) 
        {
            std::cerr << "Error opening socket for server " << (*serverConfigs)[i].getServerName() << ": " << strerror(errno) << std::endl;
            continue;
        }

        int optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) 
        {
            std::cerr << "Error setting socket options for server " << (*serverConfigs)[i].getServerName() << ": " << strerror(errno) << std::endl;
            close(sockfd);
            continue;
        }

        sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr((*serverConfigs)[i].getHost().c_str());
        serv_addr.sin_port = htons((*serverConfigs)[i].getPort());

        if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        {
            std::cerr << "ERROR on binding for server " << (*serverConfigs)[i].getServerName() << " on " << (*serverConfigs)[i].getHost() << ":" << (*serverConfigs)[i].getPort() << ": " << strerror(errno) << std::endl;
            close(sockfd);
            continue;
        }

        if (listen(sockfd, 10) < 0) 
        {
            std::cerr << "Error listening on socket for server " << (*serverConfigs)[i].getServerName() << ": " << strerror(errno) << std::endl;
            close(sockfd);
            continue;
        }
        FD_SET(sockfd, &this->readSet);
        FD_SET(sockfd, &this->writeSet);
        FD_SET(sockfd, &this->masterSet);
        if (sockfd > highestFd) 
            highestFd = sockfd;
        serverSockets.push_back(sockfd);
        (*serverConfigs)[i].setSocket(sockfd);
        
        std::cout << "Server " << (*serverConfigs)[i].getServerName() 
                  << " set up on " << (*serverConfigs)[i].getHost() 
                  << ":" << (*serverConfigs)[i].getPort() << " with socket: " << sockfd << std::endl;
    }
}

void WebServer::run() 
{
    fd_set readcpy;
    fd_set writecpy;
    while (true) 
    {
        readcpy = this->readSet;
        writecpy = this->writeSet;
       
        if (select(this->highestFd + 1, &readcpy, &writecpy, NULL, NULL) < 0) 
            std::cerr << "Error in select()." << std::endl;

        for (int i = 3; i <= this->highestFd; i++) 
        { 
            if (FD_ISSET(i, &readcpy)) 
            {
                if (std::find(serverSockets.begin(), serverSockets.end(), i) != serverSockets.end()) 
                {
                    std::cout << "New client connection on socket: " << i << std::endl;
                    acceptNewClient(i);                                               
                }
                else 
                {
                    // std::cout << "Processing client requests for socket: " << i << std::endl;
                    processClientRequests(i);
				}
            }
            if(FD_ISSET(i, &writecpy))
            {
                NetworkClient& client = GetRightClient(i);
                std::cout << "Sending data to client on socket: " << i << std::endl;
                sendDataToClient(client);
            }
        }   
    }
}

const ConfigServer& WebServer::matchServerByFd(int fd) 
{
    for (size_t i = 0; i < serverConfigs->size(); ++i)
    {
        if ((*serverConfigs)[i].getSocket() == fd)
            return (*serverConfigs)[i];
    }
   std::cerr << "No matching server found for socket fd: " << fd << std::endl;
    return (*serverConfigs)[0];
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
    //Match ServerConfig with server which the client is connected
    const ConfigServer& associatedServer = matchServerByFd(serverSocket);
    NetworkClient newClient(clientSocket, serverSocket);
    newClient.setServer(associatedServer);
    clients[clientSocket] = newClient;
    FD_SET(clientSocket, &this->readSet);
    if (clientSocket > highestFd)
        highestFd = clientSocket;
    // std::cout << "New client on socket " << clientSocket << " accepted." << std::endl;
}

void WebServer::closeClient(int clientSocket) 
{
    std::map<int, NetworkClient>::iterator it = clients.find(clientSocket);
    if (it != clients.end()) 
    {
        close(it->first);
        FD_CLR(it->first, &masterSet);
        FD_CLR(it->first, &readSet);
        FD_CLR(it->first, &writeSet);
        clients.erase(it);
        std::cout << "Client with socket " << clientSocket << " has been closed and removed." << std::endl;
    } 
    else
        std::cerr << "Attempt to close non-existent client socket." << std::endl;
}

// int extractContentLength(const std::string& request) {
//     std::string contentLengthHeader = "Content-Length: ";
//     std::size_t pos = request.find(contentLengthHeader);
//     if (pos != std::string::npos) {
//         pos += contentLengthHeader.length();
//         std::size_t endPos = request.find("\r\n", pos);
//         if (endPos != std::string::npos) {
//             std::string contentLengthStr = request.substr(pos, endPos - pos);
//             try {
//                 return std::stoi(contentLengthStr);
//             } catch (const std::invalid_argument& e) {
//                 std::cerr << "Invalid Content-Length value" << std::endl;
//                 return -1;
//             }
//         }
//     }
//     return -1; // Return -1 if Content-Length is not found
// }

void WebServer::processClientRequests(int clientSocket) 
{
    NetworkClient& client = GetRightClient(clientSocket);
    std::string requestString;
	// std::stringstream	REQ;
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
        closeClient(client.fetchConnectionSocket());
        return;
    }
   
	ConfigServer	conf = client.getServer();
    HttpRequest request(conf);
    request.parseHttpRequest(requestString);
    request.setParsingFinished(true);

    std::cout << "\n" << requestString << std::endl;
	std::cout << "durant function process=>fd of client: " << clientSocket << std::endl;
	clients[clientSocket].setRequest(request);
	FD_SET(clientSocket, &this->writeSet);

    // std::string hostHeader = request.getHeader("Host");
    // if (!hostHeader.empty())
    // {
    //     hostHeader = trimm(hostHeader);

    //     size_t portPos = hostHeader.find(":");
    //     int port = 90; // Default to 80 if no port is specified
    //     if (portPos != std::string::npos) {
    //         port = std::atoi(hostHeader.substr(portPos + 1).c_str());
    //         hostHeader = hostHeader.substr(0, portPos);
    //     } 
    //     else
    //         port = client.getServer().getPort();

    //     const ConfigServer& clientServer = matchServerByName(hostHeader, port);
    //     client.setServer(clientServer);
        // FD_SET(clientSocket, &this->writeSet);
    //     sendResponse(request, client);
    // } 
    // else 
    // {
    //     std::cerr << "Host header not found in the request." << std::endl;
    //     closeClient(client.fetchConnectionSocket());
    //     return;
    // }
}

int WebServer::sendResponseBody(NetworkClient &client) {
    std::size_t bytesSent = 0;
    std::size_t totalBytesSent = 0;
    const std::string& responseBody = client.getResponseBody();
    std::size_t responseBodyLength = client.getResponseBody().length();

    std::cout <<"Response Body: " << client.getResponseBody() << "\n";
    while (totalBytesSent < responseBodyLength) {
        bytesSent = send(client.fetchConnectionSocket(), responseBody.c_str() + totalBytesSent, responseBodyLength - totalBytesSent, 0);
        if (bytesSent <= 0) {
            std::cerr << "Error sending response body, bytesSent: " << bytesSent << std::endl;
            return bytesSent;
        }
        totalBytesSent += bytesSent;
    }
    return totalBytesSent;
}

void WebServer::sendResponse(HttpRequest &req, NetworkClient &client) {
    
	HttpResponse *resp = new HttpResponse(client);

    resp->generateResponse(req);
    if (!client.getHeaderSent()) {
        client.setResponse(client.getResponseHeader());
        client.setHeaderSent(true);
    }
    // else
	// {
	// 	std::cout << "Body: " << client.getResponseBody() << "\n";
	// 	if (client.getResponseBody().length() > 0)
	// 	{
	// 		char buffer[1024];
	// 		if (client.getOpenFile() == false)
	// 		{
	// 			client._file.open(client.getResponseBody().c_str(), std::ios::in | std::ios::binary);
	// 			client.setOpenFile(true);
	// 		}
	// 		if (client._file.good())
	// 		{
	// 			client._file.read(buffer, 1024);
	// 			client.bytes_read = client._file.gcount();
	// 			client.setResponse(std::string(buffer, client.bytes_read));
	// 		}
	// 		else
	// 		{
	// 			int ss = send(client.fetchConnectionSocket(), client.getResponseBody().c_str(), client.getResponseBody().length(), 0);
	// 			if (ss < 0 || ss == (int)client.getResponseBody().length()) {
	// 				closeClient(client.fetchConnectionSocket());
    //                 delete resp;
    //             }
	// 			return;
	// 		}
	// 	}
	// }
	// std::cout << "kidkhol hna\n";
    // std::cout << client.getHeaderSent() << "\n";
	// int result = send(client.fetchConnectionSocket(), client.getResponse().c_str(), client.getResponse().length(), 0);
	// if (result <= 0) {
	// 	closeClient(client.fetchConnectionSocket());
    //     // delete resp;
    // }
    std::cout << client.getResponse() << "\n";
    int result = send(client.fetchConnectionSocket(), client.getResponse().c_str(), client.getResponse().length(), 0);
    if (result <= 0) {
        std::cerr << "Error sending response header, result: " << result << std::endl;
        closeClient(client.fetchConnectionSocket());
		delete resp;
        return;
    }
    if (client.getHeaderSent())
	{
        char buffer[1024];
        if (!client.getOpenFile()) {
            client.openFileForReading();
        }

        if (client.isFileOpen()) {
            while (client.isFileOpen())
            {
                client.readFromFile(buffer, 1024);
                std::size_t bytesToRead = static_cast<std::size_t>(client.bytesRead());
                if (bytesToRead > 0) {
                    std::size_t totalBytesSent = 0;
                    std::size_t bytesSent = 0;
                    while (totalBytesSent < bytesToRead) {
                        bytesSent = send(client.fetchConnectionSocket(), buffer + totalBytesSent, bytesToRead - totalBytesSent, 0);
                        if (bytesSent <= 0) {
                            std::cerr << "Error sending file data, bytesSent: " << bytesSent << std::endl;
                            closeClient(client.fetchConnectionSocket());
							delete resp;
                            return;
                        }
                        totalBytesSent += bytesSent;
                    }
                } else {
                    std::cerr << "No bytes read from file, closing client connection." << std::endl;
                    std::cerr << "File open status: " << client.isFileOpen() << std::endl;
                    closeClient(client.fetchConnectionSocket());
					delete resp;
                    return;
                }
            }
            
        } else {
            std::size_t bytesSent = sendResponseBody(client);
            if (bytesSent <= 0 || bytesSent == client.getResponseBody().length()) {
                std::cerr << "Error or complete response body sent, bytesSent: " << bytesSent << std::endl;
                closeClient(client.fetchConnectionSocket());
				delete resp;
                return;
            }
        }
    }

    std::cerr << "WASH KAYUSAL HNA ??\n";
    std::cout << client.getHeaderSent() << "\n";
    std::cout << client.getResponse() << "\n";

    int res = send(client.fetchConnectionSocket(), client.getResponse().c_str(), client.getResponse().length(), 0);
    if (res <= 0) {
        std::cerr << "Error sending final response, res: " << res << std::endl;
        closeClient(client.fetchConnectionSocket());
		delete resp;
    }
    delete resp;
}

void WebServer::sendDataToClient(NetworkClient& client) 
{
    sendResponse(client.getRequest(), client);
}       

NetworkClient* WebServer::findClientBySocket(int socket) 
{
    std::map<int, NetworkClient>::iterator it = clients.find(socket);
    if (it != clients.end())
        return &it->second;
    return NULL;
}

const ConfigServer& WebServer::matchServerByName(const std::string& host, int port) 
{
    std::string hostName = trimm(host);
    size_t pos = hostName.find(":");
    bool isLocalhost = false;

    if (pos != std::string::npos)
        hostName = hostName.substr(0, pos);

    if (hostName == "localhost" || hostName == "127.0.0.1")
        isLocalhost = true;

    for (std::vector<ConfigServer>::const_iterator it = serverConfigs->begin(); it != serverConfigs->end(); ++it) 
    {
        std::ostringstream portStr;
        portStr << it->getPort();

        if ((it->getHost() == "localhost" || it->getHost() == "127.0.0.1") && isLocalhost)
        {
            if (static_cast<size_t>(port) == it->getPort())
            {
                // std::cout << "Matched server: " << it->getServerName() << " with host: " << it->getHost() << " on port: " << it->getPort() << std::endl;
                return *it;
            }
        }
        else if (it->getHost() == hostName && static_cast<size_t>(port) == it->getPort())
        {
            // std::cout << "\n******* ✅ ✅ ✅ ✅Matched server: " << it->getServerName() << " with host: " << it->getHost() << " on port: " << it->getPort() << std::endl;
            return *it;
        }
    }

    // std::cerr << "\n******* ❌ ❌ ❌ ❌No matching server found for host: " << host << " on port: " << port << ". Defaulting to first server." << std::endl;
    return (*serverConfigs)[0];
}