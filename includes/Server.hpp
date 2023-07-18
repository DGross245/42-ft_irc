#ifndef SERVER_H
# define SERVER_H

# include "Parser.hpp"
# include "Channel.hpp"
# include <exception>
# include <string>
# include <vector>
# include <sys/types.h>
# include <sys/time.h>

class Server {

public:

	Server( std::string port, std::string password );
	~Server( void );

	void					initServer( void );
	void					launchServer( void );
	void					clientIOHandler( void );
	void 					addClient( int serverSocketfd, fd_set &readfds );
	void					readMsg( Client &client, int i);
	void					executeMsg( Parser &input, Client &client );
	void					closeALLConnections( void );
	void					appendBuffer( std::string buffer );
	void					setAppendBuffer( std::string buffer);
	void					setPort( int port );
	void					setPassword( std::string &password );
	void					setTime( void );
	void					setServerfd( int serverSocketfd );
	int						getPort( void );
	int						getMaxfd( fd_set &readfds );
	std::string				&getAppendBuffer( void );
	std::string				getPassword( void );
	std::vector<Client>		&getConnections( void );
	std::vector<Channel>	&getChannels(void);
	int						getServerfd( void);
	
	class serverFailException : std::exception {

	public:

		serverFailException( std::string error );
		~serverFailException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:

	std::vector<Client>		_connections;
	std::vector<Channel>	_channels;
	int						_serverfd;
	int						_port;
	std::string				_password;
	struct timeval			_tv;
	std::string				_appendBuffer;
};

#endif
