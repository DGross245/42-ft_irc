#include "Server.hpp"
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
#include <cmath>
#include <fcntl.h>

Server::Server( std::string Port, std::string Password ) {
	this->setPort( Port ); // maybe have to parse here a little
	this->setPassword( Password ); // same for this one
	InitServer();
	return ;
}

Server::~Server( void ) {
	return ;
}

int Server::getPort( void ) {
	return (this->_port);
}

int Server::getPassword( void ) {
	return (this->_password);
}

void Server::setPort( std::string &Port ) {
	this->_port = strtod(Port.c_str(), NULL);
	return ;
}

void Server::setPassword( std::string &Password ) {
	this->_password = strtod(Password.c_str(), NULL); // subject to change
	return ;
}

void Server::InitServer( void ) {
	int ServerSocketfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in ServerAddress;
	memset(&ServerAddress, 0, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = inet_addr("10.12.6.6");
	ServerAddress.sin_port = htons(this->getPort());
	fcntl(ServerSocketfd, F_SETFL, O_NONBLOCK);
	bind(ServerSocketfd, reinterpret_cast<struct sockaddr *>(&ServerAddress), sizeof(ServerAddress));
	listen(ServerSocketfd, 1);
	
	// Reading part, just a basic test
	ClientIOHandler(ServerSocketfd);

	//CloseConnection();
	close(ServerSocketfd);
	return ;
}

void Server::CloseConnection( void ) {
	// connections are not init.
	std::vector<int>::iterator it;

	for(it = this->connections.begin(); *it != 0; it++)
		
	return ;
}

void Server::ClientIOHandler( int ServerSocketfd ) {
	// timeout time
	struct timeval tv;
	tv.tv_usec = 0.0;
	tv.tv_sec = 5.0;

	struct sockaddr_in ClientAddress;
	int ClientSocketfd;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	socklen_t ClientAddressLength = sizeof(ClientAddress);
	ClientSocketfd = accept(ServerSocketfd, reinterpret_cast<struct sockaddr *>(&ClientAddress), &ClientAddressLength);
	this->connections.push_back(ClientSocketfd);
	
	while (1) {
		fd_set rfds;
		FD_ZERO( &rfds );
		FD_SET( ClientSocketfd, &rfds );
		int kp = select( ClientSocketfd + 1, &rfds, NULL, NULL, &tv);

		switch(kp) {
			case -1:
				throw ServerFailException("select Error");
			case 0: {
				struct sockaddr_in ClientAddress;
				int ClientSocketfd;
				memset(&ClientAddress, 0, sizeof(ClientAddress));
				socklen_t ClientAddressLength = sizeof(ClientAddress);
				ClientSocketfd = accept(ServerSocketfd, reinterpret_cast<struct sockaddr *>(&ClientAddress), &ClientAddressLength);
				fcntl(ClientSocketfd, F_SETFL, O_NONBLOCK);
				this->connections.push_back(ClientSocketfd);
			}
			default: {
				if (FD_ISSET(ClientSocketfd, &rfds)) {
					char Buffer[1024];
					ssize_t bytes_read;
					bytes_read = ::recv(ClientSocketfd, Buffer, sizeof(Buffer), 0);
					if (bytes_read == -1)
						throw ServerFailException("recv Error");
					else if (bytes_read == 0) {
						// disconnected
						std::cout << "Disconnected" << std::endl;
						//delete user
						break ;
					}
					else {
						// read
						std::cout << "Client: " << std::string(Buffer, bytes_read);
					}
				}
			}
		}
	}
	return ;
}

Server::ServerFailException::~ServerFailException( void ) throw() { return ;	}
Server::ServerFailException::ServerFailException( std::string Error ) : _error(Error) { return ; }
const char *Server::ServerFailException::what() const throw() { return (this->_error.c_str());}