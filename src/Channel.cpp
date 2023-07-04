#include "Channel.hpp"
#include "Client.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <sys/socket.h>

// SearchforUser und Leavechannel vlt zu einem Template umcoden, weil benutze es doppelt
// im sinne von sucht und returnt den index, gibt es ihn nicht dann -1

Channel::Channel( std::string name, Client user ) : _name(name), _isTopicRestricted(false), _isInviteOnly(false), _founder(user) {
	return ;
}

Channel::~Channel( void ) {
	return ;
}

void Channel::leaveChannel( std::string username ) { // vlt username mit einer instanz umtauschen
	for (std::vector<Client>::iterator iterator = this->_clients.begin(); iterator != this->_clients.end(); iterator++ ) {
		if (iterator->getUsername() == username ) { // vlt anstatt username id nehmen (macht vlt auch kein unterschied)
			if (this->_founder.getSocketfd() == iterator->getSocketfd()) // Vllt dort socket vergleichen
				;//Promote some to founder
			this->_clients.erase(iterator); 
			// invited liste auch löschen
			return ;
		}
	}
	throw channelFailException("Error: User not found");
	return ;
}

void Channel::addUser( Client user ) {
	this->_clients.push_back(Client ( user ));
	return ;
}

void Channel::setSettings( void ) {
	return ;
}

std::string Channel::getChannelName( void ) {
	return (this->_name);
}

std::vector<Client> &Channel::getClients( void ) {
	return (this->_clients);
}

std::vector<Client> &Channel::getInviteList( void ) {
	return (this->_invited);
}

int	Channel::searchforUser( Client user ) {
	for (std::vector<Client>::iterator iterator = this->_invited.begin(); iterator != this->_invited.end(); iterator++ ) {
		if (iterator->getUsername() == user.getUsername() ) { // vlt anstatt username id nehmen (macht vlt auch kein unterschied)
			this->_invited.erase(iterator); 
			return (1);
		}
	}
	throw channelFailException("Error: User not found");
	return (0);
}

bool Channel::canUserJoin( Client user ) {
	if (this->_isInviteOnly == 1) {
		if (searchforUser( user ))
			return (true);
	}
	else
		return (true);
	return (false);
}

void Channel::setFounder( Client &founder ) {
	this->_founder = founder;
	return ;
}

std::string Channel::getTopic( void ) {
	return (this->_topic);
}
void Channel::setTopic( std::string topic, Client client ) {
	std::string message;
	if (this->_isTopicRestricted) {
		if (this->_founder.getSocketfd() == client.getSocketfd())
			this->_topic = topic;
		else {
			for (std::vector<Client>::iterator it = this->_op.begin(); it != this->_op.end(); it++ ) {
				if (it->getSocketfd() == client.getSocketfd() ) {
					this->_topic = topic;
					message = ":IRCSERV 332" + client.getNickname() + this->getChannelName() + ":" + this->_topic;
					send(client.getSocketfd(), message.c_str(), message.length(), 0);
					return ;
				}
			}
			message = ":IRCSERV 482" + client.getNickname() + this->getChannelName() + ":You're not a channel operator";
			send(client.getSocketfd(), message.c_str(), message.length(), 0);
		}
	}
	else {
		this->_topic = topic;
		message = ":IRCSERV 332" + client.getNickname() + this->getChannelName() + ":" + this->_topic;
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
	}
	return ;
}
Channel::channelFailException::~channelFailException( void ) throw() { return ;	}
Channel::channelFailException::channelFailException( std::string error ) : _error(error) { return ; }
const char *Channel::channelFailException::what() const throw() { return (this->_error.c_str());}