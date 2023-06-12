#ifndef CLIENT_H
# define CLIENT_H

# include <exception>
# include <string>

class IRC {

public:


	class ClientFailException : std::exception {
	
	public:

		ClientFailException( std::string Error );
		~ClientFailException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:



};

#endif