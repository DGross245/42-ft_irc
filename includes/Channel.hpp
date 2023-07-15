#ifndef CHANNEL_H
# define CHANNEL_H

# include "Parser.hpp"
# include "Client.hpp"
# include <string>
# include <vector>
# include <map>

class Channel {

public:

	Channel( std::string name, Client user );
	~Channel( void );

	void							addUser( Client user );
	bool							canUserJoin( Client user, Parser &input );
	std::vector<Client>::iterator	searchForUser( std::string nickname, std::vector<Client> &clients );

	void							setOwner( Client owner );
	void							setTopic( std::string topic, Client client );
	void							setMode( std::map<char,bool> mode);
	void							setLimit( size_t i );
	void							setPassword( std::string password );

	size_t							getLimit( void );
	Client							getOwner( void );
	std::string						getTopic( void );
	std::string						getChannelName( void );
	std::string						getPassword( void );
	std::string						getModeString( void );
	std::map<char,bool>				&getMode( void );
	std::vector<Client>				&getClients( void );
	std::vector<Client>				&getInviteList( void );
	std::vector<Client>				&getOperator( void );

private:

	size_t					_limit;
	Client					_owner;
	std::string				_name;
	std::string				_topic;
	std::string				_password;
	std::vector<Client>		_clients;
	std::vector<Client>		_operator;
	std::vector<Client>		_invited;
	std::map<char, bool>	_mode;
	std::string				_modeString;

};

#endif
