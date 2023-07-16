#include "Server.hpp"
#include "Channel.hpp"
#include "Commands.hpp"
#include "Constants.hpp"
#include "Client.hpp"
#include "Parser.hpp"

#include <exception>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <sstream>
#include <cstring>
#include <cstdlib>

Server::Server( std::string port, std::string password ) {
	if (port.find_first_not_of("0123456789") == std::string::npos) {
		int _port = static_cast<int>( strtod(port.c_str(), NULL) );
		int overflowCheck;

		std::stringstream ss(port);
		ss >> overflowCheck;
		if (ss.fail() || !ss.eof())
			throw serverFailException("Error: Invalid port. Port must be a valid port range <1024-49151>");
		if (_port < 1024 || _port > 49151)
			throw serverFailException("Error: Port out of range. Valid ports are in the range <1024-49151>");
		else
			this->setPort( _port );
	}
	else
		throw serverFailException("Error: Invalid input. Port must be a numeric value");
	if (password.length() >= 8 && password.length() <= 32) {
		if (port.find_first_of(' ') != std::string::npos)
			throw serverFailException("Error: Invalid Password. No space allowed");
		else
			this->setPassword( password );
	}
	else
		throw serverFailException("Error: Invalid Password. Password lenght should be around 8-32");
	initServer();
	return ;
}

Server::~Server( void ) {
	return ;
}

int Server::getPort( void ) {
	return (this->_port);
}

std::string Server::getPassword( void ) {
	return (this->_password);
}

std::vector<Channel> &Server::getChannels(void) {
	return (this->_channels);
}

void Server::setPort( int port ) {
	this->_port = port;
	return ;
}

std::vector<Client> &Server::getConnections(void) {
	return (this->_connections);
}

int	Server::getServerfd( void){
	return (this->_serverfd);
}

void Server::setPassword( std::string &password ) {
	this->_password = password;
	return ;
}

void Server::setServerfd( int serverSocketfd ) {
	this->_serverfd = serverSocketfd;
	return ;
}

void Server::initServer( void ) {
	int serverSocketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocketfd == -1)
		throw serverFailException("Error: socket error: Failed to create a server socket");
	int optval = 1;
	if (setsockopt(serverSocketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		close(serverSocketfd);
		throw serverFailException("Error: setsocketopt error: Failed to set the socket option for reuse");
	}
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("10.11.5.25");
	serverAddress.sin_port = htons(this->getPort());
	if (fcntl(serverSocketfd, F_SETFL, O_NONBLOCK) == -1) {
		close(serverSocketfd);
		throw serverFailException("Error: fcntl error: Failed to set the socket to non-blocking");
	}
	if (bind(serverSocketfd, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1) {
		close(serverSocketfd);
		throw serverFailException("Error: bind error: Failed to bind the socket to the specified address");
	}
	if (listen(serverSocketfd, 4) == -1) {
		close(serverSocketfd);
		throw serverFailException("Error: listen error: Failed listening on socket");
	}
	this->setServerfd( serverSocketfd );
	return ;
}

void Server::closeALLConnections( void ) {
	for (size_t i = 0; i < this->getConnections().size(); i++)
		close(this->getConnections()[i].getSocketfd());
	return ;
}

void Server::addClient( int serverSocketfd, fd_set &readfds ) {
	struct sockaddr_in clientAddress;
	int clientfd;

	memset(&clientAddress, 0, sizeof(clientAddress));
	socklen_t clientAddressLength = sizeof(clientAddress);
	clientfd = accept(serverSocketfd, reinterpret_cast<struct sockaddr *>(&clientAddress), &clientAddressLength);
	FD_SET(clientfd, &readfds);
	if (clientfd > 0) {
		this->getConnections().push_back(Client (clientfd));
		if (fcntl(clientfd, F_SETFL, O_NONBLOCK) == -1) {
			close(clientfd);
			for (std::vector<Client>::iterator it = this->getConnections().begin(); it != this->getConnections().end(); it++) {
				if (it->getSocketfd() == clientfd) {
					this->getConnections().erase(it);
					break ;
				}
			}
		}
	}
	return ;
}

void Server::executeMsg( Parser &input, Client &client ) {
	if (input.getCMD() == "PASS")
		Commands::pass(input, client , this->getPassword());
	else if (client.getPasswordAccepted()) {
		if (input.getCMD() == "CAP")
			Commands::cap(input, client);
		else if (input.getCMD() == "NICK")
			Commands::nick(input, client, this->getConnections());
		else if (input.getCMD() == "USER")
			Commands::user(input, client, this->getConnections());
		if (client.Authentication(input.getCMD())) {
			if (input.getCMD() == "PING")
				Commands::ping(input, client);
			else if (input.getCMD() == "JOIN")
				Commands::join(input, client, this->getChannels());
			else if (input.getCMD() == "QUIT")
				Commands::quit(input, client, this->getChannels(), this->getConnections());
			else if (input.getCMD() == "PRIVMSG")
				Commands::privmsg(input, client, this->getConnections(), this->getChannels());
			else if (input.getCMD() == "KICK")
				Commands::kick(input, client, this->getChannels());
			else if (input.getCMD() == "MODE")
				Commands::mode(input, client, this->getChannels());
			else if (input.getCMD() == "PART")
				Commands::part(input, client, this->getChannels());
			else if (input.getCMD() == "TOPIC")
				Commands::topic(input, client, this->getChannels());
			else if (input.getCMD() == "INVITE") {
				Commands::invite(client, input, this->getConnections(), this->getChannels());
			}
		}
	}
	return ;
}

void Server::readMsg( Client &client, int i) {
	std::string buffer(1024, '\0');
	ssize_t bytes_read;
	bytes_read = ::recv(client.getSocketfd(), &buffer[0], buffer.size(), 0);
	if (bytes_read == -1)
		throw serverFailException("recv Error");
	else if (bytes_read == 0) {
		std::cout << GREEN << "Client has disconnected from server" << std::endl;
		close(client.getSocketfd());
		this->getConnections().erase(this->_connections.begin() + i);
	}
	else {
		try
		{
			buffer.resize(bytes_read);
			while (!buffer.empty() || buffer.find("\r\n") != std::string::npos) {
				Parser input( buffer, client );
				executeMsg( input, client );
			}
		}
		catch(const Parser::parserErrorException &e)
		{
			std::cerr << e.what() << '\n';
		}
	}
	return ;
}

void Server::launchServer( void ) {
	std::cout << GREEN << "IRCSERV has launched" << RESET << std::endl;
	clientIOHandler();
	closeALLConnections();
	close(this->getServerfd());
	return ;
}

void Server::setTime( void ) {
	this->_tv.tv_usec = 0.0;
	this->_tv.tv_sec = 5.0;
	return ;
}

void Server::clientIOHandler( void ) {
	this->setTime();

	while (1) {
		fd_set readfds;
		FD_ZERO( &readfds );
		FD_SET( this->getServerfd(), &readfds );
		int maxfd = getMaxfd( readfds );

		int ready_fds = select( maxfd + 1, &readfds, NULL, NULL , &this->_tv);
		if (ready_fds == -1)
			throw serverFailException("select Error");
		else if (ready_fds == 0)
			continue;
		else {
			if (FD_ISSET( this->getServerfd(), &readfds )) {
				addClient( this->getServerfd(), readfds );
				std::cout << GREEN << "A new client has connected. Total connected clients: " << this->getConnections().size() << std::endl;
			}
			else {
				for (size_t i = 0; i < this->getConnections().size(); i++) {
					if (FD_ISSET( this->getConnections()[i].getSocketfd(), &readfds ))
						readMsg( this->getConnections()[i], i );
				}
			}
		}
	}
	return ;
}

int Server::getMaxfd( fd_set &readfds ) {
	int max = this->getServerfd();
	for (size_t i = 0; i < this->getConnections().size(); i++) {
		int clientfd = this->getConnections()[i].getSocketfd();
		FD_SET(clientfd, &readfds);
		if (clientfd > max)
			max = clientfd;
	}
	return (max);
}

Server::serverFailException::~serverFailException( void ) throw() { return ;	}
Server::serverFailException::serverFailException( std::string error ) : _error(error) { return ; }
const char *Server::serverFailException::what() const throw() { return (this->_error.c_str());}
