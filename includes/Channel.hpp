#ifndef CHANNEL_H
# define CHANNEL_H

# include <exception>
# include <string>

class Channel {

	Channel( std::string Name );
	~Channel( void );
	
public:

	class ChannelFailException : std::exception {
	
	public:

		ChannelFailException( std::string Error );
		~ChannelFailException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:

	

};

#endif