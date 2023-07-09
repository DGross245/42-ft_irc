#ifndef PARSER_H
# define PARSER_H

# include <exception>
# include <string>
# include <vector>
# include "Client.hpp"

class Parser {

public:

	Parser( std::string &buffer, Client client );
	~Parser( void );

	void parseMsg( Client client );
	void prefixHandler( std::string Prefix );
	void commandHandler( std::string Command );
	void paramHandler( std::string Param );
	void trailingHandler( std::string trailing );
	void isValidCommandLine( Client client  );
	void sendError( Client client);

	void checkCAP( Client client );
	void checkPASS( Client client );
	void checkNICK( Client client );
	void checkUSER( Client client );
	void checkQUIT( Client client );
	void checkJOIN( Client client );
	void checkMODE( Client client );
	void checkTOPIC( Client client );
	void checkINVITE( Client client );
	void checkKICK( Client client );
	void checkPRIVMSG( Client client );
	void checkPING( Client client );
	void checkPART( Client client );

	std::string &getCMD( void );
	std::vector<std::string> &getParam( void );
	std::string &getPrefix( void );
	std::string &getTrailing( void );

	class parserErrorException : std::exception {

	public:

		parserErrorException( std::string error );
		~parserErrorException( void ) throw();
		virtual const char *what() const throw();

	private:

		std::string _error;

	};

private:

	std::string					_input;
	std::string					_prefix;
	std::string					_command;
	std::vector<std::string>	_parameter;
	std::string 				_trailing;
};

#endif
