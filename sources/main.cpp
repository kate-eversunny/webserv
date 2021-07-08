
# include "Webserv.hpp"
# include "Cluster.hpp"
//# include "Server.hpp"
# include "Config.hpp"

int main(int argc,  char **argv)
{
	Cluster cluster;
	Config config;
	std::string configFile = "./config/multiServer.conf";

	try 
	{
		if ( argc == 2)
			configFile = argv[1];
		else if (argc > 2)
		{
			std::cout << "Incorrect argc"<<std::endl;
			return -1;
		}
		config.getFile(configFile);
        cluster.init(config);
		cluster.run();
	}
    catch (std::exception const & e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}