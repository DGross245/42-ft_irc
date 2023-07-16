#include "Parser.hpp"
#include "Constants.hpp"
#include "Client.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <sys/socket.h>

Parser::Parser( std::string &buffer, Client client ) {
	size_t pos = buffer.find("\r\n");
	if (pos != std::string::npos  || !buffer.empty()) {
		this->_input = buffer.substr(0, pos);
		std::string name = client.getNickname().empty() ? "*" : client.getNickname();
		std::cout << BLACK "[Server]: " BLUE BOLD " <== " RESET ORANGE << name << WHITE ": " << this->_input << RESET << std::endl;
		parseMsg( client );
		if (std::string::npos != buffer.find("\r\n"))
			buffer.erase(0, pos + 2);
		else
			buffer.erase(0, buffer.length());
	}
	else
		throw parserErrorException("ERROR: Invalid message format: missing CR LF");
	return ;
}

Parser::~Parser( void ) {
	return ;
}

// SETTER FUNCTIONS

void Parser::setPrefix( std::string prefix ) {
	this->_prefix = prefix;
	return ;
}

void Parser::setCMD( std::string command ) {
	this->_command = command;
	return ;
}

void Parser::setParameter( std::string parameter ) {
	this->_parameter.push_back(parameter);
	return ;
}

void Parser::setTrailing( std::string trailing ) {
	this->_trailing = trailing;
	return ;
}

// GETTER FUNCTIONS

std::string &Parser::getInput( void ) {
	return (this->_input);
}

std::string &Parser::getPrefix( void ) {
	return (this->_prefix);
}

std::string &Parser::getTrailing( void ) {
	return (this->_trailing);
}

std::string &Parser::getCMD( void ) {
	return (this->_command);
}

std::vector<std::string> &Parser::getParam( void ) {
	return (this->_parameter);
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
	else if (this->getCMD() == "CAP")
		checkCAP( client );
	return ;
}

void Parser::checkCAP( Client client) {
	if (this->getParam().size() < 1 || !this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::sendError( Client client ) {
	client.sendMsg(SERVER  " " ERR_NEEDMOREPARAMS " " + client.getNickname() + " :Not enough parameters\r\n");
	throw parserErrorException("Error: Invalid Command " + this->getCMD());
	return ;
}

void Parser::checkPASS( Client client ) {
	if (this->getParam().size() != 1 || !this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkNICK( Client client ) {
	if (this->getParam().size() != 1) {
		client.sendMsg(SERVER  " " ERR_NONICKNAMEGIVEN " " + client.getNickname() + " " + this->getCMD() + " :Nickname is missing\r\n");
		throw parserErrorException("Error: Invalid Command " + this->getCMD());
	}
	else if (!this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkUSER( Client client ) {
	if (this->getParam().size() != 3 || this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkQUIT( Client client ) {
	if (!this->getParam().empty())
		sendError( client );
	return ;
}

void Parser::checkJOIN( Client client ) {
	if (this->getParam().size() < 1 || this->getParam().size() > 2 || !this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkMODE( Client client ) {
	if (this->getParam().size() < 1 || this->getParam().size() > 4 || !this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkTOPIC( Client client ) {
	if (this->getParam().size() != 1)
		sendError( client );
	return ;
}

void Parser::checkINVITE( Client client ) {
	if (this->getParam().size() != 2 || !this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkKICK( Client client ) {
	if (this->getParam().size() != 2)
		sendError( client );
	return ;
}

void Parser::checkPRIVMSG( Client client ) {
	if (this->getTrailing().empty()) {
		client.sendMsg(SERVER  " " ERR_NOTEXTTOSEND " " + client.getNickname() + " :No text to send\r\n");
		throw parserErrorException("Error: Invalid Command " + this->getCMD());
	}
	if (this->getParam().size() != 1) {
		client.sendMsg(SERVER  " " ERR_NORECIPIENT " " + client.getNickname() + " :No recipient\r\n");
		throw parserErrorException("Error: Invalid Command " + this->getCMD());
	}
	return ;
}

void Parser::checkPING( Client client ) {
	if (this->getParam().size() != 1 || !this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkPART( Client client ) {
	if (this->getParam().size() != 1)
		sendError( client );
	return ;
}

void Parser::parseMsg( Client client ) {
	if (this->getInput()[0] == ':')
		prefixHandler(this->getInput().substr(1, this->getInput().find_first_of(' ')));
	commandHandler(this->getInput().substr(0, this->getInput().find_first_of(' ')));
	if (this->getInput()[0] != ':')
		paramHandler(this->getInput().substr(0, this->getInput().find_first_of("\r\n") ));
	else
		setTrailing(this->getInput().substr(1, this->getInput().find_first_of("\r\n")));
	isValidCommandLine( client );
	return ;
}

void Parser::prefixHandler( std::string prefix ) {
	this->setPrefix(prefix);
	this->getInput().erase(0, this->getInput().find_first_of(' ') + 1);
}

void Parser::commandHandler( std::string command ) {
	this->setCMD(command);
	this->getInput().erase(0, this->getInput().find_first_of(' ') + 1);
}

void Parser::paramHandler( std::string param ) {
	size_t found = 0;

	while (!param.empty()) {
		if (param == "\r\n")
			return ;
		found = param.find_first_of(": ");
		if (found != std::string::npos && param[found] == ':') {
			setTrailing(param.substr(1, param.find("\r\n")));
			return ;
		}
		else if (found == std::string::npos) {
			this->getParam().push_back(param.substr(0, param.find("\r\n") - 1));
			return ;
		}
		else
			this->getParam().push_back(param.substr(0, found));
		param.erase(0, found + 1);
	}
}

Parser::parserErrorException::~parserErrorException( void ) throw() { return ; }
Parser::parserErrorException::parserErrorException( std::string error ) : _error(error) { return ; }
const char *Parser::parserErrorException::what() const throw() { return (this->_error.c_str()); }
