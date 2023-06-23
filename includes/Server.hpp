#ifndef SERVER_H
# define SERVER_H

# include <exception>
# include <string>
# include <vector>
# include <sys/types.h>
# include <Channel.hpp>

class Server {

public:

	Server( std::string Port, std::string Password );
	~Server( void );

	void	InitServer( void );
	void	CloseConnection( void );
	int		getPort( void );
	int		getPassword( void );
	void	setPort( std::string &Port );
	void	setPassword( std::string &Password );
	void	ClientIOHandler( int ServerSocketfd );
	void 	AddClient( int ServerSocketfd );
	void	ReadMsg( int client, fd_set rfds, int i);
	void	ParseMsg( std::string Buffer );
	void	ExecuteMsg( void );
	int		SearchForChannel( std::string ChannelName );
	void	JoinChannel( std::string ChannelName , Client User );

	class ServerFailException : std::exception {
	
	public:

		ServerFailException( std::string Error );
		~ServerFailException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:

	std::vector<int> _connections;
	std::vector<Channel> _channel;
	int _port;
	int _password;

};

#endif