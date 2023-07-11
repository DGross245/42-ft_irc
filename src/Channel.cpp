#include "Channel.hpp"
#include "Client.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include "Constants.hpp"

Channel::Channel( std::string name, Client user ) : _name(name), _limit(0), _owner(user) {
	_mode['t'] = false;
	_mode['k'] = false;
	_mode['l'] = false;
	_clients.push_back(user);
	return ;
}

Channel::~Channel( void ) {
	return ;
}

// SETTER FUNCTIONS

void Channel::setLimit( size_t limit ) {
	this->_limit = limit;
	return ;
}

void Channel::setPassword( std::string password ) {
	this->_password = password;
	return ;
}

void Channel::setOwner( Client owner ) {
	this->_owner = owner;
	return ;
}

void Channel::setMode( std::map<char,bool> mode )  {
	this->_mode = mode;
	return ;
}

void Channel::setTopic( std::string topic, Client client ) {
	std::string message;
	if (this->getMode()['t'] == true) {
		if (this->getOwner().getSocketfd() == client.getSocketfd())
			this->_topic = topic;
		else {
			for (std::vector<Client>::iterator it = this->getOP().begin(); it != this->getOP().end(); it++ ) {
				if (it->getSocketfd() == client.getSocketfd() ) {
					this->_topic = topic;
					message = SERVER " " RPL_TOPIC " " + client.getNickname() + this->getChannelName() + ":" + this->getTopic() + "\r\n";
					send(client.getSocketfd(), message.c_str(), message.length(), 0);
					return ;
				}
			}
			message = SERVER " " ERR_CHANOPRIVSNEEDED " " + client.getNickname() + " " + this->getChannelName() + ":You're not a channel operator\r\n";
			send(client.getSocketfd(), message.c_str(), message.length(), 0);
		}
	}
	else {
		this->_topic = topic;
		message = SERVER " " RPL_TOPIC " " + client.getNickname() + " " + this->getChannelName() + ":" + this->getTopic() + "\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
	}
	return ;
}

// GETTER FUNCTIONS

std::string Channel::getChannelName( void ) {
	return (this->_name);
}

size_t Channel::getLimit( void ) {
	return (this->_limit);
}

std::vector<Client> &Channel::getClients( void ) {
	return (this->_clients);
}

std::vector<Client> &Channel::getInviteList( void ) {
	return (this->_invited);
}

std::vector<Client> &Channel::getOP( void ) {
	return (this->_op);
}

Client Channel::getOwner( void ) {
	return (this->_owner);
}

std::map<char,bool> &Channel::getMode( void ) {
	return (this->_mode);
}

std::string Channel::getPassword( void ) {
	return (this->_password);
}

std::string Channel::getTopic( void ) {
	return (this->_topic);
}

// NORMAL CODE

void Channel::addUser( Client user ) {
	this->getClients().push_back( user );
	return ;
}

std::vector<Client>::iterator Channel::searchForUser( std::string nickname, std::vector<Client> &clients ) {
	std::vector<Client>::iterator clientIt;
	for (clientIt = clients.begin(); clientIt != clients.end(); clientIt++ ) {
		if (clientIt->getNickname() == nickname ) {
			break ;
		}
	}
	return (clientIt);
}

bool Channel::canUserJoin( Client client, Parser &input ) {
	std::string message;
	if (this->getMode()['i'] == true) {
		std::vector<Client>::iterator inviteIt = this->searchForUser( client.getNickname(), this->getInviteList());
		if (inviteIt == this->getInviteList().end()) {
			message = SERVER " " ERR_INVITEONLYCHAN " " + client.getNickname() + " " + input.getParam()[0] + " :is Invite only restricted\r\n";
			send(client.getSocketfd(), message.c_str(), message.length(), 0);
			return (false);
		}
	}
	if (this->getMode()['k'] == true) {
		if (input.getParam()[1].empty() || this->getPassword() != input.getParam()[1]) {
			message = SERVER " " ERR_BADCHANNELKEY " " + client.getNickname() + " " + input.getParam()[0] + " :Missing or wrong channel key\r\n";
			send(client.getSocketfd(), message.c_str(), message.length(), 0);
			return (false);
		}
	}
	if (this->getMode()['l'] == true) {
		if (this->getClients().size() >= this->getLimit()) {
			message = SERVER " " ERR_CHANNELISFULL " " + client.getNickname() + " " + input.getParam()[0] + " :Limit reached, channel is to full\r\n";
			send(client.getSocketfd(), message.c_str(), message.length(), 0);
			return (false);
		}
	}
	return (true);
}

Channel::channelFailException::~channelFailException( void ) throw() { return ;	}
Channel::channelFailException::channelFailException( std::string error ) : _error(error) { return ; }
const char *Channel::channelFailException::what() const throw() { return (this->_error.c_str());}