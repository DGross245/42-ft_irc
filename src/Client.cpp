#include "Client.hpp"

Client::Client( int clientfd ) : _socketfd(clientfd) {
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

//GETTER FUNCTIONS

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

// @todo my also pw ??
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
