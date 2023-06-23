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
	ServerAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerAddress.sin_port = htons(this->getPort());
	fcntl(ServerSocketfd, F_SETFL, O_NONBLOCK);
	bind(ServerSocketfd, reinterpret_cast<struct sockaddr *>(&ServerAddress), sizeof(ServerAddress));
	listen(ServerSocketfd, 1);
	ClientIOHandler(ServerSocketfd);
	CloseConnection();
	close(ServerSocketfd);
	return ;
}

void Server::CloseConnection( void ) {
	for (size_t i = 0; i < this->_connections.size(); i++)
		close(this->_connections[i]);
	return ;
}

void Server::AddClient( int ServerSocketfd ) {
	struct sockaddr_in ClientAddress;
	int Clientfd;

	memset(&ClientAddress, 0, sizeof(ClientAddress));
	socklen_t ClientAddressLength = sizeof(ClientAddress);
	Clientfd = accept(ServerSocketfd, reinterpret_cast<struct sockaddr *>(&ClientAddress), &ClientAddressLength);
	if (Clientfd > 0) // vlt mit der Client klasse verbinden und eine instance pushen ansatt nur den socket
		this->_connections.push_back(Clientfd);
	return ;
}

void Server::ParseMsg( std::string Buffer ) {
	
}

void Server::ExecuteMsg( void ) {
	
	return ;
}

int Server::SearchForChannel( std::string ChannelName ) {
	for (std::vector<Channel>::iterator Interator = this->_channel.begin(); Interator != this->_channel.end(); Interator++ ) {
		if (Interator->getChannelName() == ChannelName )
			return (std::distance(this->_channel.begin(), Interator));
	}
	return (-1);
}

void Server::JoinChannel( std::string ChannelName, Client User) { // ChannelName, ClientID oder Client
	int	i = SearchForChannel( ChannelName );
	if (i < 0) {
		this->_channel.push_back(Channel ( ChannelName ));
		this->_channel.end()->SetSettings();
		this->_channel.end()->AddUser( User );
	}
	else {
		if (this->_channel[i].CanUserJoin( User ))
			this->_channel[i].AddUser( User );
		else
			; // Nachricht an Client : Invited only, can't join!
	}
	return ;
}

void Server::ReadMsg( int client, fd_set rfds, int i) {
	if (FD_ISSET(client, &rfds)) {
		char Buffer[1024];
		ssize_t bytes_read;
		bytes_read = ::recv(client, Buffer, sizeof(Buffer), 0);
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
				ParseMsg( Buffer );
				ExecuteMsg();
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}
		}
	}
	return ;
}

void Server::ClientIOHandler( int ServerSocketfd ) {
	struct timeval tv;
	tv.tv_usec = 0.0;
	tv.tv_sec = 5.0;

	int ClientSocketfd = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(ClientSocketfd, F_SETFL, O_NONBLOCK);
	
	while (1) {
		fd_set rfds;
		FD_ZERO( &rfds );
		FD_SET( ClientSocketfd, &rfds );
	
		int maxfd = ServerSocketfd;
        for (size_t i = 0; i < this->_connections.size(); i++) {
            int fd = this->_connections[i];
            FD_SET(fd, &rfds);
            if (fd > maxfd)
                maxfd = fd;
        }
		int kp = select( maxfd + 1, &rfds, NULL, NULL, &tv);

		if (kp == -1)
			throw ServerFailException("select Error");
		else if (kp == 0)
			AddClient( ServerSocketfd );
		else {
			for (size_t i = 0; i < this->_connections.size(); i++)
				ReadMsg( this->_connections[i], rfds, i);
		}
	}
	return ;
}

Server::ServerFailException::~ServerFailException( void ) throw() { return ;	}
Server::ServerFailException::ServerFailException( std::string Error ) : _error(Error) { return ; }
const char *Server::ServerFailException::what() const throw() { return (this->_error.c_str());}