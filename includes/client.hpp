#ifndef CLIENT_H
# define CLIENT_H

# include <exception>
# include <string>

class Client {

public:

	Client( int SocketID );
	~Client( void );

	std::string getUsername( void );
	std::string getNickname( void );
	bool getAuthentication( void );
	void setAuthentication( bool Authentication );
	void setNickname( std::string Nickname );
	void setUsername( std::string Username );
	int getSocketID( void );

	class ClientFailException : std::exception {
	
	public:

		ClientFailException( std::string Error );
		~ClientFailException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:

	std::string _username;
	std::string _nickname;
	int _SocketID;
	bool IsAuthenticated;

};

#endif