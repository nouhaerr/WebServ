#include "WebServer.hpp"
#include "../src/parsing/Config.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include "HttpRequest.hpp"
// #include "HttpResponse.hpp"
// #include "DELETE.cpp" 

// bool isValidFilePath(const std::string& path) 
// {
//     std::ifstream file(path.c_str());
//     return file.good();
// }

// void processRequest(const HttpRequest& request, HttpResponse& response) {
//     if (request.method == "DELETE") 
//     {
//         handleDeleteRequest(request, response);
//     } 
//     else 
//     {
//         // Traiter les autres types de requêtes
//         response.setStatusCode(501); // Not Implemented
//         response.setStatusMessage("Not Implemented");
//         response.setBody("");
//         response.send();
//     }
// }

// int main(int argc, char* argv[]) 
// {
//     if (argc < 2) 
//     {
//         std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
//         return 1;
//     }

//     std::string configFilePath = argv[1];
//     if (!isValidFilePath(configFilePath)) 
//     {
//         std::cerr << "Configuration file does not exist or cannot be read." << std::endl;
//         return 1;
//     }

//     try {
//         Config config(configFilePath.c_str());
//         config.parse();

//         WebServer server(config);
//         server.run();
//     } 
//     catch (const std::exception& e) {
//         std::cerr << "Failed to start server: " << e.what() << std::endl;
//         return EXIT_FAILURE;
//     }

//     return EXIT_SUCCESS;
// }
