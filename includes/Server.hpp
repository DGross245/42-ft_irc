#ifndef SERVER_H
# define SERVER_H

# include <exception>
# include <string>
# include <vector>

class Server {

public:

	Server( std::string Port, std::string Password );
	~Server( void );
	void	InitServer( void );

	int	getPort( void );
	int	getPassword( void );
	void setPort( std::string &Port );
	void setPassword( std::string &Password );
	void ClientIOHandler( int ServerSocketfd );
	class ServerFailException : std::exception {
	
	public:

		ServerFailException( std::string Error );
		~ServerFailException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:

	//std::vector<int> *connections;
	int _port;
	int _password;

};

#endif