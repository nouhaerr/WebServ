#include "WebServer.hpp"

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
    
	HttpResponse resp(client);

    resp.generateResponse(req);
    if (!client.getHeaderSent()) {
        client.setResponse(client.getResponseHeader());
        client.setHeaderSent(true);
    }
    else
	{
		std::cout << "Body: " << client.getResponseBody() << "\n";
		if (client.getResponseBody().length() > 0)
		{
			char buffer[1024];
			if (client.getOpenFile() == false)
			{
				client._file.open(client.getResponseBody().c_str(), std::ios::in | std::ios::binary);
				client.setOpenFile(true);
			}
			if (client._file.good())
			{
				client._file.read(buffer, 1024);
				client.bytes_read = client._file.gcount();
				client.setResponse(std::string(buffer, client.bytes_read));
			}
			else
			{
				int ss = send(client.fetchConnectionSocket(), client.getResponseBody().c_str(), client.getResponseBody().length(), 0);
				if (ss < 0 || ss == (int)client.getResponseBody().length()) {
					closeClient(client.fetchConnectionSocket());
                    // delete resp;
                }
				return;
			}
		}
	}
    int res = send(client.fetchConnectionSocket(), client.getResponse().c_str(), client.getResponse().length(), 0);
    if (res <= 0) {
        std::cerr << "Error sending final response, res: " << res << std::endl;
        closeClient(client.fetchConnectionSocket());
		// delete resp;
    }
    // delete resp;
}



// // 	// std::cout << "kidkhol hna\n";
// //     // std::cout << client.getHeaderSent() << "\n";
// // 	// int result = send(client.fetchConnectionSocket(), client.getResponse().c_str(), client.getResponse().length(), 0);
// // 	// if (result <= 0) {
// // 	// 	closeClient(client.fetchConnectionSocket());
// //     //     // delete resp;
// //     // }
// //     std::cout << client.getResponse() << "\n";
// //     int result = send(client.fetchConnectionSocket(), client.getResponse().c_str(), client.getResponse().length(), 0);
// //     if (result <= 0) {
// //         std::cerr << "Error sending response header, result: " << result << std::endl;
// //         closeClient(client.fetchConnectionSocket());
// // 		delete resp;
// //         return;
// //     }
// //     if (client.getHeaderSent())
// // 	{
// //         char buffer[1024];
// //         if (!client.getOpenFile()) {
// //             client.openFileForReading();
// //         }

// //         if (client.isFileOpen()) {
// //             while (client.isFileOpen())
// //             {
// //                 client.readFromFile(buffer, 1024);
// //                 std::size_t bytesToRead = static_cast<std::size_t>(client.bytesRead());
// //                 if (bytesToRead > 0) {
// //                     std::size_t totalBytesSent = 0;
// //                     std::size_t bytesSent = 0;
// //                     while (totalBytesSent < bytesToRead) {
// //                         bytesSent = send(client.fetchConnectionSocket(), buffer + totalBytesSent, bytesToRead - totalBytesSent, 0);
// //                         if (bytesSent <= 0) {
// //                             std::cerr << "Error sending file data, bytesSent: " << bytesSent << std::endl;
// //                             closeClient(client.fetchConnectionSocket());
// // 							delete resp;
// //                             return;
// //                         }
// //                         totalBytesSent += bytesSent;
// //                     }
// //                 } else {
// //                     std::cerr << "No bytes read from file, closing client connection." << std::endl;
// //                     std::cerr << "File open status: " << client.isFileOpen() << std::endl;
// //                     closeClient(client.fetchConnectionSocket());
// // 					delete resp;
// //                     return;
// //                 }
// //             }
            
// //         } else {
// //             std::size_t bytesSent = sendResponseBody(client);
// //             if (bytesSent <= 0 || bytesSent == client.getResponseBody().length()) {
// //                 std::cerr << "Error or complete response body sent, bytesSent: " << bytesSent << std::endl;
// //                 closeClient(client.fetchConnectionSocket());
// // 				delete resp;
// //                 return;
// //             }
// //         }
// //     }

// //     // std::cerr << "WASH KAYUSAL HNA ??\n";
// //     // std::cout << client.getHeaderSent() << "\n";
// //     // std::cout << client.getResponse() << "\n";

// //     int res = send(client.fetchConnectionSocket(), client.getResponse().c_str(), client.getResponse().length(), 0);
// //     if (res <= 0) {
// //         std::cerr << "Error sending final response, res: " << res << std::endl;
// //         closeClient(client.fetchConnectionSocket());
// // 		delete resp;
// //     }
// //     delete resp;
// // }
