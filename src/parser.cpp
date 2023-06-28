#include "Parser.hpp"
#include <string>
#include <iostream>

	/*2.3.1 Message format in 'pseudo' BNF

  	 The protocol messages must be extracted from the contiguous stream of
  	 octets.  The current solution is to designate two characters, CR and
  	 LF, as message separators.   Empty  messages  are  silently  ignored,
  	 which permits  use  of  the  sequence  CR-LF  between  messages
  	 without extra problems.

  	 The extracted message is parsed into the components <prefix>,
  	 <command> and list of parameters matched either by <middle> or
  	 <trailing> components.

  	 The BNF representation for this is:


	<message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
	<prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
	<command>  ::= <letter> { <letter> } | <number> <number> <number>
	<SPACE>    ::= ' ' { ' ' }
	<params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]

	<middle>   ::= <Any *non-empty* sequence of octets not including SPACE
               or NUL or CR or LF, the first of which may not be ':'>
	<trailing> ::= <Any, possibly *empty*, sequence of octets not including
                 NUL or CR or LF>

	<crlf>     ::= CR LF*/

Parser::Parser( std::string buffer ) {
	size_t pos = buffer.find("\r\n");
	if (pos != std::string::npos ) {
		this->_input = buffer.substr(0, pos);
		parseMsg();
	}
	else
		std::cerr << "Parsing Error" << std::endl;
	return ;
}

std::string Parser::getCMD( void ) {
	return (this->_command);
}

std::vector<std::string> Parser::getParam( void ) {
	return (this->_parameter);
}

Parser::~Parser( void ) {
	return ;
}

void Parser::isValidCommandLine( void ) {
	
	return ;
}

void Parser::parseMsg( void ) {
	if (this->_input[0] == ':')
		prefixHandler(this->_input.substr(1, this->_input.find_first_of(' ')));
	commandHandler(this->_input.substr(0, this->_input.find_first_of(' ')));
	if (this->_input[0] != ':')
		paramHandler(this->_input.substr(0, this->_input.find_first_of("\r\n") ));
	else
		trailingHandler(this->_input.substr(1, this->_input.find_first_of("\r\n")));
	isValidCommandLine();
	return ;
}

void Parser::prefixHandler( std::string prefix ) {
	this->_prefix = prefix;
	this->_input.erase(0, this->_input.find_first_of(' ') + 1);
}

void Parser::commandHandler( std::string command ) {
	this->_command = command;
	this->_input.erase(0, this->_input.find_first_of(' ') + 1);
}

void Parser::paramHandler( std::string param ) {
	size_t found = 0; 

	while (!param.empty()) {
		if (param == "\r\n")
			return ;
		found = param.find_first_of(": ");
		if (found != std::string::npos && param[found] == ':') {
			trailingHandler(param.substr(1, param.find("\r\n")));
			return ;
		}
		else if (found == std::string::npos) {
			this->_parameter.push_back(param.substr(0, param.find("\r\n")));
			return ;
		}
		else
			this->_parameter.push_back(param.substr(0, found));
		param.erase(0, found + 1);
	}
}

void Parser::trailingHandler( std::string trailing ) {
	this->_trailing = trailing;
	return ;
}
