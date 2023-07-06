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


void Commands::pass( Parser &input, Client client, std::string password ) {
	if (*input.getParam().begin() == password) {
		std::cout << "PW accepted!" << std::endl;
		return ;
	}
	else {
		std::string message = SERVER " " ERR_PASSWDMISMATCH " dgross :Wrong Password\r\n";
		send(client.getSocketfd(), message.c_str(), sizeof(message), 0);
	}
	return ;
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
// function join(channel, keys):
//     if !channel_exists(channel):
//         send_numeric_reply(ERR_NOSUCHCHANNEL)
//         return

//     if channel_is_invite_only(channel) and !user_is_invited(channel):
//         send_numeric_reply(ERR_CHANNELISFULL)
//         return

//     if user_is_banned(user):
//         send_numeric_reply(ERR_INVITEONLYCHAN)
//         return

//     if channel_is_password_protected(channel) and !correct_key_provided(channel, keys):
//         send_numeric_reply(ERR_NEEDMOREPARAMS)
//         return

//     add_user_to_channel(channel, user)
//     send_numeric_reply(RPL_TOPIC, channel, get_channel_topic(channel))
//     send_numeric_reply(RPL_NAMREPLY, channel, get_channel_users(channel))

// @todo bevor man joinen kann sollte man einen nickname und user angelegt haben
void Commands::join(Parser &input, Client client, std::vector<Channel> channels){
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

bool isAlphaNumeric(std::string& name) {
	for (std::string::const_iterator iterator = name.begin(); iterator != name.end(); ++iterator) {
		if (!std::isalnum(*iterator)) {
			return false;
		}
	}
	return true;
}

bool isNicknameUnique(const std::vector<Client>& connections, std::string& nickname) {
	for (std::vector<Client>::size_type i = 0; i < connections.size(); ++i) {
		if (connections[i].getNickname() == nickname) {
			return true;
		}
	}
	return false;
}

// bool checkNickname(Client &client, std::string& nickname, const std::vector<Client>& connections) {
// 	if (isNicknameUnique(connections, nickname)) {
// 		std::string errorMessage = "IRCSERV 433 " + nickname +
// 		" :Nickname is already in use. Please choose a different nickname.";
// 		send(client.getSocketfd(), errorMessage.c_str(), errorMessage.length(), 0);
// 		return false;
// 	}
// 	else if (nickname.size() > 10) {
// 		std::string errorMessage = "IRCSERV 432 " + nickname +
// 		" :Nickname is too long. Please choose a shorter nickname.";
// 		send(client.getSocketfd(), errorMessage.c_str(), errorMessage.length(), 0);
// 		return false;
// 	}
// 	else if (!isAlphaNumeric(nickname)) {
// 		std::string errorMessage = "IRCSERV 432 " + nickname +
// 		" :Nickname contains invalid symbols. Only use letters and numbers.";
// 		send(client.getSocketfd(), errorMessage.c_str(), errorMessage.length(), 0);
// 		return false;
// 	}
// 	return true;
// }

bool checkNickname(Client& client, std::string& nickname, const std::vector<Client>& connections) {
	if (isNicknameUnique(connections, nickname)) {
		std::string errorMessage = ":IRCSERV 433 " + nickname + " :Nickname is already in use. Please choose a different nickname.\r\n";
		send(client.getSocketfd(), errorMessage.c_str(), errorMessage.length(), 0);
		return false;
	} else if (nickname.size() > 10) {
		std::string errorMessage = ":IRCSERV 432 " + nickname + " :Nickname is too long. Please choose a shorter nickname.\r\n";
		send(client.getSocketfd(), errorMessage.c_str(), errorMessage.length(), 0);
		return false;
	} else if (!isAlphaNumeric(nickname)) {
		std::string errorMessage = ":IRCSERV 432 " + nickname + " :Nickname contains invalid symbols. Only use letters and numbers.\r\n";
		send(client.getSocketfd(), errorMessage.c_str(), errorMessage.length(), 0);
		return false;
	}
	return true;
}

// function handle_nick_command(client, nickname, hopcount):
//     if client.is_registered():
//         send_numeric_reply(client, ERR_ALREADYREGISTRED)
//         return

//     if nickname_is_in_use(nickname):
//         handle_nickname_collision(client, nickname)
//         return

//     if is_connected(client):
//         client.set_nickname(nickname)
//         send_numeric_reply(client, RPL_NICKCHANGE)

void Commands::nick(Parser& input, Client& client, std::vector<Client>& connections) {
	if (!checkNickname(client, input.getParam()[0], connections)) {
		return;
	} else {
		client.setNickname(input.getParam()[0]);
		std::string joinMessageClient = ":IRCSERVE 001 " + client.getNickname() +
		" :Welcome to the Internet Relay Network, " + client.getNickname() + "\r\n";
		send(client.getSocketfd(), joinMessageClient.c_str(), joinMessageClient.length(), 0);
	}
}


// function handle_user_command(client, username, hostname, servername, realname):
//     if client.is_registered():
//         send_numeric_reply(client, ERR_ALREADYREGISTRED)
//         return

//     client.set_user_info(username, hostname, servername, realname)

//     if is_local_connection(client):
//         send_welcome_messages_to_client(client)
//         notify_servers_of_new_user(client)

bool isUsernameAvailable(const std::vector<Client>& connections, const std::string& username) {
	if (username.empty())
		return true;
	for (std::vector<Client>::size_type i = 0; i < connections.size(); ++i) {
		std::cout << connections[i].getConstUsername() << " = " << username << std::endl;
		if (connections[i].getConstUsername() == username) {
		    return false;
		}
	}
	return true;
}

bool isNameValid(const std::string& name) {
	for (std::string::const_iterator iterator = name.begin(); iterator != name.end(); ++iterator) {
		if (!std::isalnum(*iterator) && *iterator != '-' && *iterator != '_') {
			return false;
		}
	}
	return true;
}

void Commands::user(Parser& input, Client& client, std::vector<Client>& connections) {
	if (!isNameValid(client.getConstUsername())) {
		return;
	}
	else if (!isUsernameAvailable(connections, input.getTrailing())) {
		return;
	}
	client.setUsername(input.getTrailing());
}


// function handle_invite_command(inviter, nickname, channel):
//     if !channel_exists(channel):
//         send_numeric_reply(inviter, ERR_NOTONCHANNEL, channel)
//         return

//     if !user_has_channel_operator_privileges(inviter, channel):
//         send_numeric_reply(inviter, ERR_CHANOPRIVSNEEDED, channel)
//         return

//     if user_is_online(nickname):
//         send_invite_to_user(nickname, inviter, channel)
//         send_numeric_reply(inviter, RPL_INVITING, nickname, channel)
//     else:
//         send_numeric_reply(inviter, ERR_NOSUCHNICK, nickname)

void Commands::invite(Client& client){
	(void) client;
	std::cout << "invite command" << std::endl;
}

Commands::commandFailException::~commandFailException( void ) throw() { return ;	}
Commands::commandFailException::commandFailException( std::string error ) : _error(error) { return ; }
const char *Commands::commandFailException::what() const throw() { return (this->_error.c_str());}
