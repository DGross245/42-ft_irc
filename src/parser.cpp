#include "Parser.hpp"
#include <string>
#include <iostream>

// gibt vllt noch ein problem mit blocking

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

Parser::Parser( std::string Buffer ) {
	size_t pos = Buffer.find("\r\n");
	if (pos != std::string::npos ) {
		this->_input = Buffer.substr(0, pos);
		ParseMsg();
		std::cout << "Command:" << this->_command << std::endl;
		for (std::vector<std::string>::iterator it = this->_parameter.begin(); it != this->_parameter.end(); it++)
			std::cout << "Param :" << *it<< std::endl;
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
// :KICK #Channel User :weil draum\r\n
void Parser::ParseMsg( void ) {
	if (this->_input[0] == ':')
		PrefixHandler(this->_input.substr(1, this->_input.find_first_of(' ')));
	CommandHandler(this->_input.substr(0, this->_input.find_first_of(' ')));
	if (this->_input[0] != ':')
		ParamHandler(this->_input.substr(0, this->_input.find_first_of("\r\n") )); // end limiter ist glaube falsch
	else
		TrailingHandler(this->_input.substr(1, this->_input.find_first_of("\r\n"))); // end limiter ist glaube falsch
	return ;
}

void Parser::PrefixHandler( std::string Prefix ) {
	// starts with ':'
	this->_prefix = Prefix;
	this->_input.erase(0, this->_input.find_first_of(' ') + 1); // vllt einfach returnen 
}

void Parser::CommandHandler( std::string Command ) {
	// after the prefix the first word after a space is the command
	this->_command = Command;
	this->_input.erase(0, this->_input.find_first_of(' ') + 1);
}

void Parser::ParamHandler( std::string Param ) {
	// everything after the command that doesnt start with a : ist a parameter
	size_t found = 0; 

	while (!Param.empty()) {
		if (Param == "\r\n")
			return ;
		found = Param.find_first_of(": ");
		if (found == ':') {
			TrailingHandler(Param.substr(1, Param.find("\r\n")));
			return ;
		}
		else if (found == std::string::npos) {
			this->_parameter.push_back(Param.substr(0, Param.find("\r\n")));
			return ;
		}
		else
			this->_parameter.push_back(Param.substr(0, found));
		Param.erase(0, found + 1);
		//nach params suchen und auf pushen
		//hier nochmal nach trailing checken
	}
}

void Parser::TrailingHandler( std::string Trailing ) {
	// everything after a command that starts with a : is a trailing
	this->_trailing = Trailing;
	return ;
}
