#ifndef SERVER_H
# define SERVER_H

# include <exception>
# include <string>

class Server {

public:

	Server( std::string Port, std::string Password );
	~Server( void );
	void	InitServer( std::string Port, std::string Password );

	class ServerFailException : std::exception {
	
	public:

		ServerFailException( std::string Error );
		~ServerFailException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:



};

#endif