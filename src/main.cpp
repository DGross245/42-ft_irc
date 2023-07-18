#include "Server.hpp"
#include "Constants.hpp"

#include <iostream>

int main (int argc, char **argv) {
	if (argc != 3)	{
		std::cerr << RED "Error: wrong input: ./ircserv <port> <password>" RESET << std::endl;
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
			std::cerr << RED << e.what() << '\n';
			return (1);
		}
	}
	return (0);
}
