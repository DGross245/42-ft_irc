#ifndef CLIENT_H
# define CLIENT_H

# include <exception>
# include <string>

class IRC {

public:


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