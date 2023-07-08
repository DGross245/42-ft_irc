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

	void setTopic( std::string topic, Client client );
	std::string getTopic( void );
	std::string getChannelName( void );
	std::vector<Client>
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

	bool				_isTopicRestricted; // if true only the founder can change the topic of the channel
	bool				_isInviteOnly;
	Client				_founder; // the founder of the channel
	std::string			_name;
	std::string			_topic; // of the channel
	std::string			_password;
	std::vector<Client>	_clients;
	std::vector<Client>	_invited; //invited clients
	std::vector<Client>	_operatorChannelMembers; // vlt doch kein vector aber mal gucken // clients which got admin rights

};

#endif
