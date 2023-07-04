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
#include <algorithm>
#include "Channel.hpp"
#include "Parser.hpp"
#include <csignal>
#include <vector>
#include <sstream>
#include <Client.hpp>
#include "Constants.hpp"
#include "Commands.hpp"

Server::Server( std::string port, std::string password ) {
	if (port.find_first_not_of("0123456789") == std::string::npos) {
		int _port = static_cast<int>( strtod(port.c_str(), nullptr) );
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

std::vector<Channel>	&Server::getChannels(void) {
	return (this->_channel);
}

std::vector<Client>		&Server::getClients(void) {
	return (this->_connections);
}

void Server::setPort( int port ) {
	this->_port = port;
	return ;
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
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // später IP noch ändern
	serverAddress.sin_port = htons(this->getPort());
	fcntl(serverSocketfd, F_SETFL, O_NONBLOCK);
	bind(serverSocketfd, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress));
	listen(serverSocketfd, 1);
	this->setServerfd( serverSocketfd );
	return ;
}

void Server::closeALLConnections( void ) {
	for (size_t i = 0; i < this->_connections.size(); i++)
		close(this->_connections[i].getSocketfd());
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
		std::cout << "New Client tries to connect" << std::endl;
		this->_connections.push_back(Client (clientfd));
		fcntl(clientfd, F_SETFL, O_NONBLOCK);
	}
	return ;
}

// all the data from the server with the connected clients etc is the sever class (there is a vectore which stores all of that informations)
void Server::executeMsg( Parser &input, Client &client ) {
	Commands	command;

	if (input.getCMD() == "CAP") {
		std::vector<std::string> params = input.getParam();
		for (std::vector<std::string>::iterator it = params.begin(); it != params.end(); it++) {
			if (*it == "END") {
				std::string message = "CAP * ACK :JOIN\r\n";
				send(client.getSocketfd(), message.c_str(), message.length(), 0);
			}
			else if (*it == "LS") {
				std::string message = "CAP * LS :JOIN\r\n";
				send(client.getSocketfd(), message.c_str(), message.length(), 0);
			}
		}
	}
	else if (input.getCMD() == "NICK") {
		command.nick(input, client, this->getClients());
	}
	else if (input.getCMD() == "USER") {
		std::string message = ":IRCSERV 001 dgross :Benutzerinformationen erfolgreich empfangen.\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
	}
	else if (input.getCMD() == "PING") {
		std::string message = "PONG :127.0.0.1";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
	}
	else if (input.getCMD() == "JOIN") {
		command.join(input, client, this->getChannels());
        // joinChannel(input.getParam()[0], client);
        // std::string joinMessageClient = ":dgross JOIN #test\r\n";
        // std::string switchBuffer = "/buffer #test\r\n";
        // // std::string message = ":dgross PRIVMSG #test :Hello, everyone!\r\n";
        // send(client.getSocketfd(), joinMessageClient.c_str(), joinMessageClient.length(), 0);
        // send(client.getSocketfd(), switchBuffer.c_str(), switchBuffer.length(), 0);
        // send(client, message.c_str(), message.length(), 0);
    }
	else if (input.getCMD() == "PASS") {
		command.pass(input, client , this->getPassword());
	}
	else if (input.getCMD() == "TOPIC") {\
		command.topic(client);
	}
	return ;
}

// !!!!! This functin is moved to commands class !!!!!!!!
// int Server::searchForChannel( std::string channelName ) {
// 	for (std::vector<Channel>::iterator iterator = this->_channel.begin(); iterator != this->_channel.end(); iterator++ ) {
// 		if (iterator->getChannelName() == channelName )
// 			return (std::distance(this->_channel.begin(), iterator));
// 	}
// 	return (-1);
// }

// !!!!! This functin is moved to commands class !!!!!!!!
// void Server::joinChannel( std::string channelName, Client user) {
// 	int	i = searchForChannel( channelName );
// 	if (i < 0) {
// 		// creating the channel if the channel does not exist
// 		this->_channel.push_back(Channel ( channelName, user ));
// 		this->_channel.end()->setSettings();
// 		this->_channel.end()->addUser( user );
// 	}
// 	else {
// 		if (this->_channel[i].canUserJoin( user )) {
// 			this->_channel[i].addUser( user );
// 			// delete user from invite list
// 		}
// 		else
// 			; // Nachricht an Client : Invited only, can't join!
// 	}
// 	return ;
// }

static void Sprinter( Parser parser) {
	// std::cout << "Prefix:" << parser.getPrefix() << std::endl;
	// std::cout << "Command:" << parser.getCMD() << std::endl;
	std::vector<std::string> test = parser.getParam();
	for (std::vector<std::string>::iterator it = test.begin(); it != test.end(); it++)
		// std::cout << "Param :" << *it << std::endl;
	// std::cout << "trailing :" << parser.getTrailing() << std::endl;
	return ;
}

void Server::readMsg( Client &client, int i) {
	char buffer[1024];
	ssize_t bytes_read;
	bytes_read = ::recv(client.getSocketfd(), buffer, sizeof(buffer), 0);
	if (bytes_read == -1)
		throw serverFailException("recv Error");
	else if (bytes_read == 0) {
		std::cout << "Disconnected" << std::endl;
		close(client.getSocketfd());
		this->_connections.erase(this->_connections.begin() + i);
	}
	else {
		try
		{
			Parser input( buffer, client );
			Sprinter( input );
			executeMsg( input, client );
		}
		catch(const Parser::parserErrorException &e)
		{
				std::cerr << e.what() << '\n';
		}
	}
	return ;
}

void Server::launchServer( void ) {
	clientIOHandler();
	closeALLConnections();
	close(this->_serverfd);
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
		FD_SET( this->_serverfd, &readfds );
		int maxfd = getMaxfd( readfds );

		int ready_fds = select( maxfd + 1, &readfds, NULL, NULL , &this->_tv);
		if (ready_fds == -1)
			throw serverFailException("select Error");
		else if (ready_fds == 0)
			continue;
		else {
			if (FD_ISSET(this->_serverfd, &readfds))
				addClient( this->_serverfd, readfds );
			else {
				for (size_t i = 0; i < this->_connections.size(); i++) {
					if (FD_ISSET(this->_connections[i].getSocketfd(), &readfds))
						readMsg( this->_connections[i], i);
				}
			}
		}
	}
	return ;
}

int Server::getMaxfd( fd_set &readfds ) {
	int max = this->_serverfd;
	for (size_t i = 0; i < this->_connections.size(); i++) {
		int clientfd = this->_connections[i].getSocketfd();
		FD_SET(clientfd, &readfds);
		if (clientfd > max)
			max = clientfd;
	}
	return (max);
}

Server::serverFailException::~serverFailException( void ) throw() { return ;	}
Server::serverFailException::serverFailException( std::string error ) : _error(error) { return ; }
const char *Server::serverFailException::what() const throw() { return (this->_error.c_str());}
