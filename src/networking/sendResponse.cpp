#include "WebServer.hpp"

// void WebServer::sendResponse(HttpRequest &req, NetworkClient &client)
// {	
// 	HttpResponse resp(client);
// 	resp.generateResponse(req);
// 	if (client.getHeaderSent() == false)
// 	{
// 		client.setResponse(client.getResponseHeader());
// 		client.setHeaderSent(true);
// 		std::cout << client.getResponse();

// 	}
// 	else
// 	{
// 		// std::cout << "Body: " << client.getResponseBody() << "\n";
// 		if (client.getResponseBody().length() > 0)
// 		{
// 			char buffer[1024];
// 			if (client.getOpenFile() == false)
// 			{
// 				client._file.open(client.getResponseBody().c_str(), std::ios::in | std::ios::binary);
// 				client.setOpenFile(true);
// 			}
// 			if (client._file.good())
// 			{
// 				client._file.read(buffer, 1024);
// 				client.bytes_read = client._file.gcount();
// 				varRead += client.bytes_read;
// 				std::cout << "BYtesRead: " << client.bytes_read << "\n";
// 				client.setResponse(std::string(buffer, client.bytes_read));
// 			}
// 			else
// 			{
// 				std::cout << "IN ELSE, SockCli: " << client.fetchConnectionSocket() << "\n";
// 				int ss = send(client.fetchConnectionSocket(), client.getResponseBody().c_str(), client.getResponseBody().length(), 0);
// 				if (ss < 0 || ss == (int)client.getResponseBody().length()) {

// 					closeClient(client.fetchConnectionSocket());
// 				}
// 				return;
// 			}
// 		}
// 	}
// 	std::cout << "Outside SockCl: " <<client.fetchConnectionSocket() << "\n";
// 	int result = send(client.fetchConnectionSocket(), client.getResponse().c_str(), client.getResponse().length(), 0);
// 	if (result <= 0 ) {
// 		// close(client.fileFd);
// 		closeClient(client.fetchConnectionSocket());
// 	}
// }

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
    std::cout << client.getResponse() << "\n";
    ssize_t result = send(client.fetchConnectionSocket(), client.getResponse().c_str(), client.getResponse().length(), 0);
    if (result == -1) {
        std::cerr << "Error sending response header, result: " << result << " errno: " << errno << " (" << strerror(errno) << ")\n";
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
                    ssize_t bytesSent = 0;
                    while (totalBytesSent < bytesToRead) {
                        // std::cout << "RESPONSE HEADER:" << buffer << std::endl;
						
						std::cout << "Bytes to Read: " << bytesToRead << "\n";
                        bytesSent = send(client.fetchConnectionSocket(), buffer + totalBytesSent, bytesToRead - totalBytesSent, 0);
						std::cout << "BytesSent: " << bytesSent << "\n";
						
						if (bytesSent == -1) {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                fd_set writefds;
                                FD_ZERO(&writefds);
                                FD_SET(client.fetchConnectionSocket(), &writefds);

                                // Wait indefinitely until the socket is ready for writing
                                if (select(client.fetchConnectionSocket() + 1, NULL, &writefds, NULL, NULL) == -1) {
                                    std::cerr << "Error in select, errno: " << errno << " (" << strerror(errno) << ")\n";
                                    closeClient(client.fetchConnectionSocket());
                                    delete resp;
                                    return;
                                }
                                continue; // Retry sending data
                            } else if (errno == EPIPE || errno == ECONNRESET) {
									// Client has closed the connection
									std::cerr << "Client disconnected unexpectedly.\n";
									closeClient(client.fetchConnectionSocket()); // Close the socket
									delete resp; // Cleanup resources
									return;
                            } else {
                                std::cerr << "Error sending file data, bytesSent: " << bytesSent << " errno: " << errno << " (" << strerror(errno) << ")\n";
                                closeClient(client.fetchConnectionSocket());
                                delete resp;
                                return;
                            }
                        }
                        totalBytesSent += static_cast<std::size_t>(bytesSent);
						std::cout << "TotalBytesSent: " << totalBytesSent << "\n";
                    }
					std::cout << "ContentLength: "<< resp->getFileSize() << "\n";
                } else {
                    std::cerr << "No bytes left to read from file, closing client connection." << std::endl;
                    closeClient(client.fetchConnectionSocket());
					delete resp;
                    return;
                }
            }
            
        } else {
            std::cerr << "File open status: " << client.isFileOpen() << std::endl;
            std::size_t bytesSent = sendResponseBody(client);
			std::cout << client.getResponseBody().length() << "\n";
            if (bytesSent <= 0 || static_cast<std::size_t>(bytesSent) == client.getResponseBody().length()) {
                std::cerr << "Error or complete response body sent, bytesSent: " << bytesSent << std::endl;
                closeClient(client.fetchConnectionSocket());
				delete resp;
                return;
            }
        }
    }
    delete resp;
}
