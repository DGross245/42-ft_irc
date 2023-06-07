#include <iostream>

int main (int argc, char **argv) {
	if (argc != 3)	{
		std::cerr << "Error: wrong input: ./ircserv <port> <password>" << std::endl;
		return (1);
	}
	else
		try
		{
			// check IP
			// check password
			// init server
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			return (1);
		}
	(void)argv;
	return (0);
}