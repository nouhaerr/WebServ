#ifndef NETWORK_CLIENT_HPP
#define NETWORK_CLIENT_HPP

#include <string>
#include <netinet/in.h>
#include "../parsing/ConfigServer.hpp"

#include "HttpRequest.hpp"

class HttpRequest;
class NetworkClient {
public:
    NetworkClient();
    NetworkClient(int socketDescriptor, int serverSocket);
    NetworkClient(const NetworkClient& source);
    ~NetworkClient();
    NetworkClient& operator=(const NetworkClient& source);

    bool hasFileBeenAccessed() const;
    void markFileAsAccessed(bool accessed);

    bool wasHeaderDispatched() const;
    void markHeaderAsDispatched(bool dispatched);

    std::string retrieveResponseContent() const;
    void updateResponseContent(const std::string& content);

    std::string retrieveBodyContent() const;
    void updateBodyContent(const std::string& content);

    std::string retrieveHeaderContent() const;
    void updateHeaderContent(const std::string& content);

    int fetchServerSocket() const;
    int fetchConnectionSocket() const;
    void assignConnectionSocket(int socket);
    sockaddr_in* fetchClientInfo();
    socklen_t* fetchAddressLength();

    bool isResponsePrepared() const;
    void setServer(const ConfigServer& server);
    friend bool operator==(const NetworkClient& lhs, const NetworkClient& rhs);
    const ConfigServer& getConfigServer() const;

    ConfigServer& getServer();

    void    setRequest(HttpRequest req);
    void	setResponseHeader(std::string respHeader);
    void    setResponseBody(std::string body);
    void    setHeaderSent(bool value);
    void    setResponse(std::string response);
    void    setOpenFile(bool value);
    HttpRequest& getRequest();
    std::string getResponseHeader();
    std::string getResponseBody();
    bool        getHeaderSent();
    std::string getResponse();
    bool        getOpenFile();
    void openFileForReading();
    bool isFileOpen() const;
    void readFromFile(char* buffer, std::streamsize bufferSize);
    std::streamsize bytesRead() const;

    void    setREQ(std::string& requestString);
    std::string&    getREQ();
    HttpRequest		_req;
    std::ifstream _file;
    int			bytes_read;

private:
    int serverSocketId;
    int connectionSocketId;
    ConfigServer server;
    socklen_t clientAddressSize;
    sockaddr_in clientDetails;
    std::string responseHeader;
    std::string responseBody;
    std::string fullResponse;
    bool headerDispatched;
    bool fileAccessed;
    bool    _headersSent;
    bool    _openFile;
    std::string _response;
    std::string REQ;
};

#endif
