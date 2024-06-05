#include "WebServer.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestParser.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <algorithm>
#include <arpa/inet.h>
#include <cctype>

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
        return it->second;
    else
        throw std::runtime_error("BUG: Potential Server error");
}

WebServer::WebServer(const Config& config)
    : highestFd(0) 
{
    serverConfigs = new std::vector<ConfigServer>(config.getServers());

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
            if(FD_ISSET(i, &writecpy))
            {
                NetworkClient client = GetRightClient(i); 
                // std::cout << "Sending data to client on socket: " << i << std::endl;
                sendDataToClient(client);
            }
            else if (FD_ISSET(i, &readcpy)) 
            {
                if (std::find(serverSockets.begin(), serverSockets.end(), i) != serverSockets.end()) 
                {
                    // std::cout << "New client connection on socket: " << i << std::endl;
                    acceptNewClient(i);                                               
                }
                else 
                {
                    //std::cout << "Processing client requests for socket: " << i << std::endl;
                    processClientRequests(i);                                                                
                }
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
   // std::cerr << "No matching server found for socket fd: " << fd << std::endl;
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

    NetworkClient newClient(clientSocket, serverSocket);
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
        // std::cout << "Client with socket " << clientSocket << " has been closed and removed." << std::endl;
    } 
    else
        std::cerr << "Attempt to close non-existent client socket." << std::endl;
}

void WebServer::processClientRequests(int clientSocket) 
{
    NetworkClient& client = GetRightClient(clientSocket);
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
        closeClient(client.fetchConnectionSocket());
        return;
    }

    HttpRequest request;
    request.parseHttpRequest(requestString);

    if (request.getErrorCode() != 0) 
    {
        std::cerr << "Error in request parsing: " << request.getErrorCode() << std::endl;
        closeClient(client.fetchConnectionSocket());
        return;
    }

    size_t bodypos = requestString.find("\r\n\r\n");
    if (bodypos != std::string::npos) 
    {
        bodypos += 4;  
        if (bodypos <= requestString.size()) 
        {
            std::string bodyContent = requestString.substr(bodypos);
            request.setBody(bodyContent);
        } 
        else 
            std::cerr << "Error: bodypos is out of range." << std::endl;
    } 
    else 
    {
        std::cerr << "Error: Could not find the end of the headers in the request." << std::endl;
        closeClient(client.fetchConnectionSocket());
        return;
    }

    request.setParsingFinished(true);

    std::string hostHeader = request.getHeader("Host");
    if (!hostHeader.empty()) 
    {
        hostHeader = trimm(hostHeader);

        size_t portPos = hostHeader.find(":");
        int port = 90; // Default to 80 if no port is specified
        if (portPos != std::string::npos) {
            port = std::atoi(hostHeader.substr(portPos + 1).c_str());
            hostHeader = hostHeader.substr(0, portPos);
        } 
        else
            port = client.getServer().getPort();

        const ConfigServer& clientServer = matchServerByName(hostHeader, port);
        FD_SET(clientSocket, &this->writeSet);
        std::string response = generateResponse(clientServer);
        send(client.fetchConnectionSocket(), response.c_str(), response.size(), 0);
    } 
    else 
    {
        std::cerr << "Host header not found in the request." << std::endl;
        closeClient(client.fetchConnectionSocket());
        return;
    }
    closeClient(client.fetchConnectionSocket());
}


void WebServer::sendDataToClient(NetworkClient& client) 
{
    if (!client.isResponsePrepared()) 
        return;

    std::string response = client.retrieveResponseContent();
    int sentBytes = send(client.fetchConnectionSocket(), response.c_str(), response.size(), 0);
    if (sentBytes < 0)
        std::cerr << "Failed to send response." << std::endl;
    closeClient(client.fetchConnectionSocket());
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

std::string WebServer::generateResponse(const ConfigServer& server) 
{
    std::string responseContent = 
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "    <title>Welcome to My Pro WebServer</title>\n"
        "    <style>\n"
        "        body, html {\n"
        "            margin: 0;\n"
        "            padding: 0;\n"
        "            width: 100%;\n"
        "            height: 100%;\n"
        "            overflow: hidden;\n"
        "            font-family: Arial, sans-serif;\n"
        "        }\n"
        "        .container {\n"
        "            position: absolute;\n"
        "            top: 50%;\n"
        "            left: 50%;\n"
        "            transform: translate(-50%, -50%);\n"
        "            text-align: center;\n"
        "            background: rgba(255, 255, 255, 0.8);\n"
        "            padding: 20px;\n"
        "            border-radius: 10px;\n"
        "        }\n"
        "        .matrix-background {\n"
        "            background-color: black;\n"
        "            color: green;\n"
        "            font-family: 'Courier New', Courier, monospace;\n"
        "        }\n"
        "        .matrix-text {\n"
        "            position: absolute;\n"
        "            top: 90%;\n"
        "            width: 100%;\n"
        "            text-align: center;\n"
        "            font-size: 1.5em;\n"
        "            color: green;\n"
        "        }\n"
        "        canvas {\n"
        "            display: block;\n"
        "            position: absolute;\n"
        "            top: 0;\n"
        "            left: 0;\n"
        "            width: 100%;\n"
        "            height: 100%;\n"
        "        }\n"
        "        #movingButton {\n"
        "            position: absolute;\n"
        "            padding: 10px 20px;\n"
        "            background-color: #007BFF;\n"
        "            color: white;\n"
        "            border: none;\n"
        "            border-radius: 5px;\n"
        "            cursor: pointer;\n"
        "            font-size: 1em;\n"
        "            transition: all 0.3s ease;\n"
        "        }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <canvas id=\"matrix\"></canvas>\n"
        "    <div class=\"container\" id=\"main-container\">\n"
        "        <h1>Welcome to My Pro WebServer</h1>\n"
        "        <p>This is a simple web server.</p>\n"
        "        <p>Served by: <strong>" + server.getServerName() + "</strong></p>\n"
        "        <button onclick=\"toggleBackground()\">Toggle Background</button>\n"
        "    </div>\n"
        "    <div class=\"matrix-text\">By: BG-1337-MAR</div>\n"
        "    <button id=\"movingButton\" onmouseover=\"moveButton()\">Attrape-moi</button>\n"
        "    <script>\n"
        "        var c = document.getElementById('matrix');\n"
        "        var ctx = c.getContext('2d');\n"
        "        c.width = window.innerWidth;\n"
        "        c.height = window.innerHeight;\n"
        "        var matrix = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789@#$%^&*()*&^%';\n"
        "        matrix = matrix.split('');\n"
        "        var font_size = 10;\n"
        "        var columns = c.width/font_size;\n"
        "        var drops = [];\n"
        "        for(var x = 0; x < columns; x++)\n"
        "            drops[x] = 1;\n"
        "        function draw() {\n"
        "            ctx.fillStyle = 'rgba(0, 0, 0, 0.05)';\n"
        "            ctx.fillRect(0, 0, c.width, c.height);\n"
        "            ctx.fillStyle = '#0F0';\n"
        "            ctx.font = font_size + 'px arial';\n"
        "            for(var i = 0; i < drops.length; i++) {\n"
        "                var text = matrix[Math.floor(Math.random()*matrix.length)];\n"
        "                ctx.fillText(text, i*font_size, drops[i]*font_size);\n"
        "                if(drops[i]*font_size > c.height && Math.random() > 0.975)\n"
        "                    drops[i] = 0;\n"
        "                drops[i]++;\n"
        "            }\n"
        "        }\n"
        "        setInterval(draw, 33);\n"
        "        function toggleBackground() {\n"
        "            var container = document.getElementById('main-container');\n"
        "            if (container.classList.contains('matrix-background')) {\n"
        "                container.classList.remove('matrix-background');\n"
        "                container.style.background = 'rgba(255, 255, 255, 0.8)';\n"
        "                container.style.color = '#333';\n"
        "            } else {\n"
        "                container.classList.add('matrix-background');\n"
        "                container.style.background = 'rgba(0, 0, 0, 0.8)';\n"
        "                container.style.color = 'green';\n"
        "            }\n"
        "        }\n"
        "        var clickCount = 0;\n"
        "        function moveButton() {\n"
        "            clickCount++;\n"
        "            if (clickCount > 5) {\n"
        "                window.location.href = 'https://1337.ma/fr/';\n"
        "                return;\n"
        "            }\n"
        "            var button = document.getElementById('movingButton');\n"
        "            var x = Math.random() * (window.innerWidth - button.offsetWidth);\n"
        "            var y = Math.random() * (window.innerHeight - button.offsetHeight);\n"
        "            button.style.left = x + 'px';\n"
        "            button.style.top = y + 'px';\n"
        "            if (clickCount == 5) {\n"
        "                button.innerText = 'I am here!';\n"
        "            }\n"
        "        }\n"
        "    </script>\n"
        "</body>\n"
        "</html>\n";

    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html; charset=UTF-8\r\n";
    response += "Content-Length: " + std::to_string(responseContent.size()) + "\r\n";
    response += "Connection: close\r\n\r\n";
    response += responseContent;

    return response;
}



