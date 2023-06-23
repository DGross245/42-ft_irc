#include "Client.hpp"

Client::Client( void ) {
	return ;
}

Client::~Client( void ) {
	return ;
}

std::string Client::getUsername( void ) {
	return (this->_username) ;
}

std::string Client::getNickname( void ) {
	return (this->_nickname);
}
