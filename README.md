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

Sockets are communication endpoints used to send and receive data between two nodes on a network. In the context of our web server, sockets are utilized to establish connections with clients and handle incoming HTTP requests.

### Request

An HTTP request is a message sent by a client to a server to initiate a specific action. It typically includes information such as the requested resource (URI), HTTP method (GET, POST, etc.), headers, and optionally, a message body. The server processes the request and sends back an HTTP response.

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
