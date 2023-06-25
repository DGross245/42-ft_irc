#include "client.hpp"

Client::Client( int ClientID ) : _SocketID(ClientID) {
	return ;
}

Client::~Client( void ) {
	return ;
}

void Client::setUsername( std::string Username ) {
	this->_username = Username;
	return ;
}

void Client::setNickname( std::string Nickname ) {
	this->_nickname = Nickname;
	return ;
}

std::string Client::getUsername( void ) {
	return (this->_username) ;
}

std::string Client::getNickname( void ) {
	return (this->_nickname);
}

bool Client::getAuthentication( void ) {
	return (this->IsAuthenticated);
}

void Client::setAuthentication( bool Authentication ) {
	this->IsAuthenticated = Authentication;
	return ;
}

int Client::getSocketID( void ) {
	return (this->_SocketID);
}

Client::ClientFailException::~ClientFailException( void ) throw() { return ;	}
Client::ClientFailException::ClientFailException( std::string Error ) : _error(Error) { return ; }
const char *Client::ClientFailException::what() const throw() { return (this->_error.c_str());}