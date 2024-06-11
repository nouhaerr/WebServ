#include "WebServer.hpp"

void WebServer::sendResponse(HttpRequest &req, NetworkClient &client)
{	
	HttpResponse resp(client);
	resp.generateResponse(req);
	if (client.getHeaderSent() == false)
	{
		client.setResponse(client.getResponseHeader());
		client.setHeaderSent(true);
		std::cout << client.getResponse();

	}
	else
	{
		// std::cout << "Body: " << client.getResponseBody() << "\n";
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
                std::size_t bytesToRead = static_cast<std::size_t>(client.bytes_read);
				std::cout << "BYtesRead: " << bytesToRead << "\n";
				client.setResponse(std::string(buffer, bytesToRead));
			}
			else
			{
				std::cout << "IN ELSE, SockCli: " << client.fetchConnectionSocket() << "\n";
				ssize_t bytesSent = send(client.fetchConnectionSocket(), client.getResponseBody().c_str(), client.getResponseBody().length(), 0);
				if (bytesSent < 0 || bytesSent == (int)client.getResponseBody().length()) {

					closeClient(client.fetchConnectionSocket());
				}
				return;
			}
		}
	}
	std::cout << "Outside SockCl: " <<client.fetchConnectionSocket() << "\n";
	ssize_t bytesSent = 0;
    bytesSent = send(client.fetchConnectionSocket(), client.getResponse().c_str(), client.getResponse().length(), 0);
	std::cout << "BytesSent: " << bytesSent << "\n";
     if (bytesSent <= 0 ) {
		closeClient(client.fetchConnectionSocket());
	}
}

