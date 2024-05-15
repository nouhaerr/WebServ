#ifndef NETWORK_CLIENT_HPP
#define NETWORK_CLIENT_HPP

#include <string>
#include <netinet/in.h>

class NetworkClient {
public:
    NetworkClient();
    NetworkClient(int socketDescriptor);
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

    friend bool operator==(const NetworkClient& lhs, const NetworkClient& rhs);

private:
    int serverSocketId;
    int connectionSocketId;
    socklen_t clientAddressSize;
    sockaddr_in clientDetails;
    std::string responseHeader;
    std::string responseBody;
    std::string fullResponse;
    bool headerDispatched;
    bool fileAccessed;
};

#endif
