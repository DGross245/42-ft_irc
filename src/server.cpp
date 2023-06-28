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

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define ORANGE  "\033[38;2;255;165;0m"

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

void Server::setServerID( int ServerSocketID ) {
	this->_serverID = ServerSocketID;
	return ;
}

void Server::InitServer( void ) {
	int ServerSocketfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in ServerAddress;
	memset(&ServerAddress, 0, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // später IP noch ändern
	ServerAddress.sin_port = htons(this->getPort());
	fcntl(ServerSocketfd, F_SETFL, O_NONBLOCK);
	bind(ServerSocketfd, reinterpret_cast<struct sockaddr *>(&ServerAddress), sizeof(ServerAddress));
	listen(ServerSocketfd, 1);
	this->setServerID( ServerSocketfd );
	return ;
}

void Server::CloseALLConnections( void ) {
	for (size_t i = 0; i < this->_connections.size(); i++)
		close(this->_connections[i].getSocketID());
	return ;
}

void Server::AddClient( int ServerSocketfd, fd_set &readfds ) {
	struct sockaddr_in ClientAddress;
	int Clientfd;

	memset(&ClientAddress, 0, sizeof(ClientAddress));
	socklen_t ClientAddressLength = sizeof(ClientAddress);
	Clientfd = accept(ServerSocketfd, reinterpret_cast<struct sockaddr *>(&ClientAddress), &ClientAddressLength);
	FD_SET(Clientfd, &readfds);
	if (Clientfd > 0) {
		std::cout << "New Client tries to connect" << std::endl;
		this->_connections.push_back(Client (Clientfd));
		fcntl(Clientfd, F_SETFL, O_NONBLOCK);
	}
	return ;
}

// all the data from the server with the connected clients etc is the sever class (there is a vectore which stores all of that informations)
// @todo add the server class to this function cause all the informations are in that class
// @todo create a own class for the commands
void Server::ExecuteMsg( Parser &Input, int client ) {
	if (Input.getCMD() == "CAP") {
		std::vector<std::string> params = Input.getParam();
		for (std::vector<std::string>::iterator it = params.begin(); it != params.end(); it++) {
			if (*it == "END") {
				std::string message = "CAP * ACK :JOIN\r\n";
				std::cout << "\n-------------------------------------\n" << "sended message:" << RED << message << RESET << std::endl;
				send(client, message.c_str(), message.length(), 0);
			}
			else if (*it == "LS") {
				std::string message = "CAP * LS :JOIN\r\n";
				std::cout << "\n-------------------------------------\n" << "sended message:" << RED << message << RESET << std::endl;
				send(client, message.c_str(), message.length(), 0);
			}
		}
	}
	else if (Input.getCMD() == "NICK") {
		std::string message = ":IRCSERV 001 jschneid :Willkommen in der IRC-Welt, jschneid!\r\n";
		std::cout << "\n-------------------------------------\n" << "sended message:" << RED << message << RESET << std::endl;
		send(client, message.c_str(), message.length(), 0);
	}
	else if (Input.getCMD() == "USER") {
		std::string message = ":IRCSERV 001 jschneid :Benutzerinformationen erfolgreich empfangen.\r\n";
		std::cout << "\n-------------------------------------\n" << "sended message:" << RED << message << RESET << std::endl;
		send(client, message.c_str(), message.length(), 0);
	}
	else if (Input.getCMD() == "PING") {
		std::string message = "PONG :127.0.0.1";
		std::cout << "\n-------------------------------------\n" << "sended message:" << RED << message << RESET << std::endl;
		send(client, message.c_str(), message.length(), 0);
	}
	else if (Input.getCMD() == "JOIN") {
		std::cout << "Join command gets executed" << std::endl;
		JoinChannel(Input.getParam()[0], client);
		std::string message = ":irc JOIN #test";
		std::cout << "\n-------------------------------------\n" << "sended message:" << RED << message << RESET << std::endl;
		send(client, message.c_str(), message.length(), 0);
	}
	return ;
}

int Server::SearchForChannel( std::string ChannelName ) {
	for (std::vector<Channel>::iterator Interator = this->_channel.begin(); Interator != this->_channel.end(); Interator++ ) {
		if (Interator->getChannelName() == ChannelName )
			return (std::distance(this->_channel.begin(), Interator));
	}
	return (-1);
}

void Server::JoinChannel( std::string ChannelName, Client User) {
	int	i = SearchForChannel( ChannelName );
	if (i < 0) {
		// creating the channel if the channel does not exist
		this->_channel.push_back(Channel ( ChannelName, User ));
		this->_channel.end()->SetSettings();
		this->_channel.end()->AddUser( User );
	}
	else {
		if (this->_channel[i].CanUserJoin( User )) {
			this->_channel[i].AddUser( User );
			// delete user from invite list
		}
		else
			; // Nachricht an Client : Invited only, can't join!
	}
	return ;
}

void Server::ReadMsg( int client, int i) {
	char Buffer[1024];
	ssize_t bytes_read;
	bytes_read = ::recv(client, Buffer, sizeof(Buffer), 0);
	std::cout << "\n-------------------------------------" << std::endl;
	std::cout <<"Recived msg: \n	" << ORANGE << Buffer << RESET << std::cout;
	if (bytes_read == -1)
		throw ServerFailException("recv Error"); // nochmal nachlesen vllt hier was anderes machen
	else if (bytes_read == 0) {
		std::cout << "Disconnected" << std::endl;
		close(client);
		this->_connections.erase(this->_connections.begin() + i);
	}
	else {
		try
		{
			Parser Input( Buffer );
			ExecuteMsg( Input, client );
		}
		catch(const std::exception& e)
		{
				std::cerr << e.what() << '\n';
		}
	}
	return ;
}

void Server::launchServer( void ) {
	ClientIOHandler();
	CloseALLConnections();
	close(this->_serverID);
	return ;
}

void Server::setTime( void ) {
	this->_tv.tv_usec = 0.0;
	this->_tv.tv_sec = 5.0;
	return ;
}

void Server::ClientIOHandler( void ) {
	this->setTime();

	while (1) {
		fd_set readfds;
		FD_ZERO( &readfds );
		FD_SET( this->_serverID, &readfds );
		int maxfd = getmaxfd( readfds );

		int ready_fds = select( maxfd + 1, &readfds, NULL, NULL , &this->_tv);
		if (ready_fds == -1)
			throw ServerFailException("select Error");
		else if (ready_fds == 0)
			continue;
		else {
			if (FD_ISSET(this->_serverID, &readfds))
				AddClient( this->_serverID, readfds );
			else {
				for (size_t i = 0; i < this->_connections.size(); i++) {
					if (FD_ISSET(this->_connections[i].getSocketID(), &readfds))
						ReadMsg( this->_connections[i].getSocketID(), i);
				}
			}
		}
	}
	return ;
}

int Server::getmaxfd( fd_set &readfds ) {
	int max = this->_serverID;
	for (size_t i = 0; i < this->_connections.size(); i++) {
		int clientfd = this->_connections[i].getSocketID();
		FD_SET(clientfd, &readfds);
		if (clientfd > max)
			max = clientfd;
	}
	return (max);
}

Server::ServerFailException::~ServerFailException( void ) throw() { return ;	}
Server::ServerFailException::ServerFailException( std::string Error ) : _error(Error) { return ; }
const char *Server::ServerFailException::what() const throw() { return (this->_error.c_str());}
