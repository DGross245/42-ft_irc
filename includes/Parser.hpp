#ifndef PARSER_H
# define PARSER_H

# include <exception>
# include <string>
# include <vector>

class Parser {

public:

	Parser( std::string Buffer );
	~Parser( void );

	void parseMsg( void );
	void prefixHandler( std::string Prefix );
	void commandHandler( std::string Command );
	void paramHandler( std::string Param );
	void trailingHandler( std::string trailing );

	std::string getCMD( void );
	std::vector<std::string> getParam( void );

private:

	std::string _input;
	std::string _prefix;
	std::string _command;
	std::vector<std::string> _parameter;
	std::string _trailing;
};

#endif