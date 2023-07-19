#include "Client.hpp"
#include "Constants.hpp"

#include <sys/socket.h>
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

void Client::setAppendBuffer( std::string buffer ) {
	if (buffer.length() > MAX_BUFFER_LENGTH) {
		this->_appendBuffer = buffer.substr(0, MAX_BUFFER_LENGTH - 2);
		this->_appendBuffer += "\r\n";
	}
	else
		this->_appendBuffer = buffer;
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

std::string Client::getAppendBuffer( void ) {
	return (this->_appendBuffer);
}

// NORMAL FUNCTIONS

void Client::sendMsg( std::string message ) {
	send(this->getSocketfd(), message.c_str(), message.length(), 0);
	std::cout << BLACK "[Server]: " LIGHT_RED "==> " CYAN SERVER RESET WHITE ": " << message << RESET << std::endl;
	return ;
}

int Client::Authentication( std::string CMD ) {
	if (this->getAuthentication() == false) {
		if (this->getPasswordAccepted() && !this->getNickname().empty() && !this->getUsername().empty()) {
			this->setAuthentication(true);
			this->sendMsg(SERVER " " RPL_WELCOME " " + this->getNickname() + " :Welcome to the Internet Relay Network, " + this->getNickname() + "\r\n");
			std::cout << BLACK "[Server]: " << DARK_GRAY BOLD "Client " << this->getNickname() << " has joined the server" << RESET << "\n" << std::endl;
			return (true);
		}
		if (CMD != "PASS" && CMD != "NICK" && CMD != "USER" && CMD != "CAP")
			this->sendMsg(SERVER " " ERR_NOTREGISTERED " * :You have not registered\r\n");
		return (false);
	}
	return (true);
}
