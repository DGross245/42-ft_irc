#ifndef SERVER_H
# define SERVER_H

# include <exception>
# include <string>
# include <vector>
# include <sys/types.h>
# include <Channel.hpp>
# include <Parser.hpp>
# include <sys/time.h>

class Server {

public:

	Server( std::string port, std::string password );
	~Server( void );

	void	initServer( void );
	void	closeALLConnections( void );
	void	clientIOHandler( void );
	void 	addClient( int serverSocketfd, fd_set &readfds );
	void	readMsg( Client client, int i);
	void	executeMsg( Parser &input, Client client );
	int		searchForChannel( std::string channelName );
	void	joinChannel( std::string channelName , Client user );
	void	launchServer( void );

	int						getPort( void );
	int						getMaxfd( fd_set &readfds );
	void					setPort( int port );
	void					setPassword( std::string &password );
	void					setTime( void );
	void					setServerfd( int serverSocketfd );
	std::string				getPassword( void );
	std::vector<Client>		&getConnections( void );
	std::vector<Channel>	&getChannels(void);

	class serverFailException : std::exception {

	public:

		serverFailException( std::string error );
		~serverFailException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:

	std::vector<Client> _connections;
	std::vector<Channel> _channel; // how many channels on the server
	int _serverfd;
	int _port;
	std::string _password;
	struct timeval _tv;
};

#endif
