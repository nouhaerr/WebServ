#include "../Macros.hpp"
#include "parsing/Config.hpp"

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

	} catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}
	return 0;
}