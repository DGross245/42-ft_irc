#ifndef CLIENT_H
# define CLIENT_H

# include <string>

class Client {

public:

	Client( int socketfd );
	~Client( void );

	int			Authentication( std::string CMD );
	void		sendMsg( std::string message );

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

private:

	std::string	_username;
	std::string	_nickname;
	int			_socketfd;
	bool		_isAuthenticated;
	bool 		_passwordAccepted;

};

#endif
