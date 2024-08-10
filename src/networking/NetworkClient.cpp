#include "NetworkClient.hpp"

NetworkClient::NetworkClient() :
    _respReady(false),
    serverSocketId(-1), connectionSocketId(-1),
    server(),
    clientAddressSize(0),
    headerDispatched(false), fileAccessed(false) ,
    _headersSent(false),
    _openFile(false),
    _response(""),
    bytesSent(0),
    _resp(false) {
        updateLastActivityTime();
        std::memset(&clientDetails, 0, sizeof(clientDetails));
}

NetworkClient::NetworkClient(int socketDescriptor, int serverSocket) :
    _respReady(false),
    serverSocketId(serverSocket),
    connectionSocketId(socketDescriptor),
    server(),
    clientAddressSize(0),
    headerDispatched(false),
    fileAccessed(false),
    _headersSent(false),
    _openFile(false),
    _response(""),
    bytesSent(0),
    _resp(false) {
    std::memset(&clientDetails, 0, sizeof(clientDetails));
}

NetworkClient::NetworkClient(const NetworkClient& source)
  : _respReady(source._respReady),
    serverSocketId(source.serverSocketId), connectionSocketId(source.connectionSocketId),
    server(source.server),
    clientAddressSize(source.clientAddressSize), clientDetails(source.clientDetails),
    responseHeader(source.responseHeader), responseBody(source.responseBody),
    fullResponse(source.fullResponse), headerDispatched(source.headerDispatched),
    fileAccessed(source.fileAccessed),
    _headersSent(source._headersSent),
    _openFile(source._openFile),
    _response(source._response),
    bytesSent(source.bytesSent),
    _resp(source._resp) {}

bool operator==(const NetworkClient& lhs, const NetworkClient& rhs) 
{
    return lhs.fetchConnectionSocket() == rhs.fetchConnectionSocket();
}


NetworkClient::~NetworkClient() {}

NetworkClient& NetworkClient::operator=(const NetworkClient& source) 
{
    if (this != &source) 
    {
        _respReady = source._respReady;
        serverSocketId = source.serverSocketId;
        connectionSocketId = source.connectionSocketId;
        server = source.server;
        clientAddressSize = source.clientAddressSize;
        clientDetails = source.clientDetails;
        responseHeader = source.responseHeader;
        responseBody = source.responseBody;
        fullResponse = source.fullResponse;
        headerDispatched = source.headerDispatched;
        fileAccessed = source.fileAccessed;
        _headersSent = source._headersSent;
        _openFile = source._openFile;
        _response = source._response;
        bytesSent = source.bytes_read;
        _resp = source._resp;
    }
    return *this;
}

bool NetworkClient::hasFileBeenAccessed() const 
{
    return fileAccessed;
}

void NetworkClient::markFileAsAccessed(bool accessed) 
{
    fileAccessed = accessed;
}

bool NetworkClient::wasHeaderDispatched() const 
{
    return headerDispatched;
}

void NetworkClient::markHeaderAsDispatched(bool dispatched) 
{
    headerDispatched = dispatched;
}

std::string NetworkClient::retrieveResponseContent() const 
{
    return fullResponse;
}

void NetworkClient::updateResponseContent(const std::string& content) 
{
    fullResponse = content;
}

std::string NetworkClient::retrieveBodyContent() const 
{
    return responseBody;
}

void NetworkClient::updateBodyContent(const std::string& content) 
{
    responseBody = content;
}

std::string NetworkClient::retrieveHeaderContent() const {
    return responseHeader;
}

void NetworkClient::updateHeaderContent(const std::string& content)
{
    responseHeader = content;
}

int NetworkClient::fetchServerSocket() const 
{
    return serverSocketId;
}

int NetworkClient::fetchConnectionSocket() const 
{
    return connectionSocketId;
}

void NetworkClient::assignConnectionSocket(int socket) 
{
    connectionSocketId = socket;
}

sockaddr_in* NetworkClient::fetchClientInfo() 
{
    return &clientDetails;
}

socklen_t* NetworkClient::fetchAddressLength() 
{
    return &clientAddressSize;
}

bool NetworkClient::isResponsePrepared() const 
{
    return !fullResponse.empty();
}

void NetworkClient::setServer(const ConfigServer& server) 
{
    this->server = server; 
}

const ConfigServer& NetworkClient::getConfigServer() const 
{
    return server;
}

ConfigServer& NetworkClient::getServer()
{
    return this->server;
}

void    NetworkClient::saveRequestData(size_t nb_bytes) {
	std::string str_bytes(this->_buffer, nb_bytes);
	this->_req.setRequestData(str_bytes);
}

void    NetworkClient::setRequest(HttpRequest req) {
    this->_req = req;
}

HttpRequest& NetworkClient::getRequest() {
    return this->_req;
}

void	NetworkClient::setResponseHeader(std::string respHeader) {
    this->responseHeader = respHeader;
}

std::string NetworkClient::getResponseHeader() {
    return this->responseHeader;
}

std::string NetworkClient::getResponseBody() {
    return this->responseBody;
}

void NetworkClient::setResponseBody(std::string body) {
    this->responseBody = body;
}

bool NetworkClient::getHeaderSent() {
    return this->_headersSent;
}
void NetworkClient::setHeaderSent(bool value) {
    this->_headersSent = value;
}

std::string NetworkClient::getResponse() {
    return this->_response;
}

void NetworkClient::setResponse(std::string response) {
    this->_response = response;
    this->_respSize = response.size();
}

void NetworkClient::set_Response(std::string response, size_t RespSize) {
    this->_response = response;
    this->_respSize = RespSize;
}

void    NetworkClient::setOpenFile(bool value) {
    this->_openFile = value;
}

bool    NetworkClient::getOpenFile() {
    return this->_openFile;
}

void NetworkClient::openFileForReading() {
    _file.open(getResponseBody().c_str(), std::ios::in | std::ios::binary);
    setOpenFile(_file.is_open());
}

bool NetworkClient::isFileOpen() const {
    return _file.is_open();
}

void NetworkClient::readFromFile(char* buffer, std::streamsize bufferSize) {
    _file.read(buffer, bufferSize);
    bytes_read = _file.gcount();
}

std::streamsize NetworkClient::bytesRead() const {
    return this->bytes_read;
}

void NetworkClient::setBytesSent(std::size_t bytes) {
    this->bytesSent = bytes;
}

std::size_t NetworkClient::getBytesSent() const {
    return this->bytesSent;
}

bool NetworkClient::isTimedOut() const {
    time_t now = time(0);
    double secondsSinceLastActivity = difftime(now, lastActivityTime);
    return secondsSinceLastActivity > 10;
}

void NetworkClient::updateLastActivityTime() {
    lastActivityTime = time(NULL);
}

void	NetworkClient::setResponseDone(bool resp) {
    this->_resp = resp;
}

bool	NetworkClient::getResponseDone() const {
    return this->_resp;
}

void NetworkClient::extendTimeout() 
{
    this->updateLastActivityTime();
    // std::cout << "Timeout extended for client with socket " << this->fetchConnectionSocket() << std::endl;
}

