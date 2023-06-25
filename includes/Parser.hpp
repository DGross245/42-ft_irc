#ifndef PARSER_H
# define PARSER_H

# include <exception>
# include <string>

class Parser {

public:

	Parser( std::string Buffer );
	~Parser( void );

	void ParseMsg( void );

private:

	std::string _input;
	std::string _parameter;
	std::string _command;

};

#endif