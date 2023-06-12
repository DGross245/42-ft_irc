#ifndef FT_IRC_H
# define FT_IRC_H

# include <exception>
# include <string>

class IRC {

public:

	IRC( std::string Port, std::string Password );
	~IRC( void );
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