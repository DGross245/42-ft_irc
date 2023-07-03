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
		send(client.getSocketfd(), message.c_str(), sizeof(message), 0);
	}
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

// function handleNickCommand(client, newNickname):
//     if newNickname is empty:
//         sendNumericResponse(client, 431, "No nickname given")
//     else if newNickname is invalid:
//         sendNumericResponse(client, 432, newNickname + " :Erroneous nickname")
//     else if newNickname is already in use:
//         sendNumericResponse(client, 433, newNickname + " :Nickname is already in use")
//     else:
//         if client has an existing nickname:
//             oldNickname = client.nickname
//             updateClientNickname(client, newNickname)
//             broadcastNicknameChange(oldNickname, newNickname)
//         else:
//             setClientNickname(client, newNickname)
//             sendWelcomeMessage(client)
// Numeric Response 431: :No nickname given

// This message indicates that you did not provide a nickname after the NICK command. You need to specify a nickname to set or change.
// Numeric Response 432: :<nickname> :Erroneous nickname

// If the nickname you provided is invalid (e.g., contains spaces or special characters), the server will respond with this message.
// Numeric Response 433: :<nickname> :Nickname is already in use

// As mentioned before, this response indicates that the nickname you requested is already in use by another user. You will need to choose a different nickname.
// Numeric Response 436: :<nickname> :Nickname collision KILL

// In case there is a conflict between two users trying to use the same nickname, the server may force one of them to change their nickname to resolve the collision. This response informs you that your nickname has been changed.


void Commands::nick(Parser &input, Client &client){
	std::cout << "nickname right now: " << client.getNickname() << std::endl;
	std::cout << "the nickname is set to: " << input.getParam()[0] << std::endl;
	std::cout << "nickname after: " << client.getNickname() << std::endl;
	client.setNickname(input.getParam()[0]);
}

Commands::commandFailException::~commandFailException( void ) throw() { return ;	}
Commands::commandFailException::commandFailException( std::string error ) : _error(error) { return ; }
const char *Commands::commandFailException::what() const throw() { return (this->_error.c_str());}
