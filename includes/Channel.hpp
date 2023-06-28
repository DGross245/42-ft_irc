#ifndef CHANNEL_H
# define CHANNEL_H

# include <exception>
# include <string>
# include "Client.hpp"
# include <vector>

class Channel {

public:

	Channel( std::string name, Client user );
	~Channel( void );

	void leaveChannel( std::string username );
	void addUser( Client user );
	void setSettings( void );
	int searchforUser( Client user );

	void setTopic( std::string topic );
	std::string getTopic( void );
	std::string getChannelName( void );
	bool canUserJoin( Client user );
	void setFounder( Client &Founder );

	class channelFailException : std::exception {
	
	public:

		channelFailException( std::string error );
		~channelFailException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:

	std::string _name;
	std::vector<Client> _clients;
	std::vector<Client> _invited;
	std::string _topic;
	bool _isTopicRestricted;
	bool _isInviteOnly;
	std::string _password;
	std::vector<Client> _op; // vlt doch kein vector aber mal gucken
	Client _founder;

};

#endif