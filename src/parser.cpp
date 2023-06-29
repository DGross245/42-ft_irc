#include "Parser.hpp"
#include <string>
#include <iostream>
#include "Constants.hpp"
#include "Client.hpp"
#include <vector>

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

std::string Parser::getPrefix( void ) {
	return (this->_prefix);
}

std::string Parser::getTrailing( void ) {
	return (this->_trailing);
}

Parser::Parser( std::string buffer, Client client ) {
	size_t pos = buffer.find("\r\n");
	if (pos != std::string::npos ) {
		this->_input = buffer.substr(0, pos);
		parseMsg( client );
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

void Parser::isValidCommandLine( Client client ) {
	if (this->getCMD() == "PASS")
		checkPASS( client );
	else if (this->getCMD() == "NICK")
		checkNICK( client );
	else if (this->getCMD() == "USER")
		checkUSER( client );
	else if (this->getCMD() == "QUIT")
		checkQUIT( client );
	else if (this->getCMD() == "JOIN")
		checkJOIN( client );
	else if (this->getCMD() == "MODE")
		checkMODE( client );
	else if (this->getCMD() == "TOPIC")
		checkTOPIC( client );
	else if (this->getCMD() == "INVITE")
		checkINVITE( client );
	else if (this->getCMD() == "KICK")
		checkKICK( client );
	else if (this->getCMD() == "PRIVMSG")
		checkPRIVMSG( client );
	else if (this->getCMD() == "PING")
		checkPING( client );
	else if (this->getCMD() == "PART")
		checkPART( client );
	return ;
}

void Parser::checkPASS( Client client ) {
	// prefix check ?
	std::vector<std::string>::iterator it = this->getParam().begin();
	if (this->getParam().size() != 1) {
		std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
		throw parserErrorException("Invalid Command");
	}
	return ;
}

void Parser::checkNICK( Client client ) {
	// prefix check ?
	std::vector<std::string>::iterator it = this->getParam().begin();
	if (this->getParam().size() != 1) {
		std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
		throw parserErrorException("Invalid Command");
	}
	if ( it[0] != ":" )
		return ;
	std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
	throw parserErrorException("Invalid Command");
	return ;
}

void Parser::checkUSER( Client client ) {
	// prefix check ?
	std::vector<std::string> checker = this->getParam();
	for (std::vector<std::string>::iterator it = checker.begin(); it != checker.end(); it++) {
		if (*it == "kek") {
			;
		}
	}
	std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
	throw parserErrorException("Invalid Command");
	return ;
}

void Parser::checkQUIT( Client client ) {
	// prefix check ?
	std::vector<std::string> checker = this->getParam();
	for (std::vector<std::string>::iterator it = checker.begin(); it != checker.end(); it++) {
		if (*it == "kek") {

		}
	}
	std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
	throw parserErrorException("Invalid Command");
	return ;
}

void Parser::checkJOIN( Client client ) {
	// prefix check ?
	std::vector<std::string> checker = this->getParam();
	for (std::vector<std::string>::iterator it = checker.begin(); it != checker.end(); it++) {
		if (*it == "kek") {

		}
	}
	std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
	throw parserErrorException("Invalid Command");
	return ;
}

void Parser::checkMODE( Client client ) {
	// prefix check ?
	std::vector<std::string> checker = this->getParam();
	for (std::vector<std::string>::iterator it = checker.begin(); it != checker.end(); it++) {
		if (*it == "kek") {

		}
	}
	std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
	throw parserErrorException("Invalid Command");
	return ;
}

void Parser::checkTOPIC( Client client ) {
	// prefix check ?
	std::vector<std::string> checker = this->getParam();
	for (std::vector<std::string>::iterator it = checker.begin(); it != checker.end(); it++) {
		if (*it == "kek") {

		}
	}
	std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
	throw parserErrorException("Invalid Command");
	return ;
}

void Parser::checkINVITE( Client client ) {
	// prefix check ?
	std::vector<std::string> checker = this->getParam();
	for (std::vector<std::string>::iterator it = checker.begin(); it != checker.end(); it++) {
		if (*it == "kek") {

		}
	}
	std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
	throw parserErrorException("Invalid Command");
	return ;
}

void Parser::checkKICK( Client client ) {
	// prefix check ?
	std::vector<std::string> checker = this->getParam();
	for (std::vector<std::string>::iterator it = checker.begin(); it != checker.end(); it++) {
		if (*it == "kek") {

		}
	}
	std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
	throw parserErrorException("Invalid Command");
	return ;
}

void Parser::checkPRIVMSG( Client client ) {
	// prefix check ?
	std::vector<std::string> checker = this->getParam();
	for (std::vector<std::string>::iterator it = checker.begin(); it != checker.end(); it++) {
		if (*it == "kek") {

		}
	}
	std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
	throw parserErrorException("Invalid Command");
	return ;
}

void Parser::checkPING( Client client ) {
	// prefix check ?
	std::vector<std::string> checker = this->getParam();
	for (std::vector<std::string>::iterator it = checker.begin(); it != checker.end(); it++) {
		if (*it == "kek") {

		}
	}
	std::string message = SERVER "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
	throw parserErrorException("Invalid Command");
	return ;
}

void Parser::checkPART( Client client ) {
	// prefix check ?
	std::vector<std::string> checker = this->getParam();
	for (std::vector<std::string>::iterator it = checker.begin(); it != checker.end(); it++) {
		if (*it == "kek") {

		}
	}
	std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
	throw parserErrorException("Invalid Command");
	return ;
}

// :<server_name> 461 <nick> <command> :Not enough parameters - ERR_NOSUCHPARAM error code
void Parser::parseMsg( Client client ) {
	if (this->_input[0] == ':')
		prefixHandler(this->_input.substr(1, this->_input.find_first_of(' ')));
	commandHandler(this->_input.substr(0, this->_input.find_first_of(' ')));
	if (this->_input[0] != ':')
		paramHandler(this->_input.substr(0, this->_input.find_first_of("\r\n") ));
	else
		trailingHandler(this->_input.substr(1, this->_input.find_first_of("\r\n")));
	isValidCommandLine( client );
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

Parser::parserErrorException::~parserErrorException( void ) throw() { return ; }
Parser::parserErrorException::parserErrorException( std::string error ) : _error(error) { return ; }
const char *Parser::parserErrorException::what() const throw() { return (this->_error.c_str()); }