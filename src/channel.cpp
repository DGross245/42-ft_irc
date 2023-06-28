#include "Channel.hpp"
#include "Client.hpp"
#include <string>
#include <iostream>
#include <vector>

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
void Channel::setTopic( std::string topic ) {
	if (this->_isTopicRestricted)
		;// hier checken ob OP oder ein Founder diesen Change ausführt
	this->_topic = topic;
	return ;
}
Channel::channelFailException::~channelFailException( void ) throw() { return ;	}
Channel::channelFailException::channelFailException( std::string error ) : _error(error) { return ; }
const char *Channel::channelFailException::what() const throw() { return (this->_error.c_str());}