#include "WebServer.hpp"
#include <iostream>

int main() 
{
    try {
        WebServer server(8080, "127.0.0.1");
        std::cout << "Server starting on port 8080..." << std::endl;
        
        server.run();
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    } 
    catch (...) 
    {
        std::cerr << "Unknown exception occurred." << std::endl;
        return 1;

    return 0;
}
}
