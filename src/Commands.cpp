#include "Parser.hpp"
#include "Commands.hpp"
#include "Channel.hpp"
#include "Constants.hpp"
#include "Client.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <map>
#include <sstream>
#include <cstdlib>

Commands::Commands() {
	return ;
}

Commands::~Commands( void ) {
	return ;
}

std::vector<Channel>::iterator	Commands::searchForChannel( std::string channelName, std::vector<Channel> &channels ) {
	std::vector<Channel>::iterator iterator = channels.end();
	for (iterator = channels.begin(); iterator != channels.end(); ++iterator) {
		if (iterator->getChannelName() == channelName)
			break ;
	}
	return (iterator);
}

void Commands::ping( Parser &input, Client client ) {
	return (client.sendMsg("PONG :" + input.getParam()[0] + "\r\n"));
}

void Commands::cap( Parser &input, Client client ) {
	if (input.getParam()[0] == "LS")
		client.sendMsg("CAP * LS :...\r\n");
	else if (input.getParam()[0] == "END")
		client.sendMsg("CAP * ACK :...\r\n");
	else {
		client.sendMsg(SERVER " 410 " + client.getNickname() + " " + input.getParam()[0] + " :Invalid CAP command\r\n");
		std::cout << BLACK "[Server]: " DARK_GRAY BOLD "User send a invalid CAP request" RESET "\n" << std::endl;
	}
	return ;
}

void Commands::pass( Parser &input, Client &client, std::string password ) {
	if (!client.getPasswordAccepted()) {
		if (*input.getParam().begin() == password) {
			std::cout << BLACK "[Server]: " DARK_GRAY BOLD "Password accepted from connecting client" RESET "\n" << std::endl;
			client.setPasswordAccepted(true);
		}
		else {
			client.sendMsg(SERVER " " ERR_PASSWDMISMATCH  " " + (client.getNickname().empty() ? "*" : client.getNickname()) + " :Wrong Password\r\n");
			client.setPasswordAccepted(false);
		}
	}
	else
		client.sendMsg(SERVER " " ERR_ALREADYREGISTRED " " + (client.getNickname().empty() ? "*" : client.getNickname()) + " :You have entered the correct password already\r\n");
	return ;
}

void Commands::topic( Parser &input, Client client, std::vector<Channel> &channels) {
	std::vector<Channel>::iterator channelIt = searchForChannel(input.getParam()[0], channels);
	if (channelIt == channels.end())
		return (client.sendMsg(SERVER " " ERR_NOSUCHCHANNEL " " + client.getNickname() + " " + input.getParam()[0] + " :No such channel\r\n"));
	std::vector<Client>::iterator clientIt = channelIt->searchForUser(client.getNickname(), channelIt->getClients());
	if (clientIt == channelIt->getClients().end())
		return (client.sendMsg(SERVER " " ERR_NOTONCHANNEL " " + client.getNickname() + " " + channelIt->getChannelName() + " :You're not in the channel\r\n"));
	if (input.getTrailing().empty()) {
		if (channelIt->getTopic().empty())
			client.sendMsg(SERVER " " RPL_NOTOPIC " " + client.getNickname() + " " + channelIt->getChannelName() + " :No topic set\r\n");
		else
			client.sendMsg(SERVER " " RPL_TOPIC " " + client.getNickname() + " " + channelIt->getChannelName() + " :" + channelIt->getTopic() + "\r\n");
	}
	else {
		if (channelIt->getMode()['t']) {
			std::vector<Client>::iterator operatorIt = channelIt->searchForUser(client.getNickname(), channelIt->getOperator());
			if (operatorIt == channelIt->getOperator().end())
				return (client.sendMsg(SERVER " " ERR_CHANOPRIVSNEEDED " " + client.getNickname() + " " + input.getParam()[0] + " :Channel privileges needed\r\n"));
		}
		channelIt->setTopic(input.getTrailing(), client);
		std::string message = SERVER " " RPL_TOPIC " " + client.getNickname() + " " + channelIt->getChannelName() + " :" + channelIt->getTopic() + "\r\n";
		forwardMsg(message, client, channelIt->getClients(), INCLUDE);
	}
	return ;
}

void Commands::kick( Parser &input, Client requestor, std::vector<Channel> &channels ) {
	std::string message;

	std::vector<Channel>::iterator channelIt = searchForChannel(input.getParam()[0], channels);
	if (channelIt == channels.end())
		return (requestor.sendMsg(SERVER " " ERR_NOSUCHCHANNEL " " + requestor.getNickname() + " " + input.getParam()[0] + " :No such channel\r\n"));
	std::vector<Client>::iterator clientIt = channelIt->searchForUser(requestor.getNickname(), channelIt->getClients());
	if (clientIt == channelIt->getClients().end())
		return (requestor.sendMsg(SERVER " " ERR_NOTONCHANNEL " " + requestor.getNickname() + " " + channelIt->getChannelName() + " :You're not in the channel\r\n"));
	std::vector<Client>::iterator targetIt = channelIt->searchForUser(input.getParam()[1], channelIt->getClients());
	if (targetIt == channelIt->getClients().end())
		return (requestor.sendMsg(SERVER " " ERR_NOSUCHNICK " " + requestor.getNickname() + " " + channelIt->getChannelName() + " :No such nickname\r\n"));
	std::vector<Client>::iterator operatorIt = channelIt->searchForUser(requestor.getNickname(), channelIt->getOperator());
	if (operatorIt == channelIt->getOperator().end())
		return (requestor.sendMsg(SERVER " " ERR_CHANOPRIVSNEEDED " " + requestor.getNickname() + " " + channelIt->getChannelName() + " :You're not channel operator\r\n"));
	if (targetIt->getSocketfd() == channelIt->getOwner().getSocketfd() && requestor.getSocketfd() != channelIt->getOwner().getSocketfd())
		return (targetIt->sendMsg(SERVER " " ERR_NOPRIVLIEGES " " + requestor.getNickname() + " " + channelIt->getChannelName() + " :You can't kick the owner of this channel\r\n"));
	else {
		if (!input.getTrailing().empty())
			message = ":" + requestor.getNickname() + " KICK " + channelIt->getChannelName() + " " + targetIt->getNickname() + " :" + input.getTrailing() + "\r\n";
		else
			message = ":" + requestor.getNickname() + " KICK " + channelIt->getChannelName() + " " + targetIt->getNickname() + "\r\n";
		forwardMsg(message, requestor, channelIt->getClients(), INCLUDE);
		if (targetIt->getSocketfd() == channelIt->getOwner().getSocketfd())
			channelIt->deleteOwner(-2);
		channelIt->getClients().erase(targetIt);
		targetIt = channelIt->searchForUser(input.getParam()[1], channelIt->getInviteList());
		if (targetIt != channelIt->getInviteList().end())
			channelIt->getInviteList().erase(targetIt);
		targetIt = channelIt->searchForUser(input.getParam()[1], channelIt->getOperator());
		if (targetIt != channelIt->getOperator().end())
			channelIt->getOperator().erase(targetIt);
		if (channelIt->getClients().size() == 0)
			channels.erase(channelIt);
	}
	return ;
}

void Commands::part( Parser &input, Client client, std::vector<Channel> &channels) {
	std::string message;
	splitByComma(input);
	for (std::vector<std::string>::iterator paramIt = input.getParam().begin(); paramIt != input.getParam().end(); ++paramIt) {
		std::vector<Channel>::iterator channelIt = searchForChannel(*paramIt, channels);
		if (channelIt == channels.end())
			return (client.sendMsg(SERVER " " ERR_NOSUCHCHANNEL " " + client.getNickname() + " " + *paramIt + " :No such channel\r\n"));
		std::vector<Client>::iterator clientIt = channelIt->searchForUser(client.getNickname(), channelIt->getClients());
		if (clientIt == channelIt->getClients().end())
			return (client.sendMsg(SERVER " " ERR_NOTONCHANNEL " " + client.getNickname() + " " + channelIt->getChannelName() + " :You're not in the channel\r\n"));
		std::vector<Client>::iterator targetIt = channelIt->searchForUser(client.getNickname(), channelIt->getClients());
		if (targetIt == channelIt->getClients().end())
			return (client.sendMsg(SERVER " " ERR_NOSUCHNICK " " + client.getNickname() + " " + *paramIt + " :No such nickname\r\n"));
		if (!input.getTrailing().empty())
			message = ":" + client.getNickname() + " PART " + channelIt->getChannelName() + " :" + input.getTrailing() + "\r\n";
		else
			message = ":" + client.getNickname() + " PART " + channelIt->getChannelName() + "\r\n";
		forwardMsg(message, client, channelIt->getClients(), INCLUDE);
		if (targetIt->getSocketfd() == channelIt->getOwner().getSocketfd())
			channelIt->deleteOwner(-2);
		channelIt->getClients().erase(targetIt);
		targetIt = channelIt->searchForUser(client.getNickname(), channelIt->getInviteList());
		if (targetIt != channelIt->getInviteList().end())
			channelIt->getInviteList().erase(targetIt);
		targetIt = channelIt->searchForUser(client.getNickname(), channelIt->getOperator());
		if (targetIt != channelIt->getOperator().end())
			channelIt->getOperator().erase(targetIt);
		if (channelIt->getClients().size() == 0)
			channels.erase(channelIt);
	}
	return ;
}

void Commands::forwardMsg(std::string message, Client target, std::vector<Client> clients, bool shouldInclude) {
	if (shouldInclude == INCLUDE) {
		for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
			it->sendMsg(message);
	}
	else {
		for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
			if (it->getSocketfd() != target.getSocketfd())
				it->sendMsg(message);
		}
	}
	return ;

}

void Commands::privmsg( Parser &input, Client client, std::vector<Client> connections, std::vector<Channel> channels) {
	std::string message;
	splitByComma(input);

	for (std::vector<std::string>::iterator paramIt = input.getParam().begin(); paramIt != input.getParam().end(); ++paramIt) {
		if (paramIt->at(0) == '#') {
			std::vector<Channel>::iterator channelIt = searchForChannel(*paramIt, channels);
			if (channelIt != channels.end()) {
				std::vector<Client>::iterator clientIt = channelIt->searchForUser(client.getNickname(), channelIt->getClients());
				message = ":" + client.getNickname() + " PRIVMSG " + channelIt->getChannelName() + " :" + input.getTrailing() + "\r\n";
				if (clientIt != channelIt->getClients().end())
					forwardMsg(message, client, channelIt->getClients(), EXCLUDE);
				else if (channelIt->getMode()['i'] == true || channelIt->getMode()['k'] == true)
					client.sendMsg(ERR_CANNOTSENDTOCHAN " " + *paramIt + " :No pemissions\r\n");
				else
					forwardMsg(message, client, channelIt->getClients(), EXCLUDE);
			}
			else
				client.sendMsg(ERR_NOSUCHCHANNEL " " + *paramIt + " :No such channel\r\n");
		}
		else {
			std::vector<Client>::iterator targetIt;
			for (targetIt = connections.begin(); targetIt != connections.end(); ++targetIt) {
				if (targetIt != connections.end() && targetIt->getNickname() == *paramIt) {
					targetIt->sendMsg(":" + client.getNickname() + " PRIVMSG " + *paramIt + " :" + input.getTrailing() + "\r\n");
					break ;
				}
			}
			if (targetIt == connections.end())
				client.sendMsg(SERVER " " ERR_NOSUCHNICK " " + client.getNickname() + " " + *paramIt + " :No such nickname\r\n");
		}

	}
	return ;
}

void Commands::quit( Parser &input, Client client, std::vector<Channel> &channels) {
	std::vector<Client>::iterator targetIt;

	for (std::vector<Channel>::iterator channelIt = channels.begin(); channelIt != channels.end(); ++channelIt) {
		targetIt = channelIt->searchForUser(client.getNickname(), channelIt->getClients());
		if (targetIt != channelIt->getClients().end())
			channelIt->getClients().erase(targetIt);
		targetIt = channelIt->searchForUser(client.getNickname(), channelIt->getInviteList());
		if (targetIt != channelIt->getInviteList().end())
			channelIt->getInviteList().erase(targetIt);
		targetIt = channelIt->searchForUser(client.getNickname(), channelIt->getOperator());
		if (targetIt != channelIt->getOperator().end())
			channelIt->getOperator().erase(targetIt);
		if (client.getSocketfd() == channelIt->getOwner().getSocketfd())
			channelIt->deleteOwner(-2);
		if (channelIt->getClients().size() == 0) {
			channels.erase(channelIt);
			channelIt--;
		}
		else {
			std::string message = ":" + client.getNickname() + " QUIT :" + input.getTrailing() + "\r\n";
			forwardMsg(message, client, channelIt->getClients(), INCLUDE);
		}
	}
	return ;
}

void Commands::mode(Parser &input, Client client , std::vector<Channel> &channels) {
	std::string modeLine = input.getParam()[1];
	bool sign = true;
	char mode;

	std::vector<Channel>::iterator channelIt = searchForChannel(input.getParam()[0], channels);
	if (channelIt == channels.end())
		return (client.sendMsg(SERVER " " ERR_NOSUCHCHANNEL " " + client.getNickname() + " " + input.getParam()[0] + " : No such channel\r\n"));
	std::vector<Client>::iterator clientIt = channelIt->searchForUser(client.getNickname(), channelIt->getOperator());
	if (clientIt == channelIt->getClients().end())
		return (client.sendMsg(SERVER " " ERR_NOTONCHANNEL " " + client.getNickname() + " " + channelIt->getChannelName() + " :You're not in the channel\r\n"));
	if (input.getParam().size() == 1)
		return (client.sendMsg(SERVER " " RPL_CHANNELMODEIS " " + client.getNickname() + " " + channelIt->getChannelName() + " +" + channelIt->getModeString() + "\r\n"));
	std::vector<Client>::iterator operatorIt = channelIt->searchForUser(client.getNickname(), channelIt->getOperator());
	if (operatorIt == channelIt->getOperator().end())
		return (client.sendMsg(SERVER " " ERR_CHANOPRIVSNEEDED " " + channelIt->getChannelName() + " :Channel privileges needed\r\n")); //
	if (input.getParam()[1].at(0) != '-' && input.getParam()[1].at(0) != '+')
		return (client.sendMsg(SERVER " " ERR_UNKNOWNMODE " " + client.getNickname() + " " + channelIt->getChannelName() + " :Unknown mode " + input.getParam()[1].at(0) + "\r\n"));

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
			if (modeIt == modeFuntion.end())
				client.sendMsg(SERVER " " ERR_UNKNOWNMODE " " + client.getNickname() + " " + channelIt->getChannelName() + " :Unknown mode " + mode + "\r\n");
			else 
				modeIt->second(sign, *channelIt, input.getParam().size() == 3 ? input.getParam()[2] : std::string(), client);
		}
	}
	return ;
}

void Commands::executeInvite( bool sign, Channel &channel, std::string param, Client client ) {
	channel.getMode()['i'] = sign;
	std::string message;

	if (sign) {
		message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " +i\r\n";
		std::cout << BLACK "[Server]: " DARK_GRAY "Channel " MAGENTA << channel.getChannelName() << DARK_GRAY " was set to: " LIGHT_GREEN "+i" RESET "\n" << std::endl;
	}
	else {
		message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " -i\r\n";
		std::cout << BLACK "[Server]: " DARK_GRAY "Channel " MAGENTA << channel.getChannelName() << DARK_GRAY " was set to: " LIGHT_RED "-i" RESET "\n" << std::endl;
	}
	forwardMsg(message, client, channel.getClients(), INCLUDE);
	(void)param;
	return ;
}

void Commands::executeKey( bool sign, Channel &channel, std::string key, Client client ) {
	std::string message;

	if (sign) {
		if (!key.empty()) {
			channel.getMode()['k'] = sign;
			channel.setPassword(key);
			message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " +k " + key + "\r\n";
			std::cout << BLACK "[Server]: " DARK_GRAY "Channel " MAGENTA << channel.getChannelName() << DARK_GRAY " was set to: " LIGHT_GREEN "+k" RESET "\n" << std::endl;
		}
		else
			return (client.sendMsg(SERVER " " ERR_NEEDMOREPARAMS " " + client.getNickname() + " " + channel.getChannelName() + " :Not enough Parameters for +k\r\n"));
	}
	else {
		channel.getMode()['k'] = sign;
		channel.setPassword("");
		message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " -k\r\n";
		std::cout << BLACK "[Server]: " DARK_GRAY "Channel " MAGENTA << channel.getChannelName() << DARK_GRAY " was set to: " LIGHT_RED "-k" RESET "\n" << std::endl;
	}
	forwardMsg(message, client, channel.getClients(), INCLUDE);
	return ;
}

void Commands::executeOperator( bool sign, Channel &channel, std::string targetName, Client client ) {
	std::string message;
	if (targetName.empty())
		return (client.sendMsg(SERVER " " ERR_NEEDMOREPARAMS " " + client.getNickname() + " " + channel.getChannelName() + " :Not enough Parameters for +/-o\r\n"));
	std::vector<Client>::iterator clientIt = channel.searchForUser(targetName, channel.getClients());
	if (clientIt == channel.getClients().end())
		return (client.sendMsg(SERVER " " ERR_NOSUCHNICK " " + client.getNickname() + " " + channel.getChannelName() + " : No such nickname\r\n"));
	std::vector<Client>::iterator targetIt = channel.searchForUser(targetName, channel.getOperator());
	if (sign) {
		if (targetIt == channel.getOperator().end()) {
			channel.getOperator().push_back(*clientIt);
			message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " +o " + targetName + "\r\n";
			forwardMsg(message, client, channel.getClients(), INCLUDE);
			std::cout << BLACK "[Server]: " DARK_GRAY "Channel " MAGENTA << channel.getChannelName() << ": " << ORANGE << clientIt->getNickname() << DARK_GRAY " was " LIGHT_GREEN "promoted " DARK_GRAY "to operator by " ORANGE << client.getNickname() <<  RESET "\n" << std::endl;
		}
	}
	else {
		if (targetIt != channel.getOperator().end()) {
			if (targetIt->getSocketfd() == channel.getOwner().getSocketfd() &&  client.getSocketfd() != channel.getOwner().getSocketfd())
				client.sendMsg(SERVER " " ERR_NOPRIVLIEGES " " + client.getNickname() + " " + channel.getChannelName() + " :You can't demote yourself as the channel owner\r\n");
			else {
				channel.getOperator().erase(targetIt);
				message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " -o " + targetName + "\r\n";
				forwardMsg(message, client, channel.getClients(), INCLUDE);
				std::cout << BLACK "[Server]: " DARK_GRAY "Channel " MAGENTA << channel.getChannelName() << ": " << ORANGE << clientIt->getNickname() << DARK_GRAY " got " LIGHT_RED "demoted" DARK_GRAY " by " << client.getNickname() << RESET "\n" << std::endl;
			}
		}
	}
	return ;
}

void Commands::executeLimit( bool sign, Channel &channel, std::string channelLimit, Client client ) {
	std::string message;

	if (sign) {
		if (!channelLimit.empty()) {
			channel.getMode()['l'] = sign;
			if (channelLimit.find_first_not_of("0123456789") == std::string::npos) {
				int limit = static_cast<int>( strtod(channelLimit.c_str(), NULL) );
				int overflowCheck;

				std::stringstream ss(channelLimit);
				ss >> overflowCheck;
				if (ss.fail() || !ss.eof())
					return;
				else if (limit < 1)
					return;
				else {
					channel.setLimit(limit);
					message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " +l\r\n";
					std::cout << BLACK "[Server]: " DARK_GRAY "Channel " MAGENTA << channel.getChannelName() << DARK_GRAY " was set to: " LIGHT_GREEN "+l " << limit << RESET "\n" << std::endl;
				}
			}
		}
		else
			return (client.sendMsg(SERVER " " ERR_NEEDMOREPARAMS " " + client.getNickname() + " " + channel.getChannelName() + " :Not enough Parameters for +l\r\n"));
	}
	else {
		channel.getMode()['l'] = sign;
		message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " -l\r\n";
		std::cout << BLACK "[Server]: " DARK_GRAY "Channel " MAGENTA << channel.getChannelName() << DARK_GRAY " was set to: " LIGHT_RED "-l" << RESET "\n" << std::endl;
	}
	forwardMsg(message, client, channel.getClients(), INCLUDE);
	return ;
}

void Commands::executeTopic( bool sign, Channel &channel, std::string param, Client client ) {
	channel.getMode()['t'] = sign;
	std::string message;

	if (sign) {
		message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " +t\r\n";
		std::cout << BLACK "[Server]: " DARK_GRAY "Channel " MAGENTA << channel.getChannelName() << DARK_GRAY " was set to: " LIGHT_GREEN "+t" << RESET "\n" << std::endl;
	}
	else {
		message = ":" + client.getNickname() + " MODE " + channel.getChannelName() + " -t\r\n";
		std::cout << BLACK "[Server]: " DARK_GRAY "Channel " MAGENTA << channel.getChannelName() << DARK_GRAY " was set to: " LIGHT_RED "-t" << RESET "\n" << std::endl;
	}
	forwardMsg(message, client, channel.getClients(), INCLUDE);
	(void)param;
	return ;
}

void Commands::sendWelcomeMessage(Client client, std::vector<Channel>::iterator channelIt) {
	std::string message;
	if (channelIt->getTopic().empty())
		client.sendMsg(SERVER " " RPL_NOTOPIC " " + client.getNickname() + " " + channelIt->getChannelName() + " :No topic set\r\n");
	else
		client.sendMsg(SERVER " " RPL_TOPIC " " + client.getNickname() + " " + channelIt->getChannelName() + " :" + channelIt->getTopic() + "\r\n");
	message = ":" + client.getNickname() + " JOIN " + channelIt->getChannelName() + "\r\n";
	forwardMsg(message, client, channelIt->getClients(), EXCLUDE);
	message = SERVER " " RPL_NAMREPLY " " + client.getNickname() + " " + channelIt->getChannelName() + " :";
	for (std::vector<Client>::iterator clientIt = channelIt->getClients().begin(); clientIt != channelIt->getClients().end(); ++clientIt) {
		std::vector<Client>::iterator operatorIt = channelIt->searchForUser(clientIt->getNickname(), channelIt->getOperator());
		if (operatorIt != channelIt->getOperator().end())
			message += "@";
		message += clientIt->getNickname() + " ";
	}
	message += "\r\n";
	client.sendMsg(message);
	client.sendMsg(SERVER " " RPL_ENDOFNAMES " " + client.getNickname() + " " + channelIt->getChannelName() + " :End of /NAMES list\r\n");
	client.sendMsg(SERVER " MODE " + channelIt->getChannelName() + " +" + channelIt->getModeString() + "\r\n");
	return ;
}

void Commands::splitByComma( Parser &input ) {
	std::vector<std::string> newParam;
	std::vector<std::string> password;
	size_t size = input.getParam().size();

	while (!input.getParam()[0].empty()) {
		size_t pos = input.getParam()[0].find(",");
		if (pos != std::string::npos) {
			newParam.push_back(input.getParam()[0].substr(0, pos));
			input.getParam()[0].erase(0, pos + 1);
		}
		else if (!input.getParam()[0].empty()) {
			newParam.push_back(input.getParam()[0]);
			input.getParam()[0].erase(0, input.getParam()[0].length());
		}
	}
	if (size == 2) {
		while (!input.getParam()[1].empty()) {
			size_t pos = input.getParam()[1].find(",");
			if (pos != std::string::npos) {
				password.push_back(input.getParam()[1].substr(0, pos));
				input.getParam()[1].erase(0, pos + 1);
			}
			else if (!input.getParam()[1].empty()) {
				password.push_back(input.getParam()[1]);
				input.getParam()[1].erase(0, input.getParam()[1].length());
			}
		}
	}
	input.setParam(newParam);
	input.setPassword(password);
	return ;
}

void Commands::join(Parser &input, Client client, std::vector<Channel> &channels){
	splitByComma(input);
	for (std::vector<std::string>::iterator ParamIt = input.getParam().begin(); ParamIt != input.getParam().end(); ++ParamIt) {
		if (ParamIt->at(0) == '#') {
			if (ParamIt->length() < 1 || ParamIt->length() > 199 ) {
				client.sendMsg(SERVER " " ERR_NOSUCHCHANNEL " " + client.getNickname() + " " + *ParamIt + " :Invalid channel name\r\n");
				continue ;
			}
			std::vector<Channel>::iterator channelIt = searchForChannel( *ParamIt, channels);
			if (channelIt == channels.end()) {
				channels.push_back(Channel ( *ParamIt, client ));
				client.sendMsg(":" + client.getNickname() + " JOIN " + *ParamIt + "\r\n");
				channelIt = searchForChannel( *ParamIt, channels);
			} 
			else {
				if (channelIt->canUserJoin( client, input )) {
					channelIt->addUser( client );
					std::vector<Client>::iterator clientIt = channelIt->searchForUser(client.getNickname(), channelIt->getInviteList());
					if (clientIt != channelIt->getInviteList().end())
						channelIt->getInviteList().erase(clientIt);
					client.sendMsg(":" + client.getNickname() + " JOIN " + *ParamIt + "\r\n");
				}
				else
					continue ;
			}
			sendWelcomeMessage(client, channelIt);
		}
		else
			client.sendMsg(SERVER " " ERR_NOSUCHCHANNEL " " + client.getNickname() + " " + *ParamIt + " :No such channel\r\n");
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
		client.sendMsg(SERVER " " ERR_NICKNAMEINUSE " " + nickname + " :Nickname is already in use. Please choose a different nickname.\r\n");
		return false;
	} else if (nickname.size() > 10) {
		client.sendMsg(SERVER " " ERR_ERRONEUSNICKNAME " " + nickname + " :Nickname is too long. Please choose a shorter nickname.\r\n");
		return false;
	} else if (!isAlphaNumeric(nickname)) {
		client.sendMsg(SERVER " " ERR_ERRONEUSNICKNAME " " + nickname + " :Nickname contains invalid symbols. Only use letters and numbers.\r\n");
		return false;
	}
	return true;
}

void Commands::nick(Parser& input, Client& client, std::vector<Client>& connections) {
	if (!checkNickname(client, input.getParam()[0], connections)) {
		return ;
	} else {
		std::string oldNickname = client.getNickname();
		client.setNickname(input.getParam()[0]);
		if (!oldNickname.empty()) {
			std::string nameChange = ":" + oldNickname + " NICK " + client.getNickname() + "\r\n";
			forwardMsg(nameChange, client, connections, INCLUDE);
		}
	}
	return ;
}

bool isUsernameAvailable(const std::vector<Client>& connections, const std::string& username) {
	if (username.empty())
		return true;
	for (std::vector<Client>::size_type i = 0; i < connections.size(); i++) {
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

void Commands::user(Parser& input, Client& client) {
	if (!isNameValid(client.getConstUsername()))
		return;
	if (!client.getUsername().empty())
		return (client.sendMsg(SERVER " " ERR_ALREADYREGISTRED " " + client.getConstUsername() + " :Username is already in use. Please choose a different username.\r\n"));
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
				client.sendMsg(SERVER " " ERR_CHANOPRIVSNEEDED " " + nickname + " " + channelName + " :You're not channel operator\r\n");
				return false;
			}
			return true;
		}
	}
	client.sendMsg(SERVER " " ERR_NOSUCHCHANNEL " " + nickname + " " + channelName + " :No such channel\r\n");
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
	client.sendMsg(":" + client.getNickname() + " " + RPL_INVITING " " + nickname + " " + channelName + "\r\n");
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
				client.sendMsg(SERVER " " ERR_USERONCHANNEL " " + invitedPerson + " " + channelName + " :is already on channel\r\n");
				return true;
			}
		}
	}
	return false;
}

void Commands::invite(Client& client, Parser& input, std::vector<Client> &connections, std::vector<Channel> &channels) {
	if (!checkPermission(client, input.getParam()[1], client.getNickname(), channels))
		return;
	if (invitedPersonIsOnChannel(client, channels, input.getParam()[1], input.getParam()[0]))
		return;
	if (!checkInvitedPerson(connections, input.getParam()[0]))
		return (client.sendMsg(SERVER " " ERR_NOSUCHNICK " " + client.getNickname() + " " + input.getParam()[1] + " :No such nickname\r\n"));
	sendInvitation(client, input.getParam()[0], input.getParam()[1], channels, connections);
}
