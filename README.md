# C++98 Web Server

Welcome to the C++98 Web Server project! This project aims to provide a simple yet functional web server implemented in C++98, suitable for basic web development needs.

## Features

- **HTTP/1.1 Support**: The server supports the HTTP/1.1 protocol for communication with clients.
- **GET, POST, and DELETE Methods**: Handle GET, POST, and DELETE requests for fetching, submitting, and deleting data.
- **Basic Routing**: Define simple routes to handle different endpoints.
- **Static File Serving**: Serve static files such as HTML, CSS, JavaScript, images, and videos.
- **CGI Support**: Execute Common Gateway Interface (CGI) scripts for dynamic content generation.
- **Configurable**: Easily configurable settings for port number, default file to serve, and more.

## Usage

1. **Clone the Repository**: 
   ```bash
    git clone https://github.com/nouhaerr/WebServ.git

2. **Compile the Code**: 
   ```bash
    make

3. **Run the Server**:

   You can run the server by executing the program `./webserv`. This will run the server with the default configuration file.
   Alternatively, you can specify a custom configuration file by providing its path as an argument. To do this, run the command:
    ```bash
      ./webserv
      ./webserv [configuration_file]
  where [configuration_file] is the path to your custom configuration file.

4. **Access the Server**:

    Open your web browser and navigate to http://localhost:9002 (or the host:port specified in the configuration).

## Configuration

You can configure the server by editing the `config/default.conf` file. Here are some configurable options:

- **listen**: The host and port number the server listens on.
- **root**: The root directory where static files are served from.
- **index**: The default file to serve when no specific file is requested.
- **allowed_methods**: The allowed HTTP methods for requests.

## Overview

### Sockets

Sockets are communication endpoints used to send and receive data between two nodes on a network. In the context of our web server, sockets are utilized to establish connections with clients and handle incoming HTTP requests
#### Server-Side Socket Functions

   On the server side, some common socket functions include:

   - **socket()**: Create a new socket.
   - **bind()**: Bind the socket to a specific address and port.
   - **listen()**: Put the socket into listening mode, allowing it to accept incoming connections.
   - **accept()**: Accept an incoming connection, returning a new socket for communication with the client.
   - **send()**: Send data over the established connection.
   - **recv()**: Receive data from the connected client.

#### Client-Side Socket Functions

   On the client side, some common socket functions include:

   - **socket()**: Create a new socket.
   - **connect()**: Initiate a connection to a specific server.
   - **send()**: Send data to the server.
   - **recv()**: Receive data from the server.
   - **close()**: Close the socket when communication is finished.

#### Non-Blocking Sockets

   Non-blocking sockets allow programs to perform other tasks while waiting for data to arrive or be sent over the network. This is achieved by setting the socket to non-blocking mode using the **fcntl()** or **ioctl()** functions. Non-blocking sockets can be useful in scenarios where responsiveness is critical, such as handling multiple client connections concurrently without blocking the server's main thread.

### Request

An HTTP request is a message sent by a client to a server to initiate a specific action. It typically includes information such as the requested resource (URI), HTTP method (GET, POST, etc.), headers, and optionally, a message body. The server processes the request and sends back an HTTP response. The request typically includes information such as:

- **Request Method**: The HTTP method (e.g., GET, POST, PUT, DELETE) indicating the desired action.
- **Request URI**: The Uniform Resource Identifier specifying the resource being requested.
- **Request Headers**: Additional metadata sent along with the request, such as user-agent information, cookies, and content type.
- **Request Body**: Optional data sent along with the request, typically used in POST or PUT requests to submit form data or upload files.

#### Request Headers

   Request headers provide additional information about the request and the client making it. Common request headers include:

   - **User-Agent**: Identifies the client application making the request (e.g., browser type and version).
   - **Content-Type**: Specifies the format of the data in the request body (e.g., application/json, multipart/form-data).
   - **Cookie**: Contains any cookies associated with the request.
   - **Accept**: Specifies the types of content the client can accept in the response.

#### Request Body

   The request body contains data sent from the client to the server, typically in POST or PUT requests. It can include form data, file uploads, or JSON/XML payloads, depending on the content type specified in the request headers. The server processes the request body to perform the desired action, such as storing data in a database or processing a form submission.

### Response

An HTTP response is a message sent by a server to a client in response to an HTTP request. It contains information about the status of the request (e.g., success or failure), along with optional data such as response headers and a message body. The client processes the response and takes appropriate action based on the status and content.

### Methods

HTTP methods define the actions that can be performed on a resource. Common HTTP methods include:

- **GET**: Retrieve data from the server.
- **POST**: Submit data to the server to create or update a resource.
- **PUT**: Update an existing resource on the server.
- **DELETE**: Remove a resource from the server.
- **HEAD**: Retrieve metadata about a resource without fetching the actual data.
- **OPTIONS**: Retrieve information about the communication options supported by the server for a given resource.
