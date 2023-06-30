# include <exception>
# include <iostream>
# include <string>
# include <vector>
# include <sys/types.h>
# include <Channel.hpp>
# include <Parser.hpp>
# include <sys/time.h>
#include "Commands.hpp"

Commands::Commands() {
	return ;
}

Commands::~Commands( void ) {
	return ;
}

void Commands::join(){
	std::cout << "join command called" << std::endl;
	// std::string joinMessageClient = ":jschneid JOIN #test\r\n";
	// std::string switchBuffer = "/buffer #test\r\n";
	// joinChannel(input.getParam()[0], client);
	// send(client.getSocketfd(), joinMessageClient.c_str(), joinMessageClient.length(), 0);
	// send(client.getSocketfd(), switchBuffer.c_str(), switchBuffer.length(), 0);
}

Commands::commandFailException::~commandFailException( void ) throw() { return ;	}
Commands::commandFailException::commandFailException( std::string error ) : _error(error) { return ; }
const char *Commands::commandFailException::what() const throw() { return (this->_error.c_str());}
