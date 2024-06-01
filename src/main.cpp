// #include "Macros.hpp"
// #include "parsing/Config.hpp"

// // int	main(int ac, char **av)
// // {
// // 	const char* conFile;
// // 	if (ac == 2)
// //         conFile = av[1];
// //     else{

// //         conFile = DEFAULT_CONFIG;
// // 	}

// // 	try {
// // 		Config config(conFile);
// // 		config.parse();

// // 	} catch (const std::exception &e)
// // 	{
// // 		std::cerr << e.what() << '\n';
// // 		return 1;
// // 	}
// // 	return 0;
// // }
#include "Macros.hpp"
#include "parsing/Config.hpp"
#include "networking/WebServer.hpp"

int	main(int ac, char **av)
{
	const char* conFile;
	if (ac == 2)
        conFile = av[1];
    else{

        conFile = DEFAULT_CONFIG;
	}

	try {
		Config config(conFile);
		config.parse();
		WebServer server(8080, "127.0.0.1");
        std::cout << "Server starting on port 8080..." << std::endl;
        
        server.run();

	} catch (const std::exception &e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
	catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
        return 1;
	}
	return 0;
}