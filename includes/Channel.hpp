#ifndef CHANNEL_H
# define CHANNEL_H

# include <exception>
# include <string>
# include "client.hpp"
# include <vector>

class Channel {

public:

	Channel( std::string Name, Client User );
	~Channel( void );

	void LeaveChannel( std::string UserName );
	void AddUser( Client User );
	void SetSettings( void );
	int SearchforUser( Client User );

	void setTopic( std::string Topic );
	std::string getTopic( void );
	std::string getChannelName( void );
	bool CanUserJoin( Client User );
	void setFounder( Client &Founder );

	class ChannelFailException : std::exception {
	
	public:

		ChannelFailException( std::string Error );
		~ChannelFailException( void ) throw();
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