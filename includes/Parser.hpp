#ifndef PARSER_H
# define PARSER_H

# include <exception>
# include <string>
# include <vector>

class Parser {

public:

	Parser( std::string Buffer );
	~Parser( void );

	void ParseMsg( void );
	void PrefixHandler( std::string Prefix );
	void CommandHandler( std::string Command );
	void ParamHandler( std::string Param );
	void TrailingHandler( std::string trailing );

	std::string getCMD( void );
	std::vector<std::string> getParam( void );

private:

	std::string _input;
	std::vector<std::string> _parameter;
	std::string _command;
	std::string _trailing;
	std::string _prefix;
};

#endif