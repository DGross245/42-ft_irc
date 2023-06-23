#include "Channel.hpp"
#include "Client.hpp"
#include <string>
#include <iostream>
#include <vector>

// SearchforUser und Leavechannel vlt zu einem Template umcoden, weil benutze es doppelt
// im sinne von sucht und returnt den index, gibt es ihn nicht dann -1

Channel::Channel( std::string Name, Client User ) : _name(Name), _isTopicRestricted(false), _isInviteOnly(false), _founder(User) {
	return ;
}

Channel::~Channel( void ) {
	return ;
}

void Channel::LeaveChannel( std::string UserName ) { // vlt UserName mit einer instanz umtauschen
	for (std::vector<Client>::iterator Interator = this->_clients.begin(); Interator != this->_clients.end(); Interator++ ) {
		if (Interator->getUsername() == UserName ) { // vlt anstatt username id nehmen (macht vlt auch kein unterschied)
			if (this->_founder.getSocketID() == Interator->getSocketID()) // Vllt dort socket vergleichen
				;//Promote some to founder
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
			return (1);
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

void Channel::setFounder( Client &Founder ) {
	this->_founder = Founder;
	return ;
}

std::string Channel::getTopic( void ) {
	return (this->_topic);
}
void Channel::setTopic( std::string Topic ) {
	if (this->_isTopicRestricted)
		;// hier checken ob OP oder ein Founder diesen Change ausführt
	this->_topic = Topic;
	return ;
}
Channel::ChannelFailException::~ChannelFailException( void ) throw() { return ;	}
Channel::ChannelFailException::ChannelFailException( std::string Error ) : _error(Error) { return ; }
const char *Channel::ChannelFailException::what() const throw() { return (this->_error.c_str());}