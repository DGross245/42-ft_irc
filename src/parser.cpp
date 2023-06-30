#include "Parser.hpp"
#include <string>
#include <iostream>
#include "Constants.hpp"
#include "Client.hpp"
#include <vector>

std::string &Parser::getPrefix( void ) {
	return (this->_prefix);
}

std::string &Parser::getTrailing( void ) {
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

std::string &Parser::getCMD( void ) {
	return (this->_command);
}

std::vector<std::string> &Parser::getParam( void ) {
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
	if (this->getParam().size() != 1 || this->getTrailing().empty()) {
		std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
		throw parserErrorException("Invalid Command");
	}
	return ;
}

void Parser::checkNICK( Client client ) {
	if (this->getParam().size() != 1 || this->getTrailing().empty()) {
		std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
		throw parserErrorException("Invalid Command");
	}
	return ;
}

void Parser::checkUSER( Client client ) {
	if (this->getParam().size() != 3 || this->getTrailing().empty()) {
		std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
		throw parserErrorException("Invalid Command");
	}
	return ;
}

void Parser::checkQUIT( Client client ) {
	if (!this->getParam().empty()) {
		std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
		throw parserErrorException("Invalid Command");
	}
	return ;
}

void Parser::checkJOIN( Client client ) {
	(void)client;
	// in progress
	return ;
}

void Parser::checkMODE( Client client ) {
	(void)client;
	// nochmal gucken ob wir multiple parameters erlauben wie +i +o -k etc.
	return ;
}

void Parser::checkTOPIC( Client client ) {
	if (this->getParam().size() != 1) {
		std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
		throw parserErrorException("Invalid Command");
	}
	return ;
}

void Parser::checkINVITE( Client client ) {
	if (this->getParam().size() != 2 || !this->getTrailing().empty()) {
		std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
		throw parserErrorException("Invalid Command");
	}
	return ;
}

void Parser::checkKICK( Client client ) {
	if (this->getParam().size() != 2) {
		std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
		throw parserErrorException("Invalid Command");
	}
	return ;
}

void Parser::checkPRIVMSG( Client client ) {
	if (this->getParam().size() != 1 || this->getTrailing().empty()) {
		std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
		throw parserErrorException("Invalid Command");
	}
	// split parameter if there is mulitple
	return ;
}

void Parser::checkPING( Client client ) {
	if (this->getParam().size() != 1 || !this->getTrailing().empty()) {
		std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
		throw parserErrorException("Invalid Command");
	}
	return ;
}

void Parser::checkPART( Client client ) {
	if (this->getParam().size() != 1 || !this->getTrailing().empty()) {
		std::string message = SERVER  "" ERR_NEEDMOREPARAMS + client.getNickname() + this->getCMD() + " :Not enough parameters";
		throw parserErrorException("Invalid Command");
	}
	// split parameter if there is mulitple
	return ;
}

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