#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include "Channel.hpp"
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

int	Commands::searchForChannel( std::string channelName, std::vector<Channel> channels ) {
	if (channels.empty()) {
		std::cout << "empty channel!\n";
		return (-1);
	}
	for (std::vector<Channel>::iterator iterator = channels.begin(); iterator != channels.end(); iterator++ ) {
		if (iterator->getChannelName() == channelName )
			return (std::distance(channels.begin(), iterator));
	}
	return (-1);
}

int	Commands::searchForUser( std::string nickname, std::vector<Client> clients ) {
	if (clients.empty()) {
		std::cout << "empty clients!\n";
		return (-1);
	}
	for (std::vector<Client>::iterator iterator = clients.begin(); iterator != clients.end(); iterator++ ) {
		if (iterator->getNickname() == nickname )
			return (std::distance(clients.begin(), iterator));
	}
	return (-1);
}

void Commands::joinChannel( std::string channelName, Client user, std::vector<Channel> &channels) {
	int	i = searchForChannel( channelName, channels);
	if (i < 0) {
		// creating the channels if the channels does not exist
		channels.push_back(Channel ( channelName, user ));
	}
	else {
		if (channels[i].canUserJoin( user )) {
			channels[i].addUser( user );
			// delete user from invite list
		}
		else
			(void)channelName; // Nachricht an Client : Invited only, can't join!
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

void Commands::kick( Parser &input, Client client, std::vector<Channel> channels ) {
	std::string message;
	int channelIndex = searchForChannel(input.getParam()[0], channels);
	int userIndex = searchForUser(input.getParam()[1], channels[channelIndex].getClients());
	if (channelIndex == -1 || userIndex == -1) {
		std::cout << "Not found\n";
		return ;
	} 
	if (client.getSocketfd() == channels[channelIndex].getFounder().getSocketfd()) {
		message = ":" + client.getNickname() + " KICK " + channels[channelIndex].getChannelName() + " " + client.getNickname() + "\r\n";
		if (!input.getTrailing().empty())
			message += input.getTrailing();
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
	}
	else {
		std::vector<Client> op = channels[channelIndex].getOP();
		for (std::vector<Client>::iterator it = op.begin(); it != op.end(); it++) {
			if (client.getSocketfd() == it->getSocketfd()) {
				message = ":" + client.getNickname() + " KICK " + channels[channelIndex].getChannelName() + " " + client.getNickname();
				if (!input.getTrailing().empty())
					message += input.getTrailing() + + "\r\n";
				else
					message += "\r\n";
				send(client.getSocketfd(), message.c_str(), message.length(), 0);
			}
		}
	}
	return ;
}

void Commands::forwardMsg( std::string message, std::vector<Client> connections) {
	for (std::vector<Client>::iterator it = connections.begin(); it != connections.end(); it++)
		send(it->getSocketfd(), message.c_str(), message.length(), 0);
	return ;
}

void Commands::privmsg( Parser &input, Client client, std::vector<Client> connections, std::vector<Channel> channels) {
	std::string receiver = input.getParam()[0];
	std::string message = input.getTrailing() + "\r\n";;
	int index;

	if (receiver.at(0) == '#') {
		index = searchForChannel(receiver, channels);
		if (index != -1) {
			// wenn nicht gucken obs invite only oder passwort geschützt ist
			if (channels[index].searchforUser(client)) {
				forwardMsg(message, connections);
				return ;
			}
			else if (channels[index].getMode()['i'] == true || channels[index].getMode()['k'] == true)
				message = ERR_CANNOTSENDTOCHAN " " + receiver + " :No pemissions";
		}
		message = ERR_NOSUCHCHANNEL " " + receiver + " :No such channel\r\n";
	}
	else {
		for (std::vector<Client>::iterator it = connections.begin(); it != connections.end(); it++) {
			if (it->getNickname() == receiver) {
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
	std::string joinMessageClient = ":dgross JOIN " + input.getParam()[0] + "\r\n";;
	std::string switchBuffer = "/buffer " + input.getParam()[0] + "\r\n";
	joinChannel(input.getParam()[0], client, channels);
	send(client.getSocketfd(), joinMessageClient.c_str(), joinMessageClient.length(), 0);
	send(client.getSocketfd(), switchBuffer.c_str(), switchBuffer.length(), 0);
}

Commands::commandFailException::~commandFailException( void ) throw() { return ;	}
Commands::commandFailException::commandFailException( std::string error ) : _error(error) { return ; }
const char *Commands::commandFailException::what() const throw() { return (this->_error.c_str());}
