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

	Server( std::string Port, std::string Password );
	~Server( void );

	void	InitServer( void );
	void	CloseALLConnections( void );
	int		getPort( void );
	int		getPassword( void );
	void	setPort( std::string &Port );
	void	setPassword( std::string &Password );
	void	ClientIOHandler( void );
	void 	AddClient( int ServerSocketfd, fd_set &readfds );
	void	ReadMsg( int client, int i);
	void	ExecuteMsg( Parser &Input, int Client );
	int		SearchForChannel( std::string ChannelName );
	void	JoinChannel( std::string ChannelName , Client User );
	void	launchServer( void );
	void	setTime( void );
	int		getmaxfd( fd_set &readfds );
	void	setServerID( int ServerSocketfd );
	
	class ServerFailException : std::exception {
	
	public:

		ServerFailException( std::string Error );
		~ServerFailException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:

	std::vector<Client> _connections;
	std::vector<Channel> _channel;
	int _serverID;
	int _port;
	int _password;
	struct timeval _tv;
};

#endif