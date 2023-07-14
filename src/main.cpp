#include <iostream>
#include "Server.hpp"
#include <iostream>
#include "Constants.hpp"

int main (int argc, char **argv) {
	if (argc != 3)	{
		std::cerr << "Error: wrong input: ./ircserv <port> <password>" << std::endl;
		return (1);
	}
	else {
		try
		{
			Server server( argv[1], argv[2]);
			server.launchServer();
		}
		catch(Server::serverFailException & e)
		{
			std::cerr << e.what() << '\n';
			std::cout << RED << "Server failed to launch" << RESET << std::endl;
			return (1);
		}
	}
	return (0);
}
