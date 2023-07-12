#ifndef CLIENT_H
# define CLIENT_H

# include <exception>
# include <string>

class Client {

public:

	Client( int socketfd );
	~Client( void );

	int			Authentication( std::string CMD );

	bool		getAuthentication( void );
	int			getSocketfd( void );
	std::string getUsername( void ) ;
	std::string getConstUsername(void) const;
	std::string getNickname( void ) const;
	bool		getPasswordAccepted( void );

	void		setAuthentication( bool authentication );
	void		setNickname( std::string nickname );
	void		setUsername( std::string username );
	void		setPasswordAccepted( bool status );

	class clientFailException : std::exception {

	public:

		clientFailException( std::string error );
		~clientFailException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:

	std::string	_username;
	std::string	_nickname;
	int			_socketfd;
	bool		_isAuthenticated;
	bool 		_passwordAccepted;
};

#endif
