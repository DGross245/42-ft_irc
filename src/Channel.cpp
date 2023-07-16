#include "Channel.hpp"
#include "Client.hpp"
#include "Constants.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <sys/socket.h>

Channel::Channel( std::string name, Client user ) : _limit(0), _owner(user), _name(name) {
	_mode['t'] = false;
	_mode['k'] = false;
	_mode['l'] = false;
	_mode['m'] = true;
	_clients.push_back(user);
	_operator.push_back(user);
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

void Channel::deleteOwner( int invalidfd ) {
	this->_owner = Client (invalidfd);
	return ;
}

void Channel::setMode( std::map<char,bool> mode )  {
	this->_mode = mode;
	return ;
}

// @todo nochmal checken ob auch alle den change mitbekommen
void Channel::setTopic( std::string topic, Client client ) {
	if (this->getMode()['t'] == true) {
		for (std::vector<Client>::iterator it = this->getOperator().begin(); it != this->getOperator().end(); it++ ) {
			if (it->getSocketfd() == client.getSocketfd() ) {
				this->_topic = topic;
				return (client.sendMsg(":" + client.getNickname() + " TOPIC " + this->getChannelName() + " :" + this->getTopic() + "\r\n"));
			}
		}
		client.sendMsg(SERVER " " ERR_CHANOPRIVSNEEDED " " + client.getNickname() + " " + this->getChannelName() + ":You're not a channel operator\r\n");
	}
	else {
		this->_topic = topic;
		client.sendMsg(":" + client.getNickname() + " TOPIC " + this->getChannelName() + " :" + this->getTopic() + "\r\n");
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

std::vector<Client> &Channel::getOperator( void ) {
	return (this->_operator);
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

std::string Channel::getModeString(void) {
	std::map<char,bool> map = this->getMode();
	this->_modeString = "";
	for (std::map<char,bool>::iterator it = map.begin(); it != map.end(); it++) {
		if (it->second == true)
			this->_modeString += it->first;
	}
	return (this->_modeString);
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
		std::vector<Client>::iterator inviteIt = searchForUser( client.getNickname(), this->getInviteList());
		if (inviteIt == this->getInviteList().end()) {
			client.sendMsg(SERVER " " ERR_INVITEONLYCHAN " " + client.getNickname() + " " + input.getParam()[0] + " :is Invite only restricted\r\n");
			return (false);
		}
	}
	if (this->getMode()['k'] == true) {
		if (input.getParam().size() != 2) {
			client.sendMsg(SERVER " " ERR_BADCHANNELKEY " " + client.getNickname() + " " + input.getParam()[0] + " :Missing or wrong channel key\r\n");
			return (false);
		}
		else if (this->getPassword() != input.getParam()[1]) {
			client.sendMsg(SERVER " " ERR_BADCHANNELKEY " " + client.getNickname() + " " + input.getParam()[0] + " :Missing or wrong channel key\r\n");
			return (false);
		}
	}
	if (this->getMode()['l'] == true) {
		if (this->getClients().size() >= this->getLimit()) {
			client.sendMsg(SERVER " " ERR_CHANNELISFULL " " + client.getNickname() + " " + input.getParam()[0] + " :Limit reached, channel is to full\r\n");
			return (false);
		}
	}
	std::vector<Client>::iterator clientIt = this->searchForUser(client.getNickname(), this->getClients());
	if (clientIt != this->getClients().end())
		return (false);
	return (true);
}
