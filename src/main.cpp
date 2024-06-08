#include "Macros.hpp"
#include "parsing/Config.hpp"
#include "networking/WebServer.hpp"

int main(int argc, char* argv[]) 
{
	const char*	conFile;
	if (argc == 2)
		conFile = argv[1];
	else{	
		conFile = DEFAULT_CONFIG;
	}

    try {
		Config config(conFile);
		config.parse();

        WebServer server(config);
        server.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}