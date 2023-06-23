#ifndef CLIENT_H
# define CLIENT_H

# include <exception>
# include <string>

class Client {

public:

	Client( void );
	~Client( void );

	std::string getUsername( void );
	std::string getNickname( void );
	
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
	bool IsAuthenticated;

};

#endif