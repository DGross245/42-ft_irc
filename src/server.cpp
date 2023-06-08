#include "ft_irc.hpp"
#include <exception>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

IRC::IRC( void ) {
	InitServer();
	std::cout << "kek" << std::endl;
	return ;
}

IRC::~IRC( void ) {
	return ;
}

void IRC::InitServer( void ) {
	int ServerSocketfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in ServerAddress;
	memset(&ServerAddress, 0, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = INADDR_ANY;
	ServerAddress.sin_port = htons(3123);
	bind(ServerSocketfd, reinterpret_cast<struct sockaddr *>(&ServerAddress), sizeof(ServerAddress));
	listen(ServerSocketfd, 1);
	struct sockaddr_in ClientAddress;
	int ClientSocketfd;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	socklen_t ClientAddressLength = sizeof(ClientAddress);
	ClientSocketfd = accept(ServerSocketfd, reinterpret_cast<struct sockaddr *>(&ClientAddress), &ClientAddressLength);
	
   char buffer[1024];
   memset(buffer, 0, 1024);
   int bytesRead = read(ClientSocketfd, buffer, 1024 - 1);
	if (bytesRead == -1) {
       std::cerr << "Failed to read data from client" << std::endl;
    } else {
        std::cout << "Received data from client: " << buffer << std::endl;
    }
	close(ClientSocketfd);
    close(ServerSocketfd);
	return ;
}

IRC::ServerFailException::~ServerFailException( void ) throw() { return ;	}
IRC::ServerFailException::ServerFailException( std::string Error ) : _error(Error) { return ; }
const char *IRC::ServerFailException::what() const throw() { return (this->_error.c_str());}