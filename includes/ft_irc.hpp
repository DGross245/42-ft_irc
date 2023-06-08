#ifndef FT_IRC_H
# define FT_IRC_H

# include <exception>
# include <string>

class IRC {

public:

	void	InitServer( void );
	IRC( void );
	~IRC( void );
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