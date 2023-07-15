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
#include <sstream>

// @todo rework all messages being send
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
		message = SERVER " 410 " + client.getNickname() + " " + input.getParam()[0] + " :Invalid CAP command\r\n";
		std::cout << "User send a invalid CAP request\n";
	}
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	return ;
}

void Commands::pass( Parser &input, Client &client, std::string password ) {
	if (*input.getParam().begin() == password) {
		client.setPasswordAccepted(true);
		return ;
	}
	else {
		std::string message = SERVER " " ERR_PASSWDMISMATCH " * :Wrong Password\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
		client.setPasswordAccepted(false);
	}
	return ;
}

// @todo also check if client is on channel / when +t is aktiv and user is not an operator we must send ERR_CHANOPRIVSNEEDED
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

// @todo ERR_NOTONCHANNEL missing
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
		message = SERVER " " ERR_NOSUCHNICK " " + requestor.getNickname() + " " + channelIt->getChannelName() + " : No such nickname\r\n";
		send(requestor.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
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
				for (std::vector<Client>::iterator clientIt = channelIt->getClients().begin(); clientIt != channelIt->getClients().end(); ++clientIt) {
					message = ":" + requestor.getNickname() + " KICK " + channelIt->getChannelName() + " " + target->getNickname();
					if (!input.getTrailing().empty())
						message += " :" + input.getTrailing() + "\r\n";
					else
						message += "\r\n";
					send(clientIt->getSocketfd(), message.c_str(), message.length(), 0);
				}
				channelIt->getClients().erase(target);
				return ;
			}
		}
	}
	return ;
}

// @todo ERR_NOTONCHANNEL, und nochmal checken ob die nachricht gesendet wird
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
	channelIt->getClients().erase(targetIt);
	if (!input.getTrailing().empty()) {
		for (std::vector<Client>::iterator it = channelIt->getClients().begin(); it != channelIt->getClients().end(); ++it) {
			if (it->getSocketfd() != client.getSocketfd()) {
				std::string message = ":" + client.getNickname() + " PART " + channelIt->getChannelName() + " :" + input.getTrailing() + "\r\n";
				send(it->getSocketfd(), message.c_str(), message.length(), 0);
			}
		}
	}
	message = ":" + client.getNickname() + " PART " + channelIt->getChannelName() + "\r\n";
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	if (channelIt->getClients().size() == 0)
		channels.erase(channelIt);
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
				message = ":" + client.getNickname() + " PRIVMSG " + receiver + " :" + input.getTrailing() + "\r\n";
				send(targetIt->getSocketfd(), message.c_str(), message.length(), 0);
				return ;
			}
		}
		message = ERR_NOSUCHNICK " " + receiver + " :No such nickname\r\n";
	}
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	return ;
}

void Commands::quit( Parser &input, Client client, std::vector<Channel> &channels, std::vector<Client> &connections) {
	for (std::vector<Channel>::iterator channelIt = channels.begin(); channelIt != channels.end(); channelIt++) {
		std::vector<Client> &clientCopy = channelIt->getClients();
		for (std::vector<Client>::iterator targetIt = clientCopy.begin(); targetIt != clientCopy.end(); targetIt++) {
			if (targetIt->getSocketfd() == client.getSocketfd()) {
				clientCopy.erase(targetIt);
				break ;
			}
		}
		for (std::vector<Client>::iterator invitedIterator = clientCopy.begin(); invitedIterator != clientCopy.end(); invitedIterator++) {
			if (invitedIterator->getSocketfd() == client.getSocketfd()) {
				clientCopy.erase(invitedIterator);
				break ;
			}
		}
		if (clientCopy.size() == 0) {
			channels.erase(channelIt);
			channelIt--;
		}
	}
	for (std::vector<Client>::iterator clientIt = connections.begin(); clientIt != connections.end(); clientIt++) {
		if (client.getNickname() != clientIt->getNickname()) {
			std::string message = ":" + client.getNickname() + " QUIT :" + input.getTrailing() + "\r\n";
			send(clientIt->getSocketfd(), message.c_str(), message.length(), 0);
		}
	}
	(void)input;
	return ;
}

void Commands::mode(Parser &input, Client client , std::vector<Channel> &channels) {
	std::string modeLine = input.getParam()[1];
	std::string message;
	bool sign = true;
	char mode;

	std::vector<Channel>::iterator channelIt = searchForChannel(input.getParam()[0], channels);
	if (channelIt == channels.end()) {
		message = SERVER " " ERR_NOSUCHCHANNEL " " + client.getNickname() + " " + input.getParam()[0] + " : No such channel\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	std::vector<Client>::iterator clientIt = channelIt->searchForUser(client.getNickname(), channelIt->getOperator());
	if (clientIt == channelIt->getClients().end()) {
		message = SERVER " " ERR_NOTONCHANNEL " " + client.getNickname() + " " + channelIt->getChannelName() + " :You're not in the channel\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	if (input.getParam().size() == 1) {
		message = SERVER " " RPL_CHANNELMODEIS " " + client.getNickname() + " " + channelIt->getChannelName() + " +" + channelIt->getModeString() + "\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
		std::cout << "lol\n";
		return ;
	}
	std::vector<Client>::iterator operatorIt = channelIt->searchForUser(client.getNickname(), channelIt->getOperator());
	if (operatorIt == channelIt->getOperator().end()) {
		message = SERVER " " ERR_CHANOPRIVSNEEDED " " + client.getNickname() + " :Channel privileges needed\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	if (input.getParam()[1].at(0) != '-' && input.getParam()[1].at(0) != '+') {
		std::string message = SERVER " " ERR_UNKNOWNMODE " " + client.getNickname() + " " + channelIt->getChannelName() + " :Unknown mode " + input.getParam()[1].at(0) + "\r\n";
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
	std::string message;
	if (sign) {
		message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " +i\r\n";
		std::cout << channel.getChannelName() << " modus was set to: +i" << std::endl;
	}
	else {
		message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " -i\r\n";
		std::cout << channel.getChannelName() << " modus was set to: -i" << std::endl;
	}
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	(void)param;
	(void)client;
	return ;
}

void Commands::executeKey( bool sign, Channel &channel, std::string param, Client client ) {
	std::string message;

	if (sign) {
		if (!param.empty()) {
			channel.getMode()['k'] = sign;
			channel.setPassword(param);
			message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " +k " + param + "\r\n";
			std::cout << channel.getChannelName() << " modus was set to: +k" << std::endl;
		}
		else
			std::string message = SERVER " " ERR_NEEDMOREPARAMS " " + client.getNickname() + " " + channel.getChannelName() + " :Not enough Parameters for +k\r\n";
	}
	else {
		channel.getMode()['k'] = sign;
		channel.setPassword("");
		message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " -k\r\n";
		std::cout << channel.getChannelName() << " modus was set to: -k" << std::endl;
	}
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
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
		message = SERVER " " ERR_NOSUCHNICK " " + client.getNickname() + " " + channel.getChannelName() + " : No such nickname\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
		return ;
	}
	std::vector<Client>::iterator operatorIt = channel.searchForUser(param, channel.getOperator());
	if (sign) {
		if (operatorIt == channel.getOperator().end()) {
			channel.getOperator().push_back(*clientIt);
			message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " +o" + param + "\r\n";
			std::cout << channel.getChannelName() << " " << clientIt->getNickname() << " was promoted to operator" << std::endl;
		}
	}
	else {
		if (operatorIt != channel.getOperator().end()) {
			if (operatorIt->getSocketfd() == channel.getOwner().getSocketfd()) {
				std::cout << "ERROR\n";
				message = SERVER " " ERR_NOPRIVLIEGES " " + client.getNickname() + " " + channel.getChannelName() + " :You can't demote yourself as the channel owner\r\n";
			}
			else {
				channel.getOperator().erase(operatorIt);
				message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " -o" + param + "\r\n";
				std::cout << channel.getChannelName() << " " << clientIt->getNickname() << " was demoted" << std::endl;
			}
		}

	}
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	(void)client;
	return ;
}

void Commands::executeLimit( bool sign, Channel &channel, std::string param, Client client ) {
	std::string message;
	(void) client;

	if (sign) {
		if (!param.empty()) {
			channel.getMode()['l'] = sign;
			if (param.find_first_not_of("0123456789") == std::string::npos) {
				int limit = static_cast<int>( strtod(param.c_str(), NULL) );
				int overflowCheck;

				std::stringstream ss(param);
				ss >> overflowCheck;
				if (ss.fail() || !ss.eof())
					return;
				else if (limit < 1)
					return;
				else {
					channel.setLimit(limit);
					message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " +l\r\n";
					std::cout << channel.getChannelName() << " modus was set to: +l" << std::endl;
				}
			}
		}
		else
			message = SERVER " " ERR_NEEDMOREPARAMS " " + client.getNickname() + " " + channel.getChannelName() + " :Not enough Parameters for +l\r\n";
	}
	else {
		channel.getMode()['l'] = sign;
		message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " -l\r\n";
		std::cout << channel.getChannelName() << " modus was set to: -l" << std::endl;
	}
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	return ;
}

void Commands::executeTopic( bool sign, Channel &channel, std::string param, Client client ) {
	channel.getMode()['t'] = sign;
	std::string message;
	if (sign) {
		message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " +t\r\n";
		std::cout << channel.getChannelName() << " modus was set to: +t" << std::endl;
	}
	else {
		message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " -t\r\n";
		std::cout << channel.getChannelName() << " modus was set to: -t" << std::endl;
	}
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	(void)param;
	(void)client;
	return ;
}

void sendWelcomeMessage(Client client, std::vector<Channel>::iterator channelIt) {
	std::string message;
	if (channelIt->getTopic().empty())
		message = SERVER " " RPL_NOTOPIC " " + client.getNickname() + " " + channelIt->getChannelName() + " :No topic set\r\n";
	else
		message = SERVER " " RPL_TOPIC " " + client.getNickname() + " " + channelIt->getChannelName() + " :" + channelIt->getTopic() + "\r\n";
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	for (std::vector<Client>::iterator it = channelIt->getClients().begin(); it != channelIt->getClients().end(); ++it) {
		if (it->getSocketfd() != client.getSocketfd()) {
			message = ":" + client.getNickname() + " JOIN " + channelIt->getChannelName() + "\r\n";;
			send(it->getSocketfd(), message.c_str(), message.length(), 0);
			std::cout << "Server: " << message << std::endl;
		}
	}
	message = SERVER " " RPL_NAMREPLY " " + client.getNickname() + " " + channelIt->getChannelName() + " :";
	for (std::vector<Client>::iterator clientIt = channelIt->getClients().begin(); clientIt != channelIt->getClients().end(); clientIt++) {
		std::vector<Client>::iterator operatorIt = channelIt->searchForUser(clientIt->getNickname(), channelIt->getOperator());
		if (operatorIt != channelIt->getOperator().end())
			message += "@";
		message += clientIt->getNickname() + " ";
	}
	message += "\r\n";
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	message = SERVER " " RPL_ENDOFNAMES " " + client.getNickname() + " " + channelIt->getChannelName() + " :End of /NAMES list\r\n";
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	message = SERVER " MODE " + channelIt->getChannelName() + " +" + channelIt->getModeString() + "\r\n";
	if (send(client.getSocketfd(), message.c_str(), message.length(), 0) == -1)
		std::cout << "ERROR\n";
	std::cout << "send:" << message << std::endl;
	return ;
}

// @todo RPL_NAMRPLY and RPL_ENDOFNAMES,
// @todo client darf nicht nochmal in den channel joinen können wo er schon drin ist
void Commands::join(Parser &input, Client client, std::vector<Channel> &channels){
	std::string message;
	if (input.getParam()[0].at(0) == '#') {
		std::vector<Channel>::iterator channelIt = searchForChannel( input.getParam()[0], channels);
		if (channelIt == channels.end()) {
			channels.push_back(Channel ( input.getParam()[0], client ));
			message = ":" + client.getNickname() + " JOIN " + input.getParam()[0] + "\r\n";;
			send(client.getSocketfd(), message.c_str(), message.length(), 0);
			channelIt = searchForChannel( input.getParam()[0], channels);
			std::cout << GREEN << "User " << client.getNickname() << " created channel " << input.getParam()[0] << RESET << std::endl;
		}
		else {
			if (channelIt->canUserJoin( client, input )) {
				channelIt->addUser( client );
				std::vector<Client>::iterator clientIt = channelIt->searchForUser(client.getNickname(), channelIt->getInviteList());
				if (clientIt != channelIt->getInviteList().end())
					channelIt->getInviteList().erase(clientIt);
				message = ":" + client.getNickname() + " JOIN " + input.getParam()[0] + "\r\n";;
				send(client.getSocketfd(), message.c_str(), message.length(), 0);
				std::cout << "server:" << message << std::endl;
			}
		}
		sendWelcomeMessage(client, channelIt);
	} else {
		message = SERVER " " ERR_NOSUCHCHANNEL " " + client.getNickname() + " " + input.getParam()[0] + " : No such channel\r\n";
		send(client.getSocketfd(), message.c_str(), message.length(), 0);
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
		std::string errorMessage = SERVER " " ERR_NICKNAMEINUSE " " + nickname + " :Nickname is already in use. Please choose a different nickname.\r\n";
		send(client.getSocketfd(), errorMessage.c_str(), errorMessage.length(), 0);
		return false;
	} else if (nickname.size() > 10) {
		std::string errorMessage = SERVER " " ERR_ERRONEUSNICKNAME " " + nickname + " :Nickname is too long. Please choose a shorter nickname.\r\n";
		send(client.getSocketfd(), errorMessage.c_str(), errorMessage.length(), 0);
		return false;
	} else if (!isAlphaNumeric(nickname)) {
		std::string errorMessage = SERVER " " ERR_ERRONEUSNICKNAME " " + nickname + " :Nickname contains invalid symbols. Only use letters and numbers.\r\n";
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
		std::string message = ":" + client.getConstUsername() + " NICK " + client.getNickname() + "\r\n";

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

bool checkInvokerRights(std::vector<Client> &operators, std::string nickname) {
	for (size_t i = 0; i < operators.size(); i++) {
		if (operators[i].getNickname() == nickname)
			return true;
	}
	return false;
}

bool checkPermission(Client &client, std::string channelName, std::string nickname, std::vector<Channel> &channels) {
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i].getChannelName() == channelName) {
			if (!checkInvokerRights(channels[i].getOperator(), nickname)) {
				std::string errorMessageClient = SERVER " " ERR_CHANOPRIVSNEEDED " " + nickname + " " + channelName + " :You're not channel operator\r\n";
				send(client.getSocketfd(), errorMessageClient.c_str(), errorMessageClient.length(), 0);
				return false;
			}
			return true;
		}
	}
	std::string errorMessageClient = SERVER " " ERR_NOSUCHCHANNEL " " + nickname + " " + channelName + " :No such channel\r\n";
	send(client.getSocketfd(), errorMessageClient.c_str(), errorMessageClient.length(), 0);
	return false;
}

bool checkInvitedPerson(std::vector<Client> &connections, std::string invitedPerson) {
	for (size_t i = 0; i < connections.size(); i++) {
		if (connections[i].getNickname() == invitedPerson) {
			return true;
		}
	}
	return false;
}

void sendInvitation(Client &client, std::string nickname, std::string channelName,  std::vector<Channel> &channels, std::vector<Client> &connections) {
	std::string inviteMessageClient = ":" + client.getNickname() + " " + RPL_INVITING " " + nickname + " " + channelName + "\r\n";
	send(client.getSocketfd(), inviteMessageClient.c_str(), inviteMessageClient.length(), 0);
	std::vector<Channel>::iterator channelIt = Commands::searchForChannel(channelName, channels);
	if (channelIt != channels.end()) {
		for(std::vector<Client>::iterator clientIt = connections.begin(); clientIt != connections.end(); clientIt++) {
			if (clientIt->getNickname() == nickname)
				channelIt->getInviteList().push_back(*clientIt);

		}
	}
}

bool invitedPersonIsOnChannel(Client &client, std::vector<Channel> &channels, std::string channelName, std::string invitedPerson) {
	std::vector<Channel>::iterator channelIt = Commands::searchForChannel(channelName, channels);
	if (channelIt != channels.end()) {
		for(std::vector<Client>::iterator clientIt = channelIt->getClients().begin(); clientIt != channelIt->getClients().end(); clientIt++) {
			if (clientIt->getNickname() == invitedPerson) {
				std::string errorMessageClient = SERVER " " ERR_USERONCHANNEL " " + invitedPerson + " " + channelName + " :is already on channel\r\n";
				send(client.getSocketfd(), errorMessageClient.c_str(), errorMessageClient.length(), 0);
				return true;
			}
		}
	}
	return false;
}

void Commands::invite(Client& client, Parser& input, std::vector<Client> &connections, std::vector<Channel> &channels) {
	if (!checkPermission(client, input.getParam()[1], client.getNickname(), channels)) {
		return;
	}
	if (invitedPersonIsOnChannel(client, channels, input.getParam()[1], input.getParam()[0])) {
		return;
	}
	if (!checkInvitedPerson(connections, input.getParam()[1])) {
		std::string errorMessageClient = SERVER " " ERR_NOSUCHNICK " " + client.getNickname() + " " + input.getParam()[1] + " :No such nickname\r\n";
		std::cout << errorMessageClient << std::endl;
		send(client.getSocketfd(), errorMessageClient.c_str(), errorMessageClient.length(), 0);
		return;
	}
	sendInvitation(client, input.getParam()[0], input.getParam()[1], channels, connections);
}

Commands::commandFailException::~commandFailException( void ) throw() { return ;	}
Commands::commandFailException::commandFailException( std::string error ) : _error(error) { return ; }
const char *Commands::commandFailException::what() const throw() { return (this->_error.c_str());}
