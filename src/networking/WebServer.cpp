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
                        if (!client->isResponsePrepared()) {
                            // std::cout << "HnNNAAAAAAAAA\n";
                            processClientRequests(*client); 
                        }
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
        if (requestString.find("\r\n\r\n") != std::string::npos) {
    		std::cout << "\n" << requestString << std::endl;
            break;
		}
    }


    if (bytesRead <= 0) 
    {
        std::cerr << "Initial read failed or connection closed by peer." << std::endl;
        closeClient(client);
        return;
    }

    // std::cout << "Request size: " << requestString.size() << std::endl;

    ConfigServer	conf = client.getServer();
    HttpRequest request(conf);
    request.parseHttpRequest(requestString);

    size_t bodypos = requestString.find("\r\n\r\n");
    if (bodypos != std::string::npos) {
        bodypos += 4;
    } 
    else 
    {
        std::cerr << "Error: Could not find the end of the headers in the request." << std::endl;
        closeClient(client);
        return;
    }

	std::cout << requestString[bodypos] << "\n";
    request.parseBody(bodypos);
    // std::cout << "Processed body: " << request.getBody() << std::endl;
    std::cout << "Matched one: " << client.getServer().getServerName() << " with "<< client.getServer().getHost() <<":" <<client.getServer().getPort() << std::endl;
	client.setRequest(request);
    //RESPONSE
    // std::string hostHeader = request.getHeader("Host");
	// hostHeader = trimm(hostHeader);
    // const ConfigServer& clientServer = matchServerByName(hostHeader);
    // client.setServer(clientServer);
    sendResponse(request, client);

    // if (!hostHeader.empty())
    // {
    //     std::cout << "Received Host header: " << hostHeader << std::endl;
    // }
    // else
    // {
    //     std::cerr << "Host header not found in the request." << std::endl;
    //     closeClient(client);
    //     return;
    // }
    closeClient(client);
}

// int WebServer::sendResponseBody(NetworkClient &client) {
//     return send(client.fetchConnectionSocket(), client.getResponseBody().c_str(), client.getResponseBody().length(), 0);
// }

int WebServer::sendResponseBody(NetworkClient &client) {
    std::size_t bytesSent = 0;
    std::size_t totalBytesSent = 0;
    const char* responseBody = client.getResponseBody().c_str();
    std::size_t responseBodyLength = client.getResponseBody().length();

    std::cout <<"Response Body: " << client.getResponseBody() << "\n";
    while (totalBytesSent < responseBodyLength) {
        bytesSent = send(client.fetchConnectionSocket(), responseBody + totalBytesSent, responseBodyLength - totalBytesSent, 0);
        if (bytesSent <= 0) {
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
    std::cout << client.getResponse() << "\n";
    int result = send(client.fetchConnectionSocket(), client.getResponse().c_str(), client.getResponse().length(), 0);
    if (result <= 0) {
        closeClient(client);
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
                            closeClient(client);
							delete resp;
                            return;
                        }
                        totalBytesSent += bytesSent;
                    }
                } else {
                    closeClient(client);
					delete resp;
                    return;
                }
            }
            
        } else {
            std::size_t bytesSent = sendResponseBody(client);
            if (bytesSent <= 0 || bytesSent == client.getResponseBody().length()) {
                closeClient(client);
				delete resp;
                return;
            }
        }
    }

    std::cout << client.getHeaderSent() << "\n";
    std::cout << client.getResponse() << "\n";

    int res = send(client.fetchConnectionSocket(), client.getResponse().c_str(), client.getResponse().length(), 0);
    if (res <= 0) {
        closeClient(client);
		delete resp;
    }
}

void WebServer::sendDataToClient(NetworkClient& client) 
{
    if (!client.isResponsePrepared()) {
        return;
	}

	// HttpResponse resp(client);

    // resp.generateResponse(client.getRequest());
    // if (client.getHeaderSent() == false) {
    //     client.setResponse(client.getResponseHeader());
	// 	client.setHeaderSent(true);
    // }
	//  else if (client.getHeaderSent() == true)
    // {
    //     std::cout << "Body: " << client.getResponseBody() << "\n";
    //     char buffer[1024];
	// 	if (!client.getOpenFile()) 
	// 		client.openFileForReading();

	// 	if (client.isFileOpen()) {
	// 		client.readFromFile(buffer, 1024);
	// 		if (client.bytesRead() > 0) {
	// 			client.setResponse(std::string(buffer, client.bytesRead()));
	// 		} else {
	// 			closeClient(client);
	// 			return;
	// 		}
	// 	} else {
	// 		std::size_t bytesSent = sendResponseBody(client);
	// 		if (bytesSent <= 0 || bytesSent == client.getResponseBody().length()) {
	// 			closeClient(client);
	// 			return;
	// 		}
	// 	} 
    // }
	// std::cout << client.getHeaderSent() << "\n";
    // std::cout << client.getResponse() << "\n";
    std::string response = client.retrieveResponseContent();
    // int sentBytes = send(client.fetchConnectionSocket(), client.getResponse().c_str(), client.getResponse().length(), 0);
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

// const ConfigServer& WebServer::matchServerByName(const std::string& host, int port) 
// {
//     std::string hostName = trimm(host);
//     size_t pos = hostName.find(":");
// 	bool isLocalhost = false;

//     if (pos != std::string::npos) 
//         hostName = hostName.substr(0, pos);
// 	if (hostName == "localhost" || hostName == "127.0.0.1")
//         isLocalhost = true;

//     std::cout << "Looking for server: " << hostName << std::endl;

//     for (std::vector<ConfigServer>::const_iterator it = serverConfigs.begin(); it != serverConfigs.end(); ++it) 
//     {
// 		std::ostringstream portStr;
//         portStr << it->getPort();

//         // std::cout << "Checking server: " << serverName << " with host: " << serverHost << std::endl;
// 		if ((it->getHost() == "localhost" || it->getHost() == "127.0.0.1") && isLocalhost)
//         {
// 			if (static_cast<size_t>(port) == it->getPort())
//             {
//                 // std::cout << "Matched server: " << it->getServerName() << " with host: " << it->getHost() << " on port: " << it->getPort() << std::endl;
//                 return *it;
//             }
// 		}
// 		else if (it->getHost() == hostName && static_cast<size_t>(port) == it->getPort())
//         {
//             // std::cout << "\n******* ✅ ✅ ✅ ✅Matched server: " << it->getServerName() << " with host: " << it->getHost() << " on port: " << it->getPort() << std::endl;
//             return *it;
//         }
//     }

//     std::cout << "Defaulting to first server." << std::endl;
//     return serverConfigs[0]; 
// }
