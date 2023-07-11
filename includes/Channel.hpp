#ifndef CHANNEL_H
# define CHANNEL_H

# include <exception>
# include <string>
# include "Client.hpp"
# include <vector>
# include <map>
# include "Parser.hpp"

class Channel {

public:

	Channel( std::string name, Client user );
	~Channel( void );

	void addUser( Client user );
	std::vector<Client>::iterator searchForUser( std::string nickname, std::vector<Client> &clients );
	bool canUserJoin( Client user, Parser &input );

	void				setOwner( Client owner );
	void				setTopic( std::string topic, Client client );
	void				setMode( std::map<char,bool> mode);
	void				setLimit( size_t i );
	void				setPassword( std::string password );
	size_t				getLimit( void );
	Client				getOwner( void );
	std::string			getTopic( void );
	std::string			getChannelName( void );
	std::string			getPassword( void );
	std::map<char,bool>	&getMode( void );
	std::vector<Client> &getClients( void );
	std::vector<Client> &getInviteList( void );
	std::vector<Client> &getOP( void );

	class channelFailException : std::exception {

	public:

		channelFailException( std::string error );
		~channelFailException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:

	std::string	_name;
	std::vector<Client> _clients;
	std::map<char, bool> _mode;
	size_t _limit;
	std::string _topic; // of the channel
	std::string _password;
	std::vector<Client> _op;  // clients which got admin rights
	Client _founder; // the founder of the channel
	// bool				_isTopicRestricted; // if true only the founder can change the topic of the channel
	// bool				_isInviteOnly;
	std::vector<Client>	_invited; //invited clients

};

#endif
