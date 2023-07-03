#include "Client.hpp"

Client::Client( int clientfd ) : _socketfd(clientfd) {
	return ;
}

Client::~Client( void ) {
	return ;
}

void Client::setUsername( std::string username ) {
	this->_username = username;
	return ;
}

void Client::setNickname( std::string nickname ) {
	this->_nickname = nickname;
	return ;
}

std::string Client::getUsername( void ) {
	return (this->_username) ;
}

std::string Client::getNickname( void ) {
	return (this->_nickname);
}

bool Client::getAuthentication( void ) {
	return (this->_isAuthenticated);
}

void Client::setAuthentication( bool authentication ) {
	this->_isAuthenticated = authentication;
	return ;
}

int Client::getSocketfd( void ) {
	return (this->_socketfd);
}

int Client::Authentication( void ) {
	if (!this->getAuthentication()) {
		if (this->getNickname().empty())
			return (0);
		if (this->getUsername().empty())
			return (0);
	}
	this->setAuthentication( true );
	return (1);
}

Client::clientFailException::~clientFailException( void ) throw() { return ;	}
Client::clientFailException::clientFailException( std::string error ) : _error(error) { return ; }
const char *Client::clientFailException::what() const throw() { return (this->_error.c_str());}