#ifndef CHANNEL_H
# define CHANNEL_H

# include <exception>
# include <string>
# include "Client.hpp"

class Channel {

public:

	Channel( std::string Name );
	~Channel( void );

	void LeaveChannel( std::string UserName );
	void AddUser( Client User );
	void SetSettings( void );
	int SearchforUser( Client User );

	std::string getChannelName( void );
	bool CanUserJoin( Client User );
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
	// ChannelOP - kp was für typ das sein soll vllt, einfach ein client
	// Founder ??

};

#endif