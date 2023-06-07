#ifndef FT_IRC_H
# define FT_IRC_H

# include <exception>
# include <string>

class IRC {

public:


	class ServerFailException : std::exception {
	
	public:

		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:



};

#endif