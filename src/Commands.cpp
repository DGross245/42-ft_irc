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

std::vector<Channel>::iterator	Commands::searchForChannel( std::string channelName, std::vector<Channel> &channels ) {
	std::vector<Channel>::iterator iterator = channels.end();
	for (iterator = channels.begin(); iterator != channels.end(); iterator++ ) {
		if (iterator->getChannelName() == channelName )
			break ;
	}
	return (iterator);
}

std::vector<Client>::iterator	Commands::searchForUser( std::string nickname, std::vector<Client> &clients ) {
	std::vector<Client>::iterator iterator = clients.end();
	for (iterator = clients.begin(); iterator != clients.end(); iterator++ ) {
		if (iterator->getNickname() == nickname )
			break ;
	}
	return (iterator);
}

void Commands::joinChannel( std::string channelName, Client user, std::vector<Channel> &channels) {
	std::vector<Channel>::iterator channelIt = searchForChannel( channelName, channels);
	if (channelIt == channels.end()) {
		// creating the channels if the channels does not exist
		channels.push_back(Channel ( channelName, user ));
	}
	else {
		if (channelIt->canUserJoin( user )) {
			channelIt->addUser( user );
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

void Commands::kick( Parser &input, Client requestor, std::vector<Channel> &channels ) {
	std::string message;
	std::vector<Channel>::iterator channelIt = searchForChannel(input.getParam()[0], channels);
	if (channelIt == channels.end()) {
		message = SERVER " " ERR_NOSUCHCHANNEL + requestor.getNickname() + " " + input.getParam()[0] + " : No such channel";
		send(requestor.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	std::vector<Client>::iterator target = searchForUser(input.getParam()[1], channelIt->getClients());
	if (target == channelIt->getClients().end()) {
		message = SERVER " " ERR_NOSUCHNICK + requestor.getNickname() + " " + input.getParam()[1] + " : No such nickname";
		send(requestor.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	if (requestor.getSocketfd() == channelIt->getFounder().getSocketfd()) {
		message = ":" + requestor.getNickname() + " KICK " + channelIt->getChannelName() + " " + target->getNickname() + "\r\n";
		if (!input.getTrailing().empty())
			message += input.getTrailing();
		send(target->getSocketfd(), message.c_str(), message.length(), 0);
		channelIt->getClients().erase(target);
	}
	else {
		std::vector<Client> op = channelIt->getOP();
		for (std::vector<Client>::iterator it = op.begin(); it != op.end(); it++) {
			if (requestor.getSocketfd() == it->getSocketfd()) {
				if (target->getSocketfd() == channelIt->getFounder().getSocketfd()) {
					message = "ERROR";
					send(target->getSocketfd(), message.c_str(), message.length(), 0);
					return ;
				}
				message = ":" + requestor.getNickname() + " KICK " + channelIt->getChannelName() + " " + target->getNickname();
				if (!input.getTrailing().empty())
					message += input.getTrailing() + + "\r\n";
				else
					message += "\r\n";
				send(target->getSocketfd(), message.c_str(), message.length(), 0);
				channelIt->getClients().erase(target);
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

	if (receiver.at(0) == '#') {
		std::vector<Channel>::iterator channelIt = searchForChannel(receiver, channels);
		if (channelIt != channels.end()) {
			// wenn nicht gucken obs invite only oder passwort geschützt ist
			if (channelIt->searchforUser(client)) {
				forwardMsg(message, connections);
				return ;
			}
			else if (channelIt->getMode()['i'] == true || channelIt->getMode()['k'] == true)
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

// @attention wenn man einen channel gejoint ist und quited segfaultet es hier
void Commands::quit( Parser &input, Client client, std::vector<Channel> &channels) {
	for (std::vector<Channel>::iterator channelIterator = channels.begin(); channelIterator != channels.end(); channelIterator++) {
		std::vector<Client> &clientCopy = channelIterator->getClients();
		for (std::vector<Client>::iterator clientIterator = clientCopy.begin(); clientIterator != clientCopy.end(); clientIterator++) {
			if (clientIterator->getSocketfd() == client.getSocketfd()) {
				clientCopy.erase(clientIterator);
				return ;
			}
			std::cout << "Done:" << clientIterator->getSocketfd()  << std::endl;
		}
		for (std::vector<Client>::iterator invitedIterator = clientCopy.begin(); invitedIterator != clientCopy.end(); invitedIterator++) {
			if (invitedIterator->getSocketfd() == client.getSocketfd()) {
				clientCopy.erase(invitedIterator);
				return ;
			}
		}
	}
	(void)input;
	//nachricht an alle schicken im channel
	return ;
}

// void Commands::mode(Parser &input, Client client , std::vector<Channel> &channels) {
// 	bool sign;
// 	while() {
// 		if (input.getParam())

// 	}
// 	return ;
// }

void Commands::join(Parser &input, Client client, std::vector<Channel> &channels){
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
