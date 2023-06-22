#ifndef CHANNEL_H
# define CHANNEL_H

# include <exception>
# include <string>

class Channel {

public:

	Channel( std::string Name );
	~Channel( void );

	void DeleteUserFormChannel( void );
	void CreateChannel( void );
	void DeleteChannel( void );
	void AddUser( void );
	std::string getChannelName( void );

	class ChannelFailException : std::exception {
	
	public:

		ChannelFailException( std::string Error );
		~ChannelFailException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:

	// channel infos
	std::string _Name;

};

#endif