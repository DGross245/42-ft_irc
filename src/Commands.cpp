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
	// std::cout << "join command called" << std::endl;
	std::string joinMessageClient = ":dgross JOIN #test\r\n";
	std::string switchBuffer = "/buffer #test\r\n";
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
		std::cout << "There are not allowed symbols in the username" << std::endl;
		return;
	}
	else if (!isUsernameAvailable(connections, input.getTrailing())) {
		std::cout << "There is already a client with the same username" << std::endl;
		return;
	}
	client.setUsername(input.getTrailing());
	std::cout << "Set the username to: " << client.getUsername() << std::endl;
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

// void Commands::invite(Client& client){
// 	std::cout << "invite command" << std::endl;
// }

Commands::commandFailException::~commandFailException( void ) throw() { return ;	}
Commands::commandFailException::commandFailException( std::string error ) : _error(error) { return ; }
const char *Commands::commandFailException::what() const throw() { return (this->_error.c_str());}
