#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <Channel.hpp>
#include <Parser.hpp>
#include <sys/time.h>
#include "Commands.hpp"
#include <sys/socket.h>
#include "Constants.hpp"
#include <unistd.h>

Commands::Commands() {
	return ;
}

Commands::~Commands( void ) {
	return ;
}

static int	searchForChannel( std::string channelName, std::vector<Channel> channels ) {
	for (std::vector<Channel>::iterator iterator = channels.begin(); iterator != channels.end(); iterator++ ) {
		if (iterator->getChannelName() == channelName )
			return (std::distance(channels.begin(), iterator));
	}
	return (-1);
}

void joinChannel( std::string channelName, Client user, std::vector<Channel> channels) {
	int	i = searchForChannel( channelName, channels);
	if (i < 0) {
		// creating the channels if the channels does not exist
		channels.push_back(Channel ( channelName, user ));
		channels.end()->setSettings();
		channels.end()->addUser( user );
	}
	else {
		if (channels[i].canUserJoin( user )) {
			channels[i].addUser( user );
			// delete user from invite list
		}
		else
			; // Nachricht an Client : Invited only, can't join!
	}
	return ;
}

void Commands::pass( Parser &input, Client client, std::string password ) {
	if (*input.getParam().begin() == password) {
		std::cout << "PW accepted!" << std::endl;
		return ;
	}
	else {
		std::string message = SERVER " " ERR_PASSWDMISMATCH " dgross :Wrong Password\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
	}
	return ;
}

void Commands::privmsg( Parser &input, Client client, std::vector<Client> connections) {
	std::string receiver = input.getParam()[0];
	std::string message;

	if (receiver.at(0) == '#') {
		//send to channel
		// check for invite only
		// send msg to Channel
		message = ERR_NOSUCHCHANNEL " " + receiver + " :No such channel\r\n";
	}
	else {
		for (std::vector<Client>::iterator it = connections.begin(); it != connections.end(); it++) {
			if (it->getNickname() == receiver) {
				std::cout << "found" << std::endl;
				message = input.getTrailing() + "\r\n";
				send(it->getSocketfd(), message.c_str(), message.length(), 0);
				return ;
			}
		}
		message = ERR_NOSUCHNICK " " + receiver + " :No such nickname\r\n";
	}
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	return ;
}

void Commands::quit( Parser &input, Client client, std::vector<Channel> channels) {
	for (std::vector<Channel>::iterator channelIterator = channels.begin(); channelIterator != channels.end(); channelIterator++) {
		std::vector<Client> clientCopy = channelIterator->getClients();
		for (std::vector<Client>::iterator clientIterator = clientCopy.begin(); clientIterator != clientCopy.end(); clientIterator++) {
			if (clientIterator->getSocketfd() == client.getSocketfd())
				clientCopy.erase(clientIterator);
		}
		for (std::vector<Client>::iterator invitedIterator = clientCopy.begin(); invitedIterator != clientCopy.end(); invitedIterator++) {
			if (invitedIterator->getSocketfd() == client.getSocketfd())
				clientCopy.erase(invitedIterator);
		}
	}
	(void)input;
	//nachricht an alle schicken im channel
	return ;
}

void Commands::join(Parser &input, Client client, std::vector<Channel> channels){
	// std::cout << "join command called" << std::endl;
	std::string joinMessageClient = ":dgross JOIN #test\r\n";
	std::string switchBuffer = "/buffer #test\r\n";
	joinChannel(input.getParam()[0], client, channels);
	send(client.getSocketfd(), joinMessageClient.c_str(), joinMessageClient.length(), 0);
	send(client.getSocketfd(), switchBuffer.c_str(), switchBuffer.length(), 0);
}

Commands::commandFailException::~commandFailException( void ) throw() { return ;	}
Commands::commandFailException::commandFailException( std::string error ) : _error(error) { return ; }
const char *Commands::commandFailException::what() const throw() { return (this->_error.c_str());}
