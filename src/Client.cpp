#include "Client.hpp"
#include <sys/socket.h>
#include "Constants.hpp"
#include <iostream>
#include <unistd.h>

Client::Client( int clientfd ) : _socketfd(clientfd), _isAuthenticated(false), _passwordAccepted(false) {
	return ;
}

Client::~Client( void ) {
	return ;
}

//SETTER FUNCTIONS

void Client::setUsername( std::string username ) {
	this->_username = username;
	return ;
}

void Client::setNickname( std::string nickname ) {
	this->_nickname = nickname;
	return ;
}

void Client::setAuthentication( bool authentication ) {
	this->_isAuthenticated = authentication;
	return ;
}

void Client::setPasswordAccepted( bool status ) {
	this->_passwordAccepted = status;
	return ;
}

//GETTER FUNCTIONS

bool Client::getPasswordAccepted( void ) {
	return (this->_passwordAccepted);
}

std::string Client::getUsername( void ) {
	return (this->_username) ;
}

std::string Client::getConstUsername(void) const {
	return (this->_username);
}

std::string Client::getNickname( void ) const {
	return (this->_nickname);
}

bool Client::getAuthentication( void ) {
	return (this->_isAuthenticated);
}


int Client::getSocketfd( void ) {
	return (this->_socketfd);
}

int Client::Authentication( std::string CMD ) {
	if (this->getAuthentication() == false) {
		if (this->getPasswordAccepted() && !this->getNickname().empty() && !this->getUsername().empty()) {
			this->setAuthentication(true);
			std::string joinMessageClient = ":IRCSERVE 001 " + this->getNickname() +
			" :Welcome to the Internet Relay Network, " + this->getNickname() + "\r\n";
			send(this->getSocketfd(), joinMessageClient.c_str(), joinMessageClient.length(), 0);
			std::cout << GREEN << "Client " << this->getNickname() << " has joined the server" << RESET << std::endl;
			return (true);
		}
		if (CMD != "PASS" && CMD != "NICK" && CMD != "USER" && CMD != "CAP") {
			std::cout << CMD << "<>" << CMD.length() << std::endl;
			std::string message = SERVER " " ERR_NOTREGISTERED " * :You have not registered\r\n";
			send(this->getSocketfd(), message.c_str(), message.length(), 0);
		}
		return (false);
	}
	return (true);
}

Client::clientFailException::~clientFailException( void ) throw() { return ;	}
Client::clientFailException::clientFailException( std::string error ) : _error(error) { return ; }
const char *Client::clientFailException::what() const throw() { return (this->_error.c_str());}
