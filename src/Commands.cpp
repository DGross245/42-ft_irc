# include <exception>
# include <iostream>
# include <string>
# include <vector>
# include <sys/types.h>
# include <Channel.hpp>
# include <Parser.hpp>
# include <sys/time.h>
#include "Commands.hpp"
#include <sys/socket.h>

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

static void joinChannel( std::string channelName, Client user, std::vector<Channel> channels) {
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

void Commands::join(Parser &input, Client client, std::vector<Channel> channels){
	// std::cout << "join command called" << std::endl;
	std::string joinMessageClient = ":jschneid JOIN #test\r\n";
	std::string switchBuffer = "/buffer #test\r\n";
	joinChannel(input.getParam()[0], client, channels);
	send(client.getSocketfd(), joinMessageClient.c_str(), joinMessageClient.length(), 0);
	send(client.getSocketfd(), switchBuffer.c_str(), switchBuffer.length(), 0);
}

Commands::commandFailException::~commandFailException( void ) throw() { return ;	}
Commands::commandFailException::commandFailException( std::string error ) : _error(error) { return ; }
const char *Commands::commandFailException::what() const throw() { return (this->_error.c_str());}
