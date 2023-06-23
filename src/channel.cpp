#include "Channel.hpp"
#include "Client.hpp"
#include <string>
#include <iostream>
#include <vector>

// SearchforUser und Leavechannel vlt zu einem Template umcoden, weil benutze es doppelt
// im sinne von sucht und returnt den index, gibt es ihn nicht dann -1

Channel::Channel( std::string Name ) : _name(Name), _isTopicRestricted(false), _isInviteOnly(false) {
	return ;
}

Channel::~Channel( void ) {
	return ;
}

void Channel::LeaveChannel( std::string UserName ) { // vlt UserName mit einer instanz umtauschen
	for (std::vector<Client>::iterator Interator = this->_clients.begin(); Interator != this->_clients.end(); Interator++ ) {
		if (Interator->getUsername() == UserName ) { // vlt anstatt username id nehmen (macht vlt auch kein unterschied)
			this->_clients.erase(Interator); 
			return ;
		}
	}
	throw ChannelFailException("Error: User not found");
	return ;
}

void Channel::AddUser( Client User ) {
	this->_clients.push_back(Client ( User ));
	return ;
}

void Channel::SetSettings( void ) {
	return ;
}

std::string Channel::getChannelName( void ) {
	return (this->_name);
}

int	Channel::SearchforUser( Client User ) {
	for (std::vector<Client>::iterator Interator = this->_invited.begin(); Interator != this->_invited.end(); Interator++ ) {
		if (Interator->getUsername() == User.getUsername() ) { // vlt anstatt username id nehmen (macht vlt auch kein unterschied)
			this->_invited.erase(Interator); 
			return ;
		}
	}
	throw ChannelFailException("Error: User not found");
	return (0);
}

bool Channel::CanUserJoin( Client User ) {
	if (this->_isInviteOnly == 1) {
		if (SearchforUser( User ))
			return (true);
	}
	else
		return (true);
	return (false);
}