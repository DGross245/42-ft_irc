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
#include <map>
#include "Client.hpp"

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
void Commands::ping( Parser &input, Client client ) {
	std::string message = "PONG :" + input.getParam()[0] + "\r\n";
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	return ;
}

void Commands::cap( Parser &input, Client client ) {
	std::string message;
	if (input.getParam()[0] == "LS")
		message = "CAP * LS :JOIN\r\n";
	else if (input.getParam()[0] == "END")
		message = "CAP * ACK :JOIN\r\n";
	else {
		message = SERVER " 410 " + client.getNickname() + " " + input.getParam()[0] + " :Invalid CAP command";
		std::cout << "User send a invalid CAP request\n";
	}
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	return ;
}

// @todo falsche passwörter gehen tzd durch
void Commands::pass( Parser &input, Client &client, std::string password ) {
	if (*input.getParam().begin() == password) {
		std::cout << "PW accepted!" << std::endl;
		client.setPasswordAccepted(true);
		return ;
	}
	else {
		std::string message = ERR_PASSWDMISMATCH " :Wrong Password\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
	}
	return ;
}

void Commands::topic( Parser &input, Client client, std::vector<Channel> &channels) {
	std::string message;
	std::vector<Channel>::iterator channelIt = searchForChannel(input.getParam()[0], channels);
	if (channelIt == channels.end()) {
		message = SERVER " " ERR_NOSUCHCHANNEL " " + client.getNickname() + " " + input.getParam()[0] + " : No such channel\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	if (input.getTrailing().empty()) {
		if (channelIt->getTopic().empty())
			message = SERVER " " RPL_NOTOPIC " " + client.getNickname() + " " + channelIt->getChannelName() + " :No topic set\r\n";
		else
			message = SERVER " " RPL_TOPIC " " + client.getNickname() + " " + channelIt->getChannelName() + " :" + channelIt->getTopic() + "\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
	}
	else
		channelIt->setTopic(input.getTrailing(), client);
	return ;
}

void Commands::kick( Parser &input, Client requestor, std::vector<Channel> &channels ) {
	std::string message;
	std::vector<Channel>::iterator channelIt = searchForChannel(input.getParam()[0], channels);
	if (channelIt == channels.end()) {
		message = SERVER " " ERR_NOSUCHCHANNEL " " + requestor.getNickname() + " " + input.getParam()[0] + " : No such channel\r\n";
		send(requestor.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	std::vector<Client>::iterator target = channelIt->searchForUser(input.getParam()[1], channelIt->getClients());
	if (target == channelIt->getClients().end()) {
		message = SERVER " " ERR_NOSUCHNICK " " + requestor.getNickname() + " " + input.getParam()[1] + " : No such nickname\r\n";
		send(requestor.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	if (requestor.getSocketfd() == channelIt->getOwner().getSocketfd()) {
		message = ":" + requestor.getNickname() + " KICK " + channelIt->getChannelName() + " " + target->getNickname() + "\r\n";
		if (!input.getTrailing().empty())
			message += input.getTrailing();
		send(target->getSocketfd(), message.c_str(), message.length(), 0);
		channelIt->getClients().erase(target);
	}
	else {
		std::vector<Client> op = channelIt->getOperator();
		for (std::vector<Client>::iterator it = op.begin(); it != op.end(); it++) {
			if (requestor.getSocketfd() == it->getSocketfd()) {
				if (target->getSocketfd() == channelIt->getOwner().getSocketfd()) {
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

void Commands::part( Parser &input, Client client, std::vector<Channel> &channels) {
	std::string message;
	std::vector<Channel>::iterator channelIt = searchForChannel(input.getParam()[0], channels);
	if (channelIt == channels.end()) {
		message = SERVER " " ERR_NOSUCHCHANNEL " " + client.getNickname() + " " + input.getParam()[0] + " : No such channel\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	std::vector<Client>::iterator target = channelIt->searchForUser(client.getNickname(), channelIt->getClients());
	if (target == channelIt->getClients().end()) {
		message = SERVER " " ERR_NOSUCHNICK " " + client.getNickname() + " " + input.getParam()[0] + " : No such nickname\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	channelIt->getClients().erase(target);
	if (!input.getTrailing().empty())
		forwardMsg(input.getTrailing() + "\r\n", channelIt->getChannelName(), client, channelIt->getClients());
	message = ":" + client.getNickname() + " PART " + channelIt->getChannelName() + "\r\n";
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	return ;
}

void Commands::forwardMsg( std::string trailing, std::string channelName, Client client, std::vector<Client> connections) {
	for (std::vector<Client>::iterator it = connections.begin(); it != connections.end(); ++it) {
		if (it->getSocketfd() != client.getSocketfd()) {
			std::string message = ":" + client.getNickname() + " PRIVMSG " + channelName + " :" + trailing;
			send(it->getSocketfd(), message.c_str(), message.length(), 0);
		}
	}
	return ;
}

// @todo clients msg are not being send or didnt reach their target
void Commands::privmsg( Parser &input, Client client, std::vector<Client> connections, std::vector<Channel> channels) {
	std::string receiver = input.getParam()[0];
	std::string message = input.getTrailing() + "\r\n";;

	if (receiver.at(0) == '#') {
		std::vector<Channel>::iterator channelIt = searchForChannel(receiver, channels);
		if (channelIt != channels.end()) {
			std::vector<Client>::iterator clientIt = channelIt->searchForUser(client.getNickname(), channelIt->getClients());
			if (clientIt != channelIt->getClients().end()) {
				forwardMsg(message, channelIt->getChannelName(), client, channelIt->getClients());
				return ;
			}
			else if (channelIt->getMode()['i'] == true || channelIt->getMode()['k'] == true)
				message = ERR_CANNOTSENDTOCHAN " " + receiver + " :No pemissions\r\n";
			else
				forwardMsg(message, channelIt->getChannelName(), client, channelIt->getClients());
		}
		else
			message = ERR_NOSUCHCHANNEL " " + receiver + " :No such channel\r\n";
	}
	else {
		for (std::vector<Client>::iterator targetIt = connections.begin(); targetIt != connections.end(); targetIt++) {
			if (targetIt != connections.end() && targetIt->getNickname() == receiver) {
				message = "PRIVMSG " + client.getNickname() + " :" + input.getTrailing() + "\r\n";
				send(targetIt->getSocketfd(), message.c_str(), message.length(), 0);
				return ;
			}
		}
		message = ERR_NOSUCHNICK " " + receiver + " :No such nickname\r\n";
	}
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	return ;
}

// @funktionier nicht ganz. user wissen nicht wenn er geleavt ist etc
void Commands::quit( Parser &input, Client client, std::vector<Channel> &channels) {
	std::string message;
	for (std::vector<Channel>::iterator channelIt = channels.begin(); channelIt != channels.end(); channelIt++) {
		std::vector<Client> &clientCopy = channelIt->getClients();
		for (std::vector<Client>::iterator clientIterator = clientCopy.begin(); clientIterator != clientCopy.end(); clientIterator++) {
			if (clientIterator->getSocketfd() == client.getSocketfd()) {
				clientCopy.erase(clientIterator);
				//message = "";
				//forwardMsg(message, channelIt->getChannelName(), client, channelIt->getClients());
				return ;
			}
		}
		for (std::vector<Client>::iterator invitedIterator = clientCopy.begin(); invitedIterator != clientCopy.end(); invitedIterator++) {
			if (invitedIterator->getSocketfd() == client.getSocketfd()) {
				clientCopy.erase(invitedIterator);
				return ;
			}
		}
	}
	(void)input;
	return ;
}

// @todo sign nochmal im auge behalten, da ich nicht weiss ob z.B /MODE o dgross einen error schmeissen soll
void Commands::mode(Parser &input, Client client , std::vector<Channel> &channels) {
	bool sign = true;
	std::string message;
	std::string modeLine = input.getParam()[1];
	char mode;
	std::vector<Channel>::iterator channelIt = searchForChannel(input.getParam()[0], channels);
	if (channelIt == channels.end()) {
		message = SERVER " " ERR_NOSUCHCHANNEL " " + client.getNickname() + " " + input.getParam()[0] + " : No such channel\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	std::vector<Client>::iterator clientIt = channelIt->searchForUser(client.getNickname(), channelIt->getOperator());
	if (clientIt == channelIt->getClients().end()) {
		message = SERVER " " ERR_CHANOPRIVSNEEDED " " + client.getNickname() + " " + input.getParam()[2] + " :Channel privileges needed\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	std::map<char,void(*)(bool ,Channel &,std::string, Client)> modeFuntion;
	modeFuntion['i'] = &executeInvite;
	modeFuntion['t'] = &executeTopic;
	modeFuntion['k'] = &executeKey;
	modeFuntion['o'] = &executeOperator;
	modeFuntion['l'] = &executeLimit;

	for (size_t i = 0; i < modeLine.length(); i++) {
		mode = modeLine[i];
		if (mode == '+')
			sign = true;
		else if (mode == '-')
			sign = false;
		else {
			std::map<char,void(*)(bool,Channel &,std::string,Client)>::iterator modeIt = modeFuntion.find(mode);
			if (modeIt == modeFuntion.end()) {
				std::string message = SERVER " " ERR_UNKNOWNMODE " " + client.getNickname() + " " + channelIt->getChannelName() + " :Unknown mode " + mode + "\r\n";
				send(client.getSocketfd(), message.c_str(), message.length(), 0);
				return ;
			}
			else {
				modeIt->second(sign, *channelIt, input.getParam().size() == 3 ? input.getParam()[2] : std::string(), client);
			}
		}
	}
	return ;
}

void Commands::executeInvite( bool sign, Channel &channel, std::string param, Client client ) {
	channel.getMode()['i'] = sign;
	if (sign)
		std::cout << channel.getChannelName() << " modus was set to: +i" << std::endl;
	else
		std::cout << channel.getChannelName() << " modus was set to: -i" << std::endl;
	(void)param;
	(void)client;
	return ;
}

void Commands::executeKey( bool sign, Channel &channel, std::string param, Client client ) {
	if (sign) {
		if (!param.empty()) {
			channel.getMode()['k'] = sign;
			channel.setPassword(param);
			std::cout << channel.getChannelName() << " modus was set to: +k" << std::endl;
		}
		else {
			std::string message = SERVER " " ERR_NEEDMOREPARAMS " " + client.getNickname() + " " + channel.getChannelName() + " :Not enough Parameters for +k\r\n";
			send(client.getSocketfd(), message.c_str(), message.length(), 0);
		}
	}
	else {
		channel.getMode()['k'] = sign;
		channel.setPassword("");
		std::cout << channel.getChannelName() << " modus was set to: -k" << std::endl;
	}
	(void)client;
	return ;
}

void Commands::executeOperator( bool sign, Channel &channel, std::string param, Client client ) {
	std::string message;
	if (param.empty()) {
		message = SERVER " " ERR_NEEDMOREPARAMS " " + client.getNickname() + " " + channel.getChannelName() + " :Not enough Parameters for +/-o\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	std::vector<Client>::iterator clientIt = channel.searchForUser(param, channel.getClients());
	if (clientIt == channel.getClients().end()) {
		message = SERVER " " ERR_NOSUCHNICK " " + client.getNickname() + " " + param + " : No such nickname\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	std::vector<Client>::iterator operatorIt = channel.searchForUser(param, channel.getOperator());
	if (sign) {
		if (operatorIt == channel.getOperator().end()) {
			channel.getOperator().push_back(*clientIt);
			std::cout << channel.getChannelName() << " " << clientIt->getNickname() << " was promoted to operator" << std::endl;
		}
	}
	else {
		if (operatorIt != channel.getOperator().end()) {
			if (operatorIt->getSocketfd() == channel.getOwner().getSocketfd()) {
				std::cout << "ERROR\n";
				message = SERVER " " ERR_NOPRIVLIEGES " " + client.getNickname() + " " + channel.getChannelName() + " :You can't demote yourself as the channel owner\r\n";
				send(client.getSocketfd(), message.c_str(), message.length(), 0);
			}
			else {
				channel.getOperator().erase(clientIt);
				std::cout << channel.getChannelName() << " " << clientIt->getNickname() << " was demoted" << std::endl;
			}
		}

	}
	(void)client;
	return ;
}

void Commands::executeLimit( bool sign, Channel &channel, std::string param, Client client ) {
	(void) client;
	if (sign) {
		if (!param.empty()) {
			channel.getMode()['l'] = sign;
			channel.setLimit(0);
			std::cout << channel.getChannelName() << " modus was set to: +l" << std::endl;
		}
		else {
			std::string message = SERVER " " ERR_NEEDMOREPARAMS " " + client.getNickname() + " " + channel.getChannelName() + " :Not enough Parameters for +l\r\n";
			send(client.getSocketfd(), message.c_str(), message.length(), 0);
		}
	}
	else {
		channel.getMode()['l'] = sign;
		std::cout << channel.getChannelName() << " modus was set to: -l" << std::endl;
	}
	return ;
}

void Commands::executeTopic( bool sign, Channel &channel, std::string param, Client client ) {
	channel.getMode()['t'] = sign;
	if (sign)
		std::cout << channel.getChannelName() << " modus was set to: +t" << std::endl;
	else
		std::cout << channel.getChannelName() << " modus was set to: -t" << std::endl;
	(void)param;
	(void)client;
	return ;
}
// @todo join wird irgendwie 2x gesendet
void Commands::join(Parser &input, Client client, std::vector<Channel> &channels){
	std::string message;
	if (input.getParam()[0].at(0) == '#')
	{
		std::vector<Channel>::iterator channelIt = searchForChannel( input.getParam()[0], channels);
		if (channelIt == channels.end()) {
			channels.push_back(Channel ( input.getParam()[0], client ));
			message = ":" + client.getNickname() + " JOIN " + input.getParam()[0] + "\r\n";;
			send(client.getSocketfd(), message.c_str(), message.length(), 0);
			message = "/buffer " + input.getParam()[0] + "\r\n";
			send(client.getSocketfd(), message.c_str(), message.length(), 0);
		}
		else {
			if (channelIt->canUserJoin( client, input )) {
				channelIt->addUser( client );
				std::vector<Client>::iterator clientIt = channelIt->searchForUser(client.getNickname(), channelIt->getInviteList());
				if (clientIt != channelIt->getInviteList().end())
					channelIt->getInviteList().erase(clientIt);
				for (std::vector<Client>::iterator it = channelIt->getClients().begin(); it != channelIt->getClients().end(); ++it) {
					message = ":" + client.getNickname() + " JOIN " + input.getParam()[0] + "\r\n";;
					send(it->getSocketfd(), message.c_str(), message.length(), 0);
				}
				message = "/buffer " + input.getParam()[0] + "\r\n";
				send(client.getSocketfd(), message.c_str(), message.length(), 0);
			}
		}
	}
	return ;
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

bool isUsernameAvailable(const std::vector<Client>& connections, const std::string& username) {
	if (username.empty())
		return true;
	for (std::vector<Client>::size_type i = 0; i < connections.size(); ++i) {
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


// 1. Check if the invoker has the necessary permissions to send invitations.

// 2. Check if the <nickname> is available on server

// 3. Check if <channel>  valid and exist.

// 4. Send an INVITE message to the target user using their connection details:
//    - Command: INVITE
//    - Parameters: <nickname> <channel>

// 5. The target user receives the invitation

// 6. If the invitation is accepted:
//    - Add the user to the list of channel members.
//    - Notify the channel members about the user's arrival.
bool checkInvokerRights(std::vector<Client> &operators, std::string nickname) {
	for (size_t i = 0; i < operators.size(); i++) {
		std::cout << "Nickname of the Operators: " << operators[i].getNickname() << std::endl;
		std::cout << nickname << " == " << operators[i].getNickname() << std::endl;
		if (operators[i].getNickname() == nickname)
			return true;
	}
		std::cout << "return flase" << std::endl;
	return false;

}
bool checkPermission(std::string channelName, std::string nickname, std::vector<Channel> &channels) {
	for (size_t i = 0; i < channels.size(); ++i) {
		std::cout << channelName << " = " << channels[i].getChannelName() << std::endl;
		if (channels[i].getChannelName() == channelName && checkInvokerRights(channels[i].getOperator(), nickname)) {
			std::cout << "Found the channel: " << channelName << std::endl;
			return true;
		}
	}
	return false;
}

bool checkInvitedPerson(std::vector<Client> &connections, std::string invitedPerson) {
	std::cout << "Bin in checkInvitedPerson" << std::endl;
	for (size_t i = 0; i < connections.size(); ++i) {
		if (connections[i].getNickname() == invitedPerson) {
			std::cout << "got it" << std::endl;
			return true;
		}
	}
	return false;
}

void sendInvitation(Client &client, std::string nickname, std::string channelName,  std::vector<Channel> &channels) {
	std::string inviteMessageClient = ":" + nickname + " INVITE " + nickname + " " + channelName + "\r\n";
	std::cout << "Send: " << inviteMessageClient << std::endl;
	send(client.getSocketfd(), inviteMessageClient.c_str(), inviteMessageClient.length(), 0);
	Commands::searchForChannel(channelName, channels);
}

void Commands::invite(Client& client, Parser& input, std::vector<Client> &connections, std::vector<Channel> &channels) {
	std::cout << "------------------------------------" << std::endl;
	// Check if the invoker has the necessary permissions to send invitations on the channel .
	if (!checkPermission(input.getParam()[1], client.getNickname(), channels)) {
		return ;
	}
	// Check if the <nickname> is available on server
	if (checkInvitedPerson(connections, input.getParam()[1])) {
		return ;
	}
	sendInvitation(client, input.getParam()[0], input.getParam()[1], channels);
}



Commands::commandFailException::~commandFailException( void ) throw() { return ;	}
Commands::commandFailException::commandFailException( std::string error ) : _error(error) { return ; }
const char *Commands::commandFailException::what() const throw() { return (this->_error.c_str());}
