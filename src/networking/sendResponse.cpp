#include "WebServer.hpp"

void WebServer::sendResponse(HttpRequest &req, NetworkClient &client) 
{
    HttpResponse resp(client);
    if (!client.getResponseDone() || resp.getRedir()) {
        resp.generateResponse(req);
        client.setResponseDone(true);
        client.isText = resp.isText();
    }

    if (!client.getHeaderSent())
	{
        client.setResponse(client.getResponseHeader());
        client.setHeaderSent(true);
        std::cout << client.getResponse();
    }
	else 
	{
        if (!client.getResponseBody().empty())
		{
            char buffer[1024];
            if (client.isText == true) {
                // std::cout << "ewew "<< client.getResponseBody() << std::endl;
                ssize_t bytesSent = send(client.fetchConnectionSocket(), client.getResponseBody().c_str(), client.getResponseBody().length(), 0);
                if (bytesSent < 0 || bytesSent == (int)client.getResponseBody().length()) {
                    std::cout << "salina\n";
                    std::remove(req.get_bodyFileName().c_str());
					closeClient(client.fetchConnectionSocket());
				}
				return ;
            }
            else if (!client.getOpenFile())
			{
                // client._file.open(client.getResponseBody().c_str(), std::ios::in | std::ios::binary);
				client.openFileForReading();
                if (!client.getOpenFile())
				{
                    std::cerr << "Failed to open file: " << client.getResponseBody() << std::endl;
                    closeClient(client.fetchConnectionSocket());
                    return;
                }
                // client.setOpenFile(true);
            }
            if (client._file.good())
			{
                client.readFromFile(buffer, 1024);
                std::size_t bytesRead = static_cast<std::size_t>(client.bytesRead());
                client.setResponse(std::string(buffer, bytesRead));
            }
			else 
			{
                closeClient(client.fetchConnectionSocket());
                return;
            }
        }
        else {
            closeClient(client.fetchConnectionSocket());
            return;
        }
    }

    ssize_t bytesSent = send(client.fetchConnectionSocket(), client.getResponse().c_str(), client.getResponse().length(), 0);
    if (bytesSent <= 0) {
        closeClient(client.fetchConnectionSocket());
    }
}
