#include <ctime>
#include <iostream>
#include <string>
#include "server.h"

int main()
{

	std::string ip = "127.0.0.1";
	std::uint16_t port = 3302;
	try
	{
		spdlog::info("starting the udp server");

		server server(ip, port);
		spdlog::info("initiating server on '{}:{}'", ip, port);

		server.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}