#ifndef CLIENT_H
# define CLIENT_H

# include <exception>
# include <string>

class Client {

public:

	Client( int socketfd );
	~Client( void );

	std::string getUsername( void );
	std::string getNickname( void ) const;
	bool getAuthentication( void );
	void setAuthentication( bool authentication );
	void setNickname( std::string nickname );
	void setUsername( std::string username );
	int getSocketfd( void );

	int Authentication( void );
	class clientFailException : std::exception {

	public:

		clientFailException( std::string error );
		~clientFailException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:

	std::string _username;
	std::string _nickname;
	int _socketfd;
	bool _isAuthenticated;

};

#endif
